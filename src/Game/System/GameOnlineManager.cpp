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
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"

namespace {
    NEW_NERVE(NrvGameOnlineManagerDisconnected, GameOnlineManager, Disconnected);
    NEW_NERVE(NrvGameOnlineManagerConnecting, GameOnlineManager, Connecting);
    NEW_NERVE(NrvGameOnlineManagerConnected, GameOnlineManager, Connected);
    NEW_NERVE(NrvGameOnlineManagerConnectedInRoom, GameOnlineManager, ConnectedInRoom);

    NetworkSystemWrapper* getNetworkSystem() {
        return SingletonHolder< GameSystem >::get()->mObjHolder->mNetworkSystem;
    }
} // namespace

GameOnlineClient::GameOnlineClient() : mGlobalID(GameOnlineConst::INVALID_PLAYER_ID), mAddress(), mActor(), mLastPingTimestamp() {}

void GameOnlineClient::init() {
    mLastPingTimestamp = OSGetTime();
}

void GameOnlineClient::reset() {
    mGlobalID = GameOnlineConst::INVALID_PLAYER_ID;
    mAddress = SockAddress();
    mLastPingTimestamp = 0;
}

void GameOnlineClient::initActor() {
    mActor = new OnlinePlayer("OnlinePlayer");
    mActor->initWithoutIter();
}

void GameOnlineClient::destroyActor() {
    if (mActor != nullptr) {
        delete mActor;
        mActor = nullptr;
    }
}

GameOnlineManager::GameOnlineManager()
    : NerveExecutor("GameOnlineManager"), mSocket(-1), mServerAddress(), mClients(GameOnlineConst::MAX_PLAYER_NUM), mTemporaryBuffer() {
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

void GameOnlineManager::initAllActor() {
    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        pClient->initActor();
    }
}

