#pragma once

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
    enum {
        TYPE_PING = 0,
        TYPE_PONG,
        TYPE_ROOM_MAKE_REQUEST,
        TYPE_ROOM_MAKE_INFO,
        TYPE_ROOM_JOIN_REQUEST,
        TYPE_ROOM_JOIN_INFO,
        TYPE_PLAYER_DATA,
        TYPE_PLAYER_STAGE,
        TYPE_PLAYER_LEAVE,
    };

    static const int BUFFER_SIZE = 0x200;
    static const int HEADER_SIZE = sizeof(GameOnlinePacketHeader);
    static const int DATA_SIZE = BUFFER_SIZE - HEADER_SIZE;

    GameOnlinePacket();

    bool loadHeader(s32 size);
    void makeHeader(u32 type, u32 id = -1);

    void read(void* pData, s32 size);
    void write(const void* pData, s32 size);

    template < typename T >
    void read(T* pData) {
        read(pData, sizeof(*pData));
    }

    template < typename T >
    void write(const T* pData) {
        write(pData, sizeof(*pData));
    }

    void reset() {
        mPosition = 0;
        mSize = 0;
    }

    u32 getTotalSize() const {
        return mSize + HEADER_SIZE;
    }

    u32 mPosition;
    u32 mSize;
    union {
        struct {
            GameOnlinePacketHeader mHeader;
            u8 mData[DATA_SIZE];
        };
        u8 mBuffer[BUFFER_SIZE];
    };
};

class GameOnlinePacketQueue {
public:
    GameOnlinePacketQueue();

    ~GameOnlinePacketQueue();

    void init(u32 capacity);
    GameOnlinePacket* push();
    GameOnlinePacket* get();
    void pop();

    GameOnlinePacket* mPackets;
    u32 mSize;
    u32 mCapacity;
};

namespace GameOnlineFunction {
    JKRHeap* createGameOnlineHeap();
    JKRHeap* getGameOnlineHeap();

    void initSockAddress(SockAddress& rAddress);
    void readSockAddress(GameOnlinePacket* pPacket, SockAddress& rAddress);
    void writeSockAddress(GameOnlinePacket* pPacket, const SockAddress& rAddress);
} // namespace GameOnlineFunction
