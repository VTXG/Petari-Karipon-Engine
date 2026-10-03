#include "Game/System/GameOnlineManager.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "revolution/os.h"
#include "revolution/os/OSThread.h"
#include <JSystem/JKernel/JKRSolidHeap.hpp>

namespace {
    NEW_NERVE(NrvGameOnlineManagerInactive, GameOnlineManager, Inactive);
    NEW_NERVE(NrvGameOnlineManagerConnecting, GameOnlineManager, Connecting);
    NEW_NERVE(NrvGameOnlineManagerActive,GameOnlineManager, Active);

    JKRSolidHeap* getGameOnlineHeap() {
        return SingletonHolder< HeapMemoryWatcher >::get()->mGameOnlineHeap;
    }
}  // namespace

void GameOnlinePacketHeader::read(JSUMemoryInputStream& rStream) {
    rStream.read(&mMagic);
    rStream.read(&mPacketID);
    rStream.read(&mTimestamp);
}

void GameOnlinePacketHeader::write(JSUMemoryOutputStream& rStream) const {
    rStream.write(mMagic);
    rStream.write(mPacketID);
    rStream.write(mTimestamp);
}

GameOnlineManagerThread::GameOnlineManagerThread(int priority, int msgCount, JKRHeap* pHeap) : OSThreadWrapper(0x8000, msgCount, priority, pHeap) {
}

void* GameOnlineManagerThread::run() {
    OSInitFastCast();

    while (true) {
        OSSleepTicks(OSMillisecondsToTicks(1000/60));
        OSYieldThread();
    }
}

GameOnlineManager::GameOnlineManager()
    : NerveExecutor("GameOnlineManager"), mSocket(-1), mPlayerStates(MAX_PLAYER_COUNT), mTemporaryBuffer(MAX_PACKET_SIZE) {
    mManagerThread = new GameOnlineManagerThread(10, 1, ::getGameOnlineHeap());
    // OSResumeThread(mManagerThread->mThread);
    OSInitMutex(&mMutex);
    initNerve(GET_NERVE_ANON(NrvGameOnlineManagerInactive));
}

void GameOnlineManager::update() {
    updateNerve();
}

bool GameOnlineManager::recv(SockAddress* pAddress) {
    s32 inSize = NetworkSystemWrapper::get()->recv(mSocket, mTemporaryBuffer.mBuffer, mTemporaryBuffer.mSize, SO_MSG_NONBLOCK, pAddress);
    if (inSize < 0) {
        return false; // TODO: Error handling
    }

    JSUMemoryInputStream inStream = mTemporaryBuffer.createInputStream();
    GameOnlinePacketHeader inHeader;
    inHeader.read(inStream);

    switch (inHeader.mMagic) {
    case GameOnlinePacketHeader::MAGIC_PING: {
        GameOnlinePacketHeader outHeader = {GameOnlinePacketHeader::MAGIC_PONG, -1, OSGetTime()};
        JSUMemoryOutputStream outStream = mTemporaryBuffer.createOutputStream();
        outHeader.write(outStream);
        break;
    }
    case GameOnlinePacketHeader::MAGIC_PONG: {
        u8 inGlobalID;
        inStream.read(&inGlobalID);
        getGlobalPlayer(inGlobalID)->mLastReplyTime = inHeader.mTimestamp;
        break;
    }
    default:
        char inMagicName[5] = {
            static_cast< char >((inHeader.mMagic >> 24) & 0xFF),
            static_cast< char >((inHeader.mMagic >> 16) & 0xFF),
            static_cast< char >((inHeader.mMagic >> 8) & 0xFF),
            static_cast< char >((inHeader.mMagic) & 0xFF),
            '\0',
        };

        OSReport("[%s:%d\n] Unknown packet '%s' size 0x%X\n", __FILE__, __LINE__, inMagicName, inSize);
        break;
    }

    return true;
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
    while (recv(&mServerAddress)) {
    }

    for (u8 i = 1; i < mPlayerStates.size(); i++) {
        while (recv(&mPlayerStates[i].mAddress)) {
        }
    }
}
