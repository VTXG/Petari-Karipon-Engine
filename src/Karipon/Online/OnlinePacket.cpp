#include "Karipon/Online/OnlinePacket.hpp"
#include <cstdio>

namespace krp {
    void copyNetworkString(char* pBuff, const char* pStr, u32 size) {
        int len = snprintf(pBuff, size, "%s", pStr) + 1;
        pBuff[len >= size ? size - 1 : len] = '\0';
    }
};
