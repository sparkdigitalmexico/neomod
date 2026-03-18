#pragma once
// Copyright (c) 2015, PG & Jeffrey Han (opsu!), All rights reserved.
#if __has_include("config.h")
#include "config.h"
#endif

#include "Vectors.h"

#include <vector>
#include <memory>

enum class SLIDERCURVETYPE : char {
    CATMULL = 'C',
    BEZIER = 'B',
    LINEAR = 'L',
    PASSTHROUGH = 'P',
};

namespace neomod {

//**********************//
//	 Curve Base Class	//
//**********************//

class SliderCurve {
   public:
    static std::unique_ptr<SliderCurve> createCurve(SLIDERCURVETYPE type, std::vector<vec2> controlPoints,
                                                    f32 pixelLength);
    static std::unique_ptr<SliderCurve> createCurve(SLIDERCURVETYPE type, std::vector<vec2> controlPoints,
                                                    f32 pixelLength, f32 curvePointsSeparation);

   public:
    SliderCurve() = delete;

    SliderCurve(const SliderCurve &) = default;
    SliderCurve &operator=(const SliderCurve &) = default;
    SliderCurve(SliderCurve &&) = default;
    SliderCurve &operator=(SliderCurve &&) = default;
    virtual ~SliderCurve() = default;

    virtual void updateStackPosition(f32 stackMulStackOffset, bool HR);

    [[nodiscard]] virtual vec2 pointAt(f32 t) const = 0;          // with stacking
    [[nodiscard]] virtual vec2 originalPointAt(f32 t) const = 0;  // without stacking

    [[nodiscard]] inline f32 getStartAngle() const { return m_startAngle; }
    [[nodiscard]] inline f32 getEndAngle() const { return m_endAngle; }

    [[nodiscard]] inline const std::vector<vec2> &getPoints() const { return m_curvePoints; }
    [[nodiscard]] inline const std::vector<std::vector<vec2>> &getPointSegments() const { return m_curvePointSegments; }

    [[nodiscard]] inline f32 getPixelLength() const { return m_pixelLength; }

    [[nodiscard]] inline vec4 getBounds() const { return m_vBounds; }                  // with stacking
    [[nodiscard]] inline vec4 getOriginalBounds() const { return m_vOriginalBounds; }  // without stacking

   protected:
    SliderCurve(std::vector<vec2> controlPoints, f32 pixelLength);

    // original input values
    std::vector<vec2> m_controlPoints;

    // these must be explicitly calculated/set in one of the subclasses
    std::vector<std::vector<vec2>> m_curvePointSegments;
    std::vector<std::vector<vec2>> m_originalCurvePointSegments;
    std::vector<vec2> m_curvePoints;
    std::vector<vec2> m_originalCurvePoints;

   private:
    vec4 m_vBounds;
    vec4 m_vOriginalBounds;

   protected:
    f32 m_startAngle;
    f32 m_endAngle;
    f32 m_pixelLength;
};
}  // namespace neomod
