#include <Geode/Geode.hpp>
#include <Geode/modify/CCScene.hpp>
#include "MotionBlurManager.hpp"

using namespace geode::prelude;

class $modify(CCScene) {
    void visit() {
        auto manager = MotionBlurManager::get();
        
        // Polling настроек каждый кадр (надёжнее слушателей)
        if (auto mod = Mod::get()) {
            manager->setEnabled(mod->getSettingValue<bool>("enabled"));
            manager->setStrength(static_cast<float>(mod->getSettingValue<double>("strength")));
        }
        
        if (!manager->isEnabled()) {
            CCScene::visit();
            return;
        }
        
        if (!manager->getCurrentRenderTexture()) {
            manager->init();
        }
        
        // Защита от рекурсии (на всякий случай)
        static bool s_isRendering = false;
        if (s_isRendering) {
            CCScene::visit();
            return;
        }
        
        s_isRendering = true;
        
        // 1. Перенаправляем рендеринг сцены в offscreen-текстуру
        auto rt = manager->getCurrentRenderTexture();
        rt->beginWithClear(0, 0, 0, 0, 0);
        CCScene::visit(); // оригинальный рендер сцены
        rt->end();
        
        // 2. Сдвигаем кольцевой буфер
        manager->advanceFrame();
        
        // 3. Рисуем на экран смесь текущего и предыдущих кадров
        manager->renderBlur();
        
        s_isRendering = false;
    }
};
