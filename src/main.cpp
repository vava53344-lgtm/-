#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

// ---------------------------------------------------------------------
// Custom dialog popup with a text input box.
// Inherits from geode::Popup — the standard Geode way to make
// complex popups (background, close button etc. included out of the box).
// ---------------------------------------------------------------------
class TextBoxPopup : public geode::Popup {
protected:
    TextInput* m_textInput = nullptr;

    bool init() {
        if (!Popup::init(280.f, 160.f)) {
            return false;
        }

        // Window title
        this->setTitle("Text Box Dialog");

        // Hint text
        auto hint = CCLabelBMFont::create("Enter text below:", "bigFont.fnt");
        hint->setScale(0.45f);
        m_mainLayer->addChildAtPosition(hint, Anchor::Center, ccp(0, 30));

        // The text box itself (single-line input with background)
        m_textInput = TextInput::create(200.f, "Type here...");
        m_textInput->setMaxCharCount(64);
        m_mainLayer->addChildAtPosition(m_textInput, Anchor::Center, ccp(0, -5));

        // Confirm button
        auto okSprite = ButtonSprite::create("OK");
        auto okBtn = CCMenuItemSpriteExtra::create(
            okSprite, this, menu_selector(TextBoxPopup::onConfirm)
        );

        auto menu = CCMenu::create();
        menu->addChild(okBtn);
        m_mainLayer->addChildAtPosition(menu, Anchor::Center, ccp(0, -45));

        return true;
    }

    void onConfirm(CCObject*) {
        auto text = m_textInput->getString();

        std::string result = text.empty()
            ? std::string("You entered nothing!")
            : ("You entered: " + std::string(text));

        // This shows the native GD alert style (same as "Move to Top" etc.)
        FLAlertLayer::create("Result", result.c_str(), "OK")->show();
    }

public:
    static TextBoxPopup* create() {
        auto ret = new TextBoxPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

// ---------------------------------------------------------------------
// Add a button to the main menu that opens our popup
// ---------------------------------------------------------------------
class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto menu = this->getChildByID("bottom-menu");
        if (menu) {
            auto btnSprite = ButtonSprite::create("TB");
            auto btn = CCMenuItemSpriteExtra::create(
                btnSprite, this, menu_selector(MyMenuLayer::onOpenTextBox)
            );
            btn->setID("textbox-dialog-button"_spr);

            menu->addChild(btn);
            menu->updateLayout();
        }

        return true;
    }

    void onOpenTextBox(CCObject*) {
        TextBoxPopup::create()->show();
    }
};
