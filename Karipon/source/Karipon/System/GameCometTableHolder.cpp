#include "Karipon/System/GameCometTableHolder.hpp"
#include "Kamek.hpp"
#include "Karipon/System/ByamlIter.hpp"
#include "Karipon/System/ByamlUtil.hpp"
#include <Game/NPC/CometEventKeeper.hpp>
#include <Game/Screen/GalaxyCometScreenFilter.hpp>
#include <Game/System/GalaxyCometScheduler.hpp>
#include <Game/Util/SceneUtil.hpp>
#include <Game/Util/StringUtil.hpp>
#include <cstdio>

#define COMET_CATEGORY_NORMAL 0
#define COMET_CATEGORY_COIN_100 1

namespace {
    static ByamlIter sCometParamsIt;
    static ByamlIter sCometCyclesIt;

    static const char* cCometCategoryTable[] = {
        /* 0x0 */ "Normal",
        /* 0x1 */ "Coin100",
    };

    template <typename T>
    static bool tryGetCometParam(T* pOut, const char* pKey, const char* pStageName, s32 scenarioNo) {
        char key[16];
        snprintf(key, sizeof(key), "Scenario%d", scenarioNo);

        ByamlIter paramIt = sCometParamsIt.getIterByKey(pStageName).getIterByKey(key);

        if (paramIt.isValid() && paramIt.tryGetValueByKey(pOut, pKey)) {
            return true;
        }

        return false;
    }

    static const GalaxyCometTimePaper* createCometTimePaper(s32 index) {
        ByamlIter cycleListIt = sCometCyclesIt.getIterByIndex(index);
        s32 size = cycleListIt.getSize();

        GalaxyCometTimePaper* pTimePaper = new GalaxyCometTimePaper[size + 1];

        for (s32 i = 0; i < size; i++) {
            ByamlIter cycleIt = cycleListIt.getIterByIndex(i);

            const char* pName = nullptr;
            cycleIt.tryGetValueByKey(&pName, "Name");
            pTimePaper[i].mGalaxyName = pName;

            const char* pCategory = nullptr;
            cycleIt.tryGetValueByKey(&pCategory, "Category");
            pTimePaper[i].mCategory = COMET_CATEGORY_NORMAL;

            if (pCategory != nullptr) {
                for (u32 category = COMET_CATEGORY_NORMAL; category <= COMET_CATEGORY_COIN_100; category++) {
                    if (MR::isEqualString(pCategory, cCometCategoryTable[category])) {
                        pTimePaper[i].mCategory = category;
                        break;
                    }
                }
            }
        }

        pTimePaper[size].mGalaxyName = nullptr;
        pTimePaper[size].mCategory = 0;
        return pTimePaper;
    }
} // namespace

void GameCometTableHolder::init() {
    ByamlIter tableRootIt = ByamlUtil::createByamlRootFromFile("/SystemData/GameCometTable.byaml");

    // Comet parameters
    {
        sCometParamsIt = tableRootIt.getIterByKey("CometParams");
    }

    // Comet cycles
    {
        sCometCyclesIt = tableRootIt.getIterByKey("CometCycles");
    }
}

namespace {
    static u32 getCometTimeLimit() {
        u32 cometTimeLimit = 0;
        tryGetCometParam(&cometTimeLimit, "TimeLimit", MR::getCurrentStageName(), MR::getCurrentScenarioNo());
        return cometTimeLimit;
    }

    static void initCometFilter(CometEventKeeper* pCometEventKeeper, const char* pCometFilter) {
        tryGetCometParam(&pCometFilter, "CometFilter", MR::getCurrentStageName(), MR::getCurrentScenarioNo());

        if (pCometFilter != nullptr) {
            GalaxyCometScreenFilter* pFilter = new GalaxyCometScreenFilter();
            pFilter->initWithoutIter();
            pFilter->_20 = true;
            pFilter->setCometType(pCometFilter);
            pCometEventKeeper->mScreenFilter = pFilter;
        }
    }

    static GalaxyCometScheduler* ctorGalaxyCometScheduler(GalaxyCometScheduler* pCometScheduler) {
        pCometScheduler->mEnabled = true;

        s32 size = sCometCyclesIt.getSize();
        pCometScheduler->mTimeTables.init(size);

        for (s32 i = 0; i < size; i++) {
            pCometScheduler->mTimeTables[i] = new GalaxyCometTimeTable(nullptr);
            pCometScheduler->mTimeTables[i]->mTimePaper = createCometTimePaper(i);
        }

        return pCometScheduler;
    }
} // namespace

extern kmSymbol init__16CometEventKeeperFv;
kmCall(&init__16CometEventKeeperFv + 0x68, getCometTimeLimit);
kmWrite32(&init__16CometEventKeeperFv + 0x6C, PPC_MR(4, 3));
kmWrite32(&init__16CometEventKeeperFv + 0x70, PPC_NOP);

extern kmSymbol init__16CometEventKeeperFv;
kmWrite32(&init__16CometEventKeeperFv + 0xA0, PPC_MR(3, 30));
kmWrite32(&init__16CometEventKeeperFv + 0xA4, PPC_LWZ(4, 0x8, 30));
kmCall(&init__16CometEventKeeperFv + 0xA8, initCometFilter);
kmWrite32(&init__16CometEventKeeperFv + 0xAC, PPC_B(0x34));

extern kmSymbol __ct__20GalaxyCometSchedulerFv;
kmWrite32(&__ct__20GalaxyCometSchedulerFv + 0x60, PPC_ADDI(4, 29, 1));

extern kmSymbol __ct__20GalaxyCometSchedulerFv;
kmBranch(&__ct__20GalaxyCometSchedulerFv, ctorGalaxyCometScheduler);
