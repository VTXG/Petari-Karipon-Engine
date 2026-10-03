#pragma once

#include "Game/System/NerveExecutor.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/DataBuffer.hpp"
#include "revolution/os/OSTime.h"

struct GameOnlinePacketHeader {
    enum Magic {
        MAGIC_PING = 'PING',
        MAGIC_PONG = 'PONG',
        MAGIC_PLAYER_DATA = 'PLAY',
    };

    void read(MR::DataStream& rStream);
    void write(MR::DataStream& rStream) const;

    /* 0x00 */ u32 mMagic;
    /* 0x04 */ u32 mPacketID;
    /* 0x08 */ OSTime mTimestamp;
};

class GameOnlinePlayerState {
public:
    GameOnlinePlayerState() : mPingTime(), mAddress(), mActor() {
        reset();
    }

    void reset() {
        mGlobalID = -1;
        mIsActive = false;
    }

    /* 0x00 */ OSTime mPingTime;
    /* 0x08 */ SockAddress mAddress;
    /* 0x10 */ void* mActor;
    /* 0x14 */ u8 mGlobalID;
    /* 0x15 */ bool mIsActive;
    /* 0x16 */ u8 _16; // padding
    /* 0x17 */ u8 _17; // padding
};

class GameOnlineManager : public NerveExecutor {
public:
    GameOnlineManager();

    virtual ~GameOnlineManager() {}

    void initPacketBuffers();
    void update();

    bool handlePacket(MR::DataStream& rInStream, SockAddress& rAddress);
    void handlePacketPing(SockAddress& rAddress);
    void handlePacketPong(GameOnlinePacketHeader& rInHeader, MR::DataStream& rInStream);

    void sendTo(MR::DataStream& rStream, SockAddress& rAddress);
    void sendAll(MR::DataStream& rStream);

    GameOnlinePlayerState* getCurrentPlayer() { return mPlayerStates.begin(); }
    GameOnlinePlayerState* getGlobalPlayer(u8 globalID);

    void exeInactive() {}
    void exeConnecting();
    void exeActive();

    MR::DataBuffer& getTemporaryPacketBuffer() {
        return mTemporaryBuffer;
    }

    /* 0x08 */ IOSFd mSocket;
    /* 0x0C */ SockAddress mServerAddress;
    /* 0x14 */ MR::AssignableArray< GameOnlinePlayerState > mPlayerStates;
    /* 0x1C */ MR::DataBuffer mTemporaryBuffer;
    /* 0x24 */ OSTime mPingTime;
};
