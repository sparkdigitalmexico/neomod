// Copyright (c) 2015, PG, 2026, WH, All rights reserved.
#include "Keyboard.h"

#include "Engine.h"
#include "Environment.h"
#include "Logging.h"
#include "KeyboardEvent.h"
#include "UniString.h"

#include <SDL3/SDL_events.h>

#include <cstring>
#include <utility>

namespace {

constexpr unsigned char ALLOCATED_FLAG = 255;
static constexpr size_t TEXT_OFFSET = offsetof(SDL_TextInputEvent, text);

// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast,cppcoreguidelines-pro-type-member-init,hicpp-member-init)

struct CapturedState final {
    CapturedState() = default;
    ~CapturedState() {
        auto* dataBytes = reinterpret_cast<unsigned char*>(&fullEv);
        if(dataBytes[sizeof(fullEv) - 1] == ALLOCATED_FLAG) {
            std::free(const_cast<void*>(reinterpret_cast<const void*>(textEv.text)));
        }
    }

    CapturedState(CapturedState&& o) noexcept {
        std::memcpy(static_cast<void*>(&fullEv), static_cast<const void*>(&o.fullEv), sizeof(fullEv));
        if(o.type() == SDL_EVENT_TEXT_INPUT) {
            auto* ourDataBytes = reinterpret_cast<unsigned char*>(&fullEv);
            if(ourDataBytes[sizeof(fullEv) - 1] != ALLOCATED_FLAG && textEv.text != nullptr) {
                textEv.text = reinterpret_cast<const char*>(&ourDataBytes[TEXT_OFFSET + sizeof(void*)]);
            }
        }
        std::memset(static_cast<void*>(&o.fullEv), 0, sizeof(fullEv));
    }

    CapturedState& operator=(CapturedState&& o) noexcept {
        if(this != &o) {
            // clean up existing state
            auto* dataBytes = reinterpret_cast<unsigned char*>(&fullEv);
            if(dataBytes[sizeof(fullEv) - 1] == ALLOCATED_FLAG) {
                std::free(const_cast<void*>(reinterpret_cast<const void*>(textEv.text)));
            }

            std::memcpy(static_cast<void*>(&fullEv), static_cast<const void*>(&o.fullEv), sizeof(fullEv));
            if(o.type() == SDL_EVENT_TEXT_INPUT) {
                if(dataBytes[sizeof(fullEv) - 1] != ALLOCATED_FLAG && textEv.text != nullptr) {
                    textEv.text = reinterpret_cast<const char*>(&dataBytes[TEXT_OFFSET + sizeof(void*)]);
                }
            }
            std::memset(static_cast<void*>(&o.fullEv), 0, sizeof(fullEv));
        }
        return *this;
    }

    CapturedState(const CapturedState&) = delete;
    CapturedState& operator=(const CapturedState&) = delete;

    CapturedState(const SDL_KeyboardEvent& sdlKeyboardEvent) {
        std::memcpy(static_cast<void*>(&keyEv), static_cast<const void*>(&sdlKeyboardEvent), sizeof(keyEv));
        auto* dataBytes = reinterpret_cast<unsigned char*>(&fullEv);
        dataBytes[sizeof(fullEv) - 1] = 0;
    }

    CapturedState(const SDL_TextInputEvent& sdlTextEvent) {
        std::memcpy(static_cast<void*>(&textEv), static_cast<const void*>(&sdlTextEvent), sizeof(textEv));
        auto* ourDataBytes = reinterpret_cast<unsigned char*>(&fullEv);
        ourDataBytes[sizeof(fullEv) - 1] = 0;
        // make a deep copy of the text
        if(!!sdlTextEvent.text && *sdlTextEvent.text != '\0') {
            const size_t len = std::strlen(sdlTextEvent.text);
            if(len < (sizeof(fullEv) - TEXT_OFFSET) - sizeof(void*)) {
                // the text can fit inside the unused padding between the const char * and end of the full SDL_Event, just copy it into there
                std::memcpy(static_cast<void*>(&ourDataBytes[TEXT_OFFSET + sizeof(void*)]),
                            static_cast<const void*>(sdlTextEvent.text), len);
                ourDataBytes[TEXT_OFFSET + sizeof(void*) + len] = '\0';
                textEv.text = reinterpret_cast<const char*>(&ourDataBytes[TEXT_OFFSET + sizeof(void*)]);
            } else {
                // too long to fit inside a full SDL_Event, allocate
                char* ptr = static_cast<char*>(std::malloc(len + 1));
                std::memcpy(static_cast<void*>(ptr), static_cast<const void*>(sdlTextEvent.text), len);
                ptr[len] = '\0';
                textEv.text = ptr;
                ourDataBytes[sizeof(fullEv) - 1] = ALLOCATED_FLAG;
            }
        }
    }

    [[nodiscard]] unsigned int type() const { return fullEv.type; }

    [[nodiscard]] operator const SDL_KeyboardEvent&() const {
        assert(type() == SDL_EVENT_KEY_DOWN || type() == SDL_EVENT_KEY_UP);
        return keyEv;
    }

