#pragma once
// Copyright (c) 2015, PG & Jeffrey Han (opsu!), All rights reserved.
#if __has_include("config.h")
#include "config.h"
#endif

#include "Vectors.h"

#include <vector>
#include <span>

namespace neomod {

enum class SLIDERCURVETYPE : char {
    CATMULL = 'C',
    BEZIER = 'B',
    LINEAR = 'L',
    PASSTHROUGH = 'P',
};

//**********************//
//	 Curve Base Class	//
//**********************//

class SliderCurve final {
   public:
    SliderCurve() = delete;

    SliderCurve(SLIDERCURVETYPE type, std::span<const vec2> controlPoints, f32 pixelLength);
    SliderCurve(SLIDERCURVETYPE type, std::span<const vec2> controlPoints, f32 pixelLength, f32 curvePointsSeparation);

    SliderCurve(const SliderCurve &) = default;
    SliderCurve &operator=(const SliderCurve &) = default;
    SliderCurve(SliderCurve &&) noexcept = default;
    SliderCurve &operator=(SliderCurve &&) noexcept = default;
    ~SliderCurve() = default;

    void updateStackPosition(f32 stackMulStackOffset, bool HR);

    [[nodiscard]] vec2 pointAt(f32 t) const;          // with stacking
    [[nodiscard]] vec2 originalPointAt(f32 t) const;  // without stacking

    [[nodiscard]] inline f32 getStartAngle() const { return m_startAngle; }
    [[nodiscard]] inline f32 getEndAngle() const { return m_endAngle; }

    [[nodiscard]] inline std::span<const vec2> getPoints() const { return m_curvePoints; }
    [[nodiscard]] inline std::span<const std::vector<vec2>> getPointSegments() const { return m_curvePointSegments; }

    [[nodiscard]] inline f32 getPixelLength() const { return m_pixelLength; }

    [[nodiscard]] inline vec4 getBounds() const { return m_vBounds; }                  // with stacking
    [[nodiscard]] inline vec4 getOriginalBounds() const { return m_vOriginalBounds; }  // without stacking

   private:
    /* these must be explicitly calculated/set in one of the subclasses */
    std::vector<std::vector<vec2>> m_curvePointSegments;
    std::vector<std::vector<vec2>> m_originalCurvePointSegments;
    std::vector<vec2> m_curvePoints;
    std::vector<vec2> m_originalCurvePoints;

    vec4 m_vBounds;
    vec4 m_vOriginalBounds;

    f32 m_startAngle;
    f32 m_endAngle;
    f32 m_pixelLength;

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201)  // nonstandard extension used : nameless struct/union
#endif

    union {
        struct {
            // type == CATMULL || type == BEZIER
            u32 m_NCurve;
        };
        struct {
            // type == CIRCULAR
            f32 m_vCircleCenterX, m_vCircleCenterY;
            f32 m_vOriginalCircleCenterX, m_vOriginalCircleCenterY;
            f32 m_radius;
            f32 m_calcStartAngleDeg;
            f32 m_calcEndAngleDeg;
        };
    };

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

    SLIDERCURVETYPE m_type;

    void constructBezier(std::span<const vec2> controlPoints, f32 curvePointsSeparation, bool line);
    void constructCatmull(std::span<const vec2> controlPoints, f32 curvePointsSeparation);
    void constructCircular(std::span<const vec2> controlPoints, f32 curvePointsSeparation);

    friend class SCEDMBuilder;
};  // namespace neomod
}  // namespace neomod
