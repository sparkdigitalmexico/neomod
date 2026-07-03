// Copyright (c) 2026, neomod video-background support.
#include "VideoPlayer.h"

#include "Image.h"
#include "Engine.h"
#include "ResourceManager.h"
#include "Logging.h"

#include <cstdio>   // SEEK_SET/CUR/END
#include <cstring>  // memcpy
#include <fstream>
#include <vector>

#if defined(MCENGINE_FEATURE_FFMPEG)
#include "FFmpegLoader.h"
using namespace Mc::FFmpeg::funcs;
#endif

namespace {
// bound per-frame cost: at most this many video frames decoded within one update()
constexpr int kDecodeBudgetPerUpdate = 8;
// if the wanted time is more than this far ahead of the shown frame, seek instead of linear-decoding
constexpr i64 kSeekAheadThresholdMS = 1500;

#if defined(MCENGINE_FEATURE_FFMPEG)
struct MemReader {
    const u8 *data;
    u64 size;
    u64 pos;
};
#endif
}  // namespace

// all decoder state lives here, so nothing FFmpeg leaks into the header
struct VideoPlayerImpl {
    Image *image{nullptr};
    bool ready{false};
    bool failed{false};

#if defined(MCENGINE_FEATURE_FFMPEG)
    std::vector<u8> fileData;  // whole file kept in memory, read through a custom AVIO
    MemReader mem{nullptr, 0, 0};

    AVIOContext *avioCtx{nullptr};
    AVFormatContext *fmtCtx{nullptr};
    AVCodecContext *codecCtx{nullptr};
    SwsContext *swsCtx{nullptr};
    AVFrame *frame{nullptr};
    AVFrame *rgbaFrame{nullptr};
    AVPacket *pkt{nullptr};

    int videoIdx{-1};
    i32 tbNum{0};              // stream time_base numerator
    i32 tbDen{1};              // stream time_base denominator
    i64 frameDurationMS{40};   // fallback (25fps) when a frame carries no pts
    i64 lastDisplayedMS{INT64_MIN};
    bool eof{false};

    std::vector<u8> packed;    // tightly-packed RGBA scratch for upload

    ~VideoPlayerImpl() {
        if(rgbaFrame) av_frame_free(&rgbaFrame);
        if(swsCtx) sws_freeContext(swsCtx);
        if(pkt) av_packet_free(&pkt);
        if(frame) av_frame_free(&frame);
        if(codecCtx) avcodec_free_context(&codecCtx);
        if(fmtCtx) avformat_close_input(&fmtCtx);
        if(avioCtx) {
            // ffmpeg may have reallocated the buffer internally, free the current one
            av_free(avioCtx->buffer);
            avioCtx->buffer = nullptr;
            avio_context_free(&avioCtx);
        }
    }
#endif
};

