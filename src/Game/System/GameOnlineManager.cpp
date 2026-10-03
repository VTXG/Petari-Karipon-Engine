#include "Game/System/GameOnlineManager.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameOnlineFunction.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/Util/DataBuffer.hpp"
#include <JSystem/JKernel/JKRUnitHeap.hpp>

#define MAX_PLAYER_COUNT 4
#define MAX_PACKET_SIZE 0x200

namespace {
    NEW_NERVE(NrvGameOnlineManagerInactive, GameOnlineManager, Inactive);
    NEW_NERVE(NrvGameOnlineManagerConnecting, GameOnlineManager, Connecting);
    NEW_NERVE(NrvGameOnlineManagerActive, GameOnlineManager, Active);
} // namespace

void GameOnlinePacketHeader::read(MR::DataStream& rStream) {
    rStream.read(&mMagic);
    rStream.read(&mPacketID);
    rStream.read(&mTimestamp);
}

void GameOnlinePacketHeader::write(MR::DataStream& rStream) const {
    rStream.write(&mMagic);
    rStream.write(&mPacketID);
    rStream.write(&mTimestamp);
}

GameOnlineManager::GameOnlineManager()
    : NerveExecutor("GameOnlineManager"), mSocket(-1), mPlayerStates(MAX_PLAYER_COUNT), mTemporaryBuffer(), mPingTime(OSGetTime()) {
    initPacketBuffers();
    initNerve(GET_NERVE_ANON(NrvGameOnlineManagerInactive));
}

void GameOnlineManager::initPacketBuffers() {
    JKRUnitHeap* pHeap = GameOnlineFunction::createGameOnlineHeap(MAX_PACKET_SIZE);
    mTemporaryBuffer.init(MAX_PACKET_SIZE, pHeap);
}

void GameOnlineManager::update() {
    updateNerve();
}

bool GameOnlineManager::handlePacket(MR::DataStream& rInStream, SockAddress& rAddress) {
    GameOnlinePacketHeader inHeader;
    inHeader.read(rInStream);

    switch (inHeader.mMagic) {
    case GameOnlinePacketHeader::MAGIC_PING:
        handlePacketPing(rAddress);
        break;
    case GameOnlinePacketHeader::MAGIC_PONG:
        handlePacketPong(inHeader, rInStream);
        break;
    default: {
        char inMagicName[5] = {
            static_cast< char >((inHeader.mMagic >> 24) & 0xFF),
            static_cast< char >((inHeader.mMagic >> 16) & 0xFF),
            static_cast< char >((inHeader.mMagic >> 8) & 0xFF),
            static_cast< char >((inHeader.mMagic) & 0xFF),
            '\0',
        };

        OSReport("[%s:%d\n] Unknown packet '%s' size 0x%X\n", __FILE__, __LINE__, inMagicName, rInStream.mSize);
        break;
    }
    }

    return true;
}

void GameOnlineManager::handlePacketPing(SockAddress& rAddress) {
    MR::DataStream outStream(mTemporaryBuffer);

    GameOnlinePacketHeader outHeader = {GameOnlinePacketHeader::MAGIC_PONG, -1, OSGetTime()};
    outHeader.write(outStream);
    outStream.write< u8 >(&getCurrentPlayer()->mGlobalID);

    sendTo(outStream, rAddress);
}

void GameOnlineManager::handlePacketPong(GameOnlinePacketHeader& rInHeader, MR::DataStream& rInStream) {
    u8 inGlobalID;
    rInStream.read(&inGlobalID);
    getGlobalPlayer(inGlobalID)->mPingTime = rInHeader.mTimestamp;
}

void GameOnlineManager::sendTo(MR::DataStream& rStream, SockAddress& rAddress) {
    NetworkSystemWrapper::get()->send(mSocket, rStream.mBuffer, rStream.mSize, SO_MSG_NONBLOCK, rAddress);
}

void GameOnlineManager::sendAll(MR::DataStream& rStream) {
    for (GameOnlinePlayerState* pPlayerState = &mPlayerStates[1]; pPlayerState != mPlayerStates.end(); pPlayerState++) {
        if (!pPlayerState->mIsActive) {
            continue;
        }

        sendTo(rStream, pPlayerState->mAddress);
    }
}

GameOnlinePlayerState* GameOnlineManager::getGlobalPlayer(u8 globalID) {
    for (GameOnlinePlayerState* pPlayerState = mPlayerStates.begin(); pPlayerState != mPlayerStates.end(); pPlayerState++) {
        if (pPlayerState->mIsActive && pPlayerState->mGlobalID == globalID) {
            return pPlayerState;
        }
    }

    return nullptr;
}

void GameOnlineManager::exeConnecting() {}

void GameOnlineManager::exeActive() {
    while (true) {
        SockAddress address;
        s32 size = NetworkSystemWrapper::get()->recv(mSocket, mTemporaryBuffer.mBuffer, mTemporaryBuffer.mSize, SO_MSG_NONBLOCK, &address);
        if (size < 0) {
            break; // TODO: Error handling
        }

        MR::DataStream inStream(mTemporaryBuffer.mBuffer, size);
        handlePacket(inStream, address);
    }

    OSTime time = OSGetTime();

    if (OSTicksToSeconds(mPingTime - time) > 3) {
        MR::DataStream outStream(mTemporaryBuffer);

        GameOnlinePacketHeader outHeader = {GameOnlinePacketHeader::MAGIC_PING, -1, time};
        outHeader.write(outStream);

        sendAll(outStream);
        mPingTime = time;
    }
}