void GameOnlineManager::destroyAllActor() {
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
        if (!pClient->isConnected()) {
            continue;
        }

        // TODO isNeedEqualStage handling
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
        if (!pClient->isConnected()) {
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
        if (pClient->isConnected()) {
            num++;
        }
    }

    return num;
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

void GameOnlineManager::handlePing(const SockAddress& rAddress) {
    sendPong(rAddress);
}

void GameOnlineManager::handlePong(OSTime timestamp, MR::DataStream& rStream) {
    u8 globalID;
    rStream.read(&globalID);

    GameOnlineClient* pClient = getClientByGlobalID(globalID);
    if (pClient->mLastPingTimestamp < timestamp) {
        pClient->mLastPingTimestamp = timestamp;
    }
}

void GameOnlineManager::handleRoomMakeInfo(MR::DataStream& rStream) {
    GameOnlineClient* pLocalClient = getLocalClient();
    pLocalClient->mGlobalID = 0;

    GameOnlineFunction::initSockAddress(pLocalClient->mAddress);
    GameOnlineFunction::readSockAddress(rStream, pLocalClient->mAddress);
}

void GameOnlineManager::handleRoomJoinRequest(MR::DataStream& rStream) {
    SockAddress address;
    GameOnlineFunction::initSockAddress(address);
    GameOnlineFunction::readSockAddress(rStream, address);
    sendRoomJoinInfo(address);
}

void GameOnlineManager::handleRoomJoinInfo(MR::DataStream& rStream) {
    u8 clientNum;

    GameOnlineClient* pLocalClient = getLocalClient();
    rStream.read(&pLocalClient->mGlobalID);
    rStream.read(&clientNum);

    for (u8 i = 0; i < clientNum; i++) {
        GameOnlineClient* pClient = getFreeClient();
        rStream.read(&pClient->mGlobalID);
        GameOnlineFunction::initSockAddress(pClient->mAddress);
        GameOnlineFunction::readSockAddress(rStream, pClient->mAddress);
        pClient->init();

        OSReport("[%s:%d] handleRoomJoinInfo %d\n", __FILE__, __LINE__, pClient->mGlobalID);
    }
}

void GameOnlineManager::handlePlayerData(OSTime timestamp, MR::DataStream& rStream) {
    u8 globalID;
    rStream.read(&globalID);

    GameOnlineClient* pClient = getClientByGlobalID(globalID);
    if (pClient == nullptr || pClient->mActor == nullptr || pClient->mLastPlayerDataTimestamp > timestamp) {
        return;
    }

    OnlinePlayer* pActor = pClient->mActor;

    // TODO something is going really wrong here
    // BCK anims also don't work
    if (pActor->mXanimePlayer == nullptr) {
        return;
    }

    rStream.read(&pActor->mPosition);
    rStream.read(&pActor->mRotation);
    rStream.read(&pActor->mScale);

    bool isAnimationSimple;
    rStream.read(&isAnimationSimple);

    if (isAnimationSimple) {
        const char* pAnimName = rStream.as< const char* >();
        rStream.mPosition += strlen(pAnimName) + 1;
        pActor->playAnimationSimple(pAnimName);
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

void GameOnlineManager::sendPing() {
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

    u8 clientNum = getConnectedClientNum();

    MR::DataStream stream(mTemporaryBuffer);
    GameOnlineFunction::initPacketHeader(stream);

    stream.write(&globalID);
    stream.write(&clientNum);

    for (GameOnlineClient* pClient = mClients.begin(); pClient != mClients.end(); pClient++) {
        if (!pClient->isConnected()) {
            continue;
        }

        stream.write(&pClient->mGlobalID);
        GameOnlineFunction::writeSockAddress(stream, pClient->mAddress);
    }

    OSTime time = OSGetTime();
    GameOnlineClient* pClient = getFreeClient();
    pClient->mGlobalID = globalID;
    pClient->mAddress = rAddress;
    pClient->mLastPingTimestamp = time;
    pClient->mLastPlayerDataTimestamp = time;

    GameOnlineFunction::writePacketHeader(stream, GameOnlineConst::PACKET_ROOM_JOIN_INFO);
    send(stream, rAddress);
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
        const char* pAnimName = pMarioActorAnim->getCurrentBckName();
        stream.writeData(pAnimName, strlen(pAnimName) + 1);
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
        OSReport("[%s:%d] NetworkSystemWrapper succeeded\n", __FILE__, __LINE__);
        setNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnected));
    } else {
        OSReport("[%s:%d] NetworkSystemWrapper failed\n", __FILE__, __LINE__);
        setNerve(GET_NERVE_ANON(NrvGameOnlineManagerDisconnected));
    }
}

void GameOnlineManager::exeConnected() {
    if (MR::isFirstStep(this)) {
        mSocket = ::getNetworkSystem()->socket(SO_AF_INET, SO_SOCK_DGRAM, 0);
    }

    sendPing();

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
            handlePong(timestamp, stream);
            break;
        case GameOnlineConst::PACKET_ROOM_MAKE_INFO:
            OSReport("[%s:%d] Received PACKET_ROOM_MAKE_INFO\n", __FILE__, __LINE__);
            handleRoomMakeInfo(stream);
            setNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnectedInRoom));
            break;
        case GameOnlineConst::PACKET_ROOM_JOIN_INFO:
            OSReport("[%s:%d] Received PACKET_ROOM_JOIN_INFO\n", __FILE__, __LINE__);
            handleRoomJoinInfo(stream);
            setNerve(GET_NERVE_ANON(NrvGameOnlineManagerConnectedInRoom));
            break;
        }
    }
}

void GameOnlineManager::exeConnectedInRoom() {
    sendPing();
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
            handlePong(timestamp, stream);
            break;
        case GameOnlineConst::PACKET_ROOM_JOIN_REQUEST:
            handleRoomJoinRequest(stream);
            break;
        case GameOnlineConst::PACKET_PLAYER_DATA:
            handlePlayerData(timestamp, stream);
            break;
        }
    }
}