#if defined(MCENGINE_FEATURE_FFMPEG)
namespace {
i64 ff_pts_ms(VideoPlayerImpl *d, AVFrame *fr) {
    i64 ts = fr->best_effort_timestamp;
    if(ts == AV_NOPTS_VALUE) ts = fr->pts;
    if(ts == AV_NOPTS_VALUE)
        return (d->lastDisplayedMS == INT64_MIN) ? 0 : (d->lastDisplayedMS + d->frameDurationMS);
    // ms = ts * 1000 * tbNum / tbDen (num is ~1 and ts moderate, fits in i64 for real videos)
    return (ts * 1000 * static_cast<i64>(d->tbNum)) / static_cast<i64>(d->tbDen);
}

// decode the next video frame into d->frame; returns false at end of stream
bool ff_decode_next(VideoPlayerImpl *d, i64 &outMS) {
    for(;;) {
        int ret = av_read_frame(d->fmtCtx, d->pkt);
        if(ret < 0) {
            // flush: drain any remaining buffered frames
            avcodec_send_packet(d->codecCtx, nullptr);
            if(avcodec_receive_frame(d->codecCtx, d->frame) >= 0) {
                outMS = ff_pts_ms(d, d->frame);
                return true;
            }
            d->eof = true;
            return false;
        }
        if(d->pkt->stream_index != d->videoIdx) {
            av_packet_unref(d->pkt);
            continue;
        }
        int sret = avcodec_send_packet(d->codecCtx, d->pkt);
        av_packet_unref(d->pkt);
        if(sret < 0) continue;
        if(avcodec_receive_frame(d->codecCtx, d->frame) >= 0) {
            outMS = ff_pts_ms(d, d->frame);
            return true;
        }
        // decoder needs more packets, keep reading
    }
}

// convert d->frame -> RGBA and upload it into d->image (synchronous reload)
void ff_upload(VideoPlayerImpl *d, i32 w, i32 h) {
    AVFrame *src = d->frame;

    // normalize deprecated YUVJ pixel formats (some decoders still emit these)
    switch(src->format) {
        case AV_PIX_FMT_YUVJ420P: src->format = AV_PIX_FMT_YUV420P; src->color_range = AVCOL_RANGE_JPEG; break;
        case AV_PIX_FMT_YUVJ422P: src->format = AV_PIX_FMT_YUV422P; src->color_range = AVCOL_RANGE_JPEG; break;
        case AV_PIX_FMT_YUVJ444P: src->format = AV_PIX_FMT_YUV444P; src->color_range = AVCOL_RANGE_JPEG; break;
        case AV_PIX_FMT_YUVJ440P: src->format = AV_PIX_FMT_YUV440P; src->color_range = AVCOL_RANGE_JPEG; break;
        default: break;
    }

    if(!d->swsCtx) d->swsCtx = sws_alloc_context();
    if(!d->swsCtx || !d->image) return;

    av_frame_unref(d->rgbaFrame);  // free previous conversion buffer
    d->rgbaFrame->format = AV_PIX_FMT_RGBA;
    d->rgbaFrame->width = w;
    d->rgbaFrame->height = h;
    if(sws_scale_frame(d->swsCtx, d->rgbaFrame, src) < 0) return;

    const int dstStride = w * Image::NUM_CHANNELS;
    const int srcStride = d->rgbaFrame->linesize[0];
    const u8 *srcData = d->rgbaFrame->data[0];

    if(srcStride == dstStride) {
        d->image->setImageData(w, h, srcData);
    } else {
        d->packed.resize(static_cast<u64>(dstStride) * h);
        for(i32 y = 0; y < h; y++)
            memcpy(d->packed.data() + static_cast<u64>(y) * dstStride,
                   srcData + static_cast<u64>(y) * srcStride, dstStride);
        d->image->setImageData(w, h, d->packed.data());
    }

    // upload synchronously on this (render) thread, same as TextureAtlas
    d->image->reload();
}

void ff_seek(VideoPlayerImpl *d, i64 targetMS) {
    if(targetMS < 0) targetMS = 0;
    const i64 ts = (d->tbNum > 0) ? (targetMS * static_cast<i64>(d->tbDen)) / (1000 * static_cast<i64>(d->tbNum)) : 0;
    if(av_seek_frame(d->fmtCtx, d->videoIdx, ts, AVSEEK_FLAG_BACKWARD) >= 0) {
        avcodec_flush_buffers(d->codecCtx);
        d->eof = false;
        d->lastDisplayedMS = INT64_MIN;  // force the next decoded frame to be shown
    }
}
}  // namespace
#endif  // MCENGINE_FEATURE_FFMPEG

VideoPlayer::VideoPlayer() : impl(std::make_unique<VideoPlayerImpl>()) {}

VideoPlayer::~VideoPlayer() {
    if(resourceManager && this->impl && this->impl->image) {
        resourceManager->destroyResource(this->impl->image);
        this->impl->image = nullptr;
    }
}

bool VideoPlayer::isReady() const { return this->impl && this->impl->ready && !this->impl->failed; }
bool VideoPlayer::hasFailed() const { return !this->impl || this->impl->failed; }
Image *VideoPlayer::getImage() const { return this->impl ? this->impl->image : nullptr; }

