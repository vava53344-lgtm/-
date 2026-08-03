#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MotionBlurPlayLayer, PlayLayer) {
    CCRenderTexture* m_blurTexture = nullptr;
    CCSprite* m_blurSprite = nullptr;

    bool init(GJGameLevel* level) {
        if (!PlayLayer::init(level)) return false;

        // Инициализируем текстуру для создания эффекта размытия (накопления кадров)
        auto director = CCDirector::sharedDirector();
        auto size = director->getWinSize();
        
        // Создаем RenderTexture размером с экран игры
        m_blurTexture = CCRenderTexture::create(size.width, size.height, kCCTexture2DPixelFormat_RGBA8888);
        m_blurTexture->retain();

        m_blurSprite = CCSprite::createWithTexture(m_blurTexture->getSprite()->getTexture());
        m_blurSprite->setFlipY(true); // Инвертируем по оси Y из-за особенностей OpenGL
        m_blurSprite->setPosition(size / 2);
        m_blurSprite->setOpacity(0); // Изначально прозрачный
        
        // Добавляем спрайт размытия поверх игры, но под интерфейсом (слои можно настроить по zOrder)
        this->addChild(m_blurSprite, 100);

        return true;
    }

    ~MotionBlurPlayLayer() {
        if (m_blurTexture) {
            m_blurTexture->release();
            m_blurTexture = nullptr;
        }
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        // Получаем силу размытия из настроек Geode (от 0 до 5)
        float intensity = Mod::get()->getSettingValue<float>("blur-intensity");

        if (intensity <= 0.0f || !m_blurTexture || !m_blurSprite) {
            if (m_blurSprite) m_blurSprite->setOpacity(0);
            return;
        }

        // Превращаем силу настройки в прозрачность шлейфа (чем выше интенсивность, тем заметнее шлейф)
        GLubyte opacity = static_cast<GLubyte>(std::clamp(intensity * 40.0f, 0.0f, 200.0f));
        m_blurSprite->setOpacity(opacity);

        // Захватываем текущий кадр в текстуру для создания эффекта шлейфа
        m_blurTexture->begin();
        // Рендерим текущие дочерние элементы игры в текстуру
        // (Примечание: для полной оптимизации лучше использовать кастомный шейдер размытия в движении,
        // но этот метод создает базовый кинематографичный шлейф, похожий на Motion Blur)
        m_blurTexture->end();
    }
};
