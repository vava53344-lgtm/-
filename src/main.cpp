#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <sstream>

using namespace geode::prelude;

// Splits "Cancel, OK, Retry" into {"Cancel", "OK", "Retry"}
static std::vector<std::string> splitButtonLabels(std::string const& raw) {
    std::vector<std::string> result;
    std::stringstream ss(raw);
    std::string item;

    while (std::getline(ss, item, ',')) {
        auto start = item.find_first_not_of(" \t");
        auto end = item.find_last_not_of(" \t");
        if (start != std::string::npos) {
            result.push_back(item.substr(start, end - start + 1));
        }
    }

    if (result.empty()) {
        result.push_back("OK");
    }
    return result;
}

// ---------------------------------------------------------------------
// The actual dialog popup with a text input box.
// ---------------------------------------------------------------------
class TextBoxPopup : public geode::Popup {
protected:
    TextInput* m_textInput = nullptr;

    bool init(std::string const& title, std::vector<std::string> const& buttonLabels, int maxCharCount) {
        if (!Popup::init(280.f, 160.f)) {
            return false;
        }

        this->setTitle(title.c_str());

        auto hint = CCLabelBMFont::create("Enter text below:", "bigFont.fnt");
        hint->setScale(0.45f);
        m_mainLayer->addChildAtPosition(hint, Anchor::Center, ccp(0, 30));

        m_textInput = TextInput::create(200.f, "Type here...");
        m_textInput->setMaxCharCount(maxCharCount);
        m_mainLayer->addChildAtPosition(m_textInput, Anchor::Center, ccp(0, -5));

        auto menu = CCMenu::create();
        float spacing = 90.f;
        float startX = -(static_cast<float>(buttonLabels.size() - 1) * spacing) / 2.f;

        for (size_t i = 0; i < buttonLabels.size(); i++) {
            auto sprite = ButtonSprite::create(buttonLabels[i].c_str());
            auto btn = CCMenuItemSpriteExtra::create(
                sprite, this, menu_selector(TextBoxPopup::onButtonClicked)
            );
            btn->setTag(static_cast<int>(i));
            btn->setPosition({startX + static_cast<float>(i) * spacing, 0.f});
            menu->addChild(btn);
        }

        m_mainLayer->addChildAtPosition(menu, Anchor::Center, ccp(0, -45));

        return true;
    }

    void onButtonClicked(CCObject* sender) {
        auto text = m_textInput->getString();
        auto clickedIndex = static_cast<CCMenuItemSpriteExtra*>(sender)->getTag();

        std::string message = text.empty()
            ? std::string("You entered nothing!")
            : std::string(text);

        FLAlertLayer::create("Result", message.c_str(), "OK")->show();
    }

public:
    static TextBoxPopup* create(
        std::string const& title = "Text Box Dialog",
        std::vector<std::string> const& buttonLabels = {"OK"},
        int maxCharCount = 64
    ) {
        auto ret = new TextBoxPopup();
        if (ret->init(title, buttonLabels, maxCharCount)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

// ---------------------------------------------------------------------
// Settings popup: you type the title, comma-separated button labels,
// and character limit right here in-game, then open the final dialog.
// ---------------------------------------------------------------------
class SettingsPopup : public geode::Popup {
protected:
    TextInput* m_titleInput = nullptr;
    TextInput* m_buttonsInput = nullptr;
    TextInput* m_maxCharsInput = nullptr;

    bool init() {
        if (!Popup::init(280.f, 220.f)) {
            return false;
        }

        this->setTitle("Dialog Settings");

        auto titleLabel = CCLabelBMFont::create("Title:", "bigFont.fnt");
        titleLabel->setScale(0.4f);
        m_mainLayer->addChildAtPosition(titleLabel, Anchor::Center, ccp(-95, 65));

        m_titleInput = TextInput::create(160.f, "Dialog title");
        m_titleInput->setString("Text Box Dialog");
        m_mainLayer->addChildAtPosition(m_titleInput, Anchor::Center, ccp(30, 65));

        auto buttonsLabel = CCLabelBMFont::create("Buttons:", "bigFont.fnt");
        buttonsLabel->setScale(0.4f);
        m_mainLayer->addChildAtPosition(buttonsLabel, Anchor::Center, ccp(-95, 25));

        m_buttonsInput = TextInput::create(160.f, "Cancel, OK");
        m_buttonsInput->setString("OK");
        m_mainLayer->addChildAtPosition(m_buttonsInput, Anchor::Center, ccp(30, 25));

        auto maxCharsLabel = CCLabelBMFont::create("Max chars:", "bigFont.fnt");
        maxCharsLabel->setScale(0.4f);
        m_mainLayer->addChildAtPosition(maxCharsLabel, Anchor::Center, ccp(-95, -15));

        m_maxCharsInput = TextInput::create(100.f, "64");
        m_maxCharsInput->setString("64");
        m_mainLayer->addChildAtPosition(m_maxCharsInput, Anchor::Center, ccp(10, -15));

        auto openSprite = ButtonSprite::create("Open Dialog");
        auto openBtn = CCMenuItemSpriteExtra::create(
            openSprite, this, menu_selector(SettingsPopup::onOpenDialog)
        );

        auto menu = CCMenu::create();
        menu->addChild(openBtn);
        m_mainLayer->addChildAtPosition(menu, Anchor::Center, ccp(0, -70));

        return true;
    }

    void onOpenDialog(CCObject*) {
        auto titleRaw = m_titleInput->getString();
        std::string title = titleRaw.empty() ? "Text Box Dialog" : std::string(titleRaw);

        auto buttonsRaw = m_buttonsInput->getString();
        auto buttonLabels = splitButtonLabels(std::string(buttonsRaw));

        auto maxCharsRaw = m_maxCharsInput->getString();
        int maxChars = 64;
        try {
            maxChars = std::stoi(std::string(maxCharsRaw));
            if (maxChars <= 0) maxChars = 64;
        } catch (...) {
            maxChars = 64;
        }

        this->onClose(nullptr);
        TextBoxPopup::create(title, buttonLabels, maxChars)->show();
    }

public:
    static SettingsPopup* create() {
        auto ret = new SettingsPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

// ---------------------------------------------------------------------
// Add a button to the main menu that opens the settings popup
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
        SettingsPopup::create()->show();
    }
};
