#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

// ---------------------------------------------------------------------
// Диалоговое окно (popup) со своим текстбоксом для ввода текста.
// Наследуемся от geode::Popup — стандартный способ делать сложные
// попапы в Geode (фон, крестик закрытия и т.д. уже готовы из коробки).
// ---------------------------------------------------------------------
class TextBoxPopup : public geode::Popup {
protected:
    TextInput* m_textInput = nullptr;
    CCLabelBMFont* m_resultLabel = nullptr;

    bool init() {
        if (!Popup::init(280.f, 180.f)) {
            return false;
        }

        // Заголовок окна
        this->setTitle("Диалоговое окно");

        // Пояснительный текст
        auto hint = CCLabelBMFont::create("Введите текст ниже:", "bigFont.fnt");
        hint->setScale(0.45f);
        m_mainLayer->addChildAtPosition(hint, Anchor::Center, ccp(0, 40));

        // Сам текстбокс (однострочное поле ввода с фоном)
        m_textInput = TextInput::create(200.f, "Введите текст...");
        m_textInput->setMaxCharCount(64);
        m_mainLayer->addChildAtPosition(m_textInput, Anchor::Center, ccp(0, 10));

        // Лейбл для вывода результата (изначально пустой)
        m_resultLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_resultLabel->setScale(0.5f);
        m_mainLayer->addChildAtPosition(m_resultLabel, Anchor::Center, ccp(0, -25));

        // Кнопка подтверждения
        auto okSprite = ButtonSprite::create("OK");
        auto okBtn = CCMenuItemSpriteExtra::create(
            okSprite, this, menu_selector(TextBoxPopup::onConfirm)
        );

        auto menu = CCMenu::create();
        menu->addChild(okBtn);
        m_mainLayer->addChildAtPosition(menu, Anchor::Center, ccp(0, -55));

        return true;
    }

    void onConfirm(CCObject*) {
        auto text = m_textInput->getString();

        if (text.empty()) {
            m_resultLabel->setString("Вы ничего не ввели!");
            m_resultLabel->setColor({255, 90, 90});
        } else {
            m_resultLabel->setString(("Вы ввели: " + text).c_str());
            m_resultLabel->setColor({120, 255, 120});
        }
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
// Добавляем кнопку в главное меню, которая открывает наш попап
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
