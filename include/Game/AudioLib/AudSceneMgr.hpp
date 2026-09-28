#pragma once

#include "Game/Util/ByamlIter.hpp"
#include <revolution/types.h>

class JAUSectionHeap;

class AudSceneMgr {
public:
    enum PlayerMode {
        PlayerMode_Mario = 1,
        PlayerMode_Luigi = 2,
    };

    AudSceneMgr(JAUSectionHeap*);

    bool isLoadDoneSystemInit();
    void loadStaticResource();
    bool isLoadDoneStaticResource();
    void loadStageResource(const char*, const char*);
    bool isLoadDoneStageResource();
    void loadScenarioResource(const char*, const char*, s32);
    bool isLoadDoneScenarioResource();
    void loadWaveSet(const ByamlIter& rIter);
    void eraseWaveSet(const ByamlIter& rIter);
    bool isLoadDoneWaveSet(const ByamlIter& rIter) const;
    void startScene();
    bool loadPlayerResource();
    bool isPlayerResourceLoaded();
    s32 findWaveBankNo(const char* pWaveArcName) const;

    void setPlayerModeMario() {
        mPlayerMode = PlayerMode_Mario;
    }

    void setPlayerModeLuigi() {
        mPlayerMode = PlayerMode_Luigi;
    }

    bool isPlayerModeMario() {
        return mPlayerMode == PlayerMode_Mario;
    }

    bool isPlayerModeLuigi() {
        return mPlayerMode == PlayerMode_Luigi;
    }

    /* 0x00 */ JAUSectionHeap* mSectionHeap;
    /* 0x04 */ u32 _4;
    /* 0x08 */ ByamlIter mStaticResource;
    /* 0x10 */ ByamlIter mStageResource;
    /* 0x18 */ ByamlIter mWaveSetStage;
    /* 0x20 */ ByamlIter mWaveSetScenario;
    /* 0x28 */ s32 mPlayerMode;
    /* 0x2C */ s32 mPrevPlayerMode;
    /* 0x30 */ bool mIsNewPlayerMode;
    /* 0x31 */ bool _1D;
};
