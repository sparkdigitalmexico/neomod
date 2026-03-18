// Copyright (c) 2015, PG & Jeffrey Han (opsu!), All rights reserved.
#include "SliderCurves.h"
#include "noinclude.h"
#include "types.h"

#include <algorithm>
#include <cmath>
#include <array>

#ifndef BUILD_TOOLS_ONLY
#include "Engine.h"
#include "Logging.h"
#include "OsuConVars.h"
#define SLIDER_CURVE_POINTS_SEPARATION cv::slider_curve_points_separation.getFloat()
#define SLIDER_CURVE_MAX_LENGTH cv::slider_curve_max_length.getFloat()
#define SLIDER_CURVE_MAX_POINTS cv::slider_curve_max_points.getVal<u32>()
#else
#include <print>
#define SLIDER_CURVE_POINTS_SEPARATION 2.5f
#define SLIDER_CURVE_MAX_LENGTH 32768.f
#define SLIDER_CURVE_MAX_POINTS 9999U
#define debugLog(...) std::println(__VA_ARGS__)
#endif

namespace neomod {
static constexpr const SLIDERCURVETYPE CIRCULAR{-(char)SLIDERCURVETYPE::PASSTHROUGH};

//*******************//
//	 Curve Builder	 //
//*******************//

// combines bezier approximation and equal-distance resampling with persistent buffers
// placed up here for friend access to SliderCurve
class SCEDMBuilder final {
   private:
    // For bezier approximator:
    // https://github.com/ppy/osu/blob/master/osu.Game/Rulesets/Objects/BezierApproximator.cs
    // https://github.com/ppy/osu-framework/blob/master/osu.Framework/MathUtils/PathApproximator.cs
    // Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.

    static constexpr f64 TOLERANCE_SQ = 0.25 * 0.25;

    // bezier approximation buffers
    std::vector<vec2> m_midpoints;
    std::vector<vec2> m_leftChild;
    std::vector<vec2> m_rightChild;
    std::vector<std::vector<vec2>> m_curvePool;
    std::vector<u32> m_poolStack;

    u32 m_poolNextFree{0};
    u32 m_bezierCount{0};

    // accumulated curve data (flat buffer + segment boundaries)
    std::vector<vec2> m_allPoints;
    std::vector<u32> m_segmentStarts;  // indices into allPoints where each segment begins

    [[nodiscard]] bool isFlatEnough(const vec2 *curve) const {
        for(u32 i = 1; i < m_bezierCount - 1; i++) {
            const f32 len = vec::length(curve[i - 1] - 2.f * curve[i] + curve[i + 1]);
            if((f64)(len * len) > TOLERANCE_SQ * 4) return false;
        }
        return true;
    }

    void subdivide(const vec2 *curve, vec2 *l, vec2 *r) {
        for(u32 i = 0; i < m_bezierCount; ++i) {
            m_midpoints[i] = curve[i];
        }

        for(u32 i = 0; i < m_bezierCount; i++) {
            l[i] = m_midpoints[0];
            r[m_bezierCount - i - 1] = m_midpoints[m_bezierCount - i - 1];

            for(u32 j = 0; j < m_bezierCount - i - 1; j++) {
                m_midpoints[j] = (m_midpoints[j] + m_midpoints[j + 1]) * 0.5f;
            }
        }
    }

    void approximate(const vec2 *curve) {
        subdivide(curve, m_leftChild.data(), m_rightChild.data());

        for(u32 i = 0; i < m_bezierCount - 1; ++i) {
            m_leftChild[m_bezierCount + i] = m_rightChild[i + 1];
        }

        m_allPoints.push_back(curve[0]);
        for(u32 i = 1; i < m_bezierCount - 1; ++i) {
            const u32 index = 2 * i;
            m_allPoints.push_back(0.25f * (m_leftChild[index - 1] + 2.f * m_leftChild[index] + m_leftChild[index + 1]));
        }
    }

    u32 acquireCurve() {
        if(m_poolNextFree >= m_curvePool.size()) {
            m_curvePool.emplace_back();
        }
        u32 idx = m_poolNextFree++;
        if(m_curvePool[idx].size() < m_bezierCount) {
            m_curvePool[idx].resize(m_bezierCount);
        }
        return idx;
    }