    [[nodiscard]] operator const SDL_TextInputEvent&() const {
        assert(type() == SDL_EVENT_TEXT_INPUT);
        return textEv;
    }

    union {
        SDL_KeyboardEvent keyEv;
        SDL_TextInputEvent textEv;
        SDL_Event fullEv;
    };
};

}  // namespace

// NOLINTEND(cppcoreguidelines-pro-type-const-cast,cppcoreguidelines-pro-type-member-init,hicpp-member-init)

struct Keyboard::KeyboardImpl {
    void dispatchKeyDown(KeyboardEvent event);
    void dispatchKeyUp(KeyboardEvent event);
    void dispatchChar(KeyboardEvent event);
    std::vector<KeyboardListener*> listeners;

    std::vector<CapturedState> eventQueue;

    KEYCODE modstate{};
};

Keyboard::Keyboard() : m_impl() {}
Keyboard::~Keyboard() = default;

void Keyboard::draw() {}

void Keyboard::update() {
    for(auto& event : m_impl->eventQueue) {
        switch(event.type()) {
            case SDL_EVENT_KEY_DOWN: {
                const auto& kevent{static_cast<const SDL_KeyboardEvent&>(event)};
                m_impl->modstate = kevent.mod;
                m_impl->dispatchKeyDown({static_cast<SCANCODE>(kevent.scancode), kevent.key,
                                         static_cast<char32_t>(kevent.key), kevent.timestamp, kevent.repeat});
                break;
            }

            case SDL_EVENT_KEY_UP: {
                const auto& kevent{static_cast<const SDL_KeyboardEvent&>(event)};
                m_impl->modstate = kevent.mod;
                m_impl->dispatchKeyUp({static_cast<SCANCODE>(kevent.scancode), kevent.key,
                                       static_cast<char32_t>(kevent.key), kevent.timestamp, kevent.repeat});
                break;
            }

            case SDL_EVENT_TEXT_INPUT: {
                const auto& tevent{static_cast<const SDL_TextInputEvent&>(event)};
                const char* evtextstr = tevent.text;
                if(unlikely(!evtextstr || evtextstr[0] == '\0'))
                    continue;  // probably should be assert() but there's no point in microoptimizing that hard

                size_t length = strlen(evtextstr);
                if(likely(length == 1)) {
                    m_impl->dispatchChar({0, 0, static_cast<char32_t>(evtextstr[0]), tevent.timestamp, false});
                } else {
                    for(char32_t chr : UniString::codepoints(std::string_view{evtextstr, length}))
                        m_impl->dispatchChar({0, 0, chr, tevent.timestamp, false});
                }
                break;
            }

            default: {
                assert(false && "unknown event in the queue");
                std::unreachable();
                break;
            }
        }
    }
    m_impl->eventQueue.clear();
}

void Keyboard::onKey(SDL_KeyboardEvent event) { m_impl->eventQueue.emplace_back(event); }
void Keyboard::onChar(SDL_TextInputEvent event) { m_impl->eventQueue.emplace_back(event); }

// actually, don't throw away queued events (otherwise we might not dispatch a queued key up for a key down we got)
void Keyboard::reset() { m_impl->modstate = env->getCurrentlyHeldKeyModifiers(); }

void Keyboard::addListener(KeyboardListener* keyboardListener, bool insertOnTop) {
    if(keyboardListener == nullptr) {
        engine->showMessageError("Keyboard Error", "addListener(NULL)!");
        return;
    }

    if(insertOnTop)
        m_impl->listeners.insert(m_impl->listeners.begin(), keyboardListener);
    else
        m_impl->listeners.push_back(keyboardListener);
}

void Keyboard::removeListener(KeyboardListener* keyboardListener) { std::erase(m_impl->listeners, keyboardListener); }

bool Keyboard::areModsHeld(KEYMOD keymodMask) const { return (m_impl->modstate & keymodMask) == keymodMask; }

bool Keyboard::isControlDown() const { return !!(m_impl->modstate & KEYMOD_CONTROL); }
bool Keyboard::isAltDown() const { return !!(m_impl->modstate & KEYMOD_ALT); }
bool Keyboard::isShiftDown() const { return !!(m_impl->modstate & KEYMOD_SHIFT); }
bool Keyboard::isSuperDown() const { return !!(m_impl->modstate & KEYMOD_SUPER); }

void Keyboard::KeyboardImpl::dispatchKeyDown(KeyboardEvent event) {
    for(auto* listener : this->listeners) {
        listener->onKeyDown(event);
        if(event.isConsumed()) {
            break;
        }
    }
}

void Keyboard::KeyboardImpl::dispatchKeyUp(KeyboardEvent event) {
    for(auto* listener : this->listeners) {
        listener->onKeyUp(event);
        if(event.isConsumed()) {
            break;
        }
    }
}

void Keyboard::KeyboardImpl::dispatchChar(KeyboardEvent event) {
    for(auto* listener : this->listeners) {
        listener->onChar(event);
        if(event.isConsumed()) {
            break;
        }
    }
}
