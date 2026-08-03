#include <Geode/Geode.hpp>
#include "MotionBlurManager.hpp"

using namespace geode::prelude;

$execute {
    if (auto mod = Mod::get()) {
        MotionBlurState::get()->m_enabled = mod->getSettingValue<bool>("enabled");
        MotionBlurState::get()->m_strength = static_cast<float>(mod->getSettingValue<double>("strength"));
    }
}
