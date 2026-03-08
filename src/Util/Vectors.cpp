// Copyright (c) 2025, WH, All rights reserved.
#include "Vectors.h"

#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"

// explicit instantiations
// (bool is broken)
// template struct glm::vec<1, bool, glm::qualifier::defaultp>;
// template struct glm::vec<2, bool, glm::qualifier::defaultp>;
// template struct glm::vec<3, bool, glm::qualifier::defaultp>;
// template struct glm::vec<4, bool, glm::qualifier::defaultp>;
template struct glm::vec<2, double, glm::qualifier::defaultp>;
template struct glm::vec<3, double, glm::qualifier::defaultp>;
template struct glm::vec<4, double, glm::qualifier::defaultp>;
template struct glm::vec<2, float, glm::qualifier::defaultp>;
template struct glm::vec<3, float, glm::qualifier::defaultp>;
template struct glm::vec<4, float, glm::qualifier::defaultp>;
template struct glm::vec<2, i32, glm::qualifier::defaultp>;
template struct glm::vec<3, i32, glm::qualifier::defaultp>;
template struct glm::vec<4, i32, glm::qualifier::defaultp>;
template struct glm::vec<2, i64, glm::qualifier::defaultp>;
template struct glm::vec<3, i64, glm::qualifier::defaultp>;
template struct glm::vec<4, i64, glm::qualifier::defaultp>;
template struct glm::vec<4, u8, glm::qualifier::defaultp>;
template struct glm::vec<2, u32, glm::qualifier::defaultp>;
template struct glm::vec<3, u32, glm::qualifier::defaultp>;
template struct glm::vec<4, u32, glm::qualifier::defaultp>;
template struct glm::vec<2, u64, glm::qualifier::defaultp>;
template struct glm::vec<3, u64, glm::qualifier::defaultp>;
template struct glm::vec<4, u64, glm::qualifier::defaultp>;

