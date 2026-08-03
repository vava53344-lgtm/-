#include "MotionBlurManager.hpp"

MotionBlurState* MotionBlurState::get() {
    static MotionBlurState instance;
    return &instance;
}

void MotionBlurState::init() {
    if (m_initialized) return;
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    for (size_t i = 0; i < FRAME_COUNT; i++) {
        m_frames[i] = CCRenderTexture::create(winSize.width, winSize.height, kCCTexture2DPixelFormat_RGBA8888);
        if (m_frames[i]) {
            m_frames[i]->retain();
            if (auto sprite = m_frames[i]->getSprite()) {
                sprite->setAnchorPoint({0.5f, 0.5f});
                sprite->setFlipY(true);
                sprite->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
            }
        }
    }
    m_initialized = true;
}

void MotionBlurState::cleanup() {
    for (size_t i = 0; i < FRAME_COUNT; i++) {
        if (m_frames[i]) {
            m_frames[i]->release();
            m_frames[i] = nullptr;
        }
    }
    m_initialized = false;
    m_currentFrame = 0;
}

CCRenderTexture* MotionBlurState::getCurrentRT() {
    return m_frames[m_currentFrame];
}

void MotionBlurState::advance() {
    m_currentFrame = (m_currentFrame + 1) % FRAME_COUNT;
}

void MotionBlurState::updateSettings() {
    if (auto mod = Mod::get()) {
        m_enabled = mod->getSettingValue<bool>("enabled");
        m_strength = static_cast<float>(mod->getSettingValue<double>("strength"));
    }
}

void MotionBlurState::renderAccumulation() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    ccBlendFunc blendNormal = {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};

    for (size_t i = 0; i < FRAME_COUNT; i++) {
        size_t idx = (m_currentFrame + FRAME_COUNT - i) % FRAME_COUNT;
        auto rt = m_frames[idx];
        if (!rt) continue;
        
        auto sprite = rt->getSprite();
        if (!sprite) continue;

        float alpha;
        if (i == 0) {
            alpha = 1.0f;
        } else {
            alpha = m_strength * std::pow(0.6f, static_cast<float>(i - 1));
        }
        alpha = std::clamp(alpha, 0.0f, 1.0f);

        sprite->setOpacity(static_cast<GLubyte>(alpha * 255));
        sprite->setBlendFunc(blendNormal);
        sprite->setPosition({winSize.width / 2, winSize.height / 2});
        sprite->setScale(1.0f);
        sprite->visit();
    }
}
