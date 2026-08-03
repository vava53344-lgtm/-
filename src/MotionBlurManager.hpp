#pragma once
#include <Geode/Geode.hpp>
#include <array>
#include <algorithm>

using namespace geode::prelude;

class MotionBlurManager {
public:
    static MotionBlurManager* get();
    
    void init();
    void cleanup();
    
    bool isEnabled() const;
    void setEnabled(bool enabled);
    
    float getStrength() const;
    void setStrength(float strength);
    
    CCRenderTexture* getCurrentRenderTexture();
    void advanceFrame();
    void renderBlur();
    
private:
    static MotionBlurManager* s_instance;
    
    bool m_enabled = true;
    float m_strength = 0.5f;
    
    static constexpr size_t FRAME_COUNT = 4;
    std::array<CCRenderTexture*, FRAME_COUNT> m_frames{};
    size_t m_currentFrame = 0;
    bool m_initialized = false;
};

