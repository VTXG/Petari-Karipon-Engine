#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class GameOnlineClient;
class XanimePlayer;

class OnlinePlayer : public LiveActor {
public:
    OnlinePlayer(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();

    void playAnimation(u32 animHash);
    void playAnimationSimple(const char* pAnimName);
    void setAnimationFrame(f32 frame);
    void setAnimationWeight(const f32* pWeights);

    /* 0x8C */ GameOnlineClient* mClient;
    /* 0x90 */ XanimePlayer* mXanimePlayer;
    /* 0x94 */ XanimePlayer* mXanimePlayerUpper;
    /* 0x98 */ bool mHasJetTurtle;
    /* 0x99 */ bool mIsHidden;
};
