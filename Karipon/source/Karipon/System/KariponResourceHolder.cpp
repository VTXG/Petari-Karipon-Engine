#include "Karipon/System/KariponResourceHolder.hpp"
#include "Karipon/System/ByamlUtil.hpp"
#include <Game/System/GalaxyCometScheduler.hpp>
#include <Game/System/GameEventFlag.hpp>
#include <Game/System/GameEventValueChecker.hpp>
#include <Game/Util/FileUtil.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <Game/Util/MemoryUtil.hpp>
#include <Game/Util/StringUtil.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <cstdio>
#include <revolution/types.h>

#define COMET_CATEGORY_NORMAL 0
#define COMET_CATEGORY_COIN_100 1

namespace {
    static const char* const cFileLoadTable[] = {
        "/SystemData/DomeParamTable.byaml",     "/SystemData/GalaxyIDTable.bcsv",       "/SystemData/GameEventFlagTable.bcsv",
        "/SystemData/GameEventValueTable.bcsv", "/SystemData/GameStoryEventTable.bcsv", "/SystemData/StageBgmTable.bcsv",
        "/SystemData/ScenarioBgmTable.bcsv",    "/SystemData/MultiBgmTable.bcsv",       "/SystemData/StageParamTable.byaml",
        "/SystemData/StageWaveTable.byaml",
    };

    static const char* const cCometCategoryTable[] = {
        /* 0x0 */ "Normal",
        /* 0x1 */ "Coin100",
    };

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

    static u32 countAndMountResource(const char* pFormat, u32 start) {
        u32 count = 0;

        while (true) {
            char filePath[0x100];
            snprintf(filePath, sizeof(filePath), pFormat, count + start);

            if (!MR::isFileExist(filePath, false)) {
                break;
            }

            MR::loadAsyncToMainRAM(filePath, nullptr, MR::getStationedHeapGDDR3(), JKRDvdRipper::ALLOC_DIRECTION_FORWARD);
            count++;
        }

        return count;
    }

    static GalaxyCometTimePaper* createCometCycleTable(void* pResource) {
        JMapInfo info;
        info.attach(pResource);

        u32 numEntries = info.getNumEntries();
        GalaxyCometTimePaper* pTable = new GalaxyCometTimePaper[numEntries + 1];

        for (s32 i = 0; i < numEntries; i++) {
            const char* pName = nullptr;
            const char* pCategory = nullptr;
            info.getValue(i, "Name", &pName);
            info.getValue(i, "Category", &pCategory);

            GalaxyCometTimePaper& rPaper = pTable[i];
            rPaper.mGalaxyName = pName;

            if (pCategory != nullptr) {
                for (u32 category = COMET_CATEGORY_NORMAL; category <= COMET_CATEGORY_COIN_100; category++) {
                    if (MR::isEqualString(pCategory, cCometCategoryTable[category])) {
                        rPaper.mCategory = category;
                        break;
                    }
                }
            } else {
                rPaper.mCategory = COMET_CATEGORY_NORMAL;
            }
        }

        pTable[numEntries].mGalaxyName = nullptr;
        pTable[numEntries].mCategory = 0;
        return pTable;
    }
} // namespace

KariponResourceHolder::KariponResourceHolder() {
    for (s32 i = 0; i < ARRAY_SIZE(cFileLoadTable); i++) {
        MR::loadAsyncToMainRAM(cFileLoadTable[i], nullptr, MR::getStationedHeapGDDR3(), JKRDvdRipper::ALLOC_DIRECTION_FORWARD);
    }

    u32 cometTableCount = countAndMountResource("/SystemData/CometCycleTable%d.bcsv", 1);
    mCometCycleTables.init(cometTableCount);
}

