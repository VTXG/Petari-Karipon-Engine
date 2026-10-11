#pragma once

#include "Game/System/NetworkSystemWrapper.hpp"
#include <revolution/os/OSTime.h>
#include <revolution/types.h>

class OnlinePlayer;

class GameOnlineClient {
public:
    static const int MAX_PLAYER_NUM = 4;
    static const int INVALID_ID = 0xFF;

    GameOnlineClient();

    void reset();
    void initActor();
    void destroyActor();
    void updateCurrentStage();

    bool isTimeout() const;
    bool isEqualCurrentStage() const;

    bool isHost() const {
        return mGlobalID == 0;
    }

    bool isValid() const {
        return mGlobalID != INVALID_ID;
    }

    void exeDisconnected();
    void exeConnecting();
    void exeConnected();

    u8 mGlobalID;
    SockAddress mAddress;
    OnlinePlayer* mActor;
    OSTime mLastPingTime;
    OSTime mLastPlayerDataTime;
    char mStageName[32];
    s32 mScenarioNo;
};
