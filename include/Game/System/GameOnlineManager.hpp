#pragma once

#include "Game/System/GameOnlineFunction.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/System/NetworkSystemWrapper.hpp"
#include "Game/Util/Array.hpp"

class OnlinePlayer;

class GameOnlineClient {
public:
    static const int INVALID_ID = 0xFF;

    GameOnlineClient();

    void reset();
    void initActor();
    void destroyActor();
    bool isConnected() const;
    bool isEqualCurrentStage() const;

    bool isHost() const {
        return mGlobalID == 0;
    }

    bool isValid() const {
        return mGlobalID != INVALID_ID;
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
    bool requestRoomMake(const char* pCode);
    bool requestRoomJoin(const char* pCode);

    void initMemory();
    void initActors();
    void destroyActors();

    bool receive(SockAddress* pAddress);
    void send(GameOnlinePacket* pPacket, const SockAddress& rAddress);
    void broadcast(GameOnlinePacket* pPacket, bool isNeedEqualStage);

    GameOnlineClient* getClientByLocalID(u8 localID);
    GameOnlineClient* getClientByGlobalID(u8 globalID);
    GameOnlineClient* getFreeClient();
    GameOnlineClient* getLocalClient() {
        return &mClients[0];
    }
    u8 getFreeClientGlobalID();
    u8 getConnectedClientNum();
    bool isAllClientConnected();
    bool isConnected() const;
    bool isRoomStateConnected() const {
        return mRoomState == ROOM_STATE_CONNECTED;
    }

    void handlePing(const SockAddress& rAddress);
    void handlePong();
    // receiveRoomMakeRequest is handled by server code
    void handleRoomMakeInfo();
    void handleRoomJoinRequest();
    void handleRoomJoinInfo();
    void handlePlayerData();
    void handlePlayerStage();

    void sendPing();
    void sendPong(const SockAddress& rAddress);
    // sendRoomMakeInfo is handled by server code
    void sendRoomJoinInfo(const SockAddress& rAddress);
    void sendPlayerData();
    void sendPlayerStage();

    void exeDisconnected();
    void exeConnecting();
    void exeConnected();
    void exeOnEndConnected();

    IOSFd mSocket;
    SockAddress mServerAddress;

    RoomState mRoomState;
    MR::AssignableArray< GameOnlineClient > mClients;
    union {
        GameOnlinePacket* mRecvPacket;
        GameOnlinePacket* mSendPacket;
    };
    GameOnlinePacketQueue mSyncPacketQueue;
};
