#pragma once

#include <revolution/os/OSTime.h>
#include <revolution/types.h>

namespace GameOnlineConst {
    enum {
        PACKET_PING,
        PACKET_PONG,
        PACKET_ROOM_MAKE_REQUEST,
        PACKET_ROOM_MAKE_INFO,
        PACKET_ROOM_JOIN_REQUEST,
        PACKET_ROOM_JOIN_INFO,
        PACKET_PLAYER_DATA,
    };

    static const int MAX_PLAYER_NUM = 4;
    static const int INVALID_PLAYER_ID = 0xFF;

    static const int PACKET_HEADER_SIZE = (sizeof(u32) * 3 + sizeof(OSTime) * 1);
    static const int PACKET_SIZE = 0x200;
} // namespace GameOnlineConst
