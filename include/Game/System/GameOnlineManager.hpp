#pragma once

#include "Game/System/GameOnlinePacket.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/Util/Array.hpp"

class JKRUnitHeap;

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

    bool receive();
    void send(GameOnlinePacket* pPacket);
    void updateSyncPackets();

    GameOnlineClient* getClientByLocalID(u8 localID);
    GameOnlineClient* getClientByGlobalID(u8 globalID);
    GameOnlineClient* getFreeClient();
    GameOnlineClient* getLocalClient() {
        return &mClients[0];
    }
    u8 getLocalIDByGlobalID(u8 globalID);
    u8 getLocalIDByAddress(const SockAddress& rAddress);
    u8 getFreeClientGlobalID();
    u8 getConnectedClientNum();
    bool isAllClientConnected();
    bool isConnected() const;

    void handlePing();
    void handlePong();
    void handleReply();
    // receiveRoomMakeRequest is handled by server code
    void handleRoomMakeInfo();
    void handleRoomJoinRequest();
    void handleRoomJoinInfo();
    void handlePlayerJoin();
    void handlePlayerLeave();
    void handlePlayerData();
    void handlePlayerStage();

    void sendPing();
    void sendPong(const SockAddress& rAddress);
    void sendReply(u32 packetID, const SockAddress& rAddress);
    // sendRoomMakeInfo is handled by server code
    void sendRoomJoinInfo(const SockAddress& rAddress);
    void sendPlayerData();
    void sendPlayerStage();

    void exeDisconnected();
    void exeConnecting();
    void exeConnected();
    void exeOnEndConnected();

    JKRUnitHeap* mOnlineHeap;
    SockAddress mServerAddress;

    IOSFd mSocket;
    RoomState mRoomState;
    MR::AssignableArray< GameOnlineClient > mClients;
    GameOnlinePacket* mRecvPacket;
    GameOnlinePacket* mSendPacket;
    GameOnlinePacketQueue mSyncPacketQueue;
};

namespace GameOnlineFunction {
    void readSockAddress(GameOnlinePacket* pPacket, SockAddress& rAddress);
    void writeSockAddress(GameOnlinePacket* pPacket, const SockAddress& rAddress);
} // namespace GameOnlineFunction
