#include "Game/NPC/CometEventKeeper.hpp"
#include "Game/NPC/CometEventExecutorTimeLimit.hpp"
#include "Game/Screen/GalaxyCometScreenFilter.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

#ifdef KRP_CORE
#include <Karipon/System/SystemDataHolder.hpp>
#endif

#ifndef KRP_CORE
struct GalaxyTimeLimitInfo {
    /* 0x0 */ const char* mGalaxyName;
    /* 0x4 */ s32 mScenarioNo;
    /* 0x8 */ u32 mTimeLimit;
};

namespace {
    static const GalaxyTimeLimitInfo sTimeLimitInfoTable[] = {
        {"EggStarGalaxy", 4, 240},      {"StarDustGalaxy", 4, 240}, {"SandClockGalaxy", 4, 270},
        {"CannonFleetGalaxy", 4, 360},  {"StarDustGalaxy", 5, 120}, {"PhantomGalaxy", 5, 60},
        {"CosmosGardenGalaxy", 5, 150}, {"FactoryGalaxy", 5, 180},  {"ReverseKingdomGalaxy", 5, 210},
    };
}; // namespace
#endif

CometEventKeeper::CometEventKeeper() : mExecutorTimeLimit(nullptr), mScreenFilter(nullptr), mCometName(nullptr), mCometStateIndex(0) {}

void CometEventKeeper::init() {
    initCometStatus();

#ifdef KRP_CORE
    krp::ByamlIter params = krp::SystemDataHolder::get()->getStageParams(MR::getCurrentStageName(), MR::getCurrentScenarioNo());
#endif

    if (isStartEvent("Red") || isStartEvent("Black")) {
#ifdef KRP_CORE
        u32 timeLimit = 0;

        if (params.isValid()) {
            params.tryGetValueByKey("TimeLimit", &timeLimit);
        }

        mExecutorTimeLimit = new CometEventExecutorTimeLimit(timeLimit);
#else
        mExecutorTimeLimit = new CometEventExecutorTimeLimit(getTimeLimitFromTable(::sTimeLimitInfoTable, ARRAY_SIZE(::sTimeLimitInfoTable)) / 60);
#endif
        mExecutorTimeLimit->initWithoutIter();
        mExecutorTimeLimit->kill();
    }

#ifdef KRP_CORE
    const char* cometFilterName = mCometName;

    if (params.isValid()) {
        params.tryGetValueByKey("CometFilter", &cometFilterName);
    }

    if (cometFilterName != nullptr) {
        mScreenFilter = new GalaxyCometScreenFilter();
        mScreenFilter->initWithoutIter();
        mScreenFilter->_20 = true;
        mScreenFilter->setCometType(cometFilterName);
    }
#else
    if (mCometName != nullptr) {
        mScreenFilter = new GalaxyCometScreenFilter();
        mScreenFilter->initWithoutIter();
        mScreenFilter->_20 = true;
        mScreenFilter->setCometType(mCometName);
    }
#endif
}

bool CometEventKeeper::isStartEvent(const char* pParam1) const {
    if (mCometName == nullptr) {
        return false;
    }

    return MR::isEqualString(mCometName, pParam1);
}

bool CometEventKeeper::isStartTimeLimitEvent() const {
    return mExecutorTimeLimit != nullptr;
}

void CometEventKeeper::startCometEventIfExist() {
    if (mExecutorTimeLimit == nullptr) {
        return;
    }

    mExecutorTimeLimit->appear();
}

void CometEventKeeper::endCometEvent() {
    if (mExecutorTimeLimit == nullptr) {
        return;
    }

    mExecutorTimeLimit->kill();
    mScreenFilter->_20 = false;
}

#ifndef KRP_CORE
u32 CometEventKeeper::getTimeLimitFromTable(const GalaxyTimeLimitInfo* pTable, int size) {
    for (int i = 0; i < size; i++) {
        const GalaxyTimeLimitInfo* pInfo = &pTable[i];

        if (pInfo->mScenarioNo != MR::getCurrentScenarioNo()) {
            continue;
        }

        if (!MR::isEqualString(MR::getCurrentStageName(), pInfo->mGalaxyName)) {
            continue;
        }

        return pInfo->mTimeLimit * 60;
    }

    return 0;
}
#endif

void CometEventKeeper::initCometStatus() {
    mCometName = MR::makeCurrentGalaxyStatusAccessor().getCometName(MR::getCurrentScenarioNo());
    mCometStateIndex = MR::getGalaxyCometStateIndexInCurrentStage();
}
