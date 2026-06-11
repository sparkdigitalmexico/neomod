#pragma once
// Copyright (c) 2017, PG, All rights reserved.
#include "ScreenBackable.h"

class CBaseUIContainer;
class CBaseUIScrollView;
class CBaseUIImage;
class CBaseUILabel;
class CBaseUIButton;

class AboutScreen final : public ScreenBackable {
    NOCOPY_NOMOVE(AboutScreen)
   public:
    AboutScreen();
    ~AboutScreen() override;

    CBaseUIContainer *setVisible(bool visible) override;

    [[nodiscard]] bool claimsArrowKeys() override { return true; }

   private:
    void updateLayout() override;
    void onBack() override;

    void onChangeClicked(CBaseUIButton *button);

    struct CHANGELOG {
        std::string title;
        std::vector<std::string> changes;
    };

    struct CHANGELOG_UI {
        CBaseUILabel *title;
        std::vector<CBaseUIButton *> changes;
    };

    void addAllChangelogs(std::vector<CHANGELOG> &&logtexts);

    CBaseUIScrollView *scrollView;

    // We store headers to be able to jump directly to them with scrollToElement
    // (unused atm, too lazy to remake the OptionsOverlay sidebar thing)

    CBaseUILabel *changelogHeader;
    std::vector<CHANGELOG_UI> changelogs;

    CBaseUILabel *creditsHeader;
    std::vector<CBaseUIButton *> creditsLines;

    CBaseUILabel *licensesHeader;
    std::vector<CBaseUIButton *> licensesLines;
};
