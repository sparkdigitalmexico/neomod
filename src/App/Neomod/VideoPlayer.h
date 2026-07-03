// Copyright (c) 2026, neomod video-background support.
//
// Streams a beatmap's background video (mp4/avi/flv/...) via the engine's
// dynamically-loaded FFmpeg, exposing the current frame as an Image that is
// kept in sync with an externally-provided song position.
//
// All decoding + GPU upload happen on the calling (render) thread inside
// update(), so this must only be driven from the main thread.
#pragma once
#ifndef NEOMOD_VIDEOPLAYER_H
#define NEOMOD_VIDEOPLAYER_H

#include "config.h"
#include "types.h"

#include <memory>
#include <string>

class Image;
struct VideoPlayerImpl;  // hides all FFmpeg types from this header

class VideoPlayer final {
   public:
    VideoPlayer();
    ~VideoPlayer();

    VideoPlayer(const VideoPlayer &) = delete;
    VideoPlayer &operator=(const VideoPlayer &) = delete;
    VideoPlayer(VideoPlayer &&) = delete;
    VideoPlayer &operator=(VideoPlayer &&) = delete;

    // Open a video file (absolute path). startOffsetMS = song time at which the
    // video's first frame should appear (from the .osu "Video,<offset>,..." event).
    // Returns true if the stream opened and a first frame decoded.
    bool load(const std::string &absPath, i32 startOffsetMS);

    // Advance playback to the given song position (ms). Decodes forward or seeks
    // as needed and uploads the current frame. Cheap no-op if nothing changed.
    void update(i32 songPositionMS);

    [[nodiscard]] bool isReady() const;
    [[nodiscard]] bool hasFailed() const;
    [[nodiscard]] Image *getImage() const;

    [[nodiscard]] const std::string &getPath() const { return this->sPath; }
    [[nodiscard]] i32 getStartOffsetMS() const { return this->iStartOffsetMS; }
    [[nodiscard]] i32 getWidth() const { return this->iWidth; }
    [[nodiscard]] i32 getHeight() const { return this->iHeight; }

   private:
    std::unique_ptr<VideoPlayerImpl> impl;

    std::string sPath;
    i32 iStartOffsetMS{0};
    i32 iWidth{0};
    i32 iHeight{0};
};

#endif  // NEOMOD_VIDEOPLAYER_H
