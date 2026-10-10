#include "Game/System/GameOnlineManager.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/OnlinePlayer.hpp"
#include "Game/System/GameOnlineFunction.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <cstdio>

/*

TODO
- add proper packet class with links
- disconnects
- ignore old and interpolate player data packets
- make update run asynchronously (?)
- fix BCK anims

*/

namespace {
    NEW_NERVE(NrvGameOnlineManagerDisconnected, GameOnlineManager, Disconnected);
    NEW_NERVE(NrvGameOnlineManagerConnecting, GameOnlineManager, Connecting);
    NEW_NERVE(NrvGameOnlineManagerConnected, GameOnlineManager, Connected);

    enum {
        PLAYER_LEAVE_DISCONNECT,
        PLAYER_LEAVE_KICK,
        PLAYER_LEAVE_TIMEOUT,
    };

    static const int MAX_PLAYER_NUM = 4;
    static const OSTime PING_RATE = 500 / (243000000u / 4 / 1000);
    static const OSTime PING_TIMEOUT = 3 * (243000000u / 4);

    NetworkSystemWrapper* getNetworkSystem() {
        return SingletonHolder< GameSystem >::get()->mObjHolder->mNetworkSystem;
    }

    GameSystemSceneController* getSceneController() {
        return SingletonHolder< GameSystem >::get()->mSceneController;
    }
} // namespace

GameOnlineClient::GameOnlineClient() : mActor() {
    reset();
}

void GameOnlineClient::reset() {
    mGlobalID = INVALID_ID;
    mAddress = SockAddress();
    mLastPingTime = 0;
    mLastPlayerDataTime = 0;
    mStageName[0] = '\0';
    mScenarioNo = 0;
}

void GameOnlineClient::initActor() {
    mActor = new OnlinePlayer("OnlinePlayer");
    mActor->mClient = this;
    mActor->initWithoutIter();
}

void GameOnlineClient::destroyActor() {
    if (mActor != nullptr) {
        delete mActor;
        mActor = nullptr;
    }
}

bool GameOnlineClient::isConnected() const {
    return isValid() && (OSGetTime() - mLastPingTime) < ::PING_TIMEOUT;
}

bool GameOnlineClient::isEqualCurrentStage() const {
    return isValid() && MR::isEqualStageName(mStageName) && mScenarioNo == MR::getCurrentScenarioNo();
}

GameOnlineManager::GameOnlineManager()
    : NerveExecutor("GameOnlineManager"), mSocket(-1), mServerAddress(), mRoomState(ROOM_STATE_DISCONNECTED), mClients(::MAX_PLAYER_NUM),
      mRecvPacket(), mSyncPacketQueue() {
    GameOnlineFunction::initSockAddress(mServerAddress);
    mServerAddress.mIP.mOctets[0] = 10;
    mServerAddress.mIP.mOctets[1] = 207;
    mServerAddress.mIP.mOctets[2] = 154;
    mServerAddress.mIP.mOctets[3] = 239;
    mServerAddress.mPort = 8787;

    initMemory();
    initNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnecting)); // debug NrvGameOnlineManagerDisconnected
}

void GameOnlineManager::update() {
    updateNerve();
}

void GameOnlineManager::initMemory() {
    JKRHeap* pOnlineHeap = GameOnlineFunction::createGameOnlineHeap();
    MR::CurrentHeapRestorer heapRestorer = MR::CurrentHeapRestorer(pOnlineHeap);

    mRecvPacket = new GameOnlinePacket();
    mSyncPacketQueue.init(0x20);
}

void GameOnlineManager::initActors() {
    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        pClient->initActor();
    }
}

void GameOnlineManager::destroyActors() {
    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        pClient->destroyActor();
    }
}

bool GameOnlineManager::receive(SockAddress* pAddress) {
    mRecvPacket->reset();

    IOSError ret = ::getNetworkSystem()->recv(mSocket, mRecvPacket->mBuffer, GameOnlinePacket::BUFFER_SIZE, SO_MSG_NONBLOCK, pAddress);
    if (ret < 0) {
        return false;
    }

    return mRecvPacket->loadHeader(ret);
}

void GameOnlineManager::send(GameOnlinePacket* pPacket, const SockAddress& rAddress) {
    ::getNetworkSystem()->send(mSocket, pPacket->mBuffer, pPacket->getTotalSize(), SO_MSG_NONBLOCK, rAddress);
}

void GameOnlineManager::broadcast(GameOnlinePacket* pPacket, bool isNeedEqualStage) {
    for (GameOnlineClient* pClient = getClientByLocalID(1); pClient != mClients.end(); pClient++) {
        if (!pClient->isValid()) {
            continue;
        }

        if (isNeedEqualStage && !pClient->isEqualCurrentStage()) {
            continue;
        }

        send(pPacket, pClient->mAddress);
    }
}