bool VideoPlayer::load(const std::string &absPath, i32 startOffsetMS) {
    this->sPath = absPath;
    this->iStartOffsetMS = startOffsetMS;

#if !defined(MCENGINE_FEATURE_FFMPEG)
    this->impl->failed = true;
    return false;
#else
    VideoPlayerImpl *d = this->impl.get();
    auto fail = [d]() -> bool { d->failed = true; return false; };

    if(!Mc::FFmpeg::init()) {
        debugLog("VideoPlayer: FFmpeg unavailable: {}", Mc::FFmpeg::getInitError());
        return fail();
    }

    // read the whole file into memory
    {
        std::ifstream f(absPath, std::ios::binary | std::ios::ate);
        if(!f) return fail();
        const std::streamsize sz = f.tellg();
        if(sz <= 0) return fail();
        f.seekg(0, std::ios::beg);
        d->fileData.resize(static_cast<size_t>(sz));
        if(!f.read(reinterpret_cast<char *>(d->fileData.data()), sz)) return fail();
        d->mem = MemReader{d->fileData.data(), static_cast<u64>(sz), 0};
    }

    constexpr int avioBufSize = 32768;
    auto *avioBuf = static_cast<u8 *>(av_malloc(avioBufSize));
    if(!avioBuf) return fail();

    d->avioCtx = avio_alloc_context(
        avioBuf, avioBufSize, 0, &d->mem,
        [](void *opaque, u8 *buf, int buf_size) -> int {
            auto *r = static_cast<MemReader *>(opaque);
            const i64 remaining = static_cast<i64>(r->size - r->pos);
            if(remaining <= 0) return AVERROR_EOF;
            const int n = (buf_size < remaining) ? buf_size : static_cast<int>(remaining);
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
                case SEEK_SET: newPos = offset; break;
                case SEEK_CUR: newPos = static_cast<i64>(r->pos) + offset; break;
                case SEEK_END: newPos = static_cast<i64>(r->size) + offset; break;
                default: return -1;
            }
            if(newPos < 0 || static_cast<u64>(newPos) > r->size) return -1;
            r->pos = static_cast<u64>(newPos);
            return newPos;
        });
    if(!d->avioCtx) {
        av_free(avioBuf);
        return fail();
    }

    d->fmtCtx = avformat_alloc_context();
    if(!d->fmtCtx) return fail();
    d->fmtCtx->pb = d->avioCtx;

    if(avformat_open_input(&d->fmtCtx, absPath.c_str(), nullptr, nullptr) != 0) return fail();
    if(avformat_find_stream_info(d->fmtCtx, nullptr) != 0) return fail();

    for(unsigned i = 0; i < d->fmtCtx->nb_streams; i++) {
        if(d->fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            d->videoIdx = static_cast<int>(i);
            break;
        }
    }
    if(d->videoIdx < 0) return fail();

    AVStream *st = d->fmtCtx->streams[d->videoIdx];
    AVCodecParameters *cp = st->codecpar;
    const AVCodec *codec = avcodec_find_decoder(cp->codec_id);
    if(!codec) return fail();

    d->codecCtx = avcodec_alloc_context3(codec);
    if(!d->codecCtx) return fail();
    if(avcodec_parameters_to_context(d->codecCtx, cp) != 0) return fail();
    if(avcodec_open2(d->codecCtx, codec, nullptr) != 0) return fail();

    d->tbNum = st->time_base.num;
    d->tbDen = st->time_base.den > 0 ? st->time_base.den : 1;
    if(st->avg_frame_rate.num > 0 && st->avg_frame_rate.den > 0)
        d->frameDurationMS = static_cast<i64>(1000.0 * st->avg_frame_rate.den / st->avg_frame_rate.num);

    d->frame = av_frame_alloc();
    d->rgbaFrame = av_frame_alloc();
    d->pkt = av_packet_alloc();
    if(!d->frame || !d->rgbaFrame || !d->pkt) return fail();

    // decode first frame to establish dimensions and create the target image
    i64 firstMS = 0;
    if(!ff_decode_next(d, firstMS)) return fail();

    this->iWidth = d->frame->width;
    this->iHeight = d->frame->height;
    if(this->iWidth <= 0 || this->iHeight <= 0 || this->iWidth > 8192 || this->iHeight > 8192) return fail();

    d->image = resourceManager->createImage(this->iWidth, this->iHeight, false, /*keepInSystemMemory=*/true);
    if(!d->image) return fail();

    ff_upload(d, this->iWidth, this->iHeight);
    d->lastDisplayedMS = firstMS;
    d->ready = true;
    return true;
#endif
}

void VideoPlayer::update(i32 songPositionMS) {
#if defined(MCENGINE_FEATURE_FFMPEG)
    VideoPlayerImpl *d = this->impl.get();
    if(!d->ready || d->failed || !d->image) return;

    const i64 target = static_cast<i64>(songPositionMS) - static_cast<i64>(this->iStartOffsetMS);
    if(target < 0) return;  // video hasn't started yet; keep the first frame parked

    // seek for backward jumps (restart/scrub) or large forward jumps; small forward gaps decode linearly
    if(d->lastDisplayedMS != INT64_MIN &&
       (target < d->lastDisplayedMS || target > d->lastDisplayedMS + kSeekAheadThresholdMS)) {
        ff_seek(d, target);
    }

    // decode forward until we reach the wanted time; upload only the final frame to avoid flicker
    int budget = kDecodeBudgetPerUpdate;
    bool decodedAny = false;
    while(budget-- > 0) {
        if(d->lastDisplayedMS != INT64_MIN && d->lastDisplayedMS >= target) break;
        i64 got = 0;
        if(!ff_decode_next(d, got)) break;  // end of stream
        d->lastDisplayedMS = got;
        decodedAny = true;
        if(got >= target) break;
    }
    if(decodedAny) ff_upload(d, this->iWidth, this->iHeight);
#else
    (void)songPositionMS;
#endif
}
