// Copyright (c) 2026, WH, All rights reserved.
#include "config.h"

#if defined(MCENGINE_FEATURE_FFMPEG)
#include "Image.h"

#include "FFmpegLoader.h"
#include "ConVar.h"

#include "Image.h"
#include "SyncOnce.h"
#include "Logging.h"

namespace Mc::FFmpeg {
using namespace funcs;

namespace {
bool loadffmpeg() {
    static Sync::once_flag ffmpeg_init_once;
    static bool ffmpeg_available{false};
    Sync::call_once(ffmpeg_init_once, []() {
        ffmpeg_available = Mc::FFmpeg::init();
        if(!ffmpeg_available) {
            logRaw("Failed to load FFmpeg: {}", Mc::FFmpeg::getInitError());
        }
    });
    return ffmpeg_available;
}

// auto ffmpeg state cleanup
struct FFState {  // NOLINT
    AVFormatContext *fmtCtx{};
    AVIOContext *avioCtx{};
    AVCodecContext *codecCtx{};
    AVPacket *pkt{};
    AVFrame *frame{};
    SwsContext *swsCtx{};
    ~FFState() {
        if(swsCtx) sws_freeContext(swsCtx);
        if(pkt) av_packet_free(&pkt);
        if(frame) av_frame_free(&frame);
        if(codecCtx) avcodec_free_context(&codecCtx);
        if(fmtCtx) avformat_close_input(&fmtCtx);
        if(avioCtx) {
            // explicitly free this buffer, ffmpeg may have reallocated it internally
            // (just annoying ffmpeg things)
            av_free(avioCtx->buffer);
            avioCtx->buffer = nullptr;
            avio_context_free(&avioCtx);
        }
    }
};

// memory reader for custom AVIO
struct MemReader {
    const u8 *data;
    u64 size;
    u64 pos;
};

}  // namespace

ImageDecodeResult decodeFFmpegFromMemory(Image *this_, const u8 *inData, u64 size) {
    if(!loadffmpeg()) return ImageDecodeResult::UNAVAILABLE;
    using enum ImageDecodeResult;

    MemReader mem{inData, size, 0};

    constexpr int avioBufSize = 32768;
    auto *avioBuf = static_cast<u8 *>(av_malloc(avioBufSize));
    if(!avioBuf) {
        logIfCV(debug_ffmpeg, "av_malloc failed");
        return FAIL;
    }

    AVIOContext *avioCtx = avio_alloc_context(
        avioBuf, avioBufSize, 0, &mem,
        [](void *opaque, u8 *buf, int buf_size) -> int {
            auto *r = static_cast<MemReader *>(opaque);
            i64 remaining = static_cast<i64>(r->size - r->pos);
            if(remaining <= 0) return AVERROR_EOF;
            int n = (buf_size < remaining) ? buf_size : static_cast<int>(remaining);
            memcpy(buf, r->data + r->pos, n);
            r->pos += n;
            return n;
        },
        nullptr,
        [](void *opaque, i64 offset, int whence) -> i64 {
            auto *r = static_cast<MemReader *>(opaque);
            if(whence == AVSEEK_SIZE) return static_cast<i64>(r->size);
            i64 newPos;  // NOLINT
            switch(whence & ~AVSEEK_FORCE) {
                case SEEK_SET:
                    newPos = offset;
                    break;
                case SEEK_CUR:
                    newPos = static_cast<i64>(r->pos) + offset;
                    break;
                case SEEK_END:
                    newPos = static_cast<i64>(r->size) + offset;
                    break;
                default:
                    return -1;
            }
            if(newPos < 0 || static_cast<u64>(newPos) > r->size) return -1;
            r->pos = static_cast<u64>(newPos);
            return newPos;
        });
    if(!avioCtx) {
        av_free(avioBuf);
        logIfCV(debug_ffmpeg, "avio_alloc_context failed");
        return FAIL;
    }

    FFState ff{.avioCtx = avioCtx};

    ff.fmtCtx = avformat_alloc_context();
    if(!ff.fmtCtx) {
        logIfCV(debug_ffmpeg, "avformat_alloc_context failed");
        return FAIL;
    }
    ff.fmtCtx->pb = avioCtx;

    // filepath is just passed as a hint for extension-based format probing (image2 demuxer); actual (memory) I/O still goes through the avioCtx
    if(int averr = avformat_open_input(&ff.fmtCtx, this_->sFilePath.c_str(), nullptr, nullptr); averr != 0) {
        logIfCV(debug_ffmpeg, "avformat_open_input failed: {}", av_err2str(averr));
        return FAIL;  // avformat_open_input nulls fmtCtx on failure
    }
    if(int averr = avformat_find_stream_info(ff.fmtCtx, nullptr); averr != 0) {
        logIfCV(debug_ffmpeg, "avformat_find_stream_info failed: {:s}", av_err2str(averr));
        return FAIL;
    }

    if(this_->isInterrupted()) return INTERRUPTED;

    // find video stream
    int videoIdx = -1;
    for(unsigned i = 0; i < ff.fmtCtx->nb_streams; i++) {
        if(ff.fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIdx = static_cast<int>(i);
            break;
        }
    }
    if(videoIdx < 0) {
        logIfCV(debug_ffmpeg, "video < 0 (not found)");
        return FAIL;
    }

    AVCodecParameters *codecPar = ff.fmtCtx->streams[videoIdx]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecPar->codec_id);
    if(!codec) {
        logIfCV(debug_ffmpeg, "avcodec_find_decoder({}) failed", (u32)codecPar->codec_id);
        return FAIL;
    }

