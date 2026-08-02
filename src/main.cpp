#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

// Сколько орбов нужно для доступа к редактору. Меняй тут.
constexpr int REQUIRED_ORBS = 1000;

class $modify(EditorOrbLockLayer, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool noUI) {
        // Сначала даём редактору инициализироваться штатно —
        // это безопаснее, чем прерывать init() и рисковать тем,
        // что вызывающий код не проверяет nullptr сцену.
        if (!LevelEditorLayer::init(level, noUI)) {
            return false;
        }

        // ВАЖНО: если сборка упадёт именно на следующей строке
        // с ошибкой вида "no member named 'getTotalCollectedCurrency'",
        // пришли лог — поправим точное название функции под текущий SDK.
        int orbs = GameStatsManager::sharedState()->getTotalCollectedCurrency();

        if (orbs < REQUIRED_ORBS) {
            // Откладываем на следующий кадр: сразу внутри init() менять
            // сцену/показывать алерт ненадёжно (см. Geode docs "Popup not showing up").
            Loader::get()->queueInMainThread([]() {
                // Возвращаемся в главное меню
                CCDirector::sharedDirector()->replaceScene(
                    CCTransitionFade::create(0.5f, MenuLayer::scene(false))
                );

                // Показываем причину — уже поверх главного меню
                std::string msg = "You need " + std::to_string(REQUIRED_ORBS) + " orbs to unlock editor";
                FLAlertLayer::create("Editor Locked", msg.c_str(), "OK")->show();
            });
        }

        return true;
    }
};
