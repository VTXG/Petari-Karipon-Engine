#include "Game/Util/ExSingletonHolder.hpp"
#include "Kamek.hpp"
#include "Karipon/System/KariponResourceHolder.hpp"
#include <Game/NPC/CometEventKeeper.hpp>
#include <Game/Screen/GalaxyCometScreenFilter.hpp>
#include <Game/System/GalaxyCometScheduler.hpp>
#include <Game/Util/SceneUtil.hpp>
namespace {
    static u32 getCometTimeLimit() {
        u32 cometTimeLimit = 0;
        ExSingletonHolder< KariponResourceHolder >::get()->tryGetStageParam(&cometTimeLimit, "TimeLimit", MR::getCurrentStageName(),
                                                                            MR::getCurrentScenarioNo());
        return cometTimeLimit;
    }

    static void initCometFilter(CometEventKeeper* pCometEventKeeper, const char* pCometFilter) {
        ExSingletonHolder< KariponResourceHolder >::get()->tryGetStageParam(&pCometFilter, "CometFilter", MR::getCurrentStageName(),
                                                                            MR::getCurrentScenarioNo());

        if (pCometFilter != nullptr) {
            GalaxyCometScreenFilter* pFilter = new GalaxyCometScreenFilter();
            pFilter->initWithoutIter();
            pFilter->_20 = true;
            pFilter->setCometType(pCometFilter);
            pCometEventKeeper->mScreenFilter = pFilter;
        }
    }

    static GalaxyCometScheduler* ctorGalaxyCometScheduler(GalaxyCometScheduler* pCometScheduler) {
        KariponResourceHolder* pHolder = ExSingletonHolder< KariponResourceHolder >::get();

        s32 size = pHolder->mCometCycleTables.size();
        pCometScheduler->mTimeTables.init(size);

        for (s32 i = 0; i < size; i++) {
            pCometScheduler->mTimeTables[i] = new GalaxyCometTimeTable(pHolder->mCometCycleTables[i]);
        }

        pCometScheduler->mEnabled = true;
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