    ff.codecCtx = avcodec_alloc_context3(codec);
    if(!ff.codecCtx) {
        logIfCV(debug_ffmpeg, "avcodec_alloc_context3 failed");
        return FAIL;
    }

    if(int averr = avcodec_parameters_to_context(ff.codecCtx, codecPar); averr != 0) {
        logIfCV(debug_ffmpeg, "avcodec_parameters_to_context failed: {:s}", av_err2str(averr));
        return FAIL;
    }

    if(int averr = avcodec_open2(ff.codecCtx, codec, nullptr); averr != 0) {
        logIfCV(debug_ffmpeg, "avcodec_open2 failed: {:s}", av_err2str(averr));
        return FAIL;
    }

    ff.pkt = av_packet_alloc();
    ff.frame = av_frame_alloc();
    if(!ff.pkt || !ff.frame) {
        logIfCV(debug_ffmpeg, "{:s} failed", !ff.pkt ? "av_packet_alloc" : "av_frame_alloc");
        return FAIL;
    }

    // decode one frame
    bool decoded = false;
    while(av_read_frame(ff.fmtCtx, ff.pkt) >= 0) {
        if(ff.pkt->stream_index == videoIdx) {
            if(avcodec_send_packet(ff.codecCtx, ff.pkt) >= 0 && avcodec_receive_frame(ff.codecCtx, ff.frame) >= 0) {
                decoded = true;
                av_packet_unref(ff.pkt);
                break;
            }
        }
        av_packet_unref(ff.pkt);
    }

    if(!decoded) {
        // flush decoder
        avcodec_send_packet(ff.codecCtx, nullptr);
        decoded = (avcodec_receive_frame(ff.codecCtx, ff.frame) >= 0);
    }

    if(!decoded) {
        logIfCV(debug_ffmpeg, "!decoded");
        return FAIL;
    }

    i32 outWidth = ff.frame->width;
    i32 outHeight = ff.frame->height;
    if(outWidth <= 0 || outHeight <= 0 || outWidth > 8192 || outHeight > 8192) {
        debugLog("Image Error: FFmpeg decoded image size is invalid or too big ({} x {})", outWidth, outHeight);
        return FAIL;
    }

    if(this_->isInterrupted()) return INTERRUPTED;

    // convert to RGBA (frame-based API handles color range/space from frame metadata)
    ff.swsCtx = sws_alloc_context();
    if(!ff.swsCtx) {
        debugLog("sws_alloc_context failed");
        return FAIL;
    }

    AVFrame *rgbaFrame = av_frame_alloc();
    if(!rgbaFrame) {
        debugLog("av_frame_alloc failed");
        return FAIL;
    }
    rgbaFrame->format = AV_PIX_FMT_RGBA;
    rgbaFrame->width = outWidth;
    rgbaFrame->height = outHeight;

    if(int averr = sws_scale_frame(ff.swsCtx, rgbaFrame, ff.frame); averr < 0) {
        av_frame_free(&rgbaFrame);
        logIfCV(debug_ffmpeg, "sws_scale_frame failed: {:s}", av_err2str(averr));
        return FAIL;
    }

    this_->rawImage = Image::SizedRGBABytes{outWidth, outHeight};
    if(!this_->rawImage.get()) {
        av_frame_free(&rgbaFrame);
        return FAIL;
    }

    // copy RGBA data (accounting for potential stride padding from swscale)
    const int srcStride = rgbaFrame->linesize[0];
    const int dstStride = outWidth * Image::NUM_CHANNELS;
    if(srcStride == dstStride) {
        memcpy(this_->rawImage.get(), rgbaFrame->data[0], static_cast<u64>(outHeight) * dstStride);
    } else {
        for(i32 y = 0; y < outHeight; y++) {
            memcpy(this_->rawImage.get() + static_cast<sSz>(y) * dstStride,
                   rgbaFrame->data[0] + static_cast<sSz>(y) * srcStride, dstStride);
        }
    }

    av_frame_free(&rgbaFrame);
    return SUCCESS;
}
}  // namespace Mc::FFmpeg

#endif
