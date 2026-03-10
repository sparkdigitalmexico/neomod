#pragma once
// Copyright (c) 2018, PG, All rights reserved.
#include "AnimationHandler.h"
#include "CBaseUIButton.h"

#include <memory>

class DatabaseBeatmap;
using BeatmapDifficulty = DatabaseBeatmap;
using BeatmapSet = DatabaseBeatmap;

namespace Replay {
struct Mods;
}

enum class ScoreGrade : uint8_t;

struct FinishedScore;
class SkinImage;

class UIAvatar;
class UIContextMenu;

class ScoreButton final : public CBaseUIButton {
    NOCOPY_NOMOVE(ScoreButton)
   public:
    static UString getModsStringForDisplay(const Replay::Mods &mods);

    enum class STYLE : uint8_t { SONG_BROWSER, TOP_RANKS };
    ScoreButton() = delete;
    ScoreButton(UIContextMenu *contextMenu, float xPos, float yPos, float xSize, float ySize,
                STYLE style = STYLE::SONG_BROWSER);
    ~ScoreButton() override;

    void draw() override;
    void update(CBaseUIEventCtx &c) override;

    void highlight();
    void resetHighlight();

    void setScore(const FinishedScore &score, const DatabaseBeatmap *map, int index = 1,
                  const UString &titleString = {}, float weight = 1.0f);
    void setIndex(int index) { this->iScoreIndexNumber = index; }

    [[nodiscard]] inline const FinishedScore &getScore() const { return *this->storedScore; }
    [[nodiscard]] u64 getScoreUnixTimestamp() const;
    [[nodiscard]] u64 getScoreScore() const;

    [[nodiscard]] inline const UString &getDateTime() const { return this->sScoreDateTime; }
    [[nodiscard]] inline int getIndex() const { return this->iScoreIndexNumber; }

    void onMouseInside() override;
    void onMouseOutside() override;

    void onFocusStolen() override;

   protected:
    void onClicked(bool left = true, bool right = false) override;

   private:
    static UString recentScoreIconString;

    void updateElapsedTimeString();

    void onRightMouseUpInside();
    void onContextMenu(const UString &text, int id = -1);
    void onUseModsClicked();
    void onDeleteScoreClicked();
    void onDeleteScoreConfirmed(const UString &text, int id);

    bool isContextMenuVisible();

    std::unique_ptr<UIAvatar> avatar{nullptr};
    UIContextMenu *contextMenu;

    // STYLE::SCORE_BROWSER
    // TODO: clean up this mess of confusing variable/names
    UString sScoreTime;
    UString sScoreUsername;
    UString sFmtedScoreScoreWithCombo;
    UString sFmtedScorePPWithCombo;
    UString sScoreAccuracy;
    UString sScoreAccuracyFC;
    UString sFmtedScoreAccuracyWithCombo;
    UString sScoreMods;
    UString sCustom;

    // STYLE::TOP_RANKS
    UString sScoreTitle;
    UString sScoreScorePPWeightedPP;
    UString sScoreScorePPWeightedWeight;
    UString sScoreWeight;

    std::vector<UString> tooltipLines;
    UString sScoreDateTime;

    // score data
    std::unique_ptr<FinishedScore> storedScore;
    int iScoreIndexNumber{1};
    u64 iScoreUnixTimestamp{0};

    ScoreGrade scoreGrade;

    STYLE style;
    AnimFloat fIndexNumberAnim;
    bool bIsPulseAnim{false};

    bool bRightClick{false};
    bool bRightClickCheck{false};
    bool is_friend{false};
};
