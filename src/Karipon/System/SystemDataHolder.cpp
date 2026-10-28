#ifdef KRP_CORE

#include "Karipon/System/SystemDataHolder.hpp"
#include "Karipon/Util/ByamlUtil.hpp"
#include <Game/System/GalaxyCometScheduler.hpp>
#include <Game/System/GameEventFlag.hpp>
#include <Game/System/GameEventValueChecker.hpp>
#include <Game/System/GameSystem.hpp>
#include <Game/Util/FileUtil.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/Util/StringUtil.hpp>
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <cstdio>

#define COMET_CATEGORY_NORMAL 0
#define COMET_CATEGORY_COIN_100 1

namespace {
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

    static const char* const cCometCategoryTable[] = {
        /* 0x0 */ "Normal",
        /* 0x1 */ "Coin100",
    };

    static void attachResource(JKRMemArchive* pArchive, JMapInfo& rIter, const char* pResourcePath) {
        rIter.attach(pArchive->getResource(pResourcePath));
    }

    static void attachResource(JKRMemArchive* pArchive, krp::ByamlIter& rIter, const char* pResourcePath) {
        rIter.attach(pArchive->getResource(pResourcePath));
    }
} // namespace

namespace krp {
    SystemDataHolder::SystemDataHolder()
        : mArchive(), mGalaxyInfoTable(), mCometTable(), mEventFlagTable(), mEventValueTable(), mStoryEventTable(), mStageParamTable(),
          mDomeParamTable() {}

    void SystemDataHolder::init() {
        mArchive = MR::receiveArchive("SystemData/SystemDataTable.bcsv");

        ::attachResource(mArchive, mGalaxyInfoTable, "GalaxyInfoTable.bcsv");
        initCometTable();

        JMapInfo gameEventFlagTable;
        ::attachResource(mArchive, gameEventFlagTable, "GameEventFlagTable.bcsv");
        initEventFlagTable(&gameEventFlagTable);

        JMapInfo gameEventValueTable;
        ::attachResource(mArchive, gameEventValueTable, "GameEventValueTable.bcsv");
        initEventValueTable(&gameEventValueTable);

        ::attachResource(mArchive, mStoryEventTable, "GameStoryEventTable.bcsv");
        ::attachResource(mArchive, mStageParamTable, "StageParamTable.byaml");
        ::attachResource(mArchive, mDomeParamTable, "DomeParamTable.byaml");
    }

    void SystemDataHolder::initCometTable() {
        char resourceName[0x100];
        bool resourceExists = false;
        u32 count = 0;

        do {
            snprintf(resourceName, sizeof(resourceName), "GalaxyCometTable%d", count + 1);

            if (mArchive->getResource(resourceName) == nullptr) {
                resourceExists = false;
            } else {
                resourceExists = true;
                count++;
            }
        } while (resourceExists);

        for (u32 tableIdx = 0; tableIdx < count; tableIdx++) {
            snprintf(resourceName, sizeof(resourceName), "GalaxyCometTable%d", tableIdx + 1);

            JMapInfo info;
            info.attach(mArchive->getResource(resourceName));

            s32 entryNum = info.getNumEntries();

            GalaxyCometTimePaper* timePaperTable = new GalaxyCometTimePaper[entryNum + 1];
            timePaperTable[entryNum].mGalaxyName = nullptr;
            timePaperTable[entryNum].mCategory = 0;

            for (s32 paperIdx = 0; paperIdx < entryNum; paperIdx++) {
                GalaxyCometTimePaper& timePaper = timePaperTable[paperIdx];
                const char* categoryName = nullptr;

                MR::getCsvDataStr(&timePaper.mGalaxyName, &info, "Name", paperIdx);
                MR::getCsvDataStr(&categoryName, &info, "Category", paperIdx);

                timePaper.mCategory = 0xFFFFFFFF;

                for (u32 type = 0; type <= ARRAY_SIZE(cEventFlagTypeTable); type++) {
                    if (MR::isEqualString(categoryName, ::cCometCategoryTable[type])) {
                        timePaper.mCategory = type;
                        break;
                    }
                }

                if (timePaper.mGalaxyName[0] == '\0' || timePaper.mCategory == 0xFFFFFFFF) {
                    OSPanic(__FILE__, __LINE__, "Invalid comet entry in table %d at index %d\n", tableIdx + 1, paperIdx);
                }
            }
        }
    }

