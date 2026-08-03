#pragma once
#include <Geode/Geode.hpp>
#include <deque>
#include <vector>

struct GhostPoint {
    cocos2d::CCPoint pos;
    float rotation = 0.f;
    float scaleX = 1.f;
    float scaleY = 1.f;
};

class MotionBlurNode : public cocos2d::CCNode {
protected:
    PlayerObject* m_player = nullptr;
    std::deque<GhostPoint> m_history;
    std::vector<cocos2d::CCSprite*> m_ghosts;
    int m_ghostCount = 6;

    bool init(PlayerObject* player);

public:
    static MotionBlurNode* create(PlayerObject* player);

    void updateBlur(float dt);
    void reset();
};
