#include "Game/System/GameOnlineFunction.hpp"
#include "Game/System/GameOnlineConst.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JKernel/JKRUnitHeap.hpp>

namespace {
    static JKRUnitHeap* sGameOnlineHeap;
}

namespace GameOnlineFunction {
    JKRHeap* createGameOnlineHeap(u32 packetSize) {
        return sGameOnlineHeap = JKRUnitHeap::create(packetSize, 0x1E000, 32, JKRHeap::sSystemHeap, true);
    }

    JKRHeap* getGameOnlineHeap() {
        return sGameOnlineHeap;
    }

    void initSockAddress(SockAddress& rAddress) {
        rAddress.mLength = sizeof(SockAddress);
        rAddress.mFamily = SO_AF_INET;
    }

    void readSockAddress(MR::DataStream& rStream, SockAddress& rAddress) {
        rStream.read(&rAddress.mIP.mOctets[0]);
        rStream.read(&rAddress.mIP.mOctets[1]);
        rStream.read(&rAddress.mIP.mOctets[2]);
        rStream.read(&rAddress.mIP.mOctets[3]);
        rStream.read(&rAddress.mPort);
    }

    void writeSockAddress(MR::DataStream& rStream, const SockAddress& rAddress) {
        rStream.write(&rAddress.mIP.mOctets[0]);
        rStream.write(&rAddress.mIP.mOctets[1]);
        rStream.write(&rAddress.mIP.mOctets[2]);
        rStream.write(&rAddress.mIP.mOctets[3]);
        rStream.write(&rAddress.mPort);
    }

    bool readPacketHeader(MR::DataStream& rStream, u32* pType, OSTime* pTimestamp) {
        if (rStream.mSize < GameOnlineConst::PACKET_HEADER_SIZE) {
            return false;
        }

        u32 size;
        u32 checksum;
        rStream.read(pType);
        rStream.read(&size);
        rStream.read(&checksum);
        rStream.read(pTimestamp);

        if (size != rStream.mSize) {
            return false;
        }

        if (size > GameOnlineConst::PACKET_HEADER_SIZE) {
            u32 calcChecksum = MR::calcCheckSum(rStream.mData + GameOnlineConst::PACKET_HEADER_SIZE, size - GameOnlineConst::PACKET_HEADER_SIZE);
            if (calcChecksum != checksum) {
                OSReport("[%s:%d] Checksum mismatch, ignoring\n", __FILE__, __LINE__);
                // return false;
            }
        }

        return true;
    }

    void initPacketHeader(MR::DataStream& rStream) {
        if (rStream.mSize < GameOnlineConst::PACKET_HEADER_SIZE) {
            return;
        }

        rStream.mPosition = GameOnlineConst::PACKET_HEADER_SIZE;
    }

    bool writePacketHeader(MR::DataStream& rStream, u32 type) {
        u32 size = rStream.adjust();
        if (size < GameOnlineConst::PACKET_HEADER_SIZE) {
            return false;
        }

        u32 checksum = 0;
        if (size > GameOnlineConst::PACKET_HEADER_SIZE) {
            checksum = MR::calcCheckSum(rStream.mData + GameOnlineConst::PACKET_HEADER_SIZE, rStream.mSize - GameOnlineConst::PACKET_HEADER_SIZE);
        }

        OSTime timestamp = OSGetTime();
        rStream.mPosition = 0;
        rStream.write(&type);
        rStream.write(&rStream.mSize);
        rStream.write(&checksum);
        rStream.write(&timestamp);
        return true;
    }
} // namespace GameOnlineFunction
