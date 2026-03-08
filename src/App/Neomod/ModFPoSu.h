#pragma once
// Copyright (c) 2019, Colin Brook & PG, All rights reserved.
#include "noinclude.h"

#include "Vectors_fwd.h"
#include "StaticPImpl.h"

class KeyboardEvent;

class ModFPoSu final {
    NOCOPY_NOMOVE(ModFPoSu);

   private:
    static constexpr const int SUBDIVISIONS = 4;

   public:
    ModFPoSu();
    ~ModFPoSu();

    void draw();
    void update();

    void onResolutionChange(vec2 newResolution);

    void onKeyDown(KeyboardEvent &key);
    void onKeyUp(KeyboardEvent &key);

    //[[nodiscard]] inline const Camera *getCamera() const { return this->camera; }

    [[nodiscard]] bool isCrosshairIntersectingScreen() const;

    void resetCamera();

   private:
    void onResolutionChange0Args();
    void onCurvedChange();
    void onDistanceChange();
    void onNoclipChange();
    void makeBackgroundCube();

    struct FPoSuImpl;
    StaticPImpl<FPoSuImpl, 276> m_impl;
};