namespace vec {

f32 radians(f32 deg) { return glm::radians(deg); }
f64 radians(f64 deg) { return glm::radians(deg); }
f32 degrees(f32 deg) { return glm::degrees(deg); }
f64 degrees(f64 deg) { return glm::degrees(deg); }

dvec2 clamp(dvec2 const& p1, double p2, double p3) { return glm::clamp(p1, p2, p3); }
dvec2 clamp(dvec2 const& p1, dvec2 const& p2, dvec2 const& p3) { return glm::clamp(p1, p2, p3); }
dvec2 max(dvec2 const& p1, double p2) { return glm::max(p1, p2); }
dvec2 max(dvec2 const& p1, dvec2 const& p2) { return glm::max(p1, p2); }
dvec2 min(dvec2 const& p1, double p2) { return glm::min(p1, p2); }
dvec2 min(dvec2 const& p1, dvec2 const& p2) { return glm::min(p1, p2); }
dvec2 abs(dvec2 const& p1) { return glm::abs(p1); }
dvec2 ceil(dvec2 const& p1) { return glm::ceil(p1); }
double length(dvec2 const& p1) { return glm::length(p1); }
double distance(dvec2 const& p1, dvec2 const& p2) { return glm::distance(p1, p2); }
double dot(dvec2 const& p1, dvec2 const& p2) { return glm::dot(p1, p2); }
dvec2 floor(dvec2 const& p1) { return glm::floor(p1); }
dvec2 round(dvec2 const& p1) { return glm::round(p1); }
dvec2 normalize(dvec2 const& p1) { return glm::normalize(p1); }
dvec3 clamp(dvec3 const& p1, double p2, double p3) { return glm::clamp(p1, p2, p3); }
dvec3 clamp(dvec3 const& p1, dvec3 const& p2, dvec3 const& p3) { return glm::clamp(p1, p2, p3); }
dvec3 max(dvec3 const& p1, double p2) { return glm::max(p1, p2); }
dvec3 max(dvec3 const& p1, dvec3 const& p2) { return glm::max(p1, p2); }
dvec3 min(dvec3 const& p1, double p2) { return glm::min(p1, p2); }
dvec3 min(dvec3 const& p1, dvec3 const& p2) { return glm::min(p1, p2); }
dvec3 cross(dvec3 const& p1, dvec3 const& p2) { return glm::cross(p1, p2); }
dvec3 abs(dvec3 const& p1) { return glm::abs(p1); }
dvec3 ceil(dvec3 const& p1) { return glm::ceil(p1); }
double length(dvec3 const& p1) { return glm::length(p1); }
double distance(dvec3 const& p1, dvec3 const& p2) { return glm::distance(p1, p2); }
double dot(dvec3 const& p1, dvec3 const& p2) { return glm::dot(p1, p2); }
dvec3 floor(dvec3 const& p1) { return glm::floor(p1); }
dvec3 round(dvec3 const& p1) { return glm::round(p1); }
dvec3 normalize(dvec3 const& p1) { return glm::normalize(p1); }
dvec4 clamp(dvec4 const& p1, double p2, double p3) { return glm::clamp(p1, p2, p3); }
dvec4 clamp(dvec4 const& p1, dvec4 const& p2, dvec4 const& p3) { return glm::clamp(p1, p2, p3); }
dvec4 max(dvec4 const& p1, double p2) { return glm::max(p1, p2); }
dvec4 max(dvec4 const& p1, dvec4 const& p2) { return glm::max(p1, p2); }
dvec4 min(dvec4 const& p1, double p2) { return glm::min(p1, p2); }
dvec4 min(dvec4 const& p1, dvec4 const& p2) { return glm::min(p1, p2); }
dvec4 abs(dvec4 const& p1) { return glm::abs(p1); }
dvec4 ceil(dvec4 const& p1) { return glm::ceil(p1); }
double length(dvec4 const& p1) { return glm::length(p1); }
double distance(dvec4 const& p1, dvec4 const& p2) { return glm::distance(p1, p2); }
double dot(dvec4 const& p1, dvec4 const& p2) { return glm::dot(p1, p2); }
dvec4 floor(dvec4 const& p1) { return glm::floor(p1); }
dvec4 round(dvec4 const& p1) { return glm::round(p1); }
dvec4 normalize(dvec4 const& p1) { return glm::normalize(p1); }
vec2 clamp(vec2 const& p1, float p2, float p3) { return glm::clamp(p1, p2, p3); }
vec2 clamp(vec2 const& p1, vec2 const& p2, vec2 const& p3) { return glm::clamp(p1, p2, p3); }
vec2 max(vec2 const& p1, float p2) { return glm::max(p1, p2); }
vec2 max(vec2 const& p1, vec2 const& p2) { return glm::max(p1, p2); }
vec2 min(vec2 const& p1, float p2) { return glm::min(p1, p2); }
vec2 min(vec2 const& p1, vec2 const& p2) { return glm::min(p1, p2); }
vec2 abs(vec2 const& p1) { return glm::abs(p1); }
vec2 ceil(vec2 const& p1) { return glm::ceil(p1); }
float length(vec2 const& p1) { return glm::length(p1); }
float distance(vec2 const& p1, vec2 const& p2) { return glm::distance(p1, p2); }
float dot(vec2 const& p1, vec2 const& p2) { return glm::dot(p1, p2); }
vec2 floor(vec2 const& p1) { return glm::floor(p1); }
vec2 round(vec2 const& p1) { return glm::round(p1); }
vec2 normalize(vec2 const& p1) { return glm::normalize(p1); }
vec3 clamp(vec3 const& p1, float p2, float p3) { return glm::clamp(p1, p2, p3); }
vec3 clamp(vec3 const& p1, vec3 const& p2, vec3 const& p3) { return glm::clamp(p1, p2, p3); }
vec3 max(vec3 const& p1, float p2) { return glm::max(p1, p2); }
vec3 max(vec3 const& p1, vec3 const& p2) { return glm::max(p1, p2); }
vec3 min(vec3 const& p1, float p2) { return glm::min(p1, p2); }
vec3 min(vec3 const& p1, vec3 const& p2) { return glm::min(p1, p2); }
vec3 cross(vec3 const& p1, vec3 const& p2) { return glm::cross(p1, p2); }
vec3 abs(vec3 const& p1) { return glm::abs(p1); }
vec3 ceil(vec3 const& p1) { return glm::ceil(p1); }
float length(vec3 const& p1) { return glm::length(p1); }
float distance(vec3 const& p1, vec3 const& p2) { return glm::distance(p1, p2); }
float dot(vec3 const& p1, vec3 const& p2) { return glm::dot(p1, p2); }
vec3 floor(vec3 const& p1) { return glm::floor(p1); }
vec3 round(vec3 const& p1) { return glm::round(p1); }
vec3 normalize(vec3 const& p1) { return glm::normalize(p1); }
vec4 clamp(vec4 const& p1, float p2, float p3) { return glm::clamp(p1, p2, p3); }
vec4 clamp(vec4 const& p1, vec4 const& p2, vec4 const& p3) { return glm::clamp(p1, p2, p3); }
vec4 max(vec4 const& p1, float p2) { return glm::max(p1, p2); }
vec4 max(vec4 const& p1, vec4 const& p2) { return glm::max(p1, p2); }
vec4 min(vec4 const& p1, float p2) { return glm::min(p1, p2); }
vec4 min(vec4 const& p1, vec4 const& p2) { return glm::min(p1, p2); }
vec4 abs(vec4 const& p1) { return glm::abs(p1); }
vec4 ceil(vec4 const& p1) { return glm::ceil(p1); }
float length(vec4 const& p1) { return glm::length(p1); }
float distance(vec4 const& p1, vec4 const& p2) { return glm::distance(p1, p2); }
float dot(vec4 const& p1, vec4 const& p2) { return glm::dot(p1, p2); }
vec4 floor(vec4 const& p1) { return glm::floor(p1); }
vec4 round(vec4 const& p1) { return glm::round(p1); }
vec4 normalize(vec4 const& p1) { return glm::normalize(p1); }
ivec2 clamp(ivec2 const& p1, i32 p2, i32 p3) { return glm::clamp(p1, p2, p3); }
ivec2 clamp(ivec2 const& p1, ivec2 const& p2, ivec2 const& p3) { return glm::clamp(p1, p2, p3); }
ivec2 max(ivec2 const& p1, i32 p2) { return glm::max(p1, p2); }
ivec2 max(ivec2 const& p1, ivec2 const& p2) { return glm::max(p1, p2); }
ivec2 min(ivec2 const& p1, i32 p2) { return glm::min(p1, p2); }
ivec2 min(ivec2 const& p1, ivec2 const& p2) { return glm::min(p1, p2); }
ivec2 abs(ivec2 const& p1) { return glm::abs(p1); }
ivec3 clamp(ivec3 const& p1, i32 p2, i32 p3) { return glm::clamp(p1, p2, p3); }
ivec3 clamp(ivec3 const& p1, ivec3 const& p2, ivec3 const& p3) { return glm::clamp(p1, p2, p3); }
ivec3 max(ivec3 const& p1, i32 p2) { return glm::max(p1, p2); }
ivec3 max(ivec3 const& p1, ivec3 const& p2) { return glm::max(p1, p2); }
ivec3 min(ivec3 const& p1, i32 p2) { return glm::min(p1, p2); }
ivec3 min(ivec3 const& p1, ivec3 const& p2) { return glm::min(p1, p2); }
ivec3 abs(ivec3 const& p1) { return glm::abs(p1); }
ivec4 clamp(ivec4 const& p1, i32 p2, i32 p3) { return glm::clamp(p1, p2, p3); }
ivec4 clamp(ivec4 const& p1, ivec4 const& p2, ivec4 const& p3) { return glm::clamp(p1, p2, p3); }
ivec4 max(ivec4 const& p1, i32 p2) { return glm::max(p1, p2); }
ivec4 max(ivec4 const& p1, ivec4 const& p2) { return glm::max(p1, p2); }
ivec4 min(ivec4 const& p1, i32 p2) { return glm::min(p1, p2); }
ivec4 min(ivec4 const& p1, ivec4 const& p2) { return glm::min(p1, p2); }
ivec4 abs(ivec4 const& p1) { return glm::abs(p1); }
lvec2 clamp(lvec2 const& p1, i64 p2, i64 p3) { return glm::clamp(p1, p2, p3); }
lvec2 clamp(lvec2 const& p1, lvec2 const& p2, lvec2 const& p3) { return glm::clamp(p1, p2, p3); }
lvec2 max(lvec2 const& p1, i64 p2) { return glm::max(p1, p2); }
lvec2 max(lvec2 const& p1, lvec2 const& p2) { return glm::max(p1, p2); }
lvec2 min(lvec2 const& p1, i64 p2) { return glm::min(p1, p2); }
lvec2 min(lvec2 const& p1, lvec2 const& p2) { return glm::min(p1, p2); }
lvec2 abs(lvec2 const& p1) { return glm::abs(p1); }
lvec3 clamp(lvec3 const& p1, i64 p2, i64 p3) { return glm::clamp(p1, p2, p3); }
lvec3 clamp(lvec3 const& p1, lvec3 const& p2, lvec3 const& p3) { return glm::clamp(p1, p2, p3); }
lvec3 max(lvec3 const& p1, i64 p2) { return glm::max(p1, p2); }
lvec3 max(lvec3 const& p1, lvec3 const& p2) { return glm::max(p1, p2); }
lvec3 min(lvec3 const& p1, i64 p2) { return glm::min(p1, p2); }
lvec3 min(lvec3 const& p1, lvec3 const& p2) { return glm::min(p1, p2); }
lvec3 abs(lvec3 const& p1) { return glm::abs(p1); }
lvec4 clamp(lvec4 const& p1, i64 p2, i64 p3) { return glm::clamp(p1, p2, p3); }
lvec4 clamp(lvec4 const& p1, lvec4 const& p2, lvec4 const& p3) { return glm::clamp(p1, p2, p3); }
lvec4 max(lvec4 const& p1, i64 p2) { return glm::max(p1, p2); }
lvec4 max(lvec4 const& p1, lvec4 const& p2) { return glm::max(p1, p2); }
lvec4 min(lvec4 const& p1, i64 p2) { return glm::min(p1, p2); }
lvec4 min(lvec4 const& p1, lvec4 const& p2) { return glm::min(p1, p2); }
lvec4 abs(lvec4 const& p1) { return glm::abs(p1); }
u8vec4 clamp(u8vec4 const& p1, u8 p2, u8 p3) { return glm::clamp(p1, p2, p3); }
u8vec4 clamp(u8vec4 const& p1, u8vec4 const& p2, u8vec4 const& p3) { return glm::clamp(p1, p2, p3); }
u8vec4 max(u8vec4 const& p1, u8 p2) { return glm::max(p1, p2); }
u8vec4 max(u8vec4 const& p1, u8vec4 const& p2) { return glm::max(p1, p2); }
u8vec4 min(u8vec4 const& p1, u8 p2) { return glm::min(p1, p2); }
u8vec4 min(u8vec4 const& p1, u8vec4 const& p2) { return glm::min(p1, p2); }
u8vec4 abs(u8vec4 const& p1) { return glm::abs(p1); }
uvec2 clamp(uvec2 const& p1, u32 p2, u32 p3) { return glm::clamp(p1, p2, p3); }
uvec2 clamp(uvec2 const& p1, uvec2 const& p2, uvec2 const& p3) { return glm::clamp(p1, p2, p3); }
uvec2 max(uvec2 const& p1, u32 p2) { return glm::max(p1, p2); }
uvec2 max(uvec2 const& p1, uvec2 const& p2) { return glm::max(p1, p2); }
uvec2 min(uvec2 const& p1, u32 p2) { return glm::min(p1, p2); }
uvec2 min(uvec2 const& p1, uvec2 const& p2) { return glm::min(p1, p2); }
uvec2 abs(uvec2 const& p1) { return glm::abs(p1); }
uvec3 clamp(uvec3 const& p1, u32 p2, u32 p3) { return glm::clamp(p1, p2, p3); }
uvec3 clamp(uvec3 const& p1, uvec3 const& p2, uvec3 const& p3) { return glm::clamp(p1, p2, p3); }
uvec3 max(uvec3 const& p1, u32 p2) { return glm::max(p1, p2); }
uvec3 max(uvec3 const& p1, uvec3 const& p2) { return glm::max(p1, p2); }
uvec3 min(uvec3 const& p1, u32 p2) { return glm::min(p1, p2); }
uvec3 min(uvec3 const& p1, uvec3 const& p2) { return glm::min(p1, p2); }
uvec3 abs(uvec3 const& p1) { return glm::abs(p1); }
uvec4 clamp(uvec4 const& p1, u32 p2, u32 p3) { return glm::clamp(p1, p2, p3); }
uvec4 clamp(uvec4 const& p1, uvec4 const& p2, uvec4 const& p3) { return glm::clamp(p1, p2, p3); }
uvec4 max(uvec4 const& p1, u32 p2) { return glm::max(p1, p2); }
uvec4 max(uvec4 const& p1, uvec4 const& p2) { return glm::max(p1, p2); }
uvec4 min(uvec4 const& p1, u32 p2) { return glm::min(p1, p2); }
uvec4 min(uvec4 const& p1, uvec4 const& p2) { return glm::min(p1, p2); }
uvec4 abs(uvec4 const& p1) { return glm::abs(p1); }
ulvec2 clamp(ulvec2 const& p1, u64 p2, u64 p3) { return glm::clamp(p1, p2, p3); }
ulvec2 clamp(ulvec2 const& p1, ulvec2 const& p2, ulvec2 const& p3) { return glm::clamp(p1, p2, p3); }
ulvec2 max(ulvec2 const& p1, u64 p2) { return glm::max(p1, p2); }
ulvec2 max(ulvec2 const& p1, ulvec2 const& p2) { return glm::max(p1, p2); }
ulvec2 min(ulvec2 const& p1, u64 p2) { return glm::min(p1, p2); }
ulvec2 min(ulvec2 const& p1, ulvec2 const& p2) { return glm::min(p1, p2); }
ulvec2 abs(ulvec2 const& p1) { return glm::abs(p1); }
ulvec3 clamp(ulvec3 const& p1, u64 p2, u64 p3) { return glm::clamp(p1, p2, p3); }
ulvec3 clamp(ulvec3 const& p1, ulvec3 const& p2, ulvec3 const& p3) { return glm::clamp(p1, p2, p3); }
ulvec3 max(ulvec3 const& p1, u64 p2) { return glm::max(p1, p2); }
ulvec3 max(ulvec3 const& p1, ulvec3 const& p2) { return glm::max(p1, p2); }
ulvec3 min(ulvec3 const& p1, u64 p2) { return glm::min(p1, p2); }
ulvec3 min(ulvec3 const& p1, ulvec3 const& p2) { return glm::min(p1, p2); }
ulvec3 abs(ulvec3 const& p1) { return glm::abs(p1); }
ulvec4 clamp(ulvec4 const& p1, u64 p2, u64 p3) { return glm::clamp(p1, p2, p3); }
ulvec4 clamp(ulvec4 const& p1, ulvec4 const& p2, ulvec4 const& p3) { return glm::clamp(p1, p2, p3); }
ulvec4 max(ulvec4 const& p1, u64 p2) { return glm::max(p1, p2); }
ulvec4 max(ulvec4 const& p1, ulvec4 const& p2) { return glm::max(p1, p2); }
ulvec4 min(ulvec4 const& p1, u64 p2) { return glm::min(p1, p2); }
ulvec4 min(ulvec4 const& p1, ulvec4 const& p2) { return glm::min(p1, p2); }
ulvec4 abs(ulvec4 const& p1) { return glm::abs(p1); }

bvec2 equal(const dvec2& p1, const dvec2& p2) { return glm::equal(p1, p2); }
bvec2 greaterThan(const dvec2& p1, const dvec2& p2) { return glm::greaterThan(p1, p2); }
bvec2 greaterThanEqual(const dvec2& p1, const dvec2& p2) { return glm::greaterThanEqual(p1, p2); }
bvec2 lessThan(const dvec2& p1, const dvec2& p2) { return glm::lessThan(p1, p2); }
bvec2 lessThanEqual(const dvec2& p1, const dvec2& p2) { return glm::lessThanEqual(p1, p2); }
bvec3 equal(const dvec3& p1, const dvec3& p2) { return glm::equal(p1, p2); }
bvec3 greaterThan(const dvec3& p1, const dvec3& p2) { return glm::greaterThan(p1, p2); }
bvec3 greaterThanEqual(const dvec3& p1, const dvec3& p2) { return glm::greaterThanEqual(p1, p2); }
bvec3 lessThan(const dvec3& p1, const dvec3& p2) { return glm::lessThan(p1, p2); }
bvec3 lessThanEqual(const dvec3& p1, const dvec3& p2) { return glm::lessThanEqual(p1, p2); }
bvec4 equal(const dvec4& p1, const dvec4& p2) { return glm::equal(p1, p2); }
bvec4 greaterThan(const dvec4& p1, const dvec4& p2) { return glm::greaterThan(p1, p2); }
bvec4 greaterThanEqual(const dvec4& p1, const dvec4& p2) { return glm::greaterThanEqual(p1, p2); }
bvec4 lessThan(const dvec4& p1, const dvec4& p2) { return glm::lessThan(p1, p2); }
bvec4 lessThanEqual(const dvec4& p1, const dvec4& p2) { return glm::lessThanEqual(p1, p2); }
bvec2 equal(const vec2& p1, const vec2& p2) { return glm::equal(p1, p2); }
bvec2 greaterThan(const vec2& p1, const vec2& p2) { return glm::greaterThan(p1, p2); }
bvec2 greaterThanEqual(const vec2& p1, const vec2& p2) { return glm::greaterThanEqual(p1, p2); }
bvec2 lessThan(const vec2& p1, const vec2& p2) { return glm::lessThan(p1, p2); }
bvec2 lessThanEqual(const vec2& p1, const vec2& p2) { return glm::lessThanEqual(p1, p2); }
bvec3 equal(const vec3& p1, const vec3& p2) { return glm::equal(p1, p2); }
bvec3 greaterThan(const vec3& p1, const vec3& p2) { return glm::greaterThan(p1, p2); }
bvec3 greaterThanEqual(const vec3& p1, const vec3& p2) { return glm::greaterThanEqual(p1, p2); }
bvec3 lessThan(const vec3& p1, const vec3& p2) { return glm::lessThan(p1, p2); }
bvec3 lessThanEqual(const vec3& p1, const vec3& p2) { return glm::lessThanEqual(p1, p2); }
bvec4 equal(const vec4& p1, const vec4& p2) { return glm::equal(p1, p2); }
bvec4 greaterThan(const vec4& p1, const vec4& p2) { return glm::greaterThan(p1, p2); }
bvec4 greaterThanEqual(const vec4& p1, const vec4& p2) { return glm::greaterThanEqual(p1, p2); }
bvec4 lessThan(const vec4& p1, const vec4& p2) { return glm::lessThan(p1, p2); }
bvec4 lessThanEqual(const vec4& p1, const vec4& p2) { return glm::lessThanEqual(p1, p2); }
bvec2 equal(const ivec2& p1, const ivec2& p2) { return glm::equal(p1, p2); }
bvec2 greaterThan(const ivec2& p1, const ivec2& p2) { return glm::greaterThan(p1, p2); }
bvec2 greaterThanEqual(const ivec2& p1, const ivec2& p2) { return glm::greaterThanEqual(p1, p2); }
bvec2 lessThan(const ivec2& p1, const ivec2& p2) { return glm::lessThan(p1, p2); }
bvec2 lessThanEqual(const ivec2& p1, const ivec2& p2) { return glm::lessThanEqual(p1, p2); }
bvec3 equal(const ivec3& p1, const ivec3& p2) { return glm::equal(p1, p2); }
bvec3 greaterThan(const ivec3& p1, const ivec3& p2) { return glm::greaterThan(p1, p2); }
bvec3 greaterThanEqual(const ivec3& p1, const ivec3& p2) { return glm::greaterThanEqual(p1, p2); }
bvec3 lessThan(const ivec3& p1, const ivec3& p2) { return glm::lessThan(p1, p2); }
bvec3 lessThanEqual(const ivec3& p1, const ivec3& p2) { return glm::lessThanEqual(p1, p2); }
bvec4 equal(const ivec4& p1, const ivec4& p2) { return glm::equal(p1, p2); }
bvec4 greaterThan(const ivec4& p1, const ivec4& p2) { return glm::greaterThan(p1, p2); }
bvec4 greaterThanEqual(const ivec4& p1, const ivec4& p2) { return glm::greaterThanEqual(p1, p2); }
bvec4 lessThan(const ivec4& p1, const ivec4& p2) { return glm::lessThan(p1, p2); }
bvec4 lessThanEqual(const ivec4& p1, const ivec4& p2) { return glm::lessThanEqual(p1, p2); }
bvec2 equal(const lvec2& p1, const lvec2& p2) { return glm::equal(p1, p2); }
bvec2 greaterThan(const lvec2& p1, const lvec2& p2) { return glm::greaterThan(p1, p2); }
bvec2 greaterThanEqual(const lvec2& p1, const lvec2& p2) { return glm::greaterThanEqual(p1, p2); }
bvec2 lessThan(const lvec2& p1, const lvec2& p2) { return glm::lessThan(p1, p2); }
bvec2 lessThanEqual(const lvec2& p1, const lvec2& p2) { return glm::lessThanEqual(p1, p2); }
bvec3 equal(const lvec3& p1, const lvec3& p2) { return glm::equal(p1, p2); }
bvec3 greaterThan(const lvec3& p1, const lvec3& p2) { return glm::greaterThan(p1, p2); }
bvec3 greaterThanEqual(const lvec3& p1, const lvec3& p2) { return glm::greaterThanEqual(p1, p2); }
bvec3 lessThan(const lvec3& p1, const lvec3& p2) { return glm::lessThan(p1, p2); }
bvec3 lessThanEqual(const lvec3& p1, const lvec3& p2) { return glm::lessThanEqual(p1, p2); }
bvec4 equal(const lvec4& p1, const lvec4& p2) { return glm::equal(p1, p2); }
bvec4 greaterThan(const lvec4& p1, const lvec4& p2) { return glm::greaterThan(p1, p2); }
bvec4 greaterThanEqual(const lvec4& p1, const lvec4& p2) { return glm::greaterThanEqual(p1, p2); }
bvec4 lessThan(const lvec4& p1, const lvec4& p2) { return glm::lessThan(p1, p2); }
bvec4 lessThanEqual(const lvec4& p1, const lvec4& p2) { return glm::lessThanEqual(p1, p2); }
bvec4 equal(const u8vec4& p1, const u8vec4& p2) { return glm::equal(p1, p2); }
bvec4 greaterThan(const u8vec4& p1, const u8vec4& p2) { return glm::greaterThan(p1, p2); }
bvec4 greaterThanEqual(const u8vec4& p1, const u8vec4& p2) { return glm::greaterThanEqual(p1, p2); }
bvec4 lessThan(const u8vec4& p1, const u8vec4& p2) { return glm::lessThan(p1, p2); }
bvec4 lessThanEqual(const u8vec4& p1, const u8vec4& p2) { return glm::lessThanEqual(p1, p2); }
bvec2 equal(const uvec2& p1, const uvec2& p2) { return glm::equal(p1, p2); }
bvec2 greaterThan(const uvec2& p1, const uvec2& p2) { return glm::greaterThan(p1, p2); }
bvec2 greaterThanEqual(const uvec2& p1, const uvec2& p2) { return glm::greaterThanEqual(p1, p2); }
bvec2 lessThan(const uvec2& p1, const uvec2& p2) { return glm::lessThan(p1, p2); }
bvec2 lessThanEqual(const uvec2& p1, const uvec2& p2) { return glm::lessThanEqual(p1, p2); }
bvec3 equal(const uvec3& p1, const uvec3& p2) { return glm::equal(p1, p2); }
bvec3 greaterThan(const uvec3& p1, const uvec3& p2) { return glm::greaterThan(p1, p2); }
bvec3 greaterThanEqual(const uvec3& p1, const uvec3& p2) { return glm::greaterThanEqual(p1, p2); }
bvec3 lessThan(const uvec3& p1, const uvec3& p2) { return glm::lessThan(p1, p2); }
bvec3 lessThanEqual(const uvec3& p1, const uvec3& p2) { return glm::lessThanEqual(p1, p2); }
bvec4 equal(const uvec4& p1, const uvec4& p2) { return glm::equal(p1, p2); }
bvec4 greaterThan(const uvec4& p1, const uvec4& p2) { return glm::greaterThan(p1, p2); }
bvec4 greaterThanEqual(const uvec4& p1, const uvec4& p2) { return glm::greaterThanEqual(p1, p2); }
bvec4 lessThan(const uvec4& p1, const uvec4& p2) { return glm::lessThan(p1, p2); }
bvec4 lessThanEqual(const uvec4& p1, const uvec4& p2) { return glm::lessThanEqual(p1, p2); }
bvec2 equal(const ulvec2& p1, const ulvec2& p2) { return glm::equal(p1, p2); }
bvec2 greaterThan(const ulvec2& p1, const ulvec2& p2) { return glm::greaterThan(p1, p2); }
bvec2 greaterThanEqual(const ulvec2& p1, const ulvec2& p2) { return glm::greaterThanEqual(p1, p2); }
bvec2 lessThan(const ulvec2& p1, const ulvec2& p2) { return glm::lessThan(p1, p2); }
bvec2 lessThanEqual(const ulvec2& p1, const ulvec2& p2) { return glm::lessThanEqual(p1, p2); }
bvec3 equal(const ulvec3& p1, const ulvec3& p2) { return glm::equal(p1, p2); }
bvec3 greaterThan(const ulvec3& p1, const ulvec3& p2) { return glm::greaterThan(p1, p2); }
bvec3 greaterThanEqual(const ulvec3& p1, const ulvec3& p2) { return glm::greaterThanEqual(p1, p2); }
bvec3 lessThan(const ulvec3& p1, const ulvec3& p2) { return glm::lessThan(p1, p2); }
bvec3 lessThanEqual(const ulvec3& p1, const ulvec3& p2) { return glm::lessThanEqual(p1, p2); }
bvec4 equal(const ulvec4& p1, const ulvec4& p2) { return glm::equal(p1, p2); }
bvec4 greaterThan(const ulvec4& p1, const ulvec4& p2) { return glm::greaterThan(p1, p2); }
bvec4 greaterThanEqual(const ulvec4& p1, const ulvec4& p2) { return glm::greaterThanEqual(p1, p2); }
bvec4 lessThan(const ulvec4& p1, const ulvec4& p2) { return glm::lessThan(p1, p2); }
bvec4 lessThanEqual(const ulvec4& p1, const ulvec4& p2) { return glm::lessThanEqual(p1, p2); }

bool all(bvec1 const& p1) { return glm::all(p1); }
bool any(bvec1 const& p1) { return glm::any(p1); }
bool all(bvec2 const& p1) { return glm::all(p1); }
bool any(bvec2 const& p1) { return glm::any(p1); }
bool all(bvec3 const& p1) { return glm::all(p1); }
bool any(bvec3 const& p1) { return glm::any(p1); }
bool all(bvec4 const& p1) { return glm::all(p1); }
bool any(bvec4 const& p1) { return glm::any(p1); }

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