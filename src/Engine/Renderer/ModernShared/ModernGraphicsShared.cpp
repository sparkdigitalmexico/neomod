// Copyright (c) 2026, WH, All rights reserved.
#include "config.h"

#if (defined(MCENGINE_FEATURE_GLES32) || defined(MCENGINE_FEATURE_DIRECTX11) || defined(MCENGINE_FEATURE_SDLGPU))

// TODO: share more code

#include "ModernGraphicsShared.h"
#include "VertexArrayObject.h"

#include <cmath>

void ModernGraphicsShared::addArcVertices(VertexArrayObject &vao, float cx, float cy, float radius, float startAngle,
                                          float endAngle) {
    constexpr float step = 0.05f;
    const int numSteps = (int)((endAngle - startAngle) / step);
    for(int i = 0; i <= numSteps; i++) {
        const float a = startAngle + (float)i * step;
        vao.addVertex(radius * std::cos(a) + cx, radius * std::sin(a) + cy);
    }
    vao.addVertex(radius * std::cos(endAngle) + cx, radius * std::sin(endAngle) + cy);
}

namespace {
// emit outer/inner vertex pairs for a triangle strip outline
inline void addArcStripVertices(VertexArrayObject &vao, float cx, float cy, float rInner, float rOuter,
                                float startAngle, float endAngle) {
    const int numSteps = std::max(1, (int)((endAngle - startAngle) / 0.05f));
    const float step = (endAngle - startAngle) / (float)numSteps;

    float a, c, s;  // NOLINT
    for(int i = 0; i < numSteps; i++) {
        a = startAngle + (float)i * step;
        c = std::cos(a);
        s = std::sin(a);
        vao.addVertex(rOuter * c + cx, rOuter * s + cy);
        vao.addVertex(rInner * c + cx, rInner * s + cy);
    }
    // i == numSteps
    a = endAngle;
    c = std::cos(a);
    s = std::sin(a);
    vao.addVertex(rOuter * c + cx, rOuter * s + cy);
    vao.addVertex(rInner * c + cx, rInner * s + cy);
}
}  // namespace

// 2d primitive drawing
void ModernGraphicsShared::drawPixels(int /*x*/, int /*y*/, int /*width*/, int /*height*/, DrawPixelsType /*type*/,
                                      const void * /*pixels*/) {
    // TODO: implement
}

void ModernGraphicsShared::drawPixel(int x, int y) {
    // TODO: this isn't really good...?
    drawLinef((float)x, (float)y, (float)x + 1.f, (float)y);
}

void ModernGraphicsShared::drawLinef(float x1, float y1, float x2, float y2) {
    this->updateTransform();
    this->setTexturing(false);

    static VertexArrayObject vao(DrawPrimitive::LINES);
    {
        vao.clear();
        vao.addVertex(x1, y1);
        vao.addVertex(x2, y2);
    }
    this->drawVAO(&vao);
}

void ModernGraphicsShared::drawRectf(const RectOptions &opts) {
    this->updateTransform();
    this->setTexturing(false);

    if(opts.cornerRadius > 0.f) {
        // draw as a filled triangle strip to avoid line rasterization differences
        // NOTE: GL_LINE_LOOP looks a lot cleaner than what seems to be possible with a LINE_STRIP in DX11/SDL_gpu,
        // so GLES3.2 overrides this case
        const float r = opts.cornerRadius;
        const float x2 = opts.x + opts.width, y2 = opts.y + opts.height;
        const float hw = opts.lineThickness / 2.f;
        const float rInner = r - hw, rOuter = r + hw;
        static VertexArrayObject vao(DrawPrimitive::TRIANGLE_STRIP);
        {
            vao.clear();
            addArcStripVertices(vao, opts.x + r, opts.y + r, rInner, rOuter, PI_F, 1.5f * PI_F);
            addArcStripVertices(vao, x2 - r, opts.y + r, rInner, rOuter, 1.5f * PI_F, 2.f * PI_F);
            addArcStripVertices(vao, x2 - r, y2 - r, rInner, rOuter, 0.f, 0.5f * PI_F);
            addArcStripVertices(vao, opts.x + r, y2 - r, rInner, rOuter, 0.5f * PI_F, PI_F);
            // close back to first vertex pair
            vao.addVertex(opts.x - hw, opts.y + r);
            vao.addVertex(opts.x + hw, opts.y + r);
        }
        this->drawVAO(&vao);
        return;
    }

    if(opts.lineThickness > 1.0f) {
        const float halfThickness = opts.lineThickness * 0.5f;

        if(opts.withColor) {
            this->setColor(opts.top);
            this->fillRectf(opts.x - halfThickness, opts.y - halfThickness, opts.width + opts.lineThickness,
                            opts.lineThickness);
            this->setColor(opts.bottom);
            this->fillRectf(opts.x - halfThickness, opts.y + opts.height - halfThickness,
                            opts.width + opts.lineThickness, opts.lineThickness);
            this->setColor(opts.left);
            this->fillRectf(opts.x - halfThickness, opts.y + halfThickness, opts.lineThickness,
                            opts.height - opts.lineThickness);
            this->setColor(opts.right);
            this->fillRectf(opts.x + opts.width - halfThickness, opts.y + halfThickness, opts.lineThickness,
                            opts.height - opts.lineThickness);
        } else {
            this->fillRectf(opts.x - halfThickness, opts.y - halfThickness, opts.width + opts.lineThickness,
                            opts.lineThickness);
            this->fillRectf(opts.x - halfThickness, opts.y + opts.height - halfThickness,
                            opts.width + opts.lineThickness, opts.lineThickness);
            this->fillRectf(opts.x - halfThickness, opts.y + halfThickness, opts.lineThickness,
                            opts.height - opts.lineThickness);
            this->fillRectf(opts.x + opts.width - halfThickness, opts.y + halfThickness, opts.lineThickness,
                            opts.height - opts.lineThickness);
        }
    } else {
        if(opts.withColor) {
            this->setColor(opts.top);
            this->drawLinef(opts.x, opts.y, opts.x + opts.width, opts.y);
            this->setColor(opts.left);
            this->drawLinef(opts.x, opts.y, opts.x, opts.y + opts.height);
            this->setColor(opts.bottom);
            this->drawLinef(opts.x, opts.y + opts.height, opts.x + opts.width, opts.y + opts.height + 0.5f);
            this->setColor(opts.right);
            this->drawLinef(opts.x + opts.width, opts.y, opts.x + opts.width, opts.y + opts.height + 0.5f);
        } else {
            this->drawLinef(opts.x, opts.y, opts.x + opts.width, opts.y);
            this->drawLinef(opts.x, opts.y, opts.x, opts.y + opts.height);
            this->drawLinef(opts.x, opts.y + opts.height, opts.x + opts.width, opts.y + opts.height + 0.5f);
            this->drawLinef(opts.x + opts.width, opts.y, opts.x + opts.width, opts.y + opts.height + 0.5f);
        }
    }
}

