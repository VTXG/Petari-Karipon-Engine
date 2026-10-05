#include "Game/System/GameOnlineManager.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/OnlinePlayer.hpp"
#include "Game/System/GameOnlineConst.hpp"
#include "Game/System/GameOnlineFunction.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/Util/DataBuffer.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <cstdio>

namespace {
    NEW_NERVE(NrvGameOnlineManagerDisconnected, GameOnlineManager, Disconnected);
    NEW_NERVE(NrvGameOnlineManagerConnecting, GameOnlineManager, Connecting);
    NEW_NERVE(NrvGameOnlineManagerConnected, GameOnlineManager, Connected);
    NEW_NERVE(NrvGameOnlineManagerConnectedInRoom, GameOnlineManager, ConnectedInRoom);

    NetworkSystemWrapper* getNetworkSystem() {
        return SingletonHolder< GameSystem >::get()->mObjHolder->mNetworkSystem;
    }

    GameSystemSceneController* getSceneController() {
        return SingletonHolder< GameSystem >::get()->mSceneController;
    }
} // namespace

GameOnlineClient::GameOnlineClient() : mGlobalID(GameOnlineConst::INVALID_PLAYER_ID), mAddress(), mActor(), mLastPingTime(), mLastPlayerDataTime(), mScenarioNo() {
    mStageName[0] = '\0';
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
    return isValid() && (OSGetTime() - mLastPingTime) < GameOnlineConst::PING_TIMEOUT;
}

bool GameOnlineClient::isEqualCurrentStage() const {
    return isValid() && MR::isEqualStageName(mStageName) && mScenarioNo == MR::getCurrentScenarioNo();
}

GameOnlineManager::GameOnlineManager()
    : NerveExecutor("GameOnlineManager"), mSocket(-1), mServerAddress(), mRoomState(ROOM_STATE_DISCONNECTED),
      mClients(GameOnlineConst::MAX_PLAYER_NUM), mTemporaryBuffer() {
    JKRHeap* pHeap = GameOnlineFunction::createGameOnlineHeap(GameOnlineConst::PACKET_SIZE);
    mTemporaryBuffer.init(GameOnlineConst::PACKET_SIZE, pHeap);

    GameOnlineFunction::initSockAddress(mServerAddress);
    mServerAddress.mIP.mOctets[0] = 10;
    mServerAddress.mIP.mOctets[1] = 207;
    mServerAddress.mIP.mOctets[2] = 154;
    mServerAddress.mIP.mOctets[3] = 239;
    mServerAddress.mPort = 8787;

    initNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnecting)); // debug NrvGameOnlineManagerDisconnected
}

void GameOnlineManager::update() {
    updateNerve();
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

bool GameOnlineManager::receive(MR::DataBuffer& rBuffer, s32* pSize, SockAddress* pAddress) {
    IOSError ret = ::getNetworkSystem()->recv(mSocket, rBuffer.mData, rBuffer.mSize, SO_MSG_NONBLOCK, pAddress);

    if (ret < 0) {
        return false;
    }

    *pSize = ret;
    return true;
}

void GameOnlineManager::send(MR::DataStream& rStream, const SockAddress& rAddress) {
    ::getNetworkSystem()->send(mSocket, rStream.mData, rStream.mSize, SO_MSG_NONBLOCK, rAddress);
}

void GameOnlineManager::broadcast(MR::DataStream& rStream, bool isNeedEqualStage) {
    for (GameOnlineClient* pClient = getClientByLocalID(1); pClient != mClients.end(); pClient++) {
        if (!pClient->isValid()) {
            continue;
        }

        if (isNeedEqualStage && !pClient->isEqualCurrentStage()) {
            continue;
        }

        send(rStream, pClient->mAddress);
    }
}

GameOnlineClient* GameOnlineManager::getClientByLocalID(u8 localID) {
    if (localID < mClients.size()) {
        return &mClients[localID];
    }

    return nullptr;
}

GameOnlineClient* GameOnlineManager::getClientByGlobalID(u8 globalID) {
    if (globalID == GameOnlineConst::INVALID_PLAYER_ID) {
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
    for (u8 globalID = 0; globalID < GameOnlineConst::MAX_PLAYER_NUM; globalID++) {
        if (getClientByGlobalID(globalID) == nullptr) {
            return globalID;
        }
    }

    return GameOnlineConst::INVALID_PLAYER_ID;
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

bool GameOnlineManager::requestRoomMake(const char* pCode) {
    if (!isNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnected))) {
        return false;
    }

    MR::DataStream stream(mTemporaryBuffer);
    GameOnlineFunction::initPacketHeader(stream);

    stream.writeData(pCode, strlen(pCode) + 1);

    GameOnlineFunction::writePacketHeader(stream, GameOnlineConst::PACKET_ROOM_MAKE_REQUEST);
    send(stream, mServerAddress);
    return true;
}

bool GameOnlineManager::requestRoomJoin(const char* pCode) {
    if (!isNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnected))) {
        return false;
    }

    MR::DataStream stream(mTemporaryBuffer);
    GameOnlineFunction::initPacketHeader(stream);

    stream.writeData(pCode, strlen(pCode) + 1);

    GameOnlineFunction::writePacketHeader(stream, GameOnlineConst::PACKET_ROOM_JOIN_REQUEST);
    send(stream, mServerAddress);
    return true;
}

