#include "MotionBlurManager.hpp"

MotionBlurManager* MotionBlurManager::s_instance = nullptr;

MotionBlurManager* MotionBlurManager::get() {
    if (!s_instance) {
        s_instance = new MotionBlurManager();
    }
    return s_instance;
}

void MotionBlurManager::init() {
    if (m_initialized) return;
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    for (size_t i = 0; i < FRAME_COUNT; i++) {
        // Создаём текстуру размером с экран
        m_frames[i] = CCRenderTexture::create(winSize.width, winSize.height);
        if (m_frames[i]) {
            m_frames[i]->retain();
            if (auto sprite = m_frames[i]->getSprite()) {
                // Центрируем спрайт и переворачиваем по Y (особенность CCRenderTexture)
                sprite->setAnchorPoint({0.5f, 0.5f});
                sprite->setFlipY(true);
            }
        }
    }
    
    m_initialized = true;
}

void MotionBlurManager::cleanup() {
    for (size_t i = 0; i < FRAME_COUNT; i++) {
        if (m_frames[i]) {
            m_frames[i]->release();
            m_frames[i] = nullptr;
        }
    }
    m_initialized = false;
}

bool MotionBlurManager::isEnabled() const {
    return m_enabled;
}

void MotionBlurManager::setEnabled(bool enabled) {
    m_enabled = enabled;
}

float MotionBlurManager::getStrength() const {
    return m_strength;
}

void MotionBlurManager::setStrength(float strength) {
    m_strength = std::clamp(strength, 0.0f, 1.0f);
}

CCRenderTexture* MotionBlurManager::getCurrentRenderTexture() {
    return m_frames[m_currentFrame];
}

void MotionBlurManager::advanceFrame() {
    m_currentFrame = (m_currentFrame + 1) % FRAME_COUNT;
}

void MotionBlurManager::renderBlur() {
    if (!m_enabled) return;
    
    auto dir = CCDirector::sharedDirector();
    auto winSize = dir->getWinSize();
    
    // Обычное альфа-смешивание
    ccBlendFunc blend = {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};
    
    // Рисуем кадры: от самого старого к текущему
    for (size_t i = 0; i < FRAME_COUNT; i++) {
        size_t idx = (m_currentFrame + FRAME_COUNT - i) % FRAME_COUNT;
        auto rt = m_frames[idx];
        if (!rt) continue;
        
        auto sprite = rt->getSprite();
        if (!sprite) continue;
        
        float alpha;
        if (i == 0) {
            // Текущий кадр (только что отрендеренный) — полностью непрозрачный
            alpha = 1.0f;
        } else {
            // Чем старше кадр, тем меньше его вклад
            float t = static_cast<float>(i) / (FRAME_COUNT - 1);
            alpha = m_strength * (1.0f - t * 0.5f);
        }
        alpha = std::clamp(alpha, 0.0f, 1.0f);
        
        sprite->setOpacity(static_cast<GLubyte>(alpha * 255));
        sprite->setBlendFunc(blend);
        sprite->setPosition({winSize.width / 2, winSize.height / 2});
        sprite->setScale(1.0f);
        
        sprite->visit();
    }
  }
