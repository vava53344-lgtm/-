#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "MotionBlurNode.hpp"

using namespace geode::prelude;

class $modify(MBPlayLayer, PlayLayer) {
    struct Fields {
        MotionBlurNode* m_blurNode = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (Mod::get()->getSettingValue<bool>("enabled") && m_player1) {
            m_fields->m_blurNode = MotionBlurNode::create(m_player1);
            if (m_fields->m_blurNode) {
                // Добавляем ноду рядом с игроком, чуть ниже него по z-order,
                // чтобы призрачные копии рисовались позади реального игрока
                if (auto parent = m_player1->getParent()) {
                    parent->addChild(m_fields->m_blurNode, m_player1->getZOrder());
                }
            }
        }

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (m_fields->m_blurNode) {
            m_fields->m_blurNode->updateBlur(dt);
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        if (m_fields->m_blurNode) {
            m_fields->m_blurNode->reset();
        }
    }
};