GameOnlineClient* GameOnlineManager::getClientByLocalID(u8 localID) {
    if (localID < mClients.size()) {
        return &mClients[localID];
    }

    return nullptr;
}

GameOnlineClient* GameOnlineManager::getClientByGlobalID(u8 globalID) {
    if (globalID == GameOnlineClient::INVALID_ID) {
        return nullptr;
    }

    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        if (pClient->mGlobalID == globalID) {
            return pClient;
        }
    }

    return nullptr;
}

GameOnlineClient* GameOnlineManager::getFreeClient() {
    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        if (!pClient->isValid()) {
            return pClient;
        }
    }

    return nullptr;
}

u8 GameOnlineManager::getFreeClientGlobalID() {
    for (u8 globalID = 0; globalID < ::MAX_PLAYER_NUM; globalID++) {
        if (getClientByGlobalID(globalID) == nullptr) {
            return globalID;
        }
    }

    return GameOnlineClient::INVALID_ID;
}

u8 GameOnlineManager::getConnectedClientNum() {
    u8 num = 0;

    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        if (pClient->isValid()) {
            num++;
        }
    }

    return num;
}

bool GameOnlineManager::isAllClientConnected() {
    for (GameOnlineClient* pClient = getClientByLocalID(1); pClient != mClients.end(); pClient++) {
        if (pClient->isValid() && !pClient->isConnected()) {
            return false;
        }
    }

    return true;
}

bool GameOnlineManager::isConnected() const {
    return isNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnected));
}

bool GameOnlineManager::requestRoomMake(const char* pCode) {
    if (!isNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnected))) {
        return false;
    }

    mSendPacket->reset();
    mSendPacket->write(pCode, strlen(pCode) + 1);
    mSendPacket->makeHeader(GameOnlinePacket::TYPE_ROOM_MAKE_REQUEST);
    send(mSendPacket, mServerAddress);
    return true;
}

bool GameOnlineManager::requestRoomJoin(const char* pCode) {
    if (!isNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnected))) {
        return false;
    }

    mSendPacket->reset();
    mSendPacket->write(pCode, strlen(pCode) + 1);
    mSendPacket->makeHeader(GameOnlinePacket::TYPE_ROOM_JOIN_REQUEST);
    send(mSendPacket, mServerAddress);
    return true;
}

void GameOnlineManager::handlePing(const SockAddress& rAddress) {
    sendPong(rAddress);
}

void GameOnlineManager::handlePong() {
    u8 globalID;
    mRecvPacket->read(&globalID);

    GameOnlineClient* pClient = getClientByGlobalID(globalID);
    if (pClient == nullptr) {
        return;
    }

    OSTime time = OSGetTime();
    if (pClient->mLastPingTime < time) {
        pClient->mLastPingTime = time;
    }
}

void GameOnlineManager::handleRoomMakeInfo() {
    GameOnlineClient* pLocalClient = getLocalClient();
    pLocalClient->mGlobalID = 0;

    GameOnlineFunction::initSockAddress(pLocalClient->mAddress);
    GameOnlineFunction::readSockAddress(mRecvPacket, pLocalClient->mAddress);

    mRoomState = ROOM_STATE_CONNECTING;
}

void GameOnlineManager::handleRoomJoinRequest() {
    if (!getLocalClient()->isHost()) {
        OSReport("[%s:%d] JOIN_REQUEST sent to non-host\n", __FILE__, __LINE__);
        return;
    }

    SockAddress address;
    GameOnlineFunction::initSockAddress(address);
    GameOnlineFunction::readSockAddress(mRecvPacket, address);
    sendRoomJoinInfo(address);
}

void GameOnlineManager::handleRoomJoinInfo() {
    u8 clientNum;
    u8 globalID;

    mRecvPacket->read(&globalID);
    mRecvPacket->read(&clientNum);

    if (mRoomState == ROOM_STATE_DISCONNECTED) {
        GameOnlineClient* pClient = getLocalClient();
        pClient->reset();
        pClient->mGlobalID = globalID;

        OSTime time = OSGetTime();
        pClient->mLastPingTime = time;
        pClient->mLastPlayerDataTime = time;
    }

    for (u8 i = 0; i < clientNum; i++) {
        mRecvPacket->read(&globalID);

        GameOnlineClient* pClient = getClientByGlobalID(globalID);
        if (pClient == nullptr) {
            pClient = getFreeClient();
            pClient->reset();
        }

        pClient->mGlobalID = globalID;
        GameOnlineFunction::initSockAddress(pClient->mAddress);
        GameOnlineFunction::readSockAddress(mRecvPacket, pClient->mAddress);
        mRecvPacket->read(pClient->mStageName, sizeof(pClient->mStageName));
        mRecvPacket->read(&pClient->mScenarioNo);
    }

    if (mRoomState == ROOM_STATE_DISCONNECTED) {
        mRoomState = ROOM_STATE_CONNECTING;
    }
}

