#include "Game/System/GameOnlineFunction.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JKernel/JKRUnitHeap.hpp>
#include <cstring>

GameOnlinePacket::GameOnlinePacket() {
    reset();
}

bool GameOnlinePacket::loadHeader(s32 size) {
    reset();

    if (size < HEADER_SIZE) {
        return false;
    }

    mSize = size - HEADER_SIZE;

    if (mSize != mHeader.mSize) {
        return false;
    }

    if (mSize > 0) {
        u32 checksum = MR::calcCheckSum(mData, mSize);

        if (mHeader.mChecksum != checksum) {
            OSReport("[%s:%d] Packet data checksum mismatch, ignoring\n", __FILE__, __LINE__);
            // return false;
        }
    }

    return true;
}

void GameOnlinePacket::makeHeader(u32 type, u32 id) {
    mHeader.mType = type;
    mHeader.mSize = mSize;
    mHeader.mChecksum = mSize > 0 ? MR::calcCheckSum(mData, mSize) : 0;
    mHeader.mPacketID = id;
    mHeader.mTimestamp = OSGetTime();
}

void GameOnlinePacket::read(void* pData, s32 size) {
    u32 end = mPosition + size;
    if (end > mSize) {
        OSReport("[%s:%d] Read overflow\n", __FILE__, __LINE__);
        return;
    }

    memcpy(pData, mData + mPosition, size);
    mPosition = end;
}

void GameOnlinePacket::write(const void* pData, s32 size) {
    u32 end = mPosition + size;
    if (end > DATA_SIZE) {
        OSReport("[%s:%d] Write overflow\n", __FILE__, __LINE__);
        return;
    }

    memcpy(mData + mPosition, pData, size);
    mPosition = end;

    if (mSize < end) {
        mSize = end;
    }
}

GameOnlinePacketQueue::GameOnlinePacketQueue() : mPackets(), mSize(), mCapacity() {}

GameOnlinePacketQueue::~GameOnlinePacketQueue() {
    if (mPackets != nullptr) {
        delete[] mPackets;
    }
}

void GameOnlinePacketQueue::init(u32 capacity) {
    if (mPackets != nullptr) {
        delete[] mPackets;
    }

    mPackets = new GameOnlinePacket[capacity];
    mCapacity = capacity;
}

GameOnlinePacket* GameOnlinePacketQueue::push() {
    if (mSize >= mCapacity) {
        return nullptr;
    }

    return &mPackets[mSize++];
}

GameOnlinePacket* GameOnlinePacketQueue::get() {
    if (mSize == 0) {
        return nullptr;
    }

    return &mPackets[mSize - 1];
}

void GameOnlinePacketQueue::pop() {
    if (mSize == 0) {
        return;
    }

    mPackets[--mSize].reset();
}

namespace {
    static JKRUnitHeap* sGameOnlineHeap;
}

namespace GameOnlineFunction {
    JKRHeap* createGameOnlineHeap() {
        return sGameOnlineHeap = JKRUnitHeap::create(sizeof(GameOnlinePacket), 0x1E000, 32, JKRHeap::sSystemHeap, true);
    }

    JKRHeap* getGameOnlineHeap() {
        return sGameOnlineHeap;
    }

    void initSockAddress(SockAddress& rAddress) {
        rAddress.mLength = sizeof(SockAddress);
        rAddress.mFamily = SO_AF_INET;
    }

    void readSockAddress(GameOnlinePacket* pPacket, SockAddress& rAddress) {
        pPacket->read(&rAddress.mIP.mOctets[0]);
        pPacket->read(&rAddress.mIP.mOctets[1]);
        pPacket->read(&rAddress.mIP.mOctets[2]);
        pPacket->read(&rAddress.mIP.mOctets[3]);
        pPacket->read(&rAddress.mPort);
    }

    void writeSockAddress(GameOnlinePacket* pPacket, const SockAddress& rAddress) {
        pPacket->write(&rAddress.mIP.mOctets[0]);
        pPacket->write(&rAddress.mIP.mOctets[1]);
        pPacket->write(&rAddress.mIP.mOctets[2]);
        pPacket->write(&rAddress.mIP.mOctets[3]);
        pPacket->write(&rAddress.mPort);
    }
} // namespace GameOnlineFunction
