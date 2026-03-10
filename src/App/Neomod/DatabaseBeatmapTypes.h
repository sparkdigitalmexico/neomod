// Copyright (c) 2020, PG & 2026, WH, All rights reserved.
#pragma once
// DatabaseBeatmap data types

#if __has_include("config.h")
#include "config.h"
#endif

#include "types.h"
#include "Vectors_fwd.h"

#include <vector>

// TODO: messy
struct SLIDER_SCORING_TIME;  // DifficultyCalculator.h

namespace DatabaseBeatmapTypes {

struct HITSAMPLE_BITS {
    u8 hitSounds{0};    // bitfield of HitSoundTypes to play
    u8 normalSet{0};    // SampleSetType of the normal sound
    u8 additionSet{0};  // SampleSetType of the whistle, finish and clap sounds
    u8 volume{0};       // volume of the sample, 1-100. if 0, use timing point volume instead
};

// raw structs (not editable, we're following db format directly)
struct TIMINGPOINT final {
    f64 offset;
    f64 msPerBeat;

    i32 sampleSet;
    i32 sampleIndex;
    i32 volume;

    bool uninherited;  // <=> timingChange
    bool kiai;

    bool operator==(const TIMINGPOINT &) const = default;
};

struct BREAK final {
    i64 startTime;
    i64 endTime;
};

struct TIMING_INFO {
    i32 offset{0};

    f32 beatLengthBase{0.f};
    f32 beatLength{0.f};

    i32 sampleSet{0};
    i32 sampleIndex{0};
    i32 volume{0};

    bool isNaN{false};

    bool operator==(const TIMING_INFO &) const = default;
};

// primitive objects

struct HITCIRCLE final {
    int x, y;
    i32 time;
    int number;
    int colorCounter;
    int colorOffset;
    bool clicked;
    HITSAMPLE_BITS samples;
};

struct SLIDER final {
    // needs extern ctors/dtors due to SLIDER_SCORING_TIME being externally defined
    SLIDER() noexcept;
    ~SLIDER() noexcept;

    SLIDER(const SLIDER &) noexcept;
    SLIDER &operator=(const SLIDER &) noexcept;
    SLIDER(SLIDER &&) noexcept;
    SLIDER &operator=(SLIDER &&) noexcept;

    int x, y;
    char type;
    int repeat;
    float pixelLength;
    i32 time;
    int number;
    int colorCounter;
    int colorOffset;
    std::vector<vec2> points;
    HITSAMPLE_BITS hoverSamples;
    std::vector<HITSAMPLE_BITS> edgeSamples;

    float sliderTime;
    float sliderTimeWithoutRepeats;
    std::vector<float> ticks;

    std::vector<SLIDER_SCORING_TIME> scoringTimesForStarCalc;
};

struct SPINNER final {
    int x, y;
    i32 time;
    i32 endTime;
    HITSAMPLE_BITS samples;
};
}  // namespace DatabaseBeatmapTypes
