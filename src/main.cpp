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

    bool init(std::string const& title, std::string const& okButtonText, int maxCharCount) {
        if (!Popup::init(280.f, 160.f)) {
            return false;
        }

        // Window title (customizable)
        this->setTitle(title.c_str());

        // Hint text
        auto hint = CCLabelBMFont::create("Enter text below:", "bigFont.fnt");
        hint->setScale(0.45f);
        m_mainLayer->addChildAtPosition(hint, Anchor::Center, ccp(0, 30));

        // The text box itself (single-line input with background)
        m_textInput = TextInput::create(200.f, "Type here...");
        m_textInput->setMaxCharCount(maxCharCount); // customizable character limit
        m_mainLayer->addChildAtPosition(m_textInput, Anchor::Center, ccp(0, -5));

        // Confirm button (customizable text)
        auto okSprite = ButtonSprite::create(okButtonText.c_str());
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

        if (text.empty()) {
            FLAlertLayer::create("Result", "You entered nothing!", "OK")->show();
        } else {
            std::string result = std::string(text);
            FLAlertLayer::create("Result", result.c_str(), "OK")->show();
        }
    }

public:
    // title / okButtonText / maxCharCount — легко менять при вызове create()
    static TextBoxPopup* create(
        std::string const& title = "Text Box Dialog",
        std::string const& okButtonText = "OK",
        int maxCharCount = 64
    ) {
        auto ret = new TextBoxPopup();
        if (ret->init(title, okButtonText, maxCharCount)) {
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
            auto btnSprite = ButtonSprite::create("TBC");
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
        // Change title, OK button text, or character limit right here
        TextBoxPopup::create("Text Box Dialog", "OK", 64)->show();
    }
};
