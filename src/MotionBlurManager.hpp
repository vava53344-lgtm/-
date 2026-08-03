#pragma once
#include <Geode/Geode.hpp>
#include <array>
#include <algorithm>

using namespace geode::prelude;

class MotionBlurState {
public:
    static MotionBlurState* get();
    void init();
    void cleanup();
    bool m_enabled;
    float m_strength;
    bool m_initialized;
    static constexpr size_t FRAME_COUNT = 4;
    std::array<CCRenderTexture*, FRAME_COUNT> m_frames;
    size_t m_currentFrame;
    CCRenderTexture* getCurrentRT();
    void advance();
    void updateSettings();
    void renderAccumulation();
};
