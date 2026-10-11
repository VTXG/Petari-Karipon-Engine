#pragma once

#include "Game/System/GameOnlineConst.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include <revolution/os/OSTime.h>
#include <revolution/types.h>

class JKRHeap;
struct SockAddress;

struct GameOnlinePacketHeader {
    /* 0x00 */ u32 mType;
    /* 0x04 */ u32 mSize;
    /* 0x08 */ u32 mChecksum;
    /* 0x0C */ u32 mPacketID;
    /* 0x10 */ OSTime mTimestamp;
};

class GameOnlinePacket {
public:
    static const int INVALID_PACKET_ID = -1;

    static const int BUFFER_SIZE = 0x200;
    static const int HEADER_SIZE = sizeof(GameOnlinePacketHeader);
    static const int DATA_SIZE = BUFFER_SIZE - HEADER_SIZE;

    GameOnlinePacket();

    bool loadHeader(s32 size);
    void makeHeader(u32 type);
    void read(void* pData, s32 size);
    void write(const void* pData, s32 size);
    void reset();

    template < typename T >
    void read(T* pData) {
        read(pData, sizeof(*pData));
    }

    template < typename T >
    void write(const T* pData) {
        write(pData, sizeof(*pData));
    }

    void setTargetBroadcast(bool isNeedEqualStage) {
        mIsBroadcast = true;
        mIsNeedEqualStage = isNeedEqualStage;
    }

    void setTargetAddress(const SockAddress& rAddress) {
        mIsBroadcast = false;
        mAddress = rAddress;
    }

    u32 getTotalSize() const {
        return mSize + HEADER_SIZE;
    }

    union {
        struct {
            GameOnlinePacketHeader mHeader;
            u8 mData[DATA_SIZE];
        };
        u8 mBuffer[BUFFER_SIZE];
    };

    u32 mPosition;
    u32 mSize;
    SockAddress mAddress;
    bool mIsBroadcast;
    bool mIsNeedEqualStage;
    bool mIsReply[GameOnlineConst::MAX_PLAYER_NUM - 1];
};

class GameOnlinePacketQueue {
public:
    GameOnlinePacketQueue();

    ~GameOnlinePacketQueue();

    void init(u32 capacity);
    GameOnlinePacket* push();
    GameOnlinePacket* get();
    GameOnlinePacket* find(u32 packetID);
    void pop();
    void reset() {
        mSize = 0;
    }

    GameOnlinePacket* mPackets;
    u32 mSize;
    u32 mCapacity;
    u32 mID;
};

namespace GameOnlineFunction {
    JKRHeap* createGameOnlineHeap();
    JKRHeap* getGameOnlineHeap();

    void readSockAddress(GameOnlinePacket* pPacket, SockAddress& rAddress);
    void writeSockAddress(GameOnlinePacket* pPacket, const SockAddress& rAddress);
} // namespace GameOnlineFunction
