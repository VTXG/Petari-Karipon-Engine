#include "Game/System/GameOnlineManager.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/OnlinePlayer.hpp"
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
#include <JSystem/JKernel/JKRUnitHeap.hpp>
#include <cstdio>

/*

TODO
- disconnects
- ignore old and interpolate player data packets

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

    static const OSTime PING_RATE = 500 * (243000000u / 4 / 1000);
    static const OSTime PING_TIMEOUT = 3000 * (243000000u / 4 / 1000);

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
    mActor = new OnlinePlayer("OnlinePlayer", this);
    mActor->initWithoutIter();
}

void GameOnlineClient::destroyActor() {
    if (mActor != nullptr) {
        delete mActor;
        mActor = nullptr;
    }
}

void GameOnlineClient::updateCurrentStage() {
    snprintf(mStageName, sizeof(mStageName), "%s", MR::getCurrentStageName());
    mScenarioNo = MR::getCurrentScenarioNo();
}

bool GameOnlineClient::isTimeout() const {
    return (OSGetTime() - mLastPingTime) > PING_TIMEOUT;
}

bool GameOnlineClient::isEqualCurrentStage() const {
    return isValid() && MR::isEqualStageName(mStageName) && mScenarioNo == MR::getCurrentScenarioNo();
}

GameOnlineManager::GameOnlineManager()
    : NerveExecutor("GameOnlineManager"), mOnlineHeap(), mServerAddress(), mSocket(-1), mRoomState(ROOM_STATE_DISCONNECTED),
      mClients(GameOnlineClient::MAX_PLAYER_NUM), mRecvPacket(), mSendPacket(), mSyncPacketQueue() {
    mServerAddress.mIP.mOctets[0] = 10;
    mServerAddress.mIP.mOctets[1] = 193;
    mServerAddress.mIP.mOctets[2] = 56;
    mServerAddress.mIP.mOctets[3] = 239;
    mServerAddress.mPort = 8787;

    initMemory();
    initNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnecting)); // debug NrvGameOnlineManagerDisconnected
}

void GameOnlineManager::update() {
    updateNerve();
}

void GameOnlineManager::initMemory() {
    mOnlineHeap = JKRUnitHeap::create(sizeof(GameOnlinePacket), 0x1E000, 32, JKRHeap::sSystemHeap, true);
    MR::CurrentHeapRestorer heapRestorer = MR::CurrentHeapRestorer(mOnlineHeap);

    mRecvPacket = new GameOnlinePacket();
    mSendPacket = new GameOnlinePacket();
    mSyncPacketQueue.init(0x20);
}

void GameOnlineManager::initActors() {
    for (GameOnlineClient* pClient = getClientByLocalID(1); pClient != mClients.end(); pClient++) {
        pClient->initActor();
    }
}

void GameOnlineManager::destroyActors() {
    for (GameOnlineClient* pClient = getClientByLocalID(1); pClient != mClients.end(); pClient++) {
        pClient->destroyActor();
    }
}

bool GameOnlineManager::receive() {
    mRecvPacket->reset();

    IOSError ret = ::getNetworkSystem()->recv(mSocket, mRecvPacket->mBuffer, GameOnlinePacket::BUFFER_SIZE, SO_MSG_NONBLOCK, &mRecvPacket->mAddress);
    if (ret < 0) {
        return false;
    }

    return mRecvPacket->loadHeader(ret);
}

void GameOnlineManager::send(GameOnlinePacket* pPacket) {
    NetworkSystemWrapper* pNetworkSystem = ::getNetworkSystem();
    pPacket->mHeader.mTimestamp = OSGetTime();

    if (!pPacket->mIsBroadcast) {
        pNetworkSystem->send(mSocket, pPacket->mBuffer, pPacket->getTotalSize(), SO_MSG_NONBLOCK, pPacket->mAddress);
        return;
    }

    for (GameOnlineClient* pClient = getClientByLocalID(1); pClient != mClients.end(); pClient++) {
        if (!pClient->isValid()) {
            continue;
        }

        if (pPacket->mIsNeedEqualStage && !pClient->isEqualCurrentStage()) {
            continue;
        }

        pNetworkSystem->send(mSocket, pPacket->mBuffer, pPacket->getTotalSize(), SO_MSG_NONBLOCK, pClient->mAddress);
    }
}

void GameOnlineManager::updateSyncPackets() {
    NetworkSystemWrapper* pNetworkSystem = ::getNetworkSystem();
    GameOnlinePacket* pPacket;

    while ((pPacket = mSyncPacketQueue.get()) != nullptr) {
        OSTime time = OSGetTime();

        if ((time - pPacket->mHeader.mTimestamp) < PING_RATE) {
            break;
        }

        pPacket->mHeader.mTimestamp = time;

        bool isPop = true;

        if (pPacket->mIsBroadcast) {
            for (u8 localID = 1; localID < GameOnlineClient::MAX_PLAYER_NUM; localID++) {
                GameOnlineClient* pClient = getClientByLocalID(localID);

                if (!pClient->isValid()) {
                    continue;
                }

                if (pPacket->mIsNeedEqualStage && !pClient->isEqualCurrentStage()) {
                    continue;
                }

                if (pPacket->testReplyState(localID)) {
                    continue;
                }

                isPop = false;
                pNetworkSystem->send(mSocket, pPacket->mBuffer, pPacket->getTotalSize(), SO_MSG_NONBLOCK, pClient->mAddress);
            }
        } else {
            u8 localID = getLocalIDByAddress(pPacket->mAddress);

            if (localID != GameOnlineClient::INVALID_ID && !pPacket->testReplyState(localID)) {
                isPop = false;
                pNetworkSystem->send(mSocket, pPacket->mBuffer, pPacket->getTotalSize(), SO_MSG_NONBLOCK, pPacket->mAddress);
            }
        }

        if (!isPop) {
            break;
        }

        mSyncPacketQueue.pop();
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
            pClient->reset();
            return pClient;
        }
    }

    return nullptr;
}

u8 GameOnlineManager::getLocalIDByGlobalID(u8 globalID) {
    GameOnlineClient* pClient = getClientByGlobalID(globalID);

    if (pClient != nullptr) {
        return pClient - mClients.mArr;
    }

    return GameOnlineClient::INVALID_ID;
}

u8 GameOnlineManager::getLocalIDByAddress(const SockAddress& rAddress) {
    for (u8 localID = 0; localID < GameOnlineClient::MAX_PLAYER_NUM; localID++) {
        GameOnlineClient* pClient = getClientByLocalID(localID);

        if (!pClient->isValid()) {
            continue;
        }

        if (pClient->mAddress == rAddress) {
            return localID;
        }
    }

    return GameOnlineClient::INVALID_ID;
}

u8 GameOnlineManager::getFreeClientGlobalID() {
    for (u8 globalID = 0; globalID < GameOnlineClient::MAX_PLAYER_NUM; globalID++) {
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
        if (pClient->isValid() && pClient->isTimeout()) {
            return false;
        }
    }

    return true;
}

bool GameOnlineManager::isConnected() const {
    return isNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnected));
}

bool GameOnlineManager::requestRoomMake(const char* pCode) {
    if (!isConnected()) {
        return false;
    }

    mSendPacket->reset();
    mSendPacket->write(pCode, strlen(pCode) + 1);
    mSendPacket->makeHeader(GameOnlinePacket::TYPE_ROOM_MAKE_REQUEST);
    mSendPacket->setTargetAddress(mServerAddress);
    send(mSendPacket);
    return true;
}

bool GameOnlineManager::requestRoomJoin(const char* pCode) {
    if (!isConnected()) {
        return false;
    }

    mSendPacket->reset();
    mSendPacket->write(pCode, strlen(pCode) + 1);
    mSendPacket->makeHeader(GameOnlinePacket::TYPE_ROOM_JOIN_REQUEST);
    mSendPacket->setTargetAddress(mServerAddress);
    send(mSendPacket);
    return true;
}

void GameOnlineManager::handlePing() {
    sendPong(mRecvPacket->mAddress);
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

void GameOnlineManager::handleReply() {
    u8 globalID;
    u32 packetID;
    mRecvPacket->read(&globalID);
    mRecvPacket->read(&packetID);

    u8 localID = getLocalIDByGlobalID(globalID);
    if (localID == GameOnlineClient::INVALID_ID) {
        return;
    }

    GameOnlinePacket* pPacket = mSyncPacketQueue.find(packetID);
    if (pPacket == nullptr) {
        return;
    }

    pPacket->onReplyState(localID);
}

void GameOnlineManager::handleRoomMakeInfo() {
    if (mRoomState != ROOM_STATE_DISCONNECTED) {
        return;
    }

    GameOnlineClient* pLocalClient = getLocalClient();
    pLocalClient->mGlobalID = 0;
    GameOnlineFunction::readSockAddress(mRecvPacket, pLocalClient->mAddress);
    pLocalClient->updateCurrentStage();

    OSTime time = OSGetTime();
    pLocalClient->mLastPingTime = time;
    pLocalClient->mLastPlayerDataTime = time;

    mRoomState = ROOM_STATE_CONNECTING;
}

void GameOnlineManager::handleRoomJoinRequest() {
    if (mRoomState != ROOM_STATE_CONNECTED) {
        return;
    }

    if (!getLocalClient()->isHost()) {
        OSReport("[%s:%d] JOIN_REQUEST sent to non-host\n", __FILE__, __LINE__);
        return;
    }

    SockAddress address;
    GameOnlineFunction::readSockAddress(mRecvPacket, address);
    sendRoomJoinInfo(address);
}

void GameOnlineManager::handleRoomJoinInfo() {
    if (mRoomState != ROOM_STATE_DISCONNECTED) {
        return;
    }

    GameOnlineClient* pLocalClient = getLocalClient();
    pLocalClient->reset();

    mRecvPacket->read(&pLocalClient->mGlobalID);
    GameOnlineFunction::readSockAddress(mRecvPacket, pLocalClient->mAddress);

    OSTime time = OSGetTime();
    pLocalClient->mLastPingTime = time;
    pLocalClient->mLastPlayerDataTime = time;

    u8 clientNum;
    mRecvPacket->read(&clientNum);

    for (u8 i = 0; i < clientNum; i++) {
        GameOnlineClient* pClient = getFreeClient();
        mRecvPacket->read(&pClient->mGlobalID);
        GameOnlineFunction::readSockAddress(mRecvPacket, pClient->mAddress);
        mRecvPacket->read(pClient->mStageName, sizeof(pClient->mStageName));
        mRecvPacket->read(&pClient->mScenarioNo);
    }

    pLocalClient->updateCurrentStage();

    GameOnlinePacket* pPacket = mSyncPacketQueue.push();
    pPacket->write(&pLocalClient->mGlobalID);
    GameOnlineFunction::writeSockAddress(pPacket, pLocalClient->mAddress);
    pPacket->write(pLocalClient->mStageName, sizeof(pLocalClient->mStageName));
    pPacket->write(&pLocalClient->mScenarioNo);
    pPacket->makeHeader(GameOnlinePacket::TYPE_PLAYER_JOIN);
    pPacket->setTargetBroadcast(false);

    mRoomState = ROOM_STATE_CONNECTING;
}

void GameOnlineManager::handlePlayerJoin() {
    u8 globalID;
    mRecvPacket->read(&globalID);

    if (getClientByGlobalID(globalID) != nullptr) {
        return;
    }

    GameOnlineClient* pClient = getFreeClient();
    pClient->mGlobalID = globalID;
    GameOnlineFunction::readSockAddress(mRecvPacket, pClient->mAddress);
    mRecvPacket->read(pClient->mStageName, sizeof(pClient->mStageName));
    mRecvPacket->read(&pClient->mScenarioNo);
}

void GameOnlineManager::handlePlayerLeave() {
    u8 globalID;
    u8 leaveReason;
    mRecvPacket->read(&globalID);
    mRecvPacket->read(&leaveReason);
    GameOnlineClient* pClient = getClientByGlobalID(globalID);

    if (pClient == nullptr) {
        return;
    }

    pClient->reset();
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
        pActor->setAnimationWeight(i, weights);
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

    if ((time - pLocalClient->mLastPingTime) < PING_RATE) {
        return;
    }

    pLocalClient->mLastPingTime = time;

    mSendPacket->reset();
    mSendPacket->makeHeader(GameOnlinePacket::TYPE_PING);
    mSendPacket->setTargetBroadcast(false);
    send(mSendPacket);
}

void GameOnlineManager::sendPong(const SockAddress& rAddress) {
    GameOnlineClient* pLocalClient = getLocalClient();

    mSendPacket->reset();
    mSendPacket->write(&pLocalClient->mGlobalID);
    mSendPacket->makeHeader(GameOnlinePacket::TYPE_PONG);
    mSendPacket->setTargetAddress(rAddress);
    send(mSendPacket);
}

void GameOnlineManager::sendReply(u32 packetID, const SockAddress& rAddress) {
    mSendPacket->reset();
    mSendPacket->write(&getLocalClient()->mGlobalID);
    mSendPacket->write(&packetID);
    mSendPacket->makeHeader(GameOnlinePacket::TYPE_REPLY);
    mSendPacket->setTargetAddress(rAddress);
    send(mSendPacket);
}

void GameOnlineManager::sendRoomJoinInfo(const SockAddress& rAddress) {
    if (getLocalIDByAddress(rAddress) != GameOnlineClient::INVALID_ID) {
        OSReport("[%s:%d] Duplicate join request\n", __FILE__, __LINE__);
        return;
    }

    u8 globalID = getFreeClientGlobalID();
    if (globalID == GameOnlineClient::INVALID_ID) {
        return;
    }

    GameOnlinePacket* pPacket = mSyncPacketQueue.push();
    pPacket->write(&globalID);
    GameOnlineFunction::writeSockAddress(pPacket, rAddress);

    u8 clientNum = getConnectedClientNum();
    pPacket->write(&clientNum);

    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        if (!pClient->isValid()) {
            continue;
        }

        pPacket->write(&pClient->mGlobalID);
        GameOnlineFunction::writeSockAddress(pPacket, pClient->mAddress);
        pPacket->write(pClient->mStageName, sizeof(pClient->mStageName));
        pPacket->write(&pClient->mScenarioNo);
    }

    pPacket->makeHeader(GameOnlinePacket::TYPE_ROOM_JOIN_INFO);
    pPacket->setTargetAddress(rAddress);

    GameOnlineClient* pClient = getFreeClient();
    pClient->mGlobalID = globalID;
    pClient->mAddress = rAddress;
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
    mSendPacket->setTargetBroadcast(true);
    send(mSendPacket);
}

void GameOnlineManager::sendPlayerStage() {
    GameOnlineClient* pLocalClient = getLocalClient();
    if (pLocalClient->isEqualCurrentStage()) {
        return;
    }

    pLocalClient->updateCurrentStage();

    GameOnlinePacket* pPacket = mSyncPacketQueue.push();
    pPacket->write(&pLocalClient->mGlobalID);
    pPacket->write(pLocalClient->mStageName, sizeof(pLocalClient->mStageName));
    pPacket->write(&pLocalClient->mScenarioNo);
    pPacket->makeHeader(GameOnlinePacket::TYPE_PLAYER_STAGE);
    pPacket->setTargetBroadcast(false);
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
        mSyncPacketQueue.reset();
    }

    sendPing();

    if (mRoomState == ROOM_STATE_CONNECTED) {
        sendPlayerStage();
        sendPlayerData();
        updateSyncPackets();
    }

    while (receive()) {
        SockAddress address = mRecvPacket->mAddress;
        u32 packetID = mRecvPacket->mHeader.mPacketID;

        switch (mRecvPacket->mHeader.mType) {
        case GameOnlinePacket::TYPE_PING:
            handlePing();
            break;
        case GameOnlinePacket::TYPE_PONG:
            handlePong();
            break;
        case GameOnlinePacket::TYPE_REPLY:
            handleReply();
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
        case GameOnlinePacket::TYPE_PLAYER_JOIN:
            handlePlayerJoin();
            break;
        case GameOnlinePacket::TYPE_PLAYER_LEAVE:
            handlePlayerLeave();
            break;
        case GameOnlinePacket::TYPE_PLAYER_DATA:
            handlePlayerData();
            break;
        case GameOnlinePacket::TYPE_PLAYER_STAGE:
            handlePlayerStage();
            break;
        }

        if (packetID != GameOnlinePacket::INVALID_PACKET_ID) {
            sendReply(packetID, address);
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
    mSyncPacketQueue.reset();
}

namespace GameOnlineFunction {
    void readSockAddress(GameOnlinePacket* pPacket, SockAddress& rAddress) {
        rAddress = SockAddress();
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
