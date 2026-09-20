#include "Karipon/System/GameEventTableHolder.hpp"
#include "Game/System/GameDataHolder.hpp"
#include "Game/System/GameDataPlayerStatus.hpp"
#include "Game/System/GameEventFlag.hpp"
#include "Game/System/GameEventFlagTable.hpp"
#include "Game/System/GameEventValueChecker.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Kamek.hpp"
#include "Karipon/System/ByamlIter.hpp"
#include "Karipon/System/ByamlUtil.hpp"

namespace {
    static MR::AssignableArray<GameEventFlag> sEventFlagTable;
    static MR::AssignableArray<GameEventValue> sEventValueTable;
    static ByamlIter sStoryEventsIt;

    static const char* cEventFlagTypeTable[] = {
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

    static void readGameEventFlag(GameEventFlag& rFlag, const ByamlIter& rIter) {
        const char* pName = nullptr;
        rIter.tryGetValueByKey(&pName, "Name");
        rFlag.mName = pName;

        bool save = nullptr;
        rIter.tryGetValueByKey(&save, "Save");
        rFlag.mSaveFlag = save ? 0 : 1;

        u32 condition1 = 0;
        rIter.tryGetValueByKey(&condition1, "Condition1");
        rFlag.mCondition1 = condition1;

        u32 condition2 = 0;
        rIter.tryGetValueByKey(&condition2, "Condition2");
        rFlag.mCondition2 = condition2;

        const char* pCondition3 = nullptr;
        rIter.tryGetValueByKey(&pCondition3, "Condition3");
        rFlag.mCondition3 = pCondition3;

        const char* pCondition4 = nullptr;
        rIter.tryGetValueByKey(&pCondition4, "Condition4");
        rFlag.mCondition4 = pCondition4;

        const char* pType = nullptr;
        rIter.tryGetValueByKey(&pType, "Type");
        rFlag.mType = GameEventFlag::Type_None;

        if (pType != nullptr) {
            for (u32 i = GameEventFlag::Type_None; i <= GameEventFlag::Type_StoryEventSync; i++) {
                if (MR::isEqualString(pType, cEventFlagTypeTable[i])) {
                    rFlag.mType = i;
                    break;
                }
            }
        }
    }

    static void readGameEventValue(GameEventValue& rValue, const ByamlIter& rIter) {
        const char* pName = nullptr;
        rIter.tryGetValueByKey(&pName, "Name");
        rValue.mName = pName;

        u32 defaultValue = 0;
        rIter.tryGetValueByKey(&defaultValue, "DefaultValue");
        rValue.mDefaultValue = defaultValue;
    }

    static u32 getStoryEventProgress(const char* pName) {
        u32 progress = 0;
        sStoryEventsIt.tryGetValueByKey(&progress, pName);
        return progress;
    }
} // namespace

void GameEventTableHolder::init() {
    void* pData = MR::receiveFile("/SystemData/GameEventTable.byaml");
    ByamlIter rootIt = ByamlUtil::createByamlRoot(static_cast<u8*>(pData));

    // Game event flag loading
    {
        ByamlIter flagsIt = rootIt.getIterByKey("EventFlags");

        s32 size = flagsIt.getSize();
        sEventFlagTable.init(size);

        for (s32 i = 0; i < size; i++) {
            readGameEventFlag(sEventFlagTable[i], flagsIt.getIterByIndex(i));
        }
    }

    // Game event value loading
    {
        ByamlIter valuesIt = rootIt.getIterByKey("EventValues");

        s32 size = valuesIt.getSize();
        sEventValueTable.init(size);

        for (s32 i = 0; i < size; i++) {
            readGameEventValue(sEventValueTable[i], valuesIt.getIterByIndex(i));
        }
    }

    // Game story event loading
    {
        sStoryEventsIt = rootIt.getIterByKey("StoryEvents");
    }
}

namespace {
    static void initFlagSortTable(GameEventFlagTableInstance* pTableInstance) {
        pTableInstance->mLength = sEventFlagTable.mMaxSize;
        pTableInstance->initSortTable();
    }

    static s32 getFlagTableSize() {
        return sEventFlagTable.mMaxSize;
    }

    static u32 getFlagSortTableSize() {
        return sEventFlagTable.mMaxSize * sizeof(GameEventFlagTableInstance::Key);
    }

    static const GameEventFlag* getFlag(s32 idx) {
        if (idx < 0 || idx >= sEventFlagTable.mMaxSize) {
            return nullptr;
        }

        return &sEventFlagTable[idx];
    }

    static s32 getFlagIndex(const GameEventFlag* pFlag) {
        return pFlag - sEventFlagTable.mArr;
    }

    static GameEventValue* getValueTable() {
        return sEventValueTable.mArr;
    }

