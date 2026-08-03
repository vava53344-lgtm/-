#include <Geode/Geode.hpp>

using namespace geode::prelude;

class MotionBlurState;

$execute {
    // Просто загружаем настройки при старте
    if (auto mod = Mod::get()) {
        // Ничего не делаем здесь, всё в CCDirector хуке
    }
}
