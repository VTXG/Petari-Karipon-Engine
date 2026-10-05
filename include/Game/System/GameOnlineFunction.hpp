#pragma once

#include "Game/Util/DataBuffer.hpp"

struct SockAddress;

namespace GameOnlineFunction {
    JKRHeap* createGameOnlineHeap(u32 packetSize);
    JKRHeap* getGameOnlineHeap();

    void initSockAddress(SockAddress& rAddress);
    void readSockAddress(MR::DataStream& rStream, SockAddress& rAddress);
    void writeSockAddress(MR::DataStream& rStream, const SockAddress& rAddress);

    bool readPacketHeader(MR::DataStream& rStream, u32* pType, OSTime* pTimestamp);
    void initPacketHeader(MR::DataStream& rStream);
    bool writePacketHeader(MR::DataStream& rStream, u32 type);
} // namespace GameOnlineFunction
