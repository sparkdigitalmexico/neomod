// Copyright (c) 2025, WH, All rights reserved.
#include "Vectors.h"

// explicit instantiations

namespace vec {

template <typename T, typename V>
    requires(std::is_floating_point_v<V>) &&
            (std::is_same_v<T, vec2> || std::is_same_v<T, vec3> || std::is_same_v<T, vec4>)
void setLength(T& vec, V len) {
    if(length(vec) > FLOAT_NORMALIZE_EPSILON) {
        vec = normalize(vec) * static_cast<float>(len);
    }
}

template void setLength(vec2&, float);
template void setLength(vec3&, float);
template void setLength(vec4&, float);

template <typename T, typename V>
    requires(std::is_floating_point_v<V>) &&
            (std::is_same_v<T, dvec2> || std::is_same_v<T, dvec3> || std::is_same_v<T, dvec4>)
void setLength(T& vec, V len) {
    if(length(vec) > DOUBLE_NORMALIZE_EPSILON) {
        vec = normalize(vec) * static_cast<double>(len);
    }
}

template void setLength(dvec2&, float);
template void setLength(dvec3&, float);
template void setLength(dvec4&, float);

template <typename V>
    requires(std::is_same_v<V, vec2> || std::is_same_v<V, vec3> || std::is_same_v<V, vec4> ||
             std::is_same_v<V, dvec2> || std::is_same_v<V, dvec3> || std::is_same_v<V, dvec4> ||
             std::is_same_v<V, ivec2> || std::is_same_v<V, ivec3> || std::is_same_v<V, ivec4> ||
             std::is_same_v<V, lvec2> || std::is_same_v<V, lvec3> || std::is_same_v<V, lvec4>)
bool allEqual(const V& vec1, const V& vec2) {
    return all(equal(vec1, vec2));
}

template bool allEqual(const vec2&, const vec2&);
template bool allEqual(const vec3&, const vec3&);
template bool allEqual(const vec4&, const vec4&);

template bool allEqual(const dvec2&, const dvec2&);
template bool allEqual(const dvec3&, const dvec3&);
template bool allEqual(const dvec4&, const dvec4&);

template bool allEqual(const ivec2&, const ivec2&);
template bool allEqual(const ivec3&, const ivec3&);
template bool allEqual(const ivec4&, const ivec4&);

template bool allEqual(const lvec2&, const lvec2&);
template bool allEqual(const lvec3&, const lvec3&);
template bool allEqual(const lvec4&, const lvec4&);

}  // namespace vec