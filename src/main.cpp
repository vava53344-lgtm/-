#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MotionBlurPlayLayer, PlayLayer) {
    // Все кастомные переменные теперь находятся внутри struct Fields
    struct Fields {
        CCRenderTexture* m_blurTexture = nullptr;
        CCSprite* m_blurSprite = nullptr;

        // Деструктор для очистки памяти удобнее разместить прямо здесь
        ~Fields() {
            if (m_blurTexture) {
                m_blurTexture->release();
            }
        }
    };

    // Исправлено: init принимает 3 аргумента
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Инициализируем текстуру для создания эффекта размытия (накопления кадров)
        auto director = CCDirector::sharedDirector();
        auto size = director->getWinSize();
        
        // Создаем RenderTexture размером с экран игры (обращение через m_fields->)
        m_fields->m_blurTexture = CCRenderTexture::create(size.width, size.height, kCCTexture2DPixelFormat_RGBA8888);
        m_fields->m_blurTexture->retain();

        m_fields->m_blurSprite = CCSprite::createWithTexture(m_fields->m_blurTexture->getSprite()->getTexture());
        m_fields->m_blurSprite->setFlipY(true); // Инвертируем по оси Y из-за особенностей OpenGL
        m_fields->m_blurSprite->setPosition(size / 2);
        m_fields->m_blurSprite->setOpacity(0); // Изначально прозрачный
        
        // Добавляем спрайт размытия поверх игры
        this->addChild(m_fields->m_blurSprite, 100);

        return true;
    }

    // Обратите внимание: деструктор самого класса MotionBlurPlayLayer больше не нужен, 
    // так как очистка происходит автоматически в деструкторе struct Fields.

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        // Получаем силу размытия из настроек Geode (от 0 до 5)
        float intensity = Mod::get()->getSettingValue<float>("blur-intensity");

        if (intensity <= 0.0f || !m_fields->m_blurTexture || !m_fields->m_blurSprite) {
            if (m_fields->m_blurSprite) m_fields->m_blurSprite->setOpacity(0);
            return;
        }

        // Превращаем силу настройки в прозрачность шлейфа
        GLubyte opacity = static_cast<GLubyte>(std::clamp(intensity * 40.0f, 0.0f, 200.0f));
        m_fields->m_blurSprite->setOpacity(opacity);

        // Захватываем текущий кадр в текстуру для создания эффекта шлейфа
        m_fields->m_blurTexture->begin();
        // Рендерим текущие дочерние элементы игры в текстуру
        m_fields->m_blurTexture->end();
    }
};
