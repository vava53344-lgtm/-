#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include "MotionBlurManager.hpp"
#include <array>
#include <algorithm>

using namespace geode::prelude;

// ============ SETTINGS POPUP ============

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

        this->setTouchEnabled(true);
        this->setTouchMode(kCCTouchesOneByOne);
        this->setTouchPriority(-500);

        auto bg = CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({260, 160});
        bg->setPosition({winSize.width / 2, winSize.height / 2 - 40});
        this->addChild(bg);

        auto title = CCLabelBMFont::create("Motion Blur", "bigFont.fnt");
        title->setScale(0.5f);
        title->setPosition({winSize.width / 2, winSize.height / 2 + 15});
        this->addChild(title);

        auto state = MotionBlurState::get();
        auto mod = Mod::get();
        if (!mod) return false;

        auto toggleMenu = CCMenu::create();
        toggleMenu->setPosition({winSize.width / 2 - 50, winSize.height / 2 - 20});

        auto toggle = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(MotionBlurSettingsPopup::onToggle),
            0.7f
        );
        toggle->setPosition({0, 0});
        toggle->toggle(state->m_enabled);
        toggleMenu->addChild(toggle);
        m_toggle = toggle;

        auto toggleLabel = CCLabelBMFont::create("Enabled", "bigFont.fnt");
        toggleLabel->setScale(0.4f);
        toggleLabel->setPosition({winSize.width / 2 + 25, winSize.height / 2 - 20});
        toggleLabel->setAnchorPoint({0, 0.5f});
        this->addChild(toggleLabel);

        this->addChild(toggleMenu);

        auto sliderLabel = CCLabelBMFont::create("Strength", "bigFont.fnt");
        sliderLabel->setScale(0.4f);
        sliderLabel->setPosition({winSize.width / 2, winSize.height / 2 - 55});
        this->addChild(sliderLabel);

        auto slider = Slider::create(this, menu_selector(MotionBlurSettingsPopup::onSlider), 0.7f);
        slider->setPosition({winSize.width / 2, winSize.height / 2 - 80});
        slider->setValue(state->m_strength);
        this->addChild(slider);
        m_slider = slider;

        m_valueLabel = CCLabelBMFont::create(
            fmt::format("{:.0f}%", state->m_strength * 100).c_str(),
            "bigFont.fnt"
        );
        m_valueLabel->setScale(0.35f);
        m_valueLabel->setPosition({winSize.width / 2, winSize.height / 2 - 100});
        this->addChild(m_valueLabel);

        auto closeMenu = CCMenu::create();
        closeMenu->setPosition({winSize.width / 2 + 110, winSize.height / 2 + 30});

        auto closeBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
            this,
            menu_selector(MotionBlurSettingsPopup::onClose)
        );
        closeBtn->setScale(0.7f);
        closeMenu->addChild(closeBtn);
        this->addChild(closeMenu);

        return true;
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
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

// ============ PAUSE LAYER BUTTON ============

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
        btn->setScale(0.6f);

        auto menu = CCMenu::create();
        menu->addChild(btn);
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

// ============ PLAYLAYER HOOK ============

class $modify(PlayLayer) {
    struct Fields {
        bool m_blurInit = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        
        auto state = MotionBlurState::get();
        state->updateSettings();
        if (state->m_enabled && !state->m_initialized) {
            state->init();
        }
        
        m_fields->m_blurInit = true;
        return true;
    }

    void visit() {
        auto state = MotionBlurState::get();
        state->updateSettings();

        if (!state->m_enabled || !m_fields->m_blurInit) {
            PlayLayer::visit();
            return;
        }

        if (state->m_isRendering) {
            PlayLayer::visit();
            return;
        }

        state->m_isRendering = true;

        auto rt = state->getCurrentRT();
        rt->beginWithClear(0, 0, 0, 0, 0);
        PlayLayer::visit();
        rt->end();

        state->advance();
        state->renderAccumulation();

        state->m_isRendering = false;
    }

    void onQuit() {
        auto state = MotionBlurState::get();
        state->cleanup();
        PlayLayer::onQuit();
    }
};
