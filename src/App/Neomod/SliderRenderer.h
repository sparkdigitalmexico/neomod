#pragma once
// Copyright (c) 2016, PG, All rights reserved.

#include "Vectors_fwd.h"
#include "Color.h"

#include <memory>
#include <span>

class Shader;
class VertexArrayObject;

namespace SliderRenderer {
std::unique_ptr<VertexArrayObject> generateVAO(std::span<const vec2> points, f32 hitcircleDiameter,
                                               vec3 translation, bool skipOOBPoints = true);

void draw(std::span<const vec2> points, std::span<const vec2> alwaysPoints, f32 hitcircleDiameter,
          f32 from = 0.0f, f32 to = 1.0f, Color undimmedColor = 0xffffffff, f32 colorRGBMultiplier = 1.0f,
          f32 alpha = 1.0f, i32 sliderTimeForRainbow = 0);
void draw(VertexArrayObject *vao, vec4 bounds, std::span<const vec2> alwaysPoints, vec2 translation, f32 scale,
          f32 hitcircleDiameter, f32 from = 0.0f, f32 to = 1.0f, Color undimmedColor = 0xffffffff,
          f32 colorRGBMultiplier = 1.0f, f32 alpha = 1.0f, i32 sliderTimeForRainbow = 0,
          bool doEnableRenderTarget = true, bool doDisableRenderTarget = true,
          bool doDrawSliderFrameBufferToScreen = true);
// for convar callbacks
void onUniformConfigChanged();
};  // namespace SliderRenderer
