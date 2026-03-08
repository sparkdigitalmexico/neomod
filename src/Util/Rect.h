#pragma once
// Copyright (c) 2012, PG, All rights reserved.
#include "config.h"
#include "noinclude.h"
#include "Vectors.h"

#ifndef BUILD_TOOLS_ONLY
#include "fmt/format.h"
#include "fmt/compile.h"
#endif

template <typename Vec = vec2>
class McRectBase {
    using scalar = Vec::value_type;

   public:
    McRectBase(scalar x = 0, scalar y = 0, scalar width = 0, scalar height = 0, bool isCentered = false);
    McRectBase(Vec pos, Vec size, bool isCentered = false);

    ~McRectBase();
    McRectBase(const McRectBase &);
    McRectBase &operator=(const McRectBase &);
    McRectBase(McRectBase &&) noexcept;
    McRectBase &operator=(McRectBase &&) noexcept;

    template <typename OtherVec>
        requires(!std::is_same_v<OtherVec, Vec>)
    McRectBase(const McRectBase<OtherVec> &other);

    // grow to a union of another rect
    void grow(const McRectBase &other);

    // grow to include a point
    void grow(Vec point);

    // loosely within (inside or equals (+ lenience amount))
    [[nodiscard]] bool contains(Vec point, scalar lenience = 0) const;

    // strictly within (not or-equal)
    [[nodiscard]] bool containsStrict(Vec point) const;

    [[nodiscard]] bool intersects(const McRectBase &rect) const;

    [[nodiscard]] McRectBase intersect(const McRectBase &rect) const;

    [[nodiscard]] McRectBase Union(const McRectBase &other) const;

    [[nodiscard]] Vec getCenter() const;
    [[nodiscard]] Vec getMax() const;

    // get
    [[nodiscard]] const Vec &getPos() const;
    [[nodiscard]] const Vec &getMin() const;
    [[nodiscard]] const Vec &getSize() const;

    [[nodiscard]] const scalar &getX() const;
    [[nodiscard]] const scalar &getY() const;
    [[nodiscard]] const scalar &getMinX() const;
    [[nodiscard]] const scalar &getMinY() const;

    [[nodiscard]] scalar getMaxX() const;
    [[nodiscard]] scalar getMaxY() const;

    [[nodiscard]] const scalar &getWidth() const;
    [[nodiscard]] const scalar &getHeight() const;

    // set
    void setMin(Vec min);
    void setMax(Vec max);
    void setMinX(scalar minx);
    void setMinY(scalar miny);
    void setMaxX(scalar maxx);
    void setMaxY(scalar maxy);
    void setPos(Vec pos);
    void setPosX(scalar posx);
    void setPosY(scalar posy);
    void setSize(Vec size);
    void setWidth(scalar width);
    void setHeight(scalar height);

    [[nodiscard]] bool operator==(const McRectBase &rhs) const;

   private:
    void set(scalar x, scalar y, scalar width, scalar height, bool isCentered = false);

    void set(Vec pos, Vec size, bool isCentered = false);

    Vec vMin;
    Vec vSize;

    template <typename>
    friend class McRectBase;

#ifndef BUILD_TOOLS_ONLY
    template <typename, typename, typename>
    friend struct fmt::formatter;
#endif
};

extern template class McRectBase<vec2>;
extern template class McRectBase<ivec2>;

using McRect = McRectBase<vec2>;
using McFRect = McRectBase<vec2>;
using McIRect = McRectBase<ivec2>;

#ifndef BUILD_TOOLS_ONLY  // avoid an unnecessary dependency on fmt when building tools only
namespace fmt {
template <typename Vec>
struct formatter<McRectBase<Vec>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx) const {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const McRectBase<Vec> &r, FormatContext &ctx) const {
        if constexpr(std::is_floating_point_v<typename McRectBase<Vec>::scalar>) {
            return format_to(ctx.out(), "({:.2f},{:.2f}): {:.2f}x{:.2f}"_cf, r.vMin.x, r.vMin.y, r.vSize.x, r.vSize.y);
        } else {
            return format_to(ctx.out(), "({},{}): {}x{}"_cf, r.vMin.x, r.vMin.y, r.vSize.x, r.vSize.y);
        }
    }
};
}  // namespace fmt
#endif
