#pragma once

#include <revolution/types.h>

namespace GameOnlineConst {
    static const int MAX_PLAYER_NUM = 4;

    enum {
        TYPE_PING = 0,
        TYPE_PONG,
        TYPE_REPLY,
        TYPE_ROOM_MAKE_REQUEST,
        TYPE_ROOM_MAKE_INFO,
        TYPE_ROOM_JOIN_REQUEST,
        TYPE_ROOM_JOIN_INFO,
        TYPE_PLAYER_JOIN,
        TYPE_PLAYER_LEAVE,
        TYPE_PLAYER_DATA,
        TYPE_PLAYER_STAGE,
    };
} // namespace GameOnlineConst
