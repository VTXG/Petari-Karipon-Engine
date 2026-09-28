#include "Game/System/GameEventFlagTable.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/StringUtil.hpp"
#include <algorithm>
#include <cstdio>

extern const JMapData GalaxyIDBCSV;

namespace {
    static MR::AssignableArray< GameEventFlag > sGameEventFlagTable;

    static const char* const cEventFlagTypeTable[] = {
        /* 0x0 */ "None",
        /* 0x1 */ "StarNum",
        /* 0x2 */ "GalaxyOpenStar",
        /* 0x3 */ "SpecialStar",
        /* 0x4 */ "EventFlag",
        /* 0x5 */ "StoryEvent",
        /* 0x6 */ "Galaxy",
        /* 0x7 */ "Comet",
        /* 0x8 */ "StarPiece",
        /* 0x9 */ "EventValueIsZero",
        /* 0xA */ "CompleteMarioAndLuigi",
        /* 0xB */ "StoryEventSync",
    };

    static void initGameEventFlag(GameEventFlag& rFlag, JMapInfo& rInfo, s32 idx) {
        const char* pType;
        u32 save;
        u32 condition1;
        u32 condition2;
        rInfo.getValue(idx, "Name", &rFlag.mName);
        rInfo.getValue(idx, "Type", &pType);
        rInfo.getValue(idx, "Save", &save);
        rInfo.getValue(idx, "Condition1", &condition1);
        rInfo.getValue(idx, "Condition2", &condition2);
        rInfo.getValue(idx, "Condition3", &rFlag.mCondition3);
        rInfo.getValue(idx, "Condition4", &rFlag.mCondition4);
        rFlag.mSaveFlag = save == 0 ? 1 : 0;
        rFlag.mCondition1 = condition1;
        rFlag.mCondition2 = condition2;

        if (rFlag.mCondition3[0] == '\0') {
            rFlag.mCondition3 = nullptr;
        }

        if (rFlag.mCondition4[0] == '\0') {
            rFlag.mCondition4 = nullptr;
        }

        if (MR::isNullOrEmptyString(pType)) {
            rFlag.mType = GameEventFlag::Type_None;
            return;
        }

        for (u32 i = GameEventFlag::Type_None; i <= GameEventFlag::Type_StoryEventSync; i++) {
            if (MR::isEqualString(pType, cEventFlagTypeTable[i])) {
                rFlag.mType = i;
                break;
            }
        }
    }

    struct GameEventFlagSortLt {
        bool operator()(GameEventFlagTableInstance::Key& key1, GameEventFlagTableInstance::Key& key2) {
            return (key1.mHashCode < key2.mHashCode) ? true : false;
        }
    };
};  // namespace

bool GameEventFlagIter::isEnd() const {
    if (GameEventFlagIter::isValid() == false) {
        return true;
    }

    return GameEventFlagTable::getFlag(mIter) == nullptr;
}

void GameEventFlagIter::goNext() {
    if (GameEventFlagIter::isValid()) {
        mIter++;
    }
}

const GameEventFlag* GameEventFlagIter::getFlag() const {
    return GameEventFlagTable::getFlag(mIter);
}

bool GameEventFlagIter::isValid() const {
    if (mIter >= 0) {
        return true;
    } else {
        return false;
    }
}

GameEventFlagTableInstance::GameEventFlagTableInstance() : mSortTable(nullptr), mLength(0) {
    GameEventFlagTable::init();
    initSortTable();
}

const GameEventFlag* GameEventFlagTableInstance::findFlag(const char* flagName) {
    int i;
    u16 hashCode = MR::getHashCode(flagName);
    i = mLength;
    i = (i * 8) / 8;
    GameEventFlagTableInstance::Key* key = mSortTable;

    while (i > 0) {
        int mid = i / 2;

        if (key[i / 2].mHashCode < hashCode) {
            i -= mid + 1;
            key = &key[mid] + 1;
        } else {
            i = mid;
        }
    }

    if (key->mHashCode == hashCode && MR::isEqualString(flagName, key->mFlag->mName)) {
        return key->mFlag;
    }

    return nullptr;
}

void GameEventFlagTableInstance::initSortTable() {
    mSortTable = new GameEventFlagTableInstance::Key[GameEventFlagTable::getTableSize()];
    mLength = GameEventFlagTable::getTableSize();

    for (int i = 0; i < mLength; i++) {
        const GameEventFlag* pFlag = GameEventFlagTable::getFlag(i);

        mSortTable[i].mFlag = pFlag;
        mSortTable[i].mHashCode = MR::getHashCode(pFlag->mName);
    }

    std::sort(&mSortTable[0], &mSortTable[mLength], GameEventFlagSortLt());
}

namespace GameEventFlagTable {
    void init() {
        if (sGameEventFlagTable.size() > 0) {
            return;
        }

        void* pFileData = MR::receiveFile("/SystemData/GameEventFlagTable.bcsv");

        JMapInfo info;
        info.attach(pFileData);

        s32 numEntries = info.getNumEntries();
        sGameEventFlagTable.init(numEntries);

        for (s32 i = 0; i < numEntries; i++) {
            initGameEventFlag(sGameEventFlagTable[i], info, i);
        }
    }

    GameEventFlagIter getBeginIter() {
        return GameEventFlagIter();
    }

    GameEventFlagAccessor makeAccessor(const char* pFlagName) {
        return findFlag(pFlagName);
    }

    s32 getTableSize() {
        return sGameEventFlagTable.mMaxSize;
    }

    const GameEventFlag* getFlag(int index) {
        if (index < 0 || index >= getTableSize()) {
            return nullptr;
        } else {
            return &::sGameEventFlagTable[index];
        }
    }

