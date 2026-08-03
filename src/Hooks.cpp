#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <array>
#include <algorithm>

using namespace geode::prelude;

// ============ MOTION BLUR STATE ============

class MotionBlurState {
public:
    static MotionBlurState* get() {
        static MotionBlurState instance;
        return &instance;
    }

    bool m_enabled = true;
    float m_strength = 0.5f;
    bool m_initialized = false;
    static constexpr size_t FRAME_COUNT = 4;
    std::array<CCRenderTexture*, FRAME_COUNT> m_frames{};
    size_t m_currentFrame = 0;

    void init() {
        if (m_initialized) return;
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        for (size_t i = 0; i < FRAME_COUNT; i++) {
            m_frames[i] = CCRenderTexture::create(winSize.width, winSize.height);
            if (m_frames[i]) {
                m_frames[i]->retain();
                if (auto sprite = m_frames[i]->getSprite()) {
                    sprite->setAnchorPoint({0.5f, 0.5f});
                    sprite->setFlipY(true);
                }
            }
        }
        m_initialized = true;
    }

    void cleanup() {
        for (size_t i = 0; i < FRAME_COUNT; i++) {
            if (m_frames[i]) {
                m_frames[i]->release();
                m_frames[i] = nullptr;
            }
        }
        m_initialized = false;
    }

    CCRenderTexture* getCurrentRT() {
        return m_frames[m_currentFrame];
    }

    void advance() {
        m_currentFrame = (m_currentFrame + 1) % FRAME_COUNT;
    }

    void updateSettings() {
        if (auto mod = Mod::get()) {
            m_enabled = mod->getSettingValue<bool>("enabled");
            m_strength = static_cast<float>(mod->getSettingValue<double>("strength"));
        }
    }

    void renderAccumulation() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        ccBlendFunc blend = {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};

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
};

// ============ SETTINGS POPUP (меньше и ниже) ============

class MotionBlurSettingsPopup : public CCLayerColor {
public:
    static MotionBlurSettingsPopup* create() {
        auto ret = new MotionBlurSettingsPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() override {
        if (!CCLayerColor::initWithColor({0, 0, 0, 150})) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Блокируем тачи
        this->setTouchEnabled(true);
        this->setTouchMode(kCCTouchesOneByOne);
        this->setTouchPriority(-500);

        // Фон попапа — МЕНЬШЕ
        auto bg = CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({260, 160});
        bg->setPosition({winSize.width / 2, winSize.height / 2 - 40}); // НИЖЕ
        this->addChild(bg);

        // Заголовок
        auto title = CCLabelBMFont::create("Motion Blur", "bigFont.fnt");
        title->setScale(0.5f);
        title->setPosition({winSize.width / 2, winSize.height / 2 + 15}); // НИЖЕ
        this->addChild(title);

        auto state = MotionBlurState::get();
        auto mod = Mod::get();
        if (!mod) return false;

        // Toggle: Enabled
        auto toggleMenu = CCMenu::create();
        toggleMenu->setPosition({winSize.width / 2 - 50, winSize.height / 2 - 20}); // НИЖЕ

        auto toggle = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(MotionBlurSettingsPopup::onToggle),
            0.7f // МЕНЬШЕ
        );
        toggle->setPosition({0, 0});
        toggle->toggle(state->m_enabled);
        toggleMenu->addChild(toggle);
        m_toggle = toggle;

        auto toggleLabel = CCLabelBMFont::create("Enabled", "bigFont.fnt");
        toggleLabel->setScale(0.4f); // МЕНЬШЕ
        toggleLabel->setPosition({winSize.width / 2 + 25, winSize.height / 2 - 20});
        toggleLabel->setAnchorPoint({0, 0.5f});
        this->addChild(toggleLabel);

        this->addChild(toggleMenu);

        // Slider: Strength
        auto sliderLabel = CCLabelBMFont::create("Strength", "bigFont.fnt");
        sliderLabel->setScale(0.4f); // МЕНЬШЕ
        sliderLabel->setPosition({winSize.width / 2, winSize.height / 2 - 55}); // НИЖЕ
        this->addChild(sliderLabel);

        auto slider = Slider::create(this, menu_selector(MotionBlurSettingsPopup::onSlider), 0.7f); // МЕНЬШЕ
        slider->setPosition({winSize.width / 2, winSize.height / 2 - 80}); // НИЖЕ
        slider->setValue(state->m_strength);
        this->addChild(slider);
        m_slider = slider;

        m_valueLabel = CCLabelBMFont::create(
            fmt::format("{:.0f}%", state->m_strength * 100).c_str(),
            "bigFont.fnt"
        );
        m_valueLabel->setScale(0.35f); // МЕНЬШЕ
        m_valueLabel->setPosition({winSize.width / 2, winSize.height / 2 - 100}); // НИЖЕ
        this->addChild(m_valueLabel);

        // Close button
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition({winSize.width / 2 + 110, winSize.height / 2 + 30}); // НИЖЕ

        auto closeBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
            this,
            menu_selector(MotionBlurSettingsPopup::onClose)
        );
        closeBtn->setScale(0.7f); // МЕНЬШЕ
        closeMenu->addChild(closeBtn);
        this->addChild(closeMenu);

        return true;
    }

    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override {
        return true;
    }

    void onToggle(CCObject*) {
        bool enabled = m_toggle->isToggled();
        MotionBlurState::get()->m_enabled = enabled;
        Mod::get()->setSettingValue("enabled", enabled);
    }

    void onSlider(CCObject*) {
        float val = m_slider->getValue();
        MotionBlurState::get()->m_strength = val;
        Mod::get()->setSettingValue("strength", static_cast<double>(val));
        m_valueLabel->setString(fmt::format("{:.0f}%", val * 100).c_str());
    }

    void onClose(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }

private:
    CCMenuItemToggler* m_toggle = nullptr;
    Slider* m_slider = nullptr;
    CCLabelBMFont* m_valueLabel = nullptr;
};

// ============ PAUSE LAYER BUTTON (НИЖЕ) ============

class $modify(MyPauseLayer, PauseLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* m_blurBtn = nullptr;
    };

    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto btn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
            this,
            menu_selector(MyPauseLayer::onMotionBlurSettings)
        );
        btn->setScale(0.6f); // МЕНЬШЕ

        auto menu = CCMenu::create();
        menu->addChild(btn);
        // НИЖЕ и ЛЕВЕЕ — под кнопкой Practice
        menu->setPosition({winSize.width - 35, 45});
        this->addChild(menu, 100);
    }

    void onMotionBlurSettings(CCObject*) {
        auto popup = MotionBlurSettingsPopup::create();
        if (popup) {
            CCDirector::sharedDirector()->getRunningScene()->addChild(popup, 999);
        }
    }
};

// ============ CCDIRECTOR HOOK (вместо CCScene) ============

class $modify(CCDirector) {
    void drawScene() {
        auto state = MotionBlurState::get();
        state->updateSettings();

        if (!state->m_enabled) {
            CCDirector::drawScene();
            return;
        }

        if (!state->m_initialized) {
            state->init();
        }

        static bool s_rendering = false;
        if (s_rendering) {
            CCDirector::drawScene();
            return;
        }

        s_rendering = true;

        auto rt = state->getCurrentRT();
        rt->beginWithClear(0, 0, 0, 0, 0);
        CCDirector::drawScene(); // оригинальный рендер
        rt->end();

        state->advance();
        state->renderAccumulation();

        s_rendering = false;
    }
};
