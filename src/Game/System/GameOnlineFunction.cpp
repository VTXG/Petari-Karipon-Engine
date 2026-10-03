#include "Game/System/GameOnlineFunction.hpp"
#include <JSystem/JKernel/JKRUnitHeap.hpp>

namespace {
    static JKRUnitHeap* sGameOnlineHeap;
}

namespace GameOnlineFunction {
    JKRUnitHeap* createGameOnlineHeap(u32 packetSize) {
        return sGameOnlineHeap = JKRUnitHeap::create(packetSize, 0x1E000, 32, JKRHeap::sSystemHeap, true);
    }

    JKRUnitHeap* getGameOnlineHeap() {
        return sGameOnlineHeap;
    }
} // namespace GameOnlineFunction
