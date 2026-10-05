#pragma once

#include "Game/System/GameOnlineConst.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/DataBuffer.hpp"

class OnlinePlayer;

class GameOnlineClient {
public:
    GameOnlineClient();

    void init();
    void reset();
    void initActor();
    void destroyActor();

    bool isHost() const {
        return mGlobalID == 0;
    }

    bool isConnected() const {
        return mGlobalID != GameOnlineConst::INVALID_PLAYER_ID;
    }

    u8 mGlobalID;
    SockAddress mAddress;
    OnlinePlayer* mActor;
    OSTime mLastPingTimestamp;
    OSTime mLastPlayerDataTimestamp;
};

class GameOnlineManager : public NerveExecutor {
public:
    GameOnlineManager();

    void update();
    void initAllActor();
    void destroyAllActor();

    bool receive(MR::DataBuffer& rBuffer, s32* pSize, SockAddress* pAddress);
    void send(MR::DataStream& rStream, const SockAddress& rAddress);
    void broadcast(MR::DataStream& rStream, bool isNeedEqualStage = false);

    GameOnlineClient* getClientByLocalID(u8 localID);
    GameOnlineClient* getClientByGlobalID(u8 globalID);
    GameOnlineClient* getFreeClient();
    GameOnlineClient* getLocalClient() {
        return &mClients[0];
    }
    u8 getFreeClientGlobalID();
    u8 getConnectedClientNum();

    bool requestRoomMake(const char* pCode);
    bool requestRoomJoin(const char* pCode);

    void handlePing(const SockAddress& rAddress);
    void handlePong(OSTime timestamp, MR::DataStream& rStream);
    // receiveRoomMakeRequest is handled by server code
    void handleRoomMakeInfo(MR::DataStream& rStream);
    void handleRoomJoinRequest(MR::DataStream& rStream);
    void handleRoomJoinInfo(MR::DataStream& rStream);
    void handlePlayerData(OSTime timestamp, MR::DataStream& rStream);

    void sendPing();
    void sendPong(const SockAddress& rAddress);
    // sendRoomMakeInfo is handled by server code
    void sendRoomJoinInfo(const SockAddress& rAddress);
    void sendPlayerData();

    void exeDisconnected();
    void exeConnecting();
    void exeConnected();
    void exeConnectedInRoom();

    IOSFd mSocket;
    SockAddress mServerAddress;
    MR::AssignableArray< GameOnlineClient > mClients;
    MR::DataBuffer mTemporaryBuffer;
};