void ModernGraphicsShared::fillRectf(const FillRectOptions &opts) {
    this->updateTransform();
    this->setTexturing(false);

    if(opts.cornerRadius > 0.f) {
        const float r = opts.cornerRadius;
        const float x2 = opts.x + opts.width, y2 = opts.y + opts.height;
        static VertexArrayObject vao(DrawPrimitive::TRIANGLE_FAN);
        {
            vao.clear();
            vao.addVertex(opts.x + opts.width * 0.5f, opts.y + opts.height * 0.5f);  // center (fan hub)

            addArcVertices(vao, opts.x + r, opts.y + r, r, PI_F, 1.5f * PI_F);
            addArcVertices(vao, x2 - r, opts.y + r, r, 1.5f * PI_F, 2.f * PI_F);
            addArcVertices(vao, x2 - r, y2 - r, r, 0.f, 0.5f * PI_F);
            addArcVertices(vao, opts.x + r, y2 - r, r, 0.5f * PI_F, PI_F);
            // close the fan back to the first perimeter vertex
            vao.addVertex(opts.x, opts.y + r);
        }
        this->drawVAO(&vao);
    } else {
        static VertexArrayObject vao(DrawPrimitive::TRIANGLE_STRIP);
        {
            vao.clear();
            vao.addVertex(opts.x, opts.y);
            vao.addVertex(opts.x, opts.y + opts.height);
            vao.addVertex(opts.x + opts.width, opts.y);
            vao.addVertex(opts.x + opts.width, opts.y + opts.height);
        }
        this->drawVAO(&vao);
    }
}

void ModernGraphicsShared::drawArcf(float cx, float cy, float radius, float startAngle, float endAngle) {
    this->updateTransform();
    this->setTexturing(false);

    static VertexArrayObject vao(DrawPrimitive::LINE_STRIP);
    {
        vao.clear();
        addArcVertices(vao, cx, cy, radius, startAngle, endAngle);
    }
    this->drawVAO(&vao);
}

void ModernGraphicsShared::fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor,
                                        Color bottomLeftColor, Color bottomRightColor) {
    this->updateTransform();
    this->setTexturing(false);  // disable texturing

    static VertexArrayObject vao(DrawPrimitive::TRIANGLE_STRIP);
    {
        vao.clear();
        vao.addVertex((float)x, (float)y);
        vao.addColor(topLeftColor);
        vao.addVertex((float)x, (float)y + (float)height);
        vao.addColor(bottomLeftColor);
        vao.addVertex((float)x + (float)width, (float)y);
        vao.addColor(topRightColor);
        vao.addVertex((float)x + (float)width, (float)y + (float)height);
        vao.addColor(bottomRightColor);
    }
    this->drawVAO(&vao);
}

void ModernGraphicsShared::drawQuad(int x, int y, int width, int height) {
    this->updateTransform();
    this->setTexturing(true);  // enable texturing

    static VertexArrayObject vao(DrawPrimitive::TRIANGLE_STRIP);
    {
        vao.clear();
        vao.addVertex((float)x, (float)y);
        vao.addTexcoord(0, 0);
        vao.addVertex((float)x, (float)y + (float)height);
        vao.addTexcoord(0, 1);
        vao.addVertex((float)x + (float)width, (float)y);
        vao.addTexcoord(1, 0);
        vao.addVertex((float)x + (float)width, (float)y + (float)height);
        vao.addTexcoord(1, 1);
    }
    this->drawVAO(&vao);
}

void ModernGraphicsShared::drawQuad(vec2 topLeft, vec2 topRight, vec2 bottomRight, vec2 bottomLeft, Color topLeftColor,
                                    Color topRightColor, Color bottomRightColor, Color bottomLeftColor) {
    this->updateTransform();
    this->setTexturing(false);  // disable texturing

    static VertexArrayObject vao(DrawPrimitive::TRIANGLE_STRIP);
    {
        vao.clear();
        vao.addVertex(topLeft.x, topLeft.y);
        vao.addColor(topLeftColor);
        vao.addVertex(bottomLeft.x, bottomLeft.y);
        vao.addColor(bottomLeftColor);
        vao.addVertex(topRight.x, topRight.y);
        vao.addColor(topRightColor);
        vao.addVertex(bottomRight.x, bottomRight.y);
        vao.addColor(bottomRightColor);
    }
    this->drawVAO(&vao);
}

#endif