bool GameOnlineManager::isConnected() const {
    return isNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnected));
}

bool GameOnlineManager::isConnectedInRoom() const {
    return isNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnectedInRoom));
}

void GameOnlineManager::handlePing(const SockAddress& rAddress) {
    sendPong(rAddress);
}

void GameOnlineManager::handlePong(MR::DataStream& rStream) {
    u8 globalID;
    rStream.read(&globalID);

    GameOnlineClient* pClient = getClientByGlobalID(globalID);
    if (pClient == nullptr) {
        return;
    }

    OSTime time = OSGetTime();
    if (pClient->mLastPingTime < time) {
        pClient->mLastPingTime = time;
    }
}

void GameOnlineManager::handleRoomMakeInfo(MR::DataStream& rStream) {
    GameOnlineClient* pLocalClient = getLocalClient();
    pLocalClient->mGlobalID = 0;

    GameOnlineFunction::initSockAddress(pLocalClient->mAddress);
    GameOnlineFunction::readSockAddress(rStream, pLocalClient->mAddress);

    mRoomState = ROOM_STATE_CONNECTING;
}

void GameOnlineManager::handleRoomJoinRequest(MR::DataStream& rStream) {
    if (!getLocalClient()->isHost()) {
        OSReport("[%s:%d] JOIN_REQUEST sent to non-host\n", __FILE__, __LINE__);
        return;
    }

    SockAddress address;
    GameOnlineFunction::initSockAddress(address);
    GameOnlineFunction::readSockAddress(rStream, address);
    sendRoomJoinInfo(address);
}

void GameOnlineManager::handleRoomJoinInfo(MR::DataStream& rStream) {
    u8 clientNum;
    u8 globalID;
    
    rStream.read(&globalID);
    rStream.read(&clientNum);

    OSReport("[%s:%d] globalID=%d clientNum=%d\n", __FILE__, __LINE__, globalID, clientNum);

    if (mRoomState == ROOM_STATE_DISCONNECTED) {
        getLocalClient()->mGlobalID = globalID;
        OSReport("[%s:%d] set self global id to %d\n", __FILE__, __LINE__, globalID);
    }

    for (u8 i = 0; i < clientNum; i++) {
        rStream.read(&globalID);
        OSReport("[%s:%d] read entry %d\n", __FILE__, __LINE__, globalID);

        GameOnlineClient* pClient = getClientByGlobalID(globalID);
        if (pClient == nullptr) {
            pClient = getFreeClient();
            OSReport("[%s:%d] new slot\n", __FILE__, __LINE__);
        }
        else {
            OSReport("[%s:%d] old slot\n", __FILE__, __LINE__);
        }

        pClient->mGlobalID = globalID;
        GameOnlineFunction::initSockAddress(pClient->mAddress);
        GameOnlineFunction::readSockAddress(rStream, pClient->mAddress);
        rStream.readData(pClient->mStageName, sizeof(pClient->mStageName));
        rStream.read(&pClient->mScenarioNo);
        pClient->mLastPingTime = 0;
        pClient->mLastPlayerDataTime = 0;
    }

    if (mRoomState == ROOM_STATE_DISCONNECTED) {
        mRoomState = ROOM_STATE_CONNECTING;
    }
}

