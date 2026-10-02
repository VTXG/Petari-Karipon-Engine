#include "Game/Util/IODataBuffer.hpp"

namespace MR {
    void fastCopy(void* pFrom, void* pTo, u32 size) {
        while (size != 0) {
            u32 fromAddr = reinterpret_cast< u32 >(pFrom);
            u32 toAddr = reinterpret_cast< u32 >(pTo);

            u32 blockSize;
            if (size >= 4 && (fromAddr & 0b11) == 0 && (toAddr & 0b11) == 0) {
                *static_cast< u32* >(pTo) = *static_cast< u32* >(pFrom);
                blockSize = sizeof(u32);
            } else if (size >= 2 && (fromAddr & 0b1) == 0 && (toAddr & 0b1) == 0) {
                *static_cast< u16* >(pTo) = *static_cast< u16* >(pFrom);
                blockSize = sizeof(u16);
            } else {
                *static_cast< u8* >(pTo) = *static_cast< u8* >(pFrom);
                blockSize = sizeof(u8);
            }

            pFrom = static_cast< u8* >(pFrom) + blockSize;
            pTo = static_cast< u8* >(pTo) + blockSize;
            size -= blockSize;
        }
    }
}
