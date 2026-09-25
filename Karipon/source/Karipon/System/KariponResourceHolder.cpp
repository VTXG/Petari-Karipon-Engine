#include "Karipon/System/KariponResourceHolder.hpp"
#include "Game/GameAudio/ExAudStageBgm.hpp"
#include "Karipon/System/ByamlUtil.hpp"
#include <Game/AudioLib/AudSoundId.hpp>
#include <Game/AudioLib/AudSoundNameConverter.hpp>
#include <Game/System/GalaxyCometScheduler.hpp>
#include <Game/System/GameEventFlag.hpp>
#include <Game/System/GameEventValueChecker.hpp>
#include <Game/Util/FileUtil.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <Game/Util/MemoryUtil.hpp>
#include <Game/Util/StringUtil.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <cstddef>
#include <cstdio>
#include <revolution/types.h>

#define COMET_CATEGORY_NORMAL 0
#define COMET_CATEGORY_COIN_100 1

namespace {
    static const char* const cFileLoadTable[] = {
        "/SystemData/DomeParamTable.byaml",     "/SystemData/GalaxyIDTable.bcsv",       "/SystemData/GameEventFlagTable.bcsv",
        "/SystemData/GameEventValueTable.bcsv", "/SystemData/GameStoryEventTable.bcsv", "/SystemData/StageBgmSetTable.bcsv",
        "/SystemData/StageBgmTable.bcsv",       "/SystemData/MultiBgmTable.bcsv",       "/SystemData/StageParamTable.byaml",
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

    static bool isValidString(const char* pStr) {
        return pStr != nullptr && pStr[0] != '\0';
    }

    static u32 getSoundID(const char* pName) {
        if (isValidString(pName)) {
            return static_cast< u32 >(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pName));
        }

        return -1;
    }

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

