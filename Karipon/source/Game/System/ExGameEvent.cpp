#include "Game/Util/ExSingletonHolder.hpp"
#include "Kamek.hpp"
#include "Karipon/System/KariponResourceHolder.hpp"
#include <Game/System/GameDataFunction.hpp>
#include <Game/System/GameDataHolder.hpp>
#include <Game/System/GameDataPlayerStatus.hpp>
#include <Game/System/GameEventFlag.hpp>
#include <Game/System/GameEventFlagTable.hpp>
#include <Game/System/GameEventValueChecker.hpp>

namespace {
    static void initFlagSortTable(GameEventFlagTableInstance* pTableInstance) {
        pTableInstance->mLength = ExSingletonHolder< KariponResourceHolder >::get()->mGameEventFlags.size();
        pTableInstance->initSortTable();
    }

    static s32 getFlagTableSize() {
        return ExSingletonHolder< KariponResourceHolder >::get()->mGameEventFlags.size();
    }

    static u32 getFlagSortTableSize() {
        return ExSingletonHolder< KariponResourceHolder >::get()->mGameEventFlags.size() * sizeof(GameEventFlagTableInstance::Key);
    }

    static const GameEventFlag* getFlag(s32 idx) {
        KariponResourceHolder* pHolder = ExSingletonHolder< KariponResourceHolder >::get();

        if (idx < 0 || idx >= pHolder->mGameEventFlags.size()) {
            return nullptr;
        }

        return &pHolder->mGameEventFlags[idx];
    }

    static s32 getFlagIndex(const GameEventFlag* pFlag) {
        return pFlag - ExSingletonHolder< KariponResourceHolder >::get()->mGameEventFlags.mArr;
    }

    static GameEventValue* getValueTable() {
        return ExSingletonHolder< KariponResourceHolder >::get()->mGameEventValues.mArr;
    }

    static u32 getValueBufferSize(GameEventValueChecker* pChecker) {
        return (pChecker->mNumValues = ExSingletonHolder< KariponResourceHolder >::get()->mGameEventValues.size()) * sizeof(u16);
    }

    static void initializeValueData(GameEventValueChecker* pChecker) {
        for (s32 i = 0; i < pChecker->mNumValues; i++) {
            pChecker->mValues[i] = ExSingletonHolder< KariponResourceHolder >::get()->mGameEventValues[i].mDefaultValue;
        }
    }

    static bool isPassedStoryEvent(GameDataHolder* pGameDataHolder, const char* pName) {
        return pGameDataHolder->mPlayerStatus->mStoryProgress >= ExSingletonHolder< KariponResourceHolder >::get()->getStoryEventProgress(pName);
    }

    static void followStoryEventByName(GameDataHolder* pGameDataHolder, const char* pName) {
        pGameDataHolder->mPlayerStatus->mStoryProgress = ExSingletonHolder< KariponResourceHolder >::get()->getStoryEventProgress(pName);
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
