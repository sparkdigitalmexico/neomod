// Copyright (c) 2026, WH, All rights reserved.
#include "EmojiRenderTest.h"

#include "Engine.h"
#include "Logging.h"
#include "ResourceManager.h"
#include "Graphics.h"
#include "Font.h"
#include "KeyBindings.h"
#include "KeyboardEvent.h"

namespace Mc::Tests {

using std::string_view_literals::operator""sv;

EmojiRenderTest::EmojiRenderTest() {
    m_font16 = resourceManager->loadFont("weblysleekuisb", "EMOJI_TEST_16", 16, true);
    m_font24 = resourceManager->loadFont("weblysleekuisb", "EMOJI_TEST_24", 24, true);
    m_font32 = resourceManager->loadFont("weblysleekuisb", "EMOJI_TEST_32", 32, true);
}

EmojiRenderTest::~EmojiRenderTest() = default;

void EmojiRenderTest::draw() {
    const float x = 20.f;
    float y = 40.f;
    const float lineSpacing = 1.8f;

    // pure emoji at different sizes
    g->setColor(0xffffffff);
    g->pushTransform();
    g->translate(x, y);
    g->drawString(m_font16, "😀🎉🌍🔥"sv);
    g->popTransform();
    y += m_font16->getHeight() * lineSpacing;

    g->pushTransform();
    g->translate(x, y);
    g->drawString(m_font24, "😀🎉🌍🔥"sv);
    g->popTransform();
    y += m_font24->getHeight() * lineSpacing;

    g->pushTransform();
    g->translate(x, y);
    g->drawString(m_font32, "😀🎉🌍🔥"sv);
    g->popTransform();
    y += m_font32->getHeight() * lineSpacing;

    // mixed text + emoji
    y += 20.f;
    g->setColor(0xffffffff);
    g->pushTransform();
    g->translate(x, y);
    g->drawString(m_font24, "Hello 👋 World 🌍"sv);  // U+1F44B  U+1F30D
    g->popTransform();
    y += m_font24->getHeight() * lineSpacing;

    // colored text + emoji (red text, emoji should stay unmodified)
    g->setColor(0xffff4444);
    g->pushTransform();
    g->translate(x, y);
    g->drawString(m_font24, "Red text 🔥"sv);  // U+1F525
    g->popTransform();
    y += m_font24->getHeight() * lineSpacing;

    // green text + emoji
    g->setColor(0xff44ff44);
    g->pushTransform();
    g->translate(x, y);
    g->drawString(m_font24, "Green text 🎉"sv);  // U+1F389
    g->popTransform();
    y += m_font24->getHeight() * lineSpacing;

    // text shadow test
    g->setColor(0xffffffff);
    g->pushTransform();
    g->translate(x, y);
    g->drawString(m_font24, "Shadow 😀 test"sv,
                  TextShadow{.col_text = rgb(255, 255, 255), .col_shadow = rgb(200, 100, 0), .offs_px = 1.5f});
    g->popTransform();
    y += m_font24->getHeight() * lineSpacing;

    // misc symbols range
    g->setColor(0xffffffff);
    g->pushTransform();
    g->translate(x, y);
    g->drawString(m_font24,
                  "Misc: ✅ ❌ ✨ ☀️"sv);  // U+2705 U+274C U+2728 U+2600
    g->popTransform();
    y += m_font24->getHeight() * lineSpacing;

    // atlas debug
    if(m_showAtlas) {
        m_font24->drawTextureAtlas();
    }

    // instructions
    g->setColor(0xaa888888);
    g->pushTransform();
    g->translate(x, (f32)engine->getScreenHeight() - 30.f);
    g->drawString(m_font16, "Press TAB to toggle atlas view"sv);
    g->popTransform();
}

void EmojiRenderTest::update() {}

void EmojiRenderTest::onKeyDown(KeyboardEvent &e) {
    if(e.getScanCode() == KEY_TAB) {
        m_showAtlas = !m_showAtlas;
        e.consume();
    }
}

}  // namespace Mc::Tests
