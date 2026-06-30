// Copyright (c) 2016, PG, All rights reserved.
#include "SliderRenderer.h"

#include "OsuConVars.h"
#include "Engine.h"
#include "GameRules.h"
#include "Osu.h"
#include "RenderTarget.h"
#include "ResourceManager.h"
#include "Environment.h"
#include "Shader.h"
#include "Skin.h"
#include "UI.h"
#include "VertexArrayObject.h"
#include "Logging.h"
#include "Graphics.h"

#include <limits>

namespace SliderRenderer {

namespace {  // static namespace

Shader *s_BLEND_SHADER{nullptr};
f32 s_UNIT_CIRCLE_VAO_DIAMETER{0.0f};

// base mesh
f32 s_MESH_CENTER_HEIGHT{0.5f};     // Camera::buildMatrixOrtho2D() uses -1 to 1 for zn/zf, so don't make this too high
i32 s_UNIT_CIRCLE_SUBDIVISIONS{0};  // see slider_body_unit_circle_subdivisions now
std::vector<f32> s_UNIT_CIRCLE;
// managed by RM (renderer-bound baking)
VertexArrayObject *s_UNIT_CIRCLE_VAO_BAKED{nullptr};

// unbaked, basic VAO containers
static CONSTINIT VertexArrayObject s_UNIT_CIRCLE_VAO{DrawPrimitive::TRIANGLE_FAN};
static CONSTINIT VertexArrayObject s_UNIT_CIRCLE_VAO_TRIANGLES{DrawPrimitive::TRIANGLES};

// tiny rendering optimization for RenderTarget
f32 s_fBoundingBoxMinX{(std::numeric_limits<f32>::max)()};
f32 s_fBoundingBoxMaxX{0.0f};
f32 s_fBoundingBoxMinY{(std::numeric_limits<f32>::max)()};
f32 s_fBoundingBoxMaxY{0.0f};

struct UniformCache {
    // convar-dependent settings (updated by convar callbacks)
    i32 style{-1};
    f32 bodyAlphaMultiplier{-1.0f};
    f32 bodyColorSaturation{-1.0f};
    f32 borderSizeMultiplier{-1.0f};
    f32 borderFeather{-1.0f};

    // uniforms that change often (colors)
    Color lastBorderColor{0};
    Color lastBodyColor{0};

