#pragma once

#include <revolution/types.h>

class JKRUnitHeap;

namespace GameOnlineFunction {
    JKRUnitHeap* createGameOnlineHeap(u32 packetSize);
    JKRUnitHeap* getGameOnlineHeap();
}