    const GameEventFlag* findFlag(const char* pFlagName) {
        return SingletonHolder< GameEventFlagTableInstance >::get()->findFlag(pFlagName);
    }

    const char* getEventFlagNameSpecialPowerStar(const char* pGalaxyName, s32 starId) {
        for (GameEventFlagIter iter = getBeginIter(); !iter.isEnd(); iter.goNext()) {
            GameEventFlagAccessor accessor = GameEventFlagAccessor(iter.getFlag());

            if (!accessor.isTypeSpecialStar()) {
                continue;
            }

            if (accessor.getStarId() != starId) {
                continue;
            }

            if (!MR::isEqualString(pGalaxyName, accessor.getGalaxyName())) {
                continue;
            }

            return accessor.getName();
        }

        return nullptr;
    }

    bool isPowerStarType(const char* pGalaxyName, s32 starId, const char* pStarType) {
        const char* pFlagName = getEventFlagNameSpecialPowerStar(pGalaxyName, starId);

        if (pFlagName == nullptr) {
            return false;
        }

        return MR::isEqualString(pFlagName, pStarType);
    }

    s32 calcExclamationGalaxyNum() {
        s32 num = -1;

        for (GameEventFlagIter iter = getBeginIter(); !iter.isEnd(); iter.goNext()) {
            GameEventFlagAccessor accessor = GameEventFlagAccessor(iter.getFlag());

            if (!accessor.isTypeStarPiece()) {
                continue;
            }

            if (num >= accessor.getStarPieceIndex()) {
                continue;
            }

            num = accessor.getStarPieceIndex();
        }

        return num + 1;
    }

    const char* getExclamationGalaxyNameFromIndex(int index) {
        for (GameEventFlagIter iter = getBeginIter(); !iter.isEnd(); iter.goNext()) {
            GameEventFlagAccessor accessor = GameEventFlagAccessor(iter.getFlag());

            if (!accessor.isTypeStarPiece()) {
                continue;
            }

            if (accessor.getStarPieceIndex() != index) {
                continue;
            }

            return accessor.getGalaxyNameWithStarPiece();
        }

        return nullptr;
    }

    bool isExist(const char* pName) {
        return findFlag(pName) != nullptr;
    }

    int getIndex(const GameEventFlag* pFlag) {
        return pFlag - &::sGameEventFlagTable[0];
    }

    bool isDependedAnother(const char* pFlagName1, const char* pFlagName2) {
        const GameEventFlag* pFlag1 = findFlag(pFlagName1);

        if (pFlag1->mType == GameEventFlag::Type_EventFlag) {
            if (pFlag1->mRequirement1 != nullptr && MR::isEqualString(pFlag1->mRequirement1, pFlagName2)) {
                return true;
            }

            if (pFlag1->mRequirement2 != nullptr && MR::isEqualString(pFlag1->mRequirement2, pFlagName2)) {
                return true;
            }
        } else if (pFlag1->mType == GameEventFlag::Type_Galaxy) {
            const char* galaxyDependedFlags[3];
            s32 length = getGalaxyDependedFlags(galaxyDependedFlags, ARRAY_SIZE(galaxyDependedFlags), pFlagName1);

            for (s32 i = 0; i < length; i++) {
                if (MR::isEqualString(galaxyDependedFlags[i], pFlagName2)) {
                    return true;
                }
            }
        }

        return false;
    }

    int getIndexFromHashCode(u16 hashCode) {
        for (GameEventFlagIter iter = getBeginIter(); !iter.isEnd(); iter.goNext()) {
            if ((iter.getFlag()->mSaveFlag & 0x1) != 0) {
                continue;
            }

            GameEventFlagAccessor accessor = GameEventFlagAccessor(iter.getFlag());

            if ((MR::getHashCode(accessor.getName()) & 0x7FFF) != hashCode) {
                continue;
            }

            return getIndex(getFlag(iter.mIter));
        }

        return -1;
    }

    s32 calcSpecialPowerStarNum(const char* pPowerStarType) {
        s32 num = 0;

        for (GameEventFlagIter iter = getBeginIter(); !iter.isEnd(); iter.goNext()) {
            GameEventFlagAccessor accessor = GameEventFlagAccessor(iter.getFlag());

            if (!accessor.isTypeSpecialStar()) {
                continue;
            }

            if (strstr(accessor.getName(), pPowerStarType) == nullptr) {
                continue;
            }

            num++;
        }

        return num;
    }

    s32 getStarPieceNumToOpenExclamationGalaxy(const char* pGalaxyName) {
        char flagName[64];
        snprintf(flagName, sizeof(flagName), "StarPiece%s", pGalaxyName);

        GameEventFlagAccessor accessor = makeAccessor(flagName);

        return accessor.getNeedStarPieceNum();
    }

    s32 calcGreenPowerStarNum() {
        return calcSpecialPowerStarNum("SpecialStarGreen");
    }

    s32 getGalaxyDependedFlags(const char** pFlags, int a1, const char* pName) {
        JMapInfo info;
        info.attach(&GalaxyIDBCSV);

        JMapInfoIter iter = info.findElement("name", pName, 0);

        s32 numFlags = 0;
        for (s32 idx = 0; idx < 3; idx++) {
            char key[32];
            snprintf(key, 32, "OpenCondition%1d", idx);
            if (info.searchItemInfo(key) < 0) {
                break;
            }

            const char* openCondition = "";
            iter.getValue(key, &openCondition);
            if (!MR::isEqualString(openCondition, "")) {
                pFlags[numFlags++] = openCondition;
            }
        }
        return numFlags;
    }
};  // namespace GameEventFlagTable
