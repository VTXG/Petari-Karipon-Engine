#include "Game/System/GameOnlinePacket.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <cstring>

GameOnlinePacket::GameOnlinePacket() {
    reset();
}

bool GameOnlinePacket::loadHeader(s32 size) {
    mPosition = 0;
    mSize = 0;
    mIsBroadcast = false;
    mIsNeedEqualStage = false;

    for (u32 i = 0; i < ARRAY_SIZE(mIsReply); i++) {
        mIsReply[i] = false;
    }

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

void GameOnlinePacket::makeHeader(u32 type) {
    mHeader.mType = type;
    mHeader.mSize = mSize;
    mHeader.mChecksum = mSize > 0 ? MR::calcCheckSum(mData, mSize) : 0;
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

void GameOnlinePacket::reset() {
    mHeader.mPacketID = INVALID_PACKET_ID;
    mPosition = 0;
    mSize = 0;
    mAddress = SockAddress();
    mIsBroadcast = false;
    mIsNeedEqualStage = false;

    for (u32 i = 0; i < ARRAY_SIZE(mIsReply); i++) {
        mIsReply[i] = false;
    }
}

GameOnlinePacketQueue::GameOnlinePacketQueue() : mPackets(), mSize(), mCapacity(), mID() {}

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

    GameOnlinePacket* pPacket = &mPackets[mSize++];
    pPacket->reset();
    pPacket->mHeader.mPacketID = mID++;
    return pPacket;
}

GameOnlinePacket* GameOnlinePacketQueue::get() {
    if (mSize == 0) {
        return nullptr;
    }

    return &mPackets[mSize - 1];
}

GameOnlinePacket* GameOnlinePacketQueue::find(u32 packetID) {
    for (u32 i = 0; i < mSize; i++) {
        if (mPackets[i].mHeader.mPacketID == packetID) {
            return &mPackets[i];
        }
    }

    return nullptr;
}

void GameOnlinePacketQueue::pop() {
    if (mSize == 0) {
        return;
    }

    mPackets[--mSize].reset();
}
