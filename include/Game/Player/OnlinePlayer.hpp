#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class XanimePlayer;
class XanimeResourceTable;

class OnlinePlayer : public LiveActor {
public:
    OnlinePlayer(const char* pName);

    virtual void init(const JMapInfoIter& rIter);

    void playAnimation(u32 animHash);
    void playAnimationSimple(const char* pAnimName);
    void setAnimationFrame(f32 frame);
    void setAnimationWeight(const f32* pWeights);

    /* 0x8C */ XanimeResourceTable* mResourceTable;
    /* 0x90 */ XanimePlayer* mXanimePlayer;
    /* 0x94 */ XanimePlayer* mXanimePlayerUpper;
    /* 0x98 */ bool mHasJetTurtle;
};