    static u32 getValueBufferSize(GameEventValueChecker* pChecker) {
        return (pChecker->mNumValues = sEventValueTable.mMaxSize) * sizeof(u16);
    }

    static void initializeValueData(GameEventValueChecker* pChecker) {
        for (s32 i = 0; i < pChecker->mNumValues; i++) {
            pChecker->mValues[i] = sEventValueTable[i].mDefaultValue;
        }
    }

    static bool isPassedStoryEvent(GameDataHolder* pGameDataHolder, const char* pName) {
        return pGameDataHolder->mPlayerStatus->mStoryProgress >= getStoryEventProgress(pName);
    }

    static void followStoryEventByName(GameDataHolder* pGameDataHolder, const char* pName) {
        pGameDataHolder->mPlayerStatus->mStoryProgress = getStoryEventProgress(pName);
    }
} // namespace

extern kmSymbol __ct__26GameEventFlagTableInstanceFv;
kmCall(&__ct__26GameEventFlagTableInstanceFv + 0x20, initFlagSortTable);

extern kmSymbol getTableSize__18GameEventFlagTableFv;
kmBranch(&getTableSize__18GameEventFlagTableFv, getFlagTableSize);

extern kmSymbol initSortTable__26GameEventFlagTableInstanceFv;
kmCall(&initSortTable__26GameEventFlagTableInstanceFv + 0x18, getFlagSortTableSize);
kmWrite32(&initSortTable__26GameEventFlagTableInstanceFv + 0x20, PPC_NOP);
kmWrite32(&initSortTable__26GameEventFlagTableInstanceFv + 0x30, PPC_NOP);

extern kmSymbol getFlag__18GameEventFlagTableFi;
kmBranch(&getFlag__18GameEventFlagTableFi, getFlag);

extern kmSymbol getIndex__18GameEventFlagTableFPC13GameEventFlag;
kmBranch(&getIndex__18GameEventFlagTableFPC13GameEventFlag, getFlagIndex);

extern kmSymbol getIndexFromHashCode__18GameEventFlagTableFUs;
kmCall(&getIndexFromHashCode__18GameEventFlagTableFUs + 0x68, getFlagIndex);
kmWrite32(&getIndexFromHashCode__18GameEventFlagTableFUs + 0x6C, PPC_B(0x44));

extern kmSymbol __ct__21GameEventValueCheckerFv;
kmCall(&__ct__21GameEventValueCheckerFv + 0x2C, getValueBufferSize);
kmWrite32(&__ct__21GameEventValueCheckerFv + 0x40, PPC_NOP);

extern kmSymbol serialize__21GameEventValueCheckerCFPUcUl;
kmCall(&serialize__21GameEventValueCheckerCFPUcUl + 0x34, getValueTable);
kmWrite32(&serialize__21GameEventValueCheckerCFPUcUl + 0x3C, PPC_MR(29, 3));

extern kmSymbol initializeData__21GameEventValueCheckerFv;
kmBranch(&initializeData__21GameEventValueCheckerFv, initializeValueData);

extern kmSymbol findIndex__21GameEventValueCheckerCFPCc;
kmWrite32(&findIndex__21GameEventValueCheckerCFPCc + 0x14, PPC_MR(27, 3));
kmWrite32(&findIndex__21GameEventValueCheckerCFPCc + 0x18, PPC_MR(28, 4));
kmWrite32(&findIndex__21GameEventValueCheckerCFPCc + 0x1C, PPC_LI(29, 0));
kmCall(&findIndex__21GameEventValueCheckerCFPCc + 0x20, getValueTable);
kmWrite32(&findIndex__21GameEventValueCheckerCFPCc + 0x24, PPC_MR(30, 3));

extern kmSymbol findIndexFromHashCode__21GameEventValueCheckerCFUs;
kmWrite32(&findIndexFromHashCode__21GameEventValueCheckerCFUs + 0x14, PPC_MR(27, 3));
kmWrite32(&findIndexFromHashCode__21GameEventValueCheckerCFUs + 0x18, PPC_MR(28, 4));
kmWrite32(&findIndexFromHashCode__21GameEventValueCheckerCFUs + 0x1C, PPC_LI(29, 0));
kmCall(&findIndexFromHashCode__21GameEventValueCheckerCFUs + 0x20, getValueTable);
kmWrite32(&findIndexFromHashCode__21GameEventValueCheckerCFUs + 0x24, PPC_MR(30, 3));

extern kmSymbol isPassedStoryEvent__14GameDataHolderCFPCc;
kmBranch(&isPassedStoryEvent__14GameDataHolderCFPCc, isPassedStoryEvent);

extern kmSymbol followStoryEventByName__14GameDataHolderFPCc;
kmBranch(&followStoryEventByName__14GameDataHolderFPCc, followStoryEventByName);