    void generateBezierPoints(const vec2 *controlPoints, u32 count) {
        m_bezierCount = count;

        if(m_bezierCount == 0) return;
        if(m_bezierCount < 2) {
            m_allPoints.push_back(controlPoints[0]);
            return;
        }

        if(m_midpoints.size() < m_bezierCount) m_midpoints.resize(m_bezierCount);
        if(m_leftChild.size() < m_bezierCount * 2 - 1) m_leftChild.resize(m_bezierCount * 2 - 1);
        if(m_rightChild.size() < m_bezierCount) m_rightChild.resize(m_bezierCount);

        m_poolNextFree = 0;
        m_poolStack.clear();

        u32 initialIdx = acquireCurve();
        std::copy_n(controlPoints, m_bezierCount, m_curvePool[initialIdx].begin());
        m_poolStack.push_back(initialIdx);

        while(!m_poolStack.empty()) {
            u32 parentIdx = m_poolStack.back();
            m_poolStack.pop_back();
            vec2 *parent = m_curvePool[parentIdx].data();

            if(isFlatEnough(parent)) {
                approximate(parent);
                continue;
            }

            u32 rightIdx = acquireCurve();
            vec2 *right = m_curvePool[rightIdx].data();

            subdivide(parent, m_leftChild.data(), right);

            std::copy_n(m_leftChild.data(), m_bezierCount, parent);

            m_poolStack.push_back(rightIdx);
            m_poolStack.push_back(parentIdx);
        }

        m_allPoints.push_back(controlPoints[m_bezierCount - 1]);
    }

   public:
    void reset() {
        m_allPoints.clear();
        m_segmentStarts.clear();
    }

    void addBezierSegment(const vec2 *controlPoints, u32 count) {
        m_segmentStarts.push_back(m_allPoints.size());
        generateBezierPoints(controlPoints, count);
    }

    void addCatmullSegment(const std::array<vec2, 4> &initPoints) {
        m_segmentStarts.push_back(m_allPoints.size());

        f32 approxLength = 0;
        for(i32 i = 1; i < 4; i++) {
            approxLength += std::max(0.0001f, vec::length(initPoints[i] - initPoints[i - 1]));
        }

        const i32 numPoints = (i32)(approxLength / 8.0f) + 2;

        for(i32 i = 0; i < numPoints; i++) {
            f32 t = (f32)i / (f32)(numPoints - 1);
            t = t + 1;  // t * (2 - 1) + 1

            const vec2 A1 = initPoints[0] * (1 - t) + initPoints[1] * t;
            const vec2 A2 = initPoints[1] * (2 - t) + initPoints[2] * (t - 1);
            const vec2 A3 = initPoints[2] * (3 - t) + initPoints[3] * (t - 2);

            const vec2 B1 = A1 * ((2 - t) * 0.5f) + A2 * (t * 0.5f);
            const vec2 B2 = A2 * ((3 - t) * 0.5f) + A3 * ((t - 1) * 0.5f);

            m_allPoints.push_back(B1 * (2 - t) + B2 * (t - 1));
        }
    }

    [[nodiscard]] bool hasSegments() const { return !m_segmentStarts.empty(); }