void GameOnlineManager::handlePlayerData(OSTime timestamp, MR::DataStream& rStream) {
    if (!::getSceneController()->isSceneInitializeState(SceneInitializeState_End)) {
        return;
    }

    u8 globalID;
    rStream.read(&globalID);

    GameOnlineClient* pClient = getClientByGlobalID(globalID);
    if (pClient == nullptr || pClient->mActor == nullptr || pClient->mLastPlayerDataTime > timestamp) {
        return;
    }

    OnlinePlayer* pActor = pClient->mActor;
    rStream.read(&pActor->mPosition);
    rStream.read(&pActor->mRotation);
    rStream.read(&pActor->mScale);

    bool isAnimationSimple;
    rStream.read(&isAnimationSimple);

    if (isAnimationSimple) {
        char animName[32];
        rStream.readData(animName, sizeof(animName));
        pActor->playAnimationSimple(animName);
    } else {
        u32 animHash;
        rStream.read(&animHash);
        pActor->playAnimation(animHash);
    }

    f32 animFrame;
    rStream.read(&animFrame);
    pActor->setAnimationFrame(animFrame);

    for (s32 i = 0; i < 4; i++) {
        f32 weights;
        rStream.read(&weights);
        pActor->mXanimePlayer->changeTrackWeight(i, weights);
    }
}

void GameOnlineManager::handlePlayerStage(MR::DataStream& rStream) {
    u8 globalID;
    rStream.read(&globalID);

    GameOnlineClient* pClient = getClientByGlobalID(globalID);
    if (pClient == nullptr) {
        return;
    }

    rStream.readData(pClient->mStageName, sizeof(pClient->mStageName));
    rStream.read(&pClient->mScenarioNo);
}

void GameOnlineManager::sendPing() {
    GameOnlineClient* pLocalClient = getLocalClient();
    OSTime time = OSGetTime();

    if ((time - pLocalClient->mLastPingTime) < GameOnlineConst::PING_RATE) {
        return;
    }

    pLocalClient->mLastPingTime = time;

    MR::DataStream stream(mTemporaryBuffer);
    GameOnlineFunction::initPacketHeader(stream);
    GameOnlineFunction::writePacketHeader(stream, GameOnlineConst::PACKET_PING);
    broadcast(stream, false);
}

void GameOnlineManager::sendPong(const SockAddress& rAddress) {
    MR::DataStream stream(mTemporaryBuffer);
    GameOnlineFunction::initPacketHeader(stream);

    GameOnlineClient* pLocalClient = getLocalClient();
    stream.write(&pLocalClient->mGlobalID);

    GameOnlineFunction::writePacketHeader(stream, GameOnlineConst::PACKET_PONG);
    send(stream, rAddress);
}

void GameOnlineManager::sendRoomJoinInfo(const SockAddress& rAddress) {
    u8 globalID = getFreeClientGlobalID();
    if (globalID == GameOnlineConst::INVALID_PLAYER_ID) {
        return;
    }

    GameOnlineClient* pClient = getFreeClient();
    pClient->mGlobalID = globalID;
    pClient->mAddress = rAddress;
    pClient->mLastPingTime = 0;
    pClient->mLastPlayerDataTime = 0;
    pClient->mStageName[0] = '\0';
    pClient->mScenarioNo = 0;

    u8 clientNum = getConnectedClientNum();

    MR::DataStream stream(mTemporaryBuffer);
    GameOnlineFunction::initPacketHeader(stream);

    stream.write(&globalID);
    stream.write(&clientNum);

    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        if (!pClient->isValid()) {
            continue;
        }

        stream.write(&pClient->mGlobalID);
        GameOnlineFunction::writeSockAddress(stream, pClient->mAddress);
        stream.writeData(pClient->mStageName, sizeof(pClient->mStageName));
        stream.write(&pClient->mScenarioNo);
    }

    GameOnlineFunction::writePacketHeader(stream, GameOnlineConst::PACKET_ROOM_JOIN_INFO);
    // send(stream, rAddress);
    broadcast(stream, false);
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

    MR::DataStream stream(mTemporaryBuffer);
    GameOnlineFunction::initPacketHeader(stream);

    GameOnlineClient* pLocalClient = getLocalClient();
    stream.write(&pLocalClient->mGlobalID);
    stream.write(&pMarioActor->mPosition);
    stream.write(&pMarioActor->mRotation);
    stream.write(&pMarioActor->mScale);

    bool isAnimationSimple = pMarioActorAnim->isAnimationRunSimple();
    stream.write(&isAnimationSimple);

    if (isAnimationSimple) {
        char animName[32];
        snprintf(animName, sizeof(animName), "%s", pMarioActorAnim->getCurrentBckName());
        stream.writeData(animName, sizeof(animName));
    } else {
        u32 animHash = MR::getHashCode(pMarioActorAnim->getCurrentAnimationName());
        stream.write(&animHash);
    }

    f32 animFrame = pMarioActorAnim->tellAnimationFrame();
    stream.write(&animFrame);

    for (s32 i = 0; i < 4; i++) {
        stream.write(&pMarioActorAnim->getCore()->mTrackList[i].mWeight);
    }

    GameOnlineFunction::writePacketHeader(stream, GameOnlineConst::PACKET_PLAYER_DATA);
    broadcast(stream, true);
}

