#include <Geode/Geode.hpp>
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

// ============ SETTINGS POPUP ============

class MotionBlurSettingsPopup : public geode::Popup<> {
public:
    static MotionBlurSettingsPopup* create() {
        auto ret = new MotionBlurSettingsPopup();
        if (ret->initAnchored(300.f, 200.f, "Motion Blur Settings")) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

protected:
    bool setup() override {
        auto state = MotionBlurState::get();
        auto mod = Mod::get();
        if (!mod) return false;

        auto winSize = m_mainLayer->getContentSize();

        // Toggle: Enabled
        auto toggle = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(MotionBlurSettingsPopup::onToggle),
            1.0f
        );
        toggle->setPosition({winSize.width / 2 - 80, winSize.height - 50});
        toggle->toggle(state->m_enabled);

        auto toggleLabel = CCLabelBMFont::create("Enabled", "bigFont.fnt");
        toggleLabel->setScale(0.5f);
        toggleLabel->setPosition({winSize.width / 2 + 20, winSize.height - 50});
        toggleLabel->setAnchorPoint({0, 0.5f});

        m_mainLayer->addChild(toggle);
        m_mainLayer->addChild(toggleLabel);
        m_toggle = toggle;

        // Slider: Strength
        auto sliderLabel = CCLabelBMFont::create("Strength", "bigFont.fnt");
        sliderLabel->setScale(0.5f);
        sliderLabel->setPosition({winSize.width / 2, winSize.height - 90});

        auto slider = Slider::create(this, menu_selector(MotionBlurSettingsPopup::onSlider), 1.0f);
        slider->setPosition({winSize.width / 2, winSize.height - 120});
        slider->setValue(state->m_strength);

        auto valueLabel = CCLabelBMFont::create(
            fmt::format("{:.0f}%", state->m_strength * 100).c_str(),
            "bigFont.fnt"
        );
        valueLabel->setScale(0.4f);
        valueLabel->setPosition({winSize.width / 2, winSize.height - 145});
        valueLabel->setTag(100);

        m_mainLayer->addChild(sliderLabel);
        m_mainLayer->addChild(slider);
        m_mainLayer->addChild(valueLabel);
        m_slider = slider;

        // Close button
        auto closeBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
            this,
            menu_selector(MotionBlurSettingsPopup::onClose)
        );
        closeBtn->setPosition({15, winSize.height - 15});
        m_buttonMenu->addChild(closeBtn);

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

        if (auto label = m_mainLayer->getChildByTag(100)) {
            if (auto bmLabel = typeinfo_cast<CCLabelBMFont*>(label)) {
                bmLabel->setString(fmt::format("{:.0f}%", val * 100).c_str());
            }
        }
    }

    void onClose(CCObject*) {
        this->onClose(nullptr);
    }

private:
    CCMenuItemToggler* m_toggle = nullptr;
    Slider* m_slider = nullptr;
};

// ============ PAUSE LAYER BUTTON ============

class $modify(PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto btn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
            this,
            menu_selector(PauseLayer::onMotionBlurSettings)
        );

        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition({winSize.width - 30, winSize.height - 30});
        this->addChild(menu, 100);
    }

    void onMotionBlurSettings(CCObject*) {
        auto popup = MotionBlurSettingsPopup::create();
        if (popup) {
            popup->show();
        }
    }
};

// ============ CCSCENE HOOK ============

class $modify(CCScene) {
    void visit() {
        auto state = MotionBlurState::get();
        state->updateSettings();

        if (!state->m_enabled) {
            CCScene::visit();
            return;
        }

        if (!state->m_initialized) {
            state->init();
        }

        static bool s_rendering = false;
        if (s_rendering) {
            CCScene::visit();
            return;
        }

        s_rendering = true;

        auto rt = state->getCurrentRT();
        rt->beginWithClear(0, 0, 0, 0, 0);
        CCScene::visit();
        rt->end();

        state->advance();
        state->renderAccumulation();

        s_rendering = false;
    }
};