void KariponResourceHolder::init() {
    mGalaxyIDTable = MR::receiveFile("/SystemData/GalaxyIDTable.bcsv");
    mStageParamTable = ByamlUtil::createByamlRootFromFile("/SystemData/StageParamTable.byaml");
    mDomeParamTable = ByamlUtil::createByamlRootFromFile("/SystemData/DomeParamTable.byaml");

    for (u32 i = 0; i < mCometCycleTables.size(); i++) {
        char filePath[0x100];
        snprintf(filePath, sizeof(filePath), "/SystemData/CometCycleTable%d.bcsv", i + 1);
        mCometCycleTables[i] = createCometCycleTable(MR::receiveFile(filePath));
    }

    initGameEventFlags(MR::receiveFile("/SystemData/GameEventFlagTable.bcsv"));
    initGameEventValues(MR::receiveFile("/SystemData/GameEventValueTable.bcsv"));
    mGameStoryEventTable.attach(MR::receiveFile("/SystemData/GameStoryEventTable.bcsv"));

    mStageBgmTable.attach(MR::receiveFile("/SystemData/StageBgmTable.bcsv"));
    mScenarioBgmTable.attach(MR::receiveFile("/SystemData/ScenarioBgmTable.bcsv"));
    mMultiBgmTable.attach(MR::receiveFile("/SystemData/MultiBgmTable.bcsv"));
    mStageWaveTable = ByamlUtil::createByamlRootFromFile("/SystemData/StageWaveTable.byaml");
}

void KariponResourceHolder::initGameEventFlags(void* pResource) {
    JMapInfo info;
    info.attach(pResource);

    u32 numEntries = info.getNumEntries();
    mGameEventFlags.init(numEntries);

    for (s32 i = 0; i < numEntries; i++) {
        const char* pName = nullptr;
        const char* pType = nullptr;
        u32 save = 0;
        u32 condition1 = 0;
        u32 condition2 = 0;
        const char* pCondition3 = nullptr;
        const char* pCondition4 = nullptr;
        info.getValue(i, "Name", &pName);
        info.getValue(i, "Type", &pType);
        info.getValue(i, "Save", &save);
        info.getValue(i, "Condition1", &condition1);
        info.getValue(i, "Condition2", &condition2);
        info.getValue(i, "Condition3", &pCondition3);
        info.getValue(i, "Condition4", &pCondition4);

        GameEventFlag& rFlag = mGameEventFlags[i];
        rFlag.mName = pName;
        rFlag.mSaveFlag = save == 0 ? 1 : 0;
        rFlag.mCondition1 = condition1;
        rFlag.mCondition2 = condition2;
        rFlag.mCondition3 = pCondition3 && pCondition3[0] != '\0' ? pCondition3 : nullptr;
        rFlag.mCondition4 = pCondition4 && pCondition4[0] != '\0' ? pCondition4 : nullptr;

        if (pType != nullptr) {
            for (u32 i = GameEventFlag::Type_None; i <= GameEventFlag::Type_StoryEventSync; i++) {
                if (MR::isEqualString(pType, cEventFlagTypeTable[i])) {
                    rFlag.mType = i;
                    break;
                }
            }
        } else {
            rFlag.mType = GameEventFlag::Type_None;
        }
    }
}

void KariponResourceHolder::initGameEventValues(void* pResource) {
    JMapInfo info;
    info.attach(pResource);

    u32 numEntries = info.getNumEntries();
    mGameEventValues.init(numEntries);

    for (s32 i = 0; i < numEntries; i++) {
        const char* pName = nullptr;
        u32 defaultValue = 0;
        info.getValue(i, "Name", &pName);
        info.getValue(i, "DefaultValue", &defaultValue);

        GameEventValue& rValue = mGameEventValues[i];
        rValue.mName = pName;
        rValue.mDefaultValue = defaultValue;
    }
}

u8 KariponResourceHolder::getStoryEventProgress(const char* pEventName) {
    JMapInfoIter iter = mGameStoryEventTable.findElement("Name", pEventName, 0);

    u32 progress = 0;
    iter.getValue("Progress", &progress);
    return progress;
}