void GameOnlineManager::sendPlayerStage() {
    GameOnlineClient* pLocalClient = getLocalClient();

    if (pLocalClient->isEqualCurrentStage()) {
        return;
    }

    snprintf(pLocalClient->mStageName, sizeof(pLocalClient->mStageName), "%s", MR::getCurrentStageName());
    pLocalClient->mScenarioNo = MR::getCurrentScenarioNo();

    MR::DataStream stream(mTemporaryBuffer);
    GameOnlineFunction::initPacketHeader(stream);

    stream.write(&pLocalClient->mGlobalID);
    stream.writeData(pLocalClient->mStageName, sizeof(pLocalClient->mStageName));
    stream.write(&pLocalClient->mScenarioNo);

    GameOnlineFunction::writePacketHeader(stream, GameOnlineConst::PACKET_PLAYER_STAGE);
    broadcast(stream, false);
}

void GameOnlineManager::exeDisconnected() {
    if (MR::isFirstStep(this)) {
        NetworkSystemWrapper* pNetworkSystem = ::getNetworkSystem();

        if (pNetworkSystem->isConnected() && mSocket >= 0) {
            pNetworkSystem->close(mSocket);
            pNetworkSystem->closeSystem();
        }

        mSocket = -1;
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

    s32 size;
    SockAddress address;

    sendPing();

    while (receive(mTemporaryBuffer, &size, &address)) {
        MR::DataStream stream(mTemporaryBuffer.mData, size);

        u32 type;
        OSTime timestamp;

        if (!GameOnlineFunction::readPacketHeader(stream, &type, &timestamp)) {
            OSReport("[%s:%d] Invalid packet received, size 0x%X\n", __FILE__, __LINE__, stream.mSize);
            continue;
        }

        switch (type) {
        case GameOnlineConst::PACKET_PING:
            handlePing(address);
            break;
        case GameOnlineConst::PACKET_PONG:
            handlePong(stream);
            break;
        case GameOnlineConst::PACKET_ROOM_MAKE_INFO:
            handleRoomMakeInfo(stream);
            break;
        case GameOnlineConst::PACKET_ROOM_JOIN_INFO:
            handleRoomJoinInfo(stream);
            break;
        }
    }

    if (mRoomState == ROOM_STATE_CONNECTING && isAllClientConnected()) {
        mRoomState = ROOM_STATE_CONNECTED;
        setNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnectedInRoom));
    }
}

void GameOnlineManager::exeConnectedInRoom() {
    sendPing();
    sendPlayerStage();
    sendPlayerData();

    s32 size;
    SockAddress address;

    while (receive(mTemporaryBuffer, &size, &address)) {
        MR::DataStream stream(mTemporaryBuffer.mData, size);

        u32 type;
        OSTime timestamp;

        if (!GameOnlineFunction::readPacketHeader(stream, &type, &timestamp)) {
            OSReport("[%s:%d] Invalid packet received, size 0x%X\n", __FILE__, __LINE__, stream.mSize);
            continue;
        }

        switch (type) {
        case GameOnlineConst::PACKET_PING:
            handlePing(address);
            break;
        case GameOnlineConst::PACKET_PONG:
            handlePong(stream);
            break;
        case GameOnlineConst::PACKET_ROOM_JOIN_REQUEST:
            handleRoomJoinRequest(stream);
            break;
        case GameOnlineConst::PACKET_ROOM_JOIN_INFO:
            handleRoomJoinInfo(stream);
            break;
        case GameOnlineConst::PACKET_PLAYER_DATA:
            handlePlayerData(timestamp, stream);
            break;
        case GameOnlineConst::PACKET_PLAYER_STAGE:
            handlePlayerStage(stream);
            break;
        }
    }
}
