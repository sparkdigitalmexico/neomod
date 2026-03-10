#pragma once
// Copyright (c) 2018, PG, All rights reserved.
#include "AnimationHandler.h"
#include "CBaseUIButton.h"

struct Skin;

class Image;

class UIPauseMenuButton final : public CBaseUIButton {
    NOCOPY_NOMOVE(UIPauseMenuButton);

   public:
    using BasicSkinImageGetter = SA::delegate<const Image *(const Skin *)>;

    UIPauseMenuButton(BasicSkinImageGetter imageGetter, float xPos, float yPos, float xSize, float ySize, std::string name);
    ~UIPauseMenuButton() override;

    void draw() override;

    void onMouseInside() override;
    void onMouseOutside() override;
    void onDisabled() override;

    void setBaseScale(float xScale, float yScale);
    void setAlpha(float alpha) { this->fAlpha = alpha; }
    void setBrightness(float brightness) { this->fBrightness = brightness; }

    [[nodiscard]] const Image* getImage() const;

   private:
    AnimVec2 vScale{1.f, 1.f};
    vec2 vBaseScale{0.f};
    float fScaleMultiplier{1.1f};

    float fAlpha{1.f};
    float fBrightness{1.0f};

    BasicSkinImageGetter imageGetter{nullptr};
};
