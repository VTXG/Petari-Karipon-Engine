#pragma once

#include "Game/System/NerveExecutor.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/System/OSThreadWrapper.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/DataBuffer.hpp"
#include <revolution/os/OSMutex.h>

class LiveActor;

#define MAX_PLAYER_COUNT 4
#define MAX_PACKET_SIZE 0x400

struct GameOnlinePacketHeader {
    enum Magic {
        MAGIC_PING = 'PING',
        MAGIC_PONG = 'PONG',
        MAGIC_PLAYER_DATA = 'PLAY',
    };

    void read(JSUMemoryInputStream& rStream);
    void write(JSUMemoryOutputStream& rStream) const;

    /* 0x00 */ u32 mMagic;
    /* 0x04 */ u32 mPacketID;
    /* 0x08 */ OSTime mTimestamp;
};

class GameOnlinePlayerState {
public:
    GameOnlinePlayerState() : mLastReplyTime(), mAddress(), mActor() {
        reset();
    }

    void reset() {
        mGlobalID = -1;
        mIsActive = false;
    }

    /* 0x00 */ OSTime mLastReplyTime;
    /* 0x08 */ SockAddress mAddress;
    /* 0x10 */ LiveActor* mActor;
    /* 0x14 */ u8 mGlobalID;
    /* 0x15 */ bool mIsActive;
    /* 0x16 */ u8 _16;  // padding
    /* 0x17 */ u8 _17;  // padding
};

class GameOnlineManagerThread : public OSThreadWrapper {
public:
    GameOnlineManagerThread(int, int, JKRHeap*);

    virtual void* run();
};

class GameOnlineManager : public NerveExecutor {
public:
    GameOnlineManager();

    virtual ~GameOnlineManager() {}

    void update();
    bool recv(SockAddress* pAddress);
    GameOnlinePlayerState* getGlobalPlayer(u8 globalID);

    void exeInactive() {}
    void exeConnecting();
    void exeActive();

    /* 0x08 */ IOSFd mSocket;
    /* 0x0C */ SockAddress mServerAddress;
    /* 0x14 */ MR::AssignableArray< GameOnlinePlayerState > mPlayerStates;
    /* 0x18 */ GameOnlineManagerThread* mManagerThread;
    /* 0x1C */ OSMutex mMutex;
    /* 0x1C */ MR::DataBuffer mTemporaryBuffer;
};