    bool needsConfigUpdate{true};  // for convar-based uniforms
};

static CONSTINIT UniformCache s_uniformCache{};

// helper function to update color uniforms (after ->enable-ing the shader)
void updateColorUniforms(Color borderColor, Color bodyColor);
// check if convar-dependent uniforms need to be updated (after ->enable-ing the shader)
void updateConfigUniforms();

// forward decls
void drawDebugLegacy(std::span<const vec2> points, f32 hitcircleDiameter, Color undimmedColor, f32 colorRGBMultiplier,
                     f32 alpha, uSz drawFromIndex, uSz drawUpToIndex);
void drawDebugVAO(VertexArrayObject *vao, vec2 translation, f32 scale, f32 from, f32 to, Color undimmedColor,
                  f32 colorRGBMultiplier, f32 alpha);

void drawFillSliderBodyPeppy(std::span<const vec2> points, VertexArrayObject *circleMesh, f32 radius, uSz drawFromIndex,
                             uSz drawUpToIndex);
void checkUpdateVars(f32 hitcircleDiameter);

Color getRainbowColor(i32 rainbowTime, f32 initOffset) {
    const f64 frequency = .3f;
    const f64 time = engine->getTime() * 20.;

    const Channel red = (Channel)(std::sin(frequency * (time * initOffset) + 0 + rainbowTime) * 127.) + 128;
    const Channel green = (Channel)(std::sin(frequency * (time * initOffset) + 2 + rainbowTime) * 127.) + 128;
    const Channel blue = (Channel)(std::sin(frequency * (time * initOffset) + 4 + rainbowTime) * 127.) + 128;

    return rgb(red, green, blue);
}

forceinline Color getBodyColor(bool doRainbow, i32 rainbowTime, f32 colorRGBMultiplier, Color undimmedColor) {
    if(doRainbow) {
        return getRainbowColor(rainbowTime, 1.5f);
    } else {
        const Color undimmedBodyColor =
            osu->getSkin()->o_slider_track_overridden ? osu->getSkin()->c_slider_track_override : undimmedColor;

        return Colors::scale(undimmedBodyColor, colorRGBMultiplier);
    }
}

forceinline Color getBorderColor(bool doRainbow, i32 rainbowTime, f32 colorRGBMultiplier, Color undimmedColor) {
    if(doRainbow) {
        return getRainbowColor(rainbowTime, 1.f);
    } else {
        const Color undimmedBorderColor =
            cv::slider_border_tint_combo_color.getBool() ? undimmedColor : osu->getSkin()->c_slider_border;

        return Colors::scale(undimmedBorderColor, colorRGBMultiplier);
    }
}

forceinline void preDrawColorSetup(const Image *gradient, i32 sliderTime, f32 colorRGBMultiplier, Color undimmedColor) {
    if(gradient) {
        // this only affects the gradient image if used (meaning shaders
        // either don't work or are disabled on purpose)
        g->setColor(argb(1.0f, colorRGBMultiplier, colorRGBMultiplier, colorRGBMultiplier));
        gradient->bind();
    } else {
        const bool doRainbow = cv::slider_rainbow.getBool();

        const Color borderColor = getBorderColor(doRainbow, sliderTime, colorRGBMultiplier, undimmedColor);
        const Color bodyColor = getBodyColor(doRainbow, sliderTime, colorRGBMultiplier, undimmedColor);

        s_BLEND_SHADER->enable();
        updateConfigUniforms();
        updateColorUniforms(borderColor, bodyColor);
    }
}

}  // namespace

// invalidate config uniforms (convar callbacks)
void onUniformConfigChanged() { s_uniformCache.needsConfigUpdate = true; }

std::unique_ptr<VertexArrayObject> generateVAO(std::span<const vec2> points, f32 hitcircleDiameter, vec3 translation,
                                               bool skipOOBPoints) {
    resourceManager->requestNextLoadUnmanaged();
    std::unique_ptr<VertexArrayObject> vao{resourceManager->createVertexArrayObject()};

    checkUpdateVars(hitcircleDiameter);

    const auto screen = osu->getVirtScreenSize();
    const vec4 bounds{
        -hitcircleDiameter - GameRules::OSU_COORD_WIDTH * 2,            // x = minX
        screen.x + hitcircleDiameter + GameRules::OSU_COORD_WIDTH * 2,  // y = maxX
        -hitcircleDiameter - GameRules::OSU_COORD_HEIGHT * 2,           // z = minY
        screen.y + hitcircleDiameter + GameRules::OSU_COORD_HEIGHT * 2  // w = maxY
    };
    const auto isOOB = [bounds](vec2 point) -> bool {
        // fuck oob sliders
        return point.x < bounds.x || point.x > bounds.y || point.y < bounds.z || point.y > bounds.w;
    };

    const vec3 xOffset = vec3(hitcircleDiameter, 0, 0);
    const vec3 yOffset = vec3(0, hitcircleDiameter, 0);

    if(!cv::slider_debug_draw_square_vao.getBool()) {  // regular fast path
        const std::span<const vec3> triangleMeshVerts = s_UNIT_CIRCLE_VAO_TRIANGLES.getVertices();
        const std::span<const vec2> triangleMeshTCs = s_UNIT_CIRCLE_VAO_TRIANGLES.getTexcoords();
        std::vector<vec2> tempTexCoords{triangleMeshTCs.size() * points.size()};
        std::vector<vec3> tempMeshVerts{triangleMeshVerts.size() * points.size()};

        uSz tempMeshVertOffset = 0;
        uSz tempMeshTCOffset = 0;
        for(const auto &point : points) {
            if(skipOOBPoints && isOOB(point)) continue;

            for(const auto &meshVertex : triangleMeshVerts) {
                tempMeshVerts[tempMeshVertOffset++] = (meshVertex + vec3(point.x, point.y, 0) + translation);
            }
            std::memcpy(&tempTexCoords[tempMeshTCOffset], triangleMeshTCs.data(),
                        triangleMeshTCs.size() * sizeof(decltype(tempTexCoords)::value_type));
            tempMeshTCOffset += triangleMeshTCs.size();
        }

        // resize to post-OOB-clipped amount
        tempMeshVerts.resize(tempMeshVertOffset);
        tempMeshVerts.shrink_to_fit();
        tempTexCoords.resize(tempMeshTCOffset);
        tempTexCoords.shrink_to_fit();
        vao->setVertices(std::move(tempMeshVerts));
        vao->setTexcoords(std::move(tempTexCoords));
    } else {  // debug
        for(const auto &point : points) {
            if(skipOOBPoints && isOOB(point)) continue;

            const vec3 topLeft = vec3(point.x, point.y, 0) - xOffset / 2.0f - yOffset / 2.0f + translation;
            const vec3 topRight = topLeft + xOffset;
            const vec3 bottomLeft = topLeft + yOffset;
            const vec3 bottomRight = bottomLeft + xOffset;

            vao->addVertices(std::array<vec3, 6>{topLeft,      //
                                                 bottomLeft,   //
                                                 bottomRight,  //
                                                 topLeft,      //
                                                 bottomRight,  //
                                                 topRight});
            vao->addTexcoords(std::array<vec2, 6>{vec2{0, 0},  //
                                                  vec2{0, 1},  //
                                                  vec2{1, 1},  //
                                                  vec2{0, 0},  //
                                                  vec2{1, 1},  //
                                                  vec2{1, 0}});
        }
    }

    if(vao->getNumVertices() > 0) {
        resourceManager->loadResource(vao.get());
    } else {
        debugLog("ERROR: Zero triangles!");
    }

    return vao;
}

void draw(const DrawLegacyParams &p) {
    if(cv::slider_alpha_multiplier.getFloat() <= 0.0f || p.alpha <= 0.0f) return;

    checkUpdateVars(p.hitcircleDiameter);

    const uSz drawFromIndex = std::clamp<uSz>((uSz)std::round((f64)p.points.size() * p.from), 0UZ, p.points.size());
    const uSz drawUpToIndex = std::clamp<uSz>((uSz)std::round((f64)p.points.size() * p.to), 0UZ, p.points.size());

    // debug sliders
    if(cv::slider_debug_draw.getBool()) {
        drawDebugLegacy(p.points, p.hitcircleDiameter, p.undimmedColor, p.colorRGBMultiplier, p.alpha, drawFromIndex,
                        drawUpToIndex);
        return;  // nothing more to draw here
    }

    // reset
    s_fBoundingBoxMinX = (std::numeric_limits<f32>::max)();
    s_fBoundingBoxMaxX = 0.0f;
    s_fBoundingBoxMinY = (std::numeric_limits<f32>::max)();
    s_fBoundingBoxMaxY = 0.0f;

    auto *sliderFrameBuffer = osu->getSliderFrameBuffer();

    // draw entire slider into framebuffer
    g->setDepthBuffer(true);
    g->setBlending(false);
    {
        sliderFrameBuffer->enable();
        {
            const Image *gradient = nullptr;
            const bool useGradientImage = cv::slider_use_gradient_image.getBool();
            if(useGradientImage) {
                gradient = osu->getSkin()->i_slider_gradient;
            }
            // enables shader if gradient is not being used
            preDrawColorSetup(gradient, p.sliderTimeForRainbow, p.colorRGBMultiplier, p.undimmedColor);

            // draw curve mesh
            drawFillSliderBodyPeppy(
                p.points, (cv::slider_legacy_use_baked_vao.getBool() ? s_UNIT_CIRCLE_VAO_BAKED : &s_UNIT_CIRCLE_VAO),
                p.hitcircleDiameter / 2.0f, drawFromIndex, drawUpToIndex);

            if(p.alwaysPoints.size() > 0)
                drawFillSliderBodyPeppy(p.alwaysPoints, s_UNIT_CIRCLE_VAO_BAKED, p.hitcircleDiameter / 2.0f, 0,
                                        p.alwaysPoints.size());

            if(!useGradientImage) {
                s_BLEND_SHADER->disable();
            } else {
                gradient->unbind();
            }
        }
        sliderFrameBuffer->disable();
    }
    g->setBlending(true);
    g->setDepthBuffer(false);

    // now draw the slider to the screen (with alpha blending enabled again)
    const f32 pixelFudge = 2.0f;
    s_fBoundingBoxMinX -= pixelFudge;
    s_fBoundingBoxMaxX += pixelFudge;
    s_fBoundingBoxMinY -= pixelFudge;
    s_fBoundingBoxMaxY += pixelFudge;

    sliderFrameBuffer->setColor(argb(p.alpha * cv::slider_alpha_multiplier.getFloat(), 1.0f, 1.0f, 1.0f));
    sliderFrameBuffer->drawRect((i32)s_fBoundingBoxMinX, (i32)s_fBoundingBoxMinY,
                                (i32)(s_fBoundingBoxMaxX - s_fBoundingBoxMinX),
                                (i32)(s_fBoundingBoxMaxY - s_fBoundingBoxMinY));
}

void draw(const DrawVAOParams &p) {
    if((cv::slider_alpha_multiplier.getFloat() <= 0.0f && p.doDrawSliderFrameBufferToScreen) ||
       (p.alpha <= 0.0f && p.doDrawSliderFrameBufferToScreen) || p.vao == nullptr)
        return;

    checkUpdateVars(p.hitcircleDiameter);

    if(cv::slider_debug_draw_square_vao.getBool()) {
        drawDebugVAO(p.vao, p.translation, p.scale, p.from, p.to, p.undimmedColor, p.colorRGBMultiplier, p.alpha);
        return;
    }

    auto *sliderFrameBuffer = osu->getSliderFrameBuffer();

    // reset
    s_fBoundingBoxMinX = (std::numeric_limits<f32>::max)();
    s_fBoundingBoxMaxX = 0.0f;
    s_fBoundingBoxMinY = (std::numeric_limits<f32>::max)();
    s_fBoundingBoxMaxY = 0.0f;

    // draw entire slider into framebuffer
    g->setDepthBuffer(true);
    g->setBlending(false);
    {
        if(p.doEnableRenderTarget) sliderFrameBuffer->enable();

        // render
        {
            const Image *gradient = nullptr;
            const bool useGradientImage = cv::slider_use_gradient_image.getBool();
            if(useGradientImage) {
                gradient = osu->getSkin()->i_slider_gradient;
            }
            // enables shader if gradient is not being used
            preDrawColorSetup(gradient, p.sliderTimeForRainbow, p.colorRGBMultiplier, p.undimmedColor);

            // when smoothsnake's alwaysPoint cone sits between the VAO's last cone and the next one,
            // over-extend the VAO by a cone so its last cone hides the alwaysPoint and the cap is
            // formed by a single smooth arc instead of two arcs kinking at the cone intersection
            const i32 vertsPerCone = (i32)s_UNIT_CIRCLE_VAO_TRIANGLES.getVertices().size();
            f32 effectiveTo = p.to;
            if(p.alwaysPoints.size() > 0 && p.to < 1.0f && vertsPerCone > 0) {
                const i32 totalCones = (i32)p.vao->getNumVertices() / vertsPerCone;
                if(totalCones > 0) effectiveTo = std::min(1.0f, p.to + (1.0f / (f32)totalCones));
            }

            // draw curve mesh
            p.vao->setDrawPercent(p.from, effectiveTo, vertsPerCone);
            g->pushTransform();
            {
                g->scale(p.scale, p.scale);
                g->translate(p.translation.x, p.translation.y);
                /// g->scale(scaleToApplyAfterTranslationX, scaleToApplyAfterTranslationY); // aspire slider
                /// distortions

                g->drawVAO(p.vao);
            }
            g->popTransform();

            if(p.alwaysPoints.size() > 0)
                drawFillSliderBodyPeppy(p.alwaysPoints, s_UNIT_CIRCLE_VAO_BAKED, p.hitcircleDiameter / 2.0f, 0,
                                        p.alwaysPoints.size());

            if(!useGradientImage) {
                s_BLEND_SHADER->disable();
            } else {
                gradient->unbind();
            }
        }

        if(p.doDisableRenderTarget) sliderFrameBuffer->disable();
    }
    g->setBlending(true);
    g->setDepthBuffer(false);

    // optional bounds performance optimization to reduce rt blending overdraw
    const vec2 screen = osu->getVirtScreenSize();
    if(p.bounds != vec4{}) {
        const f32 pixelFudge = 2.0f;
        s_fBoundingBoxMinX = std::max(0.0f, p.bounds.x - p.hitcircleDiameter / 2.0f - pixelFudge);
        s_fBoundingBoxMaxX = std::min(screen.x, p.bounds.z + p.hitcircleDiameter / 2.0f + pixelFudge);
        s_fBoundingBoxMinY = std::max(0.0f, p.bounds.y - p.hitcircleDiameter / 2.0f - pixelFudge);
        s_fBoundingBoxMaxY = std::min(screen.y, p.bounds.w + p.hitcircleDiameter / 2.0f + pixelFudge);
    } else {
        s_fBoundingBoxMinX = 0.0f;
        s_fBoundingBoxMaxX = screen.x;
        s_fBoundingBoxMinY = 0.0f;
        s_fBoundingBoxMaxY = screen.y;
    }

    if(p.doDrawSliderFrameBufferToScreen) {
        sliderFrameBuffer->setColor(argb(p.alpha * cv::slider_alpha_multiplier.getFloat(), 1.0f, 1.0f, 1.0f));
        sliderFrameBuffer->drawRect((i32)s_fBoundingBoxMinX, (i32)s_fBoundingBoxMinY,
                                    (i32)(s_fBoundingBoxMaxX - s_fBoundingBoxMinX),
                                    (i32)(s_fBoundingBoxMaxY - s_fBoundingBoxMinY));
    }
}

namespace {  // static

void drawFillSliderBodyPeppy(std::span<const vec2> points, VertexArrayObject *circleMesh, f32 radius, uSz drawFromIndex,
                             uSz drawUpToIndex) {
    const auto screen = osu->getVirtScreenSize();

    g->pushTransform();
    {
        // now, translate and draw the master vao for every curve point
        f32 startX = 0.0f;
        f32 startY = 0.0f;
        for(uSz i = drawFromIndex; i < drawUpToIndex; ++i) {
            const f32 x = points[i].x;
            const f32 y = points[i].y;

            // fuck oob sliders
            if(x < -radius * 2 || x > screen.x + radius * 2 || y < -radius * 2 || y > screen.y + radius * 2) continue;

            g->translate(x - startX, y - startY, 0);
            g->drawVAO(circleMesh);

            startX = x;
            startY = y;

            if(x - radius < s_fBoundingBoxMinX) s_fBoundingBoxMinX = x - radius;
            if(x + radius > s_fBoundingBoxMaxX) s_fBoundingBoxMaxX = x + radius;
            if(y - radius < s_fBoundingBoxMinY) s_fBoundingBoxMinY = y - radius;
            if(y + radius > s_fBoundingBoxMaxY) s_fBoundingBoxMaxY = y + radius;
        }
    }
    g->popTransform();
}

void checkUpdateVars(f32 hitcircleDiameter) {
    // static globals

    if(!env->usingGL()) {
        // NOTE: compensate for zn/zf Camera::buildMatrixOrtho2DDXLH() differences compared to OpenGL
        if(s_MESH_CENTER_HEIGHT > 0.0f) s_MESH_CENTER_HEIGHT = -s_MESH_CENTER_HEIGHT;
    }

    // build shaders and circle mesh
    if(s_BLEND_SHADER == nullptr)  // only do this once
        s_BLEND_SHADER = resourceManager->createShaderAuto("slider");

    const i32 subdivisions = cv::slider_body_unit_circle_subdivisions.getInt();
    if(subdivisions != s_UNIT_CIRCLE_SUBDIVISIONS) {
        s_UNIT_CIRCLE_SUBDIVISIONS = subdivisions;

        // build unit cone
        {
            s_UNIT_CIRCLE.clear();

            // tip of the cone
            // texture coordinates
            s_UNIT_CIRCLE.push_back(1.0f);
            s_UNIT_CIRCLE.push_back(0.0f);

            // position
            s_UNIT_CIRCLE.push_back(0.0f);
            s_UNIT_CIRCLE.push_back(0.0f);
            s_UNIT_CIRCLE.push_back(s_MESH_CENTER_HEIGHT);

            for(i32 j = 0; j < subdivisions; ++j) {
                const f32 phase = (f32)j * PI_F * 2.0f / (f32)subdivisions;

                // texture coordinates
                s_UNIT_CIRCLE.push_back(0.0f);
                s_UNIT_CIRCLE.push_back(0.0f);

                // positon
                s_UNIT_CIRCLE.push_back((f32)std::sin(phase));
                s_UNIT_CIRCLE.push_back((f32)std::cos(phase));
                s_UNIT_CIRCLE.push_back(0.0f);
            }

            // texture coordinates
            s_UNIT_CIRCLE.push_back(0.0f);
            s_UNIT_CIRCLE.push_back(0.0f);

            // positon
            s_UNIT_CIRCLE.push_back((f32)std::sin(0.0f));
            s_UNIT_CIRCLE.push_back((f32)std::cos(0.0f));
            s_UNIT_CIRCLE.push_back(0.0f);
        }
    }

    // build vaos
    if(s_UNIT_CIRCLE_VAO_BAKED == nullptr)
        s_UNIT_CIRCLE_VAO_BAKED = resourceManager->createVertexArrayObject(DrawPrimitive::TRIANGLE_FAN);

    // (re-)generate master circle mesh (centered) if the size changed
    // dynamic mods like minimize or wobble have to use the legacy renderer anyway, since the slider shape may change
    // every frame
    if(hitcircleDiameter != s_UNIT_CIRCLE_VAO_DIAMETER) {
        const f32 radius = hitcircleDiameter / 2.0f;

        s_UNIT_CIRCLE_VAO_BAKED->release();

        // triangle fan
        s_UNIT_CIRCLE_VAO_DIAMETER = hitcircleDiameter;
        s_UNIT_CIRCLE_VAO.clear();
        for(i32 i = 0; i < s_UNIT_CIRCLE.size() / 5; i++) {
            vec3 vertexPos = vec3((radius * s_UNIT_CIRCLE[i * 5 + 2]), (radius * s_UNIT_CIRCLE[i * 5 + 3]),
                                  s_UNIT_CIRCLE[i * 5 + 4]);
            vec2 vertexTexcoord = vec2(s_UNIT_CIRCLE[i * 5 + 0], s_UNIT_CIRCLE[i * 5 + 1]);

            s_UNIT_CIRCLE_VAO.addVertex(vertexPos);
            s_UNIT_CIRCLE_VAO.addTexcoord(vertexTexcoord);

            s_UNIT_CIRCLE_VAO_BAKED->addVertex(vertexPos);
            s_UNIT_CIRCLE_VAO_BAKED->addTexcoord(vertexTexcoord);
        }

        resourceManager->loadResource(s_UNIT_CIRCLE_VAO_BAKED);

        // pure triangles (needed for VertexArrayObject, because we can't merge multiple triangle fan meshes into one
        // VertexArrayObject)
        s_UNIT_CIRCLE_VAO_TRIANGLES.clear();
        vec3 startVertex =
            vec3((radius * s_UNIT_CIRCLE[0 * 5 + 2]), (radius * s_UNIT_CIRCLE[0 * 5 + 3]), s_UNIT_CIRCLE[0 * 5 + 4]);
        vec2 startUV = vec2(s_UNIT_CIRCLE[0 * 5 + 0], s_UNIT_CIRCLE[0 * 5 + 1]);
        for(i32 i = 1; i < s_UNIT_CIRCLE.size() / 5 - 1; i++) {
            // center
            s_UNIT_CIRCLE_VAO_TRIANGLES.addVertex(startVertex);
            s_UNIT_CIRCLE_VAO_TRIANGLES.addTexcoord(startUV);

            // pizza slice edge 1
            s_UNIT_CIRCLE_VAO_TRIANGLES.addVertex(vec3((radius * s_UNIT_CIRCLE[i * 5 + 2]),
                                                       (radius * s_UNIT_CIRCLE[i * 5 + 3]), s_UNIT_CIRCLE[i * 5 + 4]));
            s_UNIT_CIRCLE_VAO_TRIANGLES.addTexcoord(vec2(s_UNIT_CIRCLE[i * 5 + 0], s_UNIT_CIRCLE[i * 5 + 1]));

            // pizza slice edge 2
            s_UNIT_CIRCLE_VAO_TRIANGLES.addVertex(vec3((radius * s_UNIT_CIRCLE[(i + 1) * 5 + 2]),
                                                       (radius * s_UNIT_CIRCLE[(i + 1) * 5 + 3]),
                                                       s_UNIT_CIRCLE[(i + 1) * 5 + 4]));
            s_UNIT_CIRCLE_VAO_TRIANGLES.addTexcoord(
                vec2(s_UNIT_CIRCLE[(i + 1) * 5 + 0], s_UNIT_CIRCLE[(i + 1) * 5 + 1]));
        }
    }
}

// helper function to update color uniforms
void updateColorUniforms(Color borderColor, Color bodyColor) {
    if(!s_BLEND_SHADER) return;

    if(s_uniformCache.lastBorderColor != borderColor) {
        s_BLEND_SHADER->setUniform3f("colBorder", borderColor.Rf(), borderColor.Gf(), borderColor.Bf());
        s_uniformCache.lastBorderColor = borderColor;
    }

    if(s_uniformCache.lastBodyColor != bodyColor) {
        s_BLEND_SHADER->setUniform3f("colBody", bodyColor.Rf(), bodyColor.Gf(), bodyColor.Bf());
        s_uniformCache.lastBodyColor = bodyColor;
    }
}

void updateConfigUniforms() {
    if(!s_BLEND_SHADER || !s_uniformCache.needsConfigUpdate) return;

    const i32 newStyle = cv::slider_osu_next_style.getBool() ? 1 : 0;
    const f32 newBodyAlpha = cv::slider_body_alpha_multiplier.getFloat();
    const f32 newBodySat = cv::slider_body_color_saturation.getFloat();
    const f32 newBorderSize = cv::slider_border_size_multiplier.getFloat();
    const f32 newBorderFeather = cv::slider_border_feather.getFloat();

    if(s_uniformCache.style != newStyle) {
        s_BLEND_SHADER->setUniform1i("style", newStyle);
        s_uniformCache.style = newStyle;
    }

    if(s_uniformCache.bodyAlphaMultiplier != newBodyAlpha) {
        s_BLEND_SHADER->setUniform1f("bodyAlphaMultiplier", newBodyAlpha);
        s_uniformCache.bodyAlphaMultiplier = newBodyAlpha;
    }

    if(s_uniformCache.bodyColorSaturation != newBodySat) {
        s_BLEND_SHADER->setUniform1f("bodyColorSaturation", newBodySat);
        s_uniformCache.bodyColorSaturation = newBodySat;
    }

    if(s_uniformCache.borderSizeMultiplier != newBorderSize) {
        s_BLEND_SHADER->setUniform1f("borderSizeMultiplier", newBorderSize);
        s_uniformCache.borderSizeMultiplier = newBorderSize;
    }

    if(s_uniformCache.borderFeather != newBorderFeather) {
        s_BLEND_SHADER->setUniform1f("borderFeather", newBorderFeather);
        s_uniformCache.borderFeather = newBorderFeather;
    }

    s_uniformCache.needsConfigUpdate = false;
}

static CONSTINIT VertexArrayObject quadDebugVAO{DrawPrimitive::QUADS};

void drawDebugLegacy(std::span<const vec2> points, f32 hitcircleDiameter, Color undimmedColor, f32 colorRGBMultiplier,
                     f32 alpha, uSz drawFromIndex, uSz drawUpToIndex) {
    const f32 circleImageScale = hitcircleDiameter / (f32)osu->getSkin()->i_hitcircle.getWidth();
    const f32 circleImageScaleInv = (1.0f / circleImageScale);

    const auto width = (f32)osu->getSkin()->i_hitcircle.getWidth();
    const auto height = (f32)osu->getSkin()->i_hitcircle.getHeight();

    const f32 x = (-width / 2.0f);
    const f32 y = (-height / 2.0f);
    const f32 z = -1.0f;

    g->pushTransform();
    {
        g->scale(circleImageScale, circleImageScale);

        const Color dimmedColor = Colors::scale(undimmedColor, colorRGBMultiplier);

        g->setColor(Color(dimmedColor).setA(alpha * cv::slider_alpha_multiplier.getFloat()));

        osu->getSkin()->i_hitcircle.bind();
        {
            for(uSz i = drawFromIndex; i < drawUpToIndex; i++) {
                const vec2 point = points[i] * circleImageScaleInv;

                quadDebugVAO.clear();
                {
                    quadDebugVAO.addTexcoord(0, 0);
                    quadDebugVAO.addVertex(point.x + x, point.y + y, z);

                    quadDebugVAO.addTexcoord(0, 1);
                    quadDebugVAO.addVertex(point.x + x, point.y + y + height, z);

                    quadDebugVAO.addTexcoord(1, 1);
                    quadDebugVAO.addVertex(point.x + x + width, point.y + y + height, z);

                    quadDebugVAO.addTexcoord(1, 0);
                    quadDebugVAO.addVertex(point.x + x + width, point.y + y, z);
                }
                g->drawVAO(&quadDebugVAO);
            }
        }
        osu->getSkin()->i_hitcircle.unbind();
    }
    g->popTransform();
    return;
}

void drawDebugVAO(VertexArrayObject *vao, vec2 translation, f32 scale, f32 from, f32 to, Color undimmedColor,
                  f32 colorRGBMultiplier, f32 alpha) {
    const Color dimmedColor = Colors::scale(undimmedColor, colorRGBMultiplier);

    g->setColor(Color(dimmedColor).setA(alpha * cv::slider_alpha_multiplier.getFloat()));

    osu->getSkin()->i_hitcircle.bind();

    vao->setDrawPercent(from, to, 6);  // HACKHACK: hardcoded magic number
    {
        g->pushTransform();
        {
            g->scale(scale, scale);
            g->translate(translation.x, translation.y);

            g->drawVAO(vao);
        }
        g->popTransform();
    }

    return;
}

}  // namespace

}  // namespace SliderRenderer