    void build(SliderCurve &curve, u32 nCurve, f32 pixelLength);
};

// the final step for building bezier/catmull curves
void SCEDMBuilder::build(SliderCurve &curve, u32 nCurve, f32 pixelLength) {
    if(m_allPoints.empty()) {
        debugLog("(SliderCurveBuilder) ERROR: allPoints.size() == 0!!!");
        return;
    }

    // add sentinel for easier iteration
    m_segmentStarts.push_back(m_allPoints.size());

    u32 curSegment = 0;
    u32 curPoint = 0;
    u32 segmentEnd = m_segmentStarts[1];

    f32 distanceAt = 0.0f;
    f32 lastDistanceAt = 0.0f;

    vec2 lastCurve = m_allPoints[0];

    // length of the curve should be equal to the pixel length
    // for each distance, try to get in between the two points that are between it
    vec2 lastCurvePointForNextSegmentStart{0.f};
    std::vector<vec2> curCurvePoints;
    for(i64 i = 0; i < (nCurve + 1LL); i++) {
        const f32 temp_dist = (f32)((f32)i * pixelLength) / (f32)nCurve;
        const f32 prefDistance =
            /*trunc*/ (f32)(i32)((std::isfinite(temp_dist) && temp_dist >= (f32)(std::numeric_limits<i32>::min()) &&
                                  temp_dist <= (f32)(std::numeric_limits<i32>::max()))
                                     ? temp_dist
                                     : 0.f);

        while(distanceAt < prefDistance) {
            lastDistanceAt = distanceAt;
            if(curPoint < m_allPoints.size()) lastCurve = m_allPoints[curPoint];

            // jump to next point
            curPoint++;

            if(curPoint >= segmentEnd) {
                // jump to next segment
                curSegment++;

                if(curCurvePoints.size() > 0) {
                    curve.m_curvePointSegments.push_back(std::move(curCurvePoints));
                    curCurvePoints.clear();

                    // prepare the next segment by setting/adding the starting point to the exact end point of the
                    // previous segment
                    // this also enables an optimization, namely that startcaps only have to be drawn
                    // [for every segment] if the startpoint != endpoint in the loop
                    if(curve.m_curvePoints.size() > 0) curCurvePoints.push_back(lastCurvePointForNextSegmentStart);
                }

                if(curSegment + 1 < m_segmentStarts.size()) {
                    segmentEnd = m_segmentStarts[curSegment + 1];
                } else {
                    curPoint = m_allPoints.size() - 1;
                    if(lastDistanceAt == distanceAt) {
                        // out of points even though the preferred distance hasn't been reached
                        break;
                    }
                }
            }

            if(curPoint < m_allPoints.size()) {
                distanceAt += vec::length(m_allPoints[curPoint] - m_allPoints[curPoint - 1]);
            }
        }

        const vec2 thisCurve = (curPoint < m_allPoints.size()) ? m_allPoints[curPoint] : vec2{0.f, 0.f};

        // interpolate the point between the two closest distances
        curve.m_curvePoints.emplace_back();
        curCurvePoints.emplace_back();
        if(distanceAt - lastDistanceAt > 1) {
            const f32 t = (prefDistance - lastDistanceAt) / (distanceAt - lastDistanceAt);
            curve.m_curvePoints[i] =
                vec2(std::lerp(lastCurve.x, thisCurve.x, t), std::lerp(lastCurve.y, thisCurve.y, t));
        } else
            curve.m_curvePoints[i] = thisCurve;

        // add the point to the current segment
        // (this is not using the lerp'd point! would cause mm mesh artifacts if it did)
        lastCurvePointForNextSegmentStart = thisCurve;
        curCurvePoints.back() = thisCurve;
    }

    // if we only had one segment, no jump to any next curve has occurred (and therefore no insertion of the segment
    // into the vector) manually add the lone segment here
    if(curCurvePoints.size() > 0) curve.m_curvePointSegments.push_back(std::move(curCurvePoints));

    // sanity check
    // spec: FIXME: at least one of my maps triggers this (in upstream mcosu too), try to fix
    if(curve.m_curvePoints.size() == 0) {
        debugLog("(SliderCurveBuilder) ERROR: curvePoints.size() == 0!!!");
        return;
    }

    // make sure that the uninterpolated segment points are exactly as long as the pixelLength
    // this is necessary because we can't use the lerp'd points for the segments
    f32 segmentedLength = 0.0f;
    for(const auto &curvePointSegment : curve.m_curvePointSegments) {
        for(u32 p = 0; p < curvePointSegment.size(); p++) {
            segmentedLength += ((p == 0) ? 0 : vec::length(curvePointSegment[p] - curvePointSegment[p - 1]));
        }
    }

    // TODO: this is still incorrect. sliders are sometimes too long or start too late, even if only for a few
    // pixels
    if(segmentedLength > pixelLength && curve.m_curvePointSegments.size() > 1 &&
       curve.m_curvePointSegments[0].size() > 1) {
        f32 excess = segmentedLength - pixelLength;
        while(excess > 0) {
            for(i64 s = (i64)curve.m_curvePointSegments.size() - 1; s >= 0; s--) {
                for(i64 p = (i64)curve.m_curvePointSegments[s].size() - 1; p >= 0; p--) {
                    const f32 curLength =
                        (p == 0) ? 0
                                 : vec::length(curve.m_curvePointSegments[s][p] - curve.m_curvePointSegments[s][p - 1]);
                    if(curLength >= excess && p != 0) {
                        vec2 segmentVector =
                            vec::normalize(curve.m_curvePointSegments[s][p] - curve.m_curvePointSegments[s][p - 1]);
                        curve.m_curvePointSegments[s][p] = curve.m_curvePointSegments[s][p] - segmentVector * excess;
                        excess = 0.0f;
                        break;
                    } else {
                        curve.m_curvePointSegments[s].erase(curve.m_curvePointSegments[s].begin() + p);
                        excess -= curLength;
                    }
                }
            }
        }
    }

    // calculate start and end angles for possible repeats
    // (good enough and cheaper than calculating it live every frame)
    if(curve.m_curvePoints.size() > 1) {
        vec2 c1 = curve.m_curvePoints[0];
        u32 cnt = 1;
        vec2 c2 = curve.m_curvePoints[cnt++];
        while(cnt <= nCurve && cnt < curve.m_curvePoints.size() && vec::length(c2 - c1) < 1) {
            c2 = curve.m_curvePoints[cnt++];
        }
        curve.m_startAngle = std::atan2(c2.y - c1.y, c2.x - c1.x) * 180.f / PI_F;
    }

    if(curve.m_curvePoints.size() > 1) {
        if(nCurve < curve.m_curvePoints.size()) {
            vec2 c1 = curve.m_curvePoints[nCurve];
            i64 cnt = nCurve - 1;
            vec2 c2 = curve.m_curvePoints[cnt--];
            while(cnt >= 0 && vec::length(c2 - c1) < 1) {
                c2 = curve.m_curvePoints[cnt--];
            }
            curve.m_endAngle = std::atan2(c2.y - c1.y, c2.x - c1.x) * 180.f / PI_F;
        }
    }
}

// used as a calculation buffer to avoid reallocations on each curve creation (for catmull/bezier curves)
static thread_local SCEDMBuilder g_curveBuilder;

//***********************//
//	 Curve Subclasses	 //
//***********************//

//*******************//
//	 Bezier Curves	 //
//*******************//

void SliderCurve::constructBezier(std::span<const vec2> controlPoints, f32 curvePointsSeparation, bool line) {
    const u32 max_points = SLIDER_CURVE_MAX_POINTS;
    m_NCurve = std::min((u32)(m_pixelLength / std::clamp<f32>(curvePointsSeparation, 1.0f, 100.0f)), max_points);

    const u32 numControlPoints = controlPoints.size();

    g_curveBuilder.reset();

    // Beziers: splits points into different Beziers if has the same points (red anchor points)
    // a b c - c d - d e f g
    // Lines: generate a new curve for each sequential pair
    // ab  bc  cd  de  ef  fg
    u32 segmentStart = 0;
    for(u32 i = 1; i < numControlPoints; i++) {
        if(line) {
            g_curveBuilder.addBezierSegment(&controlPoints[i - 1], 2);
        } else if(controlPoints[i] == controlPoints[i - 1]) {
            // red anchor point - end current segment
            if(i - segmentStart >= 2) {
                g_curveBuilder.addBezierSegment(&controlPoints[segmentStart], i - segmentStart);
            }
            segmentStart = i;
        }
    }

    // handle final segment (non-line mode only)
    if(!line && numControlPoints - segmentStart >= 2) {
        g_curveBuilder.addBezierSegment(&controlPoints[segmentStart], numControlPoints - segmentStart);
    }

    if(g_curveBuilder.hasSegments()) {
        g_curveBuilder.build(*this, m_NCurve, m_pixelLength);
    } else {
        debugLog(
            "ERROR: no segments (line: {} numControlPoints: {} pixelLength: {} "
            "curvePointsSeparation: {})",
            line, numControlPoints, m_pixelLength, curvePointsSeparation);
    }
}

//********************//
//   Catmull Curves   //
//********************//

void SliderCurve::constructCatmull(std::span<const vec2> controlPoints, f32 curvePointsSeparation) {
    const u32 max_points = SLIDER_CURVE_MAX_POINTS;
    m_NCurve = std::min((u32)(m_pixelLength / std::clamp<f32>(curvePointsSeparation, 1.0f, 100.0f)), max_points);

    const u32 numControlPoints = controlPoints.size();

    g_curveBuilder.reset();

    // build temporary 4-point windows for catmull-rom
    // repeat the first and last points as control points if they differ from neighbors

    // handle first point duplication
    const bool duplicateFirst = (controlPoints[0].x != controlPoints[1].x || controlPoints[0].y != controlPoints[1].y);

    // handle last point duplication
    const bool duplicateLast = (controlPoints[numControlPoints - 1].x != controlPoints[numControlPoints - 2].x ||
                                controlPoints[numControlPoints - 1].y != controlPoints[numControlPoints - 2].y);

    // calculate effective point count
    u32 effectiveCount = numControlPoints + (duplicateFirst ? 1 : 0) + (duplicateLast ? 1 : 0);

    auto getPoint = [&](u32 idx) -> vec2 {
        if(duplicateFirst) {
            if(idx == 0) return controlPoints[0];
            idx--;
        }
        if(idx < numControlPoints) return controlPoints[idx];
        return controlPoints[numControlPoints - 1];
    };

    std::array<vec2, 4> catmullPoints;  // NOLINT
    for(u32 i = 0; i + 3 < effectiveCount; i++) {
        catmullPoints[0] = getPoint(i);
        catmullPoints[1] = getPoint(i + 1);
        catmullPoints[2] = getPoint(i + 2);
        catmullPoints[3] = getPoint(i + 3);
        g_curveBuilder.addCatmullSegment(catmullPoints);
    }

    // handle final window if we have exactly 4 effective points remaining
    if(effectiveCount >= 4) {
        catmullPoints[0] = getPoint(effectiveCount - 4);
        catmullPoints[1] = getPoint(effectiveCount - 3);
        catmullPoints[2] = getPoint(effectiveCount - 2);
        catmullPoints[3] = getPoint(effectiveCount - 1);

        // only add if not already added in the loop
        if(effectiveCount == 4) {
            g_curveBuilder.addCatmullSegment(catmullPoints);
        }
    }

    if(g_curveBuilder.hasSegments()) {
        g_curveBuilder.build(*this, m_NCurve, m_pixelLength);
    } else {
        debugLog("ERROR: catmulls.size() == 0 (numControlPoints: {} pixelLength: {} curvePointsSeparation: {})",
                 numControlPoints, m_pixelLength, curvePointsSeparation);
    }
}

//**********************//
//	 Circular Curves	//
//**********************//

void SliderCurve::constructCircular(std::span<const vec2> controlPoints, f32 curvePointsSeparation) {
    // initialize
    m_vCircleCenterX = 0.f;
    m_vCircleCenterY = 0.f;
    m_radius = 0.f;
    m_calcStartAngleDeg = 0.f;
    m_calcEndAngleDeg = 0.f;

    if(controlPoints.size() != 3) {
        debugLog("ERROR: controlPoints.size() != 3");
        return;
    }

    // helpers
    static constexpr auto intersect = [] [[gnu::always_inline]] (vec2 a, vec2 ta, vec2 b, vec2 tb) -> vec2 {
        const f32 des = (tb.x * ta.y - tb.y * ta.x);
        if(std::abs(des) < 0.0001f) {
            debugLog("constructCircular::intersect() ERROR: Vectors are parallel!!!");
            return {0.f, 0.f};
        }

        const f32 u = ((b.y - a.y) * ta.x + (a.x - b.x) * ta.y) / des;
        return (b + vec2(tb.x * u, tb.y * u));
    };
    static constexpr auto isIn = [] [[gnu::always_inline]] (f32 a, f32 b, f32 c) -> bool {
        return ((b > a && b < c) || (b < a && b > c));
    };

    // construct the three points
    const vec2 start = controlPoints[0];
    const vec2 mid = controlPoints[1];
    const vec2 end = controlPoints[2];

    // find the circle center
    const vec2 mida = start + (mid - start) * 0.5f;
    const vec2 midb = end + (mid - end) * 0.5f;

    vec2 nora = mid - start;
    f32 temp = nora.x;
    nora.x = -nora.y;
    nora.y = temp;
    vec2 norb = mid - end;
    temp = norb.x;
    norb.x = -norb.y;
    norb.y = temp;

    const vec2 intersection = intersect(mida, nora, midb, norb);
    m_vCircleCenterX = intersection.x;
    m_vCircleCenterY = intersection.y;

    // find the angles relative to the circle center
    const vec2 startAngPoint = start - intersection;
    const vec2 midAngPoint = mid - intersection;
    const vec2 endAngPoint = end - intersection;

    m_calcStartAngleDeg = (f32)std::atan2(startAngPoint.y, startAngPoint.x);
    const f32 midAng = (f32)std::atan2(midAngPoint.y, midAngPoint.x);
    m_calcEndAngleDeg = (f32)std::atan2(endAngPoint.y, endAngPoint.x);

    // find the angles that pass through midAng

    // clang-format off
    if(!isIn(m_calcStartAngleDeg, midAng, m_calcEndAngleDeg)) {
        if(
            (std::abs(m_calcStartAngleDeg + 2.f * PI_F - m_calcEndAngleDeg) < 2.f * PI_F) &&
                 isIn(m_calcStartAngleDeg + (2.f * PI_F), midAng, m_calcEndAngleDeg)
            ) {
            m_calcStartAngleDeg += 2.f * PI_F;
        } else if(
            (std::abs(m_calcStartAngleDeg - (m_calcEndAngleDeg + 2.f * PI_F)) < 2.f * PI_F) &&
                 isIn(m_calcStartAngleDeg, midAng, m_calcEndAngleDeg + (2.f * PI_F))
            ) {
            m_calcEndAngleDeg += 2.f * PI_F;
        } else if(
            (std::abs(m_calcStartAngleDeg - 2.f * PI_F - m_calcEndAngleDeg) < 2.f * PI_F) &&
                 isIn(m_calcStartAngleDeg - (2.f * PI_F), midAng, m_calcEndAngleDeg)
            ) {
            m_calcStartAngleDeg -= 2.f * PI_F;
        } else if(
            (std::abs(m_calcStartAngleDeg - (m_calcEndAngleDeg - 2.f * PI_F)) < 2.f * PI_F) &&
                 isIn(m_calcStartAngleDeg, midAng, m_calcEndAngleDeg - (2.f * PI_F))
            ) {
            m_calcEndAngleDeg -= 2.f * PI_F;
        } else {
            debugLog("ERROR: Cannot find angles between midAng ({} {} {})",
                     m_calcStartAngleDeg, midAng, m_calcEndAngleDeg);
            return;
        }
    }
    // clang-format on

    // find an angle with an arc length of pixelLength along this circle
    m_radius = vec::length(startAngPoint);
    const f32 arcAng = m_pixelLength / m_radius;  // len = theta * r / theta = len / r

    // now use it for our new end angle
    m_calcEndAngleDeg =
        (m_calcEndAngleDeg > m_calcStartAngleDeg) ? m_calcStartAngleDeg + arcAng : m_calcStartAngleDeg - arcAng;

    // find the angles to draw for repeats
    m_endAngle = (f32)((m_calcEndAngleDeg + (m_calcStartAngleDeg > m_calcEndAngleDeg ? PI_F / 2.0f : -PI_F / 2.0f)) *
                       180.0f / PI_F);
    m_startAngle =
        (f32)((m_calcStartAngleDeg + (m_calcStartAngleDeg > m_calcEndAngleDeg ? -PI_F / 2.0f : PI_F / 2.0f)) * 180.0f /
              PI_F);

    // calculate points
    const f32 max_points = SLIDER_CURVE_MAX_POINTS;
    const f32 steps = std::min(m_pixelLength / (std::clamp<f32>(curvePointsSeparation, 1.0f, 100.0f)), max_points);
    const i32 intSteps = (i32)std::round(steps) + 2;  // must guarantee an int range of 0 to steps!
    for(i32 i = 0; i < intSteps; i++) {
        f32 t = std::clamp<f32>((f32)i / steps, 0.0f, 1.0f);
        m_curvePoints.push_back(pointAt(t));

        if(t >= 1.0f)  // early break if we've already reached the end
            break;
    }

    // add the segment (no special logic here for SliderCurveCircumscribedCircle, just add the entire vector)
    m_curvePointSegments.emplace_back(m_curvePoints);  // copy
}

//******************************//
//	 Curve Base Class Factory	//
//******************************//

SliderCurve::SliderCurve(SLIDERCURVETYPE ctorType, std::span<const vec2> controlPoints, f32 ctorPixelLength)
    : SliderCurve(ctorType, controlPoints, ctorPixelLength, SLIDER_CURVE_POINTS_SEPARATION) {}

// they are initialized in the construct<type> functions
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
SliderCurve::SliderCurve(SLIDERCURVETYPE ctorType, std::span<const vec2> controlPoints, f32 ctorPixelLength,
                         f32 curvePointsSeparation)
    : m_vBounds(std::numeric_limits<f32>::max(),  // minX
                std::numeric_limits<f32>::max(),  // minY
                0.0f,                             // maxX
                0.0f                              // maxY
                ),
      m_startAngle(0.f),
      m_endAngle(0.f),
      m_pixelLength(std::abs(ctorPixelLength)) {
    using enum SLIDERCURVETYPE;
    if(ctorType == PASSTHROUGH && controlPoints.size() == 3) {
        vec2 nora = controlPoints[1] - controlPoints[0];
        vec2 norb = controlPoints[1] - controlPoints[2];

        f32 temp = nora.x;
        nora.x = -nora.y;
        nora.y = temp;
        temp = norb.x;
        norb.x = -norb.y;
        norb.y = temp;

        // TODO: to properly support all aspire sliders (e.g. Ping), need to use osu circular arc calc + subdivide line
        // segments if they are too big

        if(std::abs(norb.x * nora.y - norb.y * nora.x) < 0.00001f) {
            m_type = BEZIER;
            constructBezier(controlPoints, curvePointsSeparation,
                            false);  // vectors parallel, use linear bezier instead
        } else {
            m_type = CIRCULAR;
            constructCircular(controlPoints, curvePointsSeparation);
        }
    } else if(ctorType == CATMULL) {
        m_type = CATMULL;
        constructCatmull(controlPoints, curvePointsSeparation);
    } else {
        m_type = BEZIER;
        constructBezier(controlPoints, curvePointsSeparation, (ctorType == LINEAR));
    }

    // calculate bounds
    for(vec2 point : m_curvePoints) {
        if(point.x < m_vBounds.x) m_vBounds.x = point.x;
        if(point.x > m_vBounds.z) m_vBounds.z = point.x;
        if(point.y < m_vBounds.y) m_vBounds.y = point.y;
        if(point.y > m_vBounds.w) m_vBounds.w = point.y;
    }
}

vec2 SliderCurve::pointAt(f32 t) const {
    using enum SLIDERCURVETYPE;
    if(m_type != CIRCULAR /* BEZIER || CATMULL */) {
        if(m_curvePoints.size() < 1) return {0.f, 0.f};

        const f64 indexD = (f64)t * m_NCurve;
        const u32 index = (u32)indexD;
        if(index >= m_NCurve) {
            if(m_NCurve < m_curvePoints.size())
                return m_curvePoints[m_NCurve];
            else {
                debugLog("(SliderCurve) ERROR: Illegal index {:d}!!!", m_NCurve);
                return {0.f, 0.f};
            }
        } else {
            if(index + 1 >= m_curvePoints.size()) {
                debugLog("(SliderCurve) ERROR: Illegal index {:d}!!!", index);
                return {0.f, 0.f};
            }

            const vec2 poi = m_curvePoints[index];
            const vec2 poi2 = m_curvePoints[index + 1];

            const f32 t2 = (f32)(indexD - (f64)index);

            return {std::lerp(poi.x, poi2.x, t2), std::lerp(poi.y, poi2.y, t2)};
        }
    } else {  // CIRCULAR
        const f32 sanityRange =
            SLIDER_CURVE_MAX_LENGTH;  // NOTE: added to fix some aspire problems (endless drawFollowPoints and star calc etc.)
        const f32 ang = std::lerp(m_calcStartAngleDeg, m_calcEndAngleDeg, t);

        return {std::clamp<f32>(std::cos(ang) * m_radius + m_vCircleCenterX, -sanityRange, sanityRange),
                std::clamp<f32>(std::sin(ang) * m_radius + m_vCircleCenterY, -sanityRange, sanityRange)};
    }
}

}  // namespace neomod
