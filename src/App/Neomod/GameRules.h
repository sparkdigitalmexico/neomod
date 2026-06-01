#pragma once
// Copyright (c) 2016, PG, All rights reserved.

#include "noinclude.h"
#include "types.h"
#include "Vectors.h"

class AbstractBeatmapInterface;

namespace GameRules {

inline constexpr const int OSU_COORD_WIDTH{512};
inline constexpr const int OSU_COORD_HEIGHT{384};

//********************//
//  Positional Audio  //
//********************//

forceinline float osuCoords2Pan(float x) { return (x / (float)OSU_COORD_WIDTH - 0.5f) * 0.8f; }

//************************//
//	Hitobject Animations  //
//************************//

// this scales the fadeout duration with the current speed multiplier
float getFadeOutTime(float animationSpeedMultiplier = 1.f);
i32 getFadeInTime();

//********************//
//	Hitobject Timing  //
//********************//

constexpr forceinline float getMinHitWindow300() { return 80.f; }
constexpr forceinline float getMidHitWindow300() { return 50.f; }
constexpr forceinline float getMaxHitWindow300() { return 20.f; }

constexpr forceinline float getMinHitWindow100() { return 140.f; }
constexpr forceinline float getMidHitWindow100() { return 100.f; }
constexpr forceinline float getMaxHitWindow100() { return 60.f; }

constexpr forceinline float getMinHitWindow50() { return 200.f; }
constexpr forceinline float getMidHitWindow50() { return 150.f; }
constexpr forceinline float getMaxHitWindow50() { return 100.f; }

constexpr forceinline float getHitWindowMiss() { return 400.f; }

// respect mods and overrides
float getMinApproachTime();
float getMidApproachTime();
float getMaxApproachTime();

// AR 5 -> 1200 ms
template <typename T>
forceinline T mapDifficultyRange(T scaledDiff, T min, T mid, T max)
    requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
{
    if(scaledDiff == (T)5.)
        return mid;
    else if(scaledDiff > (T)5.)
        return mid + (max - mid) * (scaledDiff - (T)5.) / (T)5.;
    else
        return mid - (mid - min) * ((T)5. - scaledDiff) / (T)5.;
}

float arToMilliseconds(float AR);

inline INLINE_BODY float odTo50HitWindowMS(float OD) {
    return mapDifficultyRange(OD, getMinHitWindow50(), getMidHitWindow50(), getMaxHitWindow50());
}
inline INLINE_BODY float odTo100HitWindowMS(float OD) {
    return mapDifficultyRange(OD, getMinHitWindow100(), getMidHitWindow100(), getMaxHitWindow100());
}
inline INLINE_BODY float odTo300HitWindowMS(float OD) {
    return mapDifficultyRange(OD, getMinHitWindow300(), getMidHitWindow300(), getMaxHitWindow300());
}

// 1200 ms -> AR 5
forceinline float mapDifficultyRangeInv(float val, float min, float mid, float max) {
    if(val == mid)
        return 5.0f;
    else if(val < mid)  // > 5.0f (inverted)
        return ((val * 5.0f - mid * 5.0f) / (max - mid)) + 5.0f;
    else  // < 5.0f (inverted)
        return 5.0f - ((mid * 5.0f - val * 5.0f) / (mid - min));
}

// AR 9, speed 1.5 -> AR 10.3
float arWithSpeed(float AR, float speed);

// OD 9, speed 1.5 -> OD 10.4
inline INLINE_BODY float odWithSpeed(float OD, float speed) {
    float hittableTime = odTo300HitWindowMS(OD);
    return mapDifficultyRangeInv(hittableTime / speed, getMinHitWindow300(), getMidHitWindow300(),
                                 getMaxHitWindow300());
}

inline INLINE_BODY float getApproachTimeForStacking(float AR) {
    return mapDifficultyRange(AR, getMinApproachTime(), getMidApproachTime(), getMaxApproachTime());
}

// raw spins required per second
float getSpinnerSpinsPerSecond(const AbstractBeatmapInterface *beatmap);

inline INLINE_BODY float getSpinnerRotationsForSpeedMultiplier(const AbstractBeatmapInterface *beatmap,
                                                               i32 spinnerDuration, float speedMultiplier) {
    /// return (int)((float)spinnerDuration / 1000.0f * getSpinnerSpinsPerSecond(beatmap)); // actual
    return (int)((((float)spinnerDuration / 1000.0f * getSpinnerSpinsPerSecond(beatmap)) * 0.5f) *
                 (std::min(1.0f / speedMultiplier, 1.0f)));  // Mc
}

// spinner length compensated rotations
// respect all mods and overrides
float getSpinnerRotationsForSpeedMultiplier(const AbstractBeatmapInterface *beatmap, i32 spinnerDuration);

//*************//
//	Playfield  //
//*************//

float getPlayfieldScaleFactor();

forceinline vec2 getPlayfieldSize() {
    const float scaleFactor = getPlayfieldScaleFactor();

    return {(float)OSU_COORD_WIDTH * scaleFactor, (float)OSU_COORD_HEIGHT * scaleFactor};
}

vec2 getPlayfieldOffset();

inline vec2 getPlayfieldCenter() {
    const float scaleFactor = getPlayfieldScaleFactor();
    const vec2 playfieldOffset = getPlayfieldOffset();

    return {(OSU_COORD_WIDTH / 2.f) * scaleFactor + playfieldOffset.x,
            (OSU_COORD_HEIGHT / 2.f) * scaleFactor + playfieldOffset.y};
}

//*********************//
//	Hitobject Scaling  //
//*********************//

// "Builds of osu! up to 2013-05-04 had the gamefield being rounded down, which caused incorrect radius calculations
// in widescreen cases. This ratio adjusts to allow for old replays to work post-fix, which in turn increases the
// lenience for all plays, but by an amount so small it should only be effective in replays."
inline constexpr const float broken_gamefield_rounding_allowance{1.00041f};

forceinline f32 getRawHitCircleScale(f32 CS) {
    return std::max(0.0f, ((1.0f - 0.7f * (CS - 5.0f) / 5.0f) / 2.0f) * broken_gamefield_rounding_allowance);
}

// gives the circle diameter in osu!pixels, goes negative above CS 12.1429
forceinline f32 getRawHitCircleDiameter(f32 CS) { return getRawHitCircleScale(CS) * 128.0f; }

// scales osu!pixels to the actual playfield size
forceinline f32 getHitCircleXMultiplier() { return getPlayfieldSize().x / OSU_COORD_WIDTH; }

}  // namespace GameRules
