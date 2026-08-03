#include <Geode/Geode.hpp>
#include "MotionBlurManager.hpp"

using namespace geode::prelude;

$execute {
    // Инициализируем менеджер при загрузке мода
    MotionBlurManager::get()->init();
    
    // Загружаем стартовые значения из mod.json
    if (auto mod = Mod::get()) {
        MotionBlurManager::get()->setEnabled(mod->getSettingValue<bool>("enabled"));
        MotionBlurManager::get()->setStrength(static_cast<float>(mod->getSettingValue<double>("strength")));
    }
}