void GameOnlineManager::handlePlayerData() {
    if (!::getSceneController()->isSceneInitializeState(SceneInitializeState_End)) {
        return;
    }

    u8 globalID;
    mRecvPacket->read(&globalID);

    GameOnlineClient* pClient = getClientByGlobalID(globalID);
    if (pClient == nullptr || pClient->mActor == nullptr || pClient->mLastPlayerDataTime > mRecvPacket->mHeader.mTimestamp) {
        return;
    }

    OnlinePlayer* pActor = pClient->mActor;
    mRecvPacket->read(&pActor->mPosition);
    mRecvPacket->read(&pActor->mRotation);
    mRecvPacket->read(&pActor->mScale);

    bool isAnimationSimple;
    mRecvPacket->read(&isAnimationSimple);

    if (isAnimationSimple) {
        char animName[32];
        mRecvPacket->read(animName, sizeof(animName));
        pActor->playAnimationSimple(animName);
    } else {
        u32 animHash;
        mRecvPacket->read(&animHash);
        pActor->playAnimation(animHash);
    }

    f32 animFrame;
    mRecvPacket->read(&animFrame);
    pActor->setAnimationFrame(animFrame);

    for (s32 i = 0; i < 4; i++) {
        f32 weights;
        mRecvPacket->read(&weights);
        pActor->mXanimePlayer->changeTrackWeight(i, weights);
    }
}

void GameOnlineManager::handlePlayerStage() {
    u8 globalID;
    mRecvPacket->read(&globalID);

    GameOnlineClient* pClient = getClientByGlobalID(globalID);
    if (pClient == nullptr) {
        return;
    }

    mRecvPacket->read(pClient->mStageName, sizeof(pClient->mStageName));
    mRecvPacket->read(&pClient->mScenarioNo);
}

void GameOnlineManager::sendPing() {
    GameOnlineClient* pLocalClient = getLocalClient();
    OSTime time = OSGetTime();

    if ((time - pLocalClient->mLastPingTime) < ::PING_RATE) {
        return;
    }

    pLocalClient->mLastPingTime = time;

    mSendPacket->reset();
    mSendPacket->makeHeader(GameOnlinePacket::TYPE_PING);
    broadcast(mSendPacket, false);
}

void GameOnlineManager::sendPong(const SockAddress& rAddress) {
    GameOnlineClient* pLocalClient = getLocalClient();

    mSendPacket->reset();
    mSendPacket->write(&pLocalClient->mGlobalID);
    mSendPacket->makeHeader(GameOnlinePacket::TYPE_PONG);
    send(mSendPacket, rAddress);
}

void GameOnlineManager::sendRoomJoinInfo(const SockAddress& rAddress) {
    u8 globalID = getFreeClientGlobalID();
    if (globalID == GameOnlineClient::INVALID_ID) {
        return;
    }

    GameOnlineClient* pClient = getFreeClient();
    pClient->reset();
    pClient->mGlobalID = globalID;
    pClient->mAddress = rAddress;

    u8 clientNum = getConnectedClientNum();
    mSendPacket->reset();
    mSendPacket->write(&globalID);
    mSendPacket->write(&clientNum);

    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        if (!pClient->isValid()) {
            continue;
        }

        mSendPacket->write(&pClient->mGlobalID);
        GameOnlineFunction::writeSockAddress(mSendPacket, pClient->mAddress);
        mSendPacket->write(pClient->mStageName, sizeof(pClient->mStageName));
        mSendPacket->write(&pClient->mScenarioNo);
    }

    mSendPacket->makeHeader(GameOnlinePacket::TYPE_ROOM_JOIN_INFO);
    broadcast(mSendPacket, false);
}

