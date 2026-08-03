#include "MotionBlurNode.hpp"

using namespace cocos2d;
using namespace geode::prelude;

MotionBlurNode* MotionBlurNode::create(PlayerObject* player) {
    auto ret = new MotionBlurNode();
    if (ret && ret->init(player)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool MotionBlurNode::init(PlayerObject* player) {
    if (!CCNode::init()) return false;
    m_player = player;

    // Кол-во "призраков" в шлейфе — можно вынести в настройки мода
    m_ghostCount = static_cast<int>(
        Mod::get()->getSettingValue<int64_t>("ghost-count")
    );
    if (m_ghostCount < 1) m_ghostCount = 1;

    auto baseSprite = player->m_playerSprite;
    if (!baseSprite) return false;

    for (int i = 0; i < m_ghostCount; i++) {
        auto ghost = CCSprite::createWithSpriteFrame(baseSprite->displayFrame());
        if (!ghost) continue;

        ghost->setOpacity(0);
        ghost->setColor(player->getColor());
        ghost->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
        ghost->setAnchorPoint(baseSprite->getAnchorPoint());

        this->addChild(ghost, -1);
        m_ghosts.push_back(ghost);
    }

    return true;
}

void MotionBlurNode::updateBlur(float dt) {
    if (!m_player || m_ghosts.empty()) return;

    GhostPoint gp;
    gp.pos = m_player->getPosition();
    gp.rotation = m_player->getRotation();
    gp.scaleX = m_player->getScaleX();
    gp.scaleY = m_player->getScaleY();

    m_history.push_front(gp);
    while (static_cast<int>(m_history.size()) > m_ghostCount) {
        m_history.pop_back();
    }

    double intensity = Mod::get()->getSettingValue<double>("intensity");
    float maxAlpha = static_cast<float>(std::clamp(intensity, 0.0, 1.0)) * 255.f;

    for (size_t i = 0; i < m_ghosts.size(); i++) {
        auto ghost = m_ghosts[i];
        if (i < m_history.size()) {
            auto& point = m_history[i];

            ghost->setPosition(this->convertToNodeSpace(point.pos));
            ghost->setRotation(point.rotation);
            ghost->setScaleX(point.scaleX);
            ghost->setScaleY(point.scaleY);

            float t = 1.f - static_cast<float>(i) / static_cast<float>(m_ghosts.size());
            ghost->setOpacity(static_cast<GLubyte>(maxAlpha * t));
        } else {
            ghost->setOpacity(0);
        }
    }
}

void MotionBlurNode::reset() {
    m_history.clear();
    for (auto ghost : m_ghosts) {
        ghost->setOpacity(0);
    }
}
