#pragma once

// TODO: Update

#ifdef KRP_ONLINE

#include <Game/LiveActor/LiveActor.hpp>

class GameOnlineClient;
class XanimePlayer;

class OnlinePlayer : public LiveActor {
public:
    OnlinePlayer(const char* pName, GameOnlineClient* pClient);

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();

    void playAnimation(u32 animHash);
    void playAnimationSimple(const char* pAnimName);
    void setAnimationFrame(f32 frame);
    void setAnimationWeight(u32 track, f32 weight);
    void updateVisibility();

    /* 0x8C */ GameOnlineClient* mClient;
    /* 0x90 */ XanimePlayer* mXanimePlayer;
    /* 0x94 */ XanimePlayer* mXanimePlayerUpper;
    /* 0x98 */ bool mHasJetTurtle;
    /* 0x99 */ bool mIsVisible;
};

#endif