void GameOnlineManager::sendPlayerData() {
    if (!MR::isExistMario()) {
        return;
    }

    MarioActor* pMarioActor = MarioAccess::getPlayerActor();
    if (pMarioActor == nullptr) {
        return;
    }

    XanimePlayer* pMarioActorAnim = pMarioActor->mMarioAnim->mXanimePlayer;
    if (pMarioActorAnim == nullptr) {
        return;
    }

    GameOnlineClient* pLocalClient = getLocalClient();
    mSendPacket->reset();
    mSendPacket->write(&pLocalClient->mGlobalID);
    mSendPacket->write(&pMarioActor->mPosition);
    mSendPacket->write(&pMarioActor->mRotation);
    mSendPacket->write(&pMarioActor->mScale);

    bool isAnimationSimple = pMarioActorAnim->isAnimationRunSimple();
    mSendPacket->write(&isAnimationSimple);

    if (isAnimationSimple) {
        char animName[32];
        snprintf(animName, sizeof(animName), "%s", pMarioActorAnim->getCurrentBckName());
        mSendPacket->write(animName, sizeof(animName));
    } else {
        u32 animHash = MR::getHashCode(pMarioActorAnim->getCurrentAnimationName());
        mSendPacket->write(&animHash);
    }

    f32 animFrame = pMarioActorAnim->tellAnimationFrame();
    mSendPacket->write(&animFrame);

    for (s32 i = 0; i < 4; i++) {
        mSendPacket->write(&pMarioActorAnim->getCore()->mTrackList[i].mWeight);
    }

    mSendPacket->makeHeader(GameOnlinePacket::TYPE_PLAYER_DATA);
    broadcast(mSendPacket, true);
}

void GameOnlineManager::sendPlayerStage() {
    GameOnlineClient* pLocalClient = getLocalClient();
    if (pLocalClient->isEqualCurrentStage()) {
        return;
    }

    snprintf(pLocalClient->mStageName, sizeof(pLocalClient->mStageName), "%s", MR::getCurrentStageName());
    pLocalClient->mScenarioNo = MR::getCurrentScenarioNo();

    mSendPacket->reset();
    mSendPacket->write(&pLocalClient->mGlobalID);
    mSendPacket->write(pLocalClient->mStageName, sizeof(pLocalClient->mStageName));
    mSendPacket->write(&pLocalClient->mScenarioNo);

    mSendPacket->makeHeader(GameOnlinePacket::TYPE_PLAYER_STAGE);
    broadcast(mSendPacket, false);
}

void GameOnlineManager::exeDisconnected() {
    // ? do we really wanna shut down the network system orrr
    if (MR::isFirstStep(this)) {
        NetworkSystemWrapper* pNetworkSystem = ::getNetworkSystem();

        if (pNetworkSystem->isConnected()) {
            pNetworkSystem->closeSystem();
        }
    }
}

void GameOnlineManager::exeConnecting() {
    NetworkSystemWrapper* pNetworkSystem = ::getNetworkSystem();

    if (MR::isFirstStep(this)) {
        pNetworkSystem->initSystem();
    }

    if (pNetworkSystem->isConnecting()) {
        return;
    }

    if (pNetworkSystem->isConnected()) {
        setNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnected));
    } else {
        setNerve(GET_NERVE_ANON(NrvGameOnlineManagerDisconnected));
    }
}

void GameOnlineManager::exeConnected() {
    if (MR::isFirstStep(this)) {
        mSocket = ::getNetworkSystem()->socket(SO_AF_INET, SO_SOCK_DGRAM, 0);
        mRoomState = ROOM_STATE_DISCONNECTED;
    }

    sendPing();

    if (isRoomStateConnected()) {
        sendPlayerStage();
        sendPlayerData();
    }

    SockAddress address;
    while (receive(&address)) {
        switch (mRecvPacket->mHeader.mType) {
        case GameOnlinePacket::TYPE_PING:
            handlePing(address);
            break;
        case GameOnlinePacket::TYPE_PONG:
            handlePong();
            break;
        case GameOnlinePacket::TYPE_ROOM_MAKE_INFO:
            handleRoomMakeInfo();
            break;
        case GameOnlinePacket::TYPE_ROOM_JOIN_REQUEST:
            handleRoomJoinRequest();
            break;
        case GameOnlinePacket::TYPE_ROOM_JOIN_INFO:
            handleRoomJoinInfo();
            break;
        case GameOnlinePacket::TYPE_PLAYER_DATA:
            handlePlayerData();
            break;
        case GameOnlinePacket::TYPE_PLAYER_STAGE:
            handlePlayerStage();
            break;
        }
    }

    if (mRoomState == ROOM_STATE_CONNECTING && isAllClientConnected()) {
        mRoomState = ROOM_STATE_CONNECTED;
    }
}

void GameOnlineManager::exeOnEndConnected() {
    NetworkSystemWrapper* pNetworkSystem = ::getNetworkSystem();
    if (pNetworkSystem->isConnected() && mSocket >= 0) {
        pNetworkSystem->close(mSocket);
        pNetworkSystem->closeSystem();
    }

    mSocket = -1;
    mRoomState = ROOM_STATE_DISCONNECTED;
}