    static u32 countJMapField(JMapInfo& rInfo, const char* pFormat) {
        u32 count = 0;

        while (true) {
            char key[0x32];
            snprintf(key, sizeof(key), pFormat, count);

            if (rInfo.searchItemInfo(key) < 0) {
                break;
            }

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

    static void initGameEventFlag(GameEventFlag& rFlag, JMapInfo& rInfo, s32 idx) {
        const char* pName = nullptr;
        const char* pType = nullptr;
        u32 save = 0;
        u32 condition1 = 0;
        u32 condition2 = 0;
        const char* pCondition3 = nullptr;
        const char* pCondition4 = nullptr;
        rInfo.getValue(idx, "Name", &pName);
        rInfo.getValue(idx, "Type", &pType);
        rInfo.getValue(idx, "Save", &save);
        rInfo.getValue(idx, "Condition1", &condition1);
        rInfo.getValue(idx, "Condition2", &condition2);
        rInfo.getValue(idx, "Condition3", &pCondition3);
        rInfo.getValue(idx, "Condition4", &pCondition4);
        rFlag.mName = pName;
        rFlag.mSaveFlag = save == 0 ? 1 : 0;
        rFlag.mCondition1 = condition1;
        rFlag.mCondition2 = condition2;
        rFlag.mCondition3 = isValidString(pCondition3) ? pCondition3 : nullptr;
        rFlag.mCondition4 = isValidString(pCondition4) ? pCondition4 : nullptr;

        if (isValidString(pType)) {
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

    static void initGameEventValue(GameEventValue& rValue, JMapInfo& rInfo, s32 idx) {
        const char* pName = nullptr;
        u32 defaultValue = 0;
        rInfo.getValue(idx, "Name", &pName);
        rInfo.getValue(idx, "DefaultValue", &defaultValue);
        rValue.mName = pName;
        rValue.mDefaultValue = defaultValue;
    }

    static void initStageBgmEntry(StageBgmEntry& rEntry, JMapInfo& rInfo, s32 idx) {
        const char* pName = nullptr;
        s32 scenarioNo = 0;
        const char* pBgmIdName = nullptr;
        s32 startType = 0;
        rInfo.getValue(idx, "StageName", &pName);
        rInfo.getValue(idx, "ScenarioNo", &scenarioNo);
        rInfo.getValue(idx, "BgmIdName", &pBgmIdName);
        rInfo.getValue(idx, "StartType", &startType);
        rEntry.mName = pName;
        rEntry.mScenarioNo = scenarioNo;
        rEntry.mBgmId = getSoundID(pBgmIdName);
        rEntry.mStartType = startType;
    }

    static void initStageBgmSetEntry(StageBgmSetEntry& rEntry, JMapInfo& rInfo, s32 idx, u32 changeBgmIDFieldNum, u32 changeBgmStateFieldNum) {
        const char* pName = nullptr;
        rInfo.getValue(idx, "StageName", &pName);
        rEntry.mName = pName;

        rEntry.mChangeBgmId.init(changeBgmIDFieldNum);

        for (u32 i = 0; i < changeBgmIDFieldNum; i++) {
            char key[0x32];
            snprintf(key, sizeof(key), "ChangeBgmIdName%d", i);

            const char* pChangeBgmIdName = nullptr;
            rInfo.getValue(i, key, &pChangeBgmIdName);
            rEntry.mChangeBgmId[i] = getSoundID(pChangeBgmIdName);
        }

        rEntry.mChangeBgmState.init(changeBgmStateFieldNum);

        for (u32 i = 0; i < changeBgmStateFieldNum; i++) {
            char key[0x32];
            snprintf(key, sizeof(key), "ChangeBgmState%d", i);

            s32 changeBgmState = -1;
            rInfo.getValue(i, key, &changeBgmState);
            rEntry.mChangeBgmState[i] = changeBgmState;
        }
    }

    static void initMultiBgmSet(MultiBgmSetEntry& rEntry, JMapInfo& rInfo, s32 idx) {
        const char* pMultiIdName = nullptr;
        const char* pSeqIdName = nullptr;
        const char* pStreamIdName = nullptr;
        f32 beatMul = 1.0f;
        u32 introBeats = 0;
        u32 loopBeats = 0;
        u32 loopStartSamples = 0;
        u32 loopEndSamples = 0;
        rInfo.getValue(idx, "MultiIdName", &pMultiIdName);
        rInfo.getValue(idx, "SeqIdName", &pSeqIdName);
        rInfo.getValue(idx, "StreamIdName", &pStreamIdName);
        rInfo.getValue(idx, "BeatMul", &beatMul);
        rInfo.getValue(idx, "IntroBeats", &introBeats);
        rInfo.getValue(idx, "LoopBeats", &loopBeats);
        rInfo.getValue(idx, "LoopStartSamples", &loopStartSamples);
        rInfo.getValue(idx, "LoopEndSamples", &loopEndSamples);
        rEntry.mMultiId = getSoundID(pMultiIdName);
        rEntry.mSettings.mSeqId = getSoundID(pSeqIdName);
        rEntry.mSettings.mStreamId = getSoundID(pStreamIdName);
        rEntry.mSettings.mBeatMul = beatMul;
        rEntry.mSettings.mIntroBeats = introBeats;
        rEntry.mSettings.mLoopBeats = loopBeats;
        rEntry.mSettings.mLoopStartSamples = loopStartSamples;
        rEntry.mSettings.mLoopEndSamples = loopEndSamples;
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
    // GalaxyIDTable loading
    mGalaxyIDTable = MR::receiveFile("/SystemData/GalaxyIDTable.bcsv");

    // StageParamTable loading
    mStageParamTable = ByamlUtil::createByamlRootFromFile("/SystemData/StageParamTable.byaml");

    // DomeParamTable loading
    mDomeParamTable = ByamlUtil::createByamlRootFromFile("/SystemData/DomeParamTable.byaml");

    // CometCycleTable loading
    for (s32 i = 0; i < mCometCycleTables.size(); i++) {
        char filePath[0x100];
        snprintf(filePath, sizeof(filePath), "/SystemData/CometCycleTable%d.bcsv", i + 1);
        mCometCycleTables[i] = ::createCometCycleTable(MR::receiveFile(filePath));
    }

    // GameEventFlagTable loading
    {
        JMapInfo info;
        info.attach(MR::receiveFile("/SystemData/GameEventFlagTable.bcsv"));

        s32 numEntries = info.getNumEntries();
        mGameEventFlags.init(numEntries);

        for (s32 i = 0; i < numEntries; i++) {
            ::initGameEventFlag(mGameEventFlags[i], info, i);
        }
    }

    // GameEventValueTable loading
    {
        JMapInfo info;
        info.attach(MR::receiveFile("/SystemData/GameEventValueTable.bcsv"));

        s32 numEntries = info.getNumEntries();
        mGameEventValues.init(numEntries);

        for (s32 i = 0; i < numEntries; i++) {
            ::initGameEventValue(mGameEventValues[i], info, i);
        }
    }

    // GameStoryEventTable loading
    mGameStoryEventTable.attach(MR::receiveFile("/SystemData/GameStoryEventTable.bcsv"));

    // StageWaveTable loading
    mStageWaveTable = ByamlUtil::createByamlRootFromFile("/SystemData/StageWaveTable.byaml");
}

void KariponResourceHolder::initAfterStationedResourceLoaded() {
    // StageBgmTable loading
    {
        JMapInfo info;
        info.attach(MR::receiveFile("/SystemData/StageBgmTable.bcsv"));

        s32 numEntries = info.getNumEntries();
        mStageBgmTable.init(numEntries);

        for (s32 i = 0; i < numEntries; i++) {
            ::initStageBgmEntry(mStageBgmTable[i], info, i);
        }
    }

    // StageBgmSetTable loading
    {
        JMapInfo info;
        info.attach(MR::receiveFile("/SystemData/StageBgmSetTable.bcsv"));

        s32 numEntries = info.getNumEntries();
        mStageBgmSetTable.init(numEntries);

        u32 changeBgmIDFieldNum = countJMapField(info, "ChangeBgmIdName%d");
        u32 changeBgmStateFieldNum = countJMapField(info, "ChangeBgmState%d");

        for (s32 i = 0; i < numEntries; i++) {
            ::initStageBgmSetEntry(mStageBgmSetTable[i], info, i, changeBgmIDFieldNum, changeBgmStateFieldNum);
        }
    }

    // MultiBgmTable loading
    {
        JMapInfo info;
        info.attach(MR::receiveFile("/SystemData/MultiBgmTable.bcsv"));

        s32 numEntries = info.getNumEntries();
        mMultiBgmTable.init(numEntries);

        for (s32 i = 0; i < numEntries; i++) {
            ::initMultiBgmSet(mMultiBgmTable[i], info, i);
        }
    }
}

u8 KariponResourceHolder::getStoryEventProgress(const char* pEventName) {
    JMapInfoIter iter = mGameStoryEventTable.findElement("Name", pEventName, 0);

    u32 progress = 0;
    iter.getValue("Progress", &progress);
    return progress;
}

const StageBgmEntry* KariponResourceHolder::findStageBgmEntry(const char* pStageName, s32 scenarioNo) {
    const StageBgmEntry* pDefaultEntry = nullptr;

    for (const StageBgmEntry* pEntry = mStageBgmTable.begin(); pEntry < mStageBgmTable.end(); pEntry++) {
        if (!MR::isEqualString(pEntry->mName, pStageName)) {
            continue;
        }

        if (pEntry->mScenarioNo == scenarioNo) {
            return pEntry;
        }

        if (pEntry->mScenarioNo == 0) {
            pDefaultEntry = pEntry;
        }
    }

    return pDefaultEntry;
}

const StageBgmSetEntry* KariponResourceHolder::findStageBgmSetEntry(const char* pStageName) {
    for (const StageBgmSetEntry* pEntry = mStageBgmSetTable.begin(); pEntry < mStageBgmSetTable.end(); pEntry++) {
        if (MR::isEqualString(pEntry->mName, pStageName)) {
            return pEntry;
        }
    }

    return nullptr;
}

const MultiBgmSetEntry* KariponResourceHolder::findMultiBgmSetEntry(u32 id) {
    if (id < mMultiBgmTable.size()) {
        if ((mMultiBgmTable[id].mMultiId & 0xFEFEFFFF) == id) {
            return &mMultiBgmTable[id];
        }
    }

    for (const MultiBgmSetEntry* pEntry = mMultiBgmTable.begin(); pEntry < mMultiBgmTable.end(); pEntry++) {
        if ((mMultiBgmTable[id].mMultiId & 0xFEFEFFFF) == id) {
            return pEntry;
        }
    }

    return nullptr;
}
