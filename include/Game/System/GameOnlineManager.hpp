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

    void initActor();
    void destroyActor();
    bool isConnected() const;
    bool isEqualCurrentStage() const;

    bool isHost() const {
        return mGlobalID == 0;
    }

    bool isValid() const {
        return mGlobalID != GameOnlineConst::INVALID_PLAYER_ID;
    }

    void exeDisconnected();
    void exeConnecting();
    void exeConnected();

    u8 mGlobalID;
    SockAddress mAddress;
    OnlinePlayer* mActor;
    OSTime mLastPingTime;
    OSTime mLastPlayerDataTime;
    char mStageName[32];
    s32 mScenarioNo;
};

class GameOnlineManager : public NerveExecutor {
public:
    enum RoomState {
        ROOM_STATE_DISCONNECTED,
        ROOM_STATE_CONNECTING,
        ROOM_STATE_CONNECTED,
    };

    GameOnlineManager();

    void update();
    void initActors();
    void destroyActors();

    bool receive(MR::DataBuffer& rBuffer, s32* pSize, SockAddress* pAddress);
    void send(MR::DataStream& rStream, const SockAddress& rAddress);
    void broadcast(MR::DataStream& rStream, bool isNeedEqualStage);

    GameOnlineClient* getClientByLocalID(u8 localID);
    GameOnlineClient* getClientByGlobalID(u8 globalID);
    GameOnlineClient* getFreeClient();
    GameOnlineClient* getLocalClient() {
        return &mClients[0];
    }
    u8 getFreeClientGlobalID();
    u8 getConnectedClientNum();
    bool isAllClientConnected();

    bool requestRoomMake(const char* pCode);
    bool requestRoomJoin(const char* pCode);
    bool isConnected() const;
    bool isConnectedInRoom() const;

    void handlePing(const SockAddress& rAddress);
    void handlePong(MR::DataStream& rStream);
    // receiveRoomMakeRequest is handled by server code
    void handleRoomMakeInfo(MR::DataStream& rStream);
    void handleRoomJoinRequest(MR::DataStream& rStream);
    void handleRoomJoinInfo(MR::DataStream& rStream);
    void handlePlayerData(OSTime timestamp, MR::DataStream& rStream);
    void handlePlayerStage(MR::DataStream& rStream);

    void sendPing();
    void sendPong(const SockAddress& rAddress);
    // sendRoomMakeInfo is handled by server code
    void sendRoomJoinInfo(const SockAddress& rAddress);
    void sendPlayerData();
    void sendPlayerStage();

    void exeDisconnected();
    void exeConnecting();
    void exeConnected();
    void exeConnectedInRoom();

    IOSFd mSocket;
    SockAddress mServerAddress;

    RoomState mRoomState;
    MR::AssignableArray< GameOnlineClient > mClients;
    MR::DataBuffer mTemporaryBuffer;
};
