#pragma once
// Copyright (c) 2026, kiwec, All rights reserved.
#include "types.h"
#include "UIScreen.h"

#include <functional>

class LoadingScreen;
class Image;

class LoadingScreen : public UIOverlay {
    NOCOPY_NOMOVE(LoadingScreen)
   public:
    LoadingScreen() = delete;
    LoadingScreen(UIScreen *parent) : UIOverlay(parent) {}
    ~LoadingScreen() override { this->onFinished(); }

    void update(CBaseUIEventCtx &c) override;

    inline void draw() final {
        if(!this->isVisible()) return;
        this->drawBackground();
        this->drawProgress();
        this->drawLoadingSpinner();
    }

    void onKeyDown(KeyboardEvent &e) override;

    inline bool isVisible() final { return UIOverlay::isVisible() && !this->onFinishedCalled; }
    [[nodiscard]] virtual inline bool isFinished() const { return this->progress >= 1.f; }

   protected:
    virtual void drawBackground();
    virtual void drawProgress();
    virtual void drawLoadingSpinner();

    virtual f32 updateProgress() { return this->progress; }
    virtual void finish() { this->progress = 1.f; }
    f32 progress{0.f};

   private:
    void onFinished() {
        if(this->onFinishedCalled) return;
        this->onFinishedCalled = true;
        this->finish();
        return;
    }

    bool onFinishedCalled{false};
};
