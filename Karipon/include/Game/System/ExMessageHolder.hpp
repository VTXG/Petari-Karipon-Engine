#pragma once

#include <Game/NPC/TalkNodeCtrl.hpp>
#include <Game/System/MessageHolder.hpp>

class ExMessageHolder : public MessageHolder {
public:
    void resetZoneMessageData() {
        mZoneMessageData = nullptr;
        mZoneMessageCount = 0;
    }

    /* 0x0C */ MessageData** mZoneMessageData;
    /* 0x10 */ s32 mZoneMessageCount;
};

class ExMessageSystem {
public:
    static MessageData* getZoneMessageData(s32 id);
};

class ExTalkNodeCtrl : public TalkNodeCtrl {
public:
    /* 0x48 */ MessageData* mMessageData;
};
