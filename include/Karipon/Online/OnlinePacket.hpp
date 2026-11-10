#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/os/OSTime.h>
#include <revolution/types.h>

namespace krp {
    static void copyNetworkString(char* pBuff, const char* pStr, u32 size);

    enum OnlinePacketType {
        OnlinePacketType_Ping,
        OnlinePacketType_Pong,

        OnlinePacketType_RoomJoin,
        OnlinePacketType_RoomCreate,
        OnlinePacketType_RoomInfo,

        OnlinePacketType_PlayerJoin,
        OnlinePacketType_PlayerLeave,
        OnlinePacketType_PlayerData,
        OnlinePacketType_PlayerStage,
    };

    template < u32 Size >
    struct OnlineString {
        void set(const char* pStr) {
            copyNetworkString(mData, pStr, sizeof(mData));
        }

        char mData[Size];
    };

    struct OnlinePacket {
        /* 0x00 */ u32 mHash;
        /* 0x04 */ u32 mType;
        /* 0x08 */ OSTime mTimestamp;
        /* 0x10 */ u8 mPlayerID;
        /* 0x11 */ u8 mReserved[3];
    };

    struct OnlinePacketPlayerData : OnlinePacket {
        /* 0x14 */ TVec3f mPosition;
        /* 0x18 */ TVec3f mRotation;
        /* 0x1C */ TVec3f mScale;

        union {
            /* 0x20 */ u32 mAnimNameHash;
            /* 0x20 */ OnlineString< 32 > mAnimName;
        };

        /* 0x40 */ 
    };
}; // namespace krp
