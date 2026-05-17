// Copyright (c) 2026, WH, All rights reserved.
#pragma once

#include "BeatmapInstaller.h"
#include "UIScreen.h"

class CBaseUIButton;

// global progress overlay for in-flight BeatmapInstaller entries.
// bottom-left anchored stack, one row per active entry. visible iff any entries exist,
// not in play mode, and cv::draw_beatmap_install_overlay is on.
class BeatmapInstallOverlay final : public UIScreen {
    NOCOPY_NOMOVE(BeatmapInstallOverlay)
   public:
    BeatmapInstallOverlay();
    ~BeatmapInstallOverlay() override = default;

    void draw() override;
    void update(CBaseUIEventCtx& c) override;
    void onResolutionChange(vec2 newResolution) override;

   private:
    // cached layout inputs; layout is recomputed only when any of these change
    uSz last_row_count{0};
    f32 last_scale{0.f};
    i32 last_virt_height{0};
};