    void SystemDataHolder::initEventFlagTable(const JMapInfo* pInfo) {
        s32 entryNum = pInfo->getNumEntries();
        mEventFlagTable.init(entryNum);

        for (s32 i = 0; i < entryNum; i++) {
            GameEventFlag& eventFlag = mEventFlagTable[i];
            const char* typeName = nullptr;
            bool save = false;

            MR::getCsvDataStr(&eventFlag.mName, pInfo, "Name", i);
            MR::getCsvDataStr(&typeName, pInfo, "Type", i);
            MR::getCsvDataBool(&save, pInfo, "Save", i);
            MR::getCsvDataU8(&eventFlag.mCondition1, pInfo, "Condition1", i);
            MR::getCsvDataU8(&eventFlag.mCondition2, pInfo, "Condition2", i);
            MR::getCsvDataStrOrNULL(&eventFlag.mCondition3, pInfo, "Condition3", i);
            MR::getCsvDataStrOrNULL(&eventFlag.mCondition4, pInfo, "Condition4", i);

            eventFlag.mSaveFlag = save == false ? 1 : 0;
            eventFlag.mType = 0xFF;

            for (u32 type = 0; type <= ARRAY_SIZE(cEventFlagTypeTable); type++) {
                if (MR::isEqualString(typeName, ::cEventFlagTypeTable[type])) {
                    eventFlag.mType = type;
                    break;
                }
            }

            if (eventFlag.mName[0] == '\0' || eventFlag.mType == 0xFF) {
                OSPanic(__FILE__, __LINE__, "Invalid event flag at index %d\n", i);
            }
        }
    }

    void SystemDataHolder::initEventValueTable(const JMapInfo* pInfo) {
        s32 entryNum = pInfo->getNumEntries();
        mEventValueTable.init(entryNum);

        for (s32 i = 0; i < entryNum; i++) {
            GameEventValue& eventValue = mEventValueTable[i];
            s32 defaultValue = 0;

            MR::getCsvDataStrOrNULL(&eventValue.mName, pInfo, "Name", i);
            MR::getCsvDataS32(&defaultValue, pInfo, "DefaultValue", i);

            eventValue.mDefaultValue = defaultValue & 0xFFFF;

            if (eventValue.mName[0] == '\0') {
                OSPanic(__FILE__, __LINE__, "Invalid event value at index %d\n", i);
            }
        }
    }

    JMapInfoIter SystemDataHolder::getGalaxyInfo(const char* pName) {
        return mGalaxyInfoTable.findElement("Name", pName, 0);
    }

    u8 SystemDataHolder::getStoryEventProgress(const char* pName) {
        JMapInfoIter iter = mStoryEventTable.findElement("Name", pName, 0);

        u32 progress = 0;
        iter.getValue("Progress", &progress);
        return progress;
    }

    krp::ByamlIter SystemDataHolder::getStageParams(const char* pName, s32 scenarioNo) {
        krp::ByamlIter rootIter = mStageParamTable[pName];

        if (scenarioNo > 0) {
            char key[16];
            snprintf(key, sizeof(key), "Scenario%d", scenarioNo);

            krp::ByamlIter scenarioIter = rootIter[key];
            if (scenarioIter.isValid()) {
                return scenarioIter;
            }
        }

        return rootIter["Common"];
    }

    krp::ByamlIter SystemDataHolder::getDomeParams(s32 scenarioNo) {
        return mDomeParamTable[scenarioNo - 1];
    }

    SystemDataHolder* SystemDataHolder::get() {
        return SingletonHolder< GameSystem >::get()->mSystemDataHolder;
    }
}; // namespace krp

#endif
