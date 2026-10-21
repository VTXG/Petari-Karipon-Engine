#include "Game/NPC/CometEventKeeper.hpp"
#include "Game/NPC/CometEventExecutorTimeLimit.hpp"
#include "Game/Screen/GalaxyCometScreenFilter.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

#ifdef KRP_CORE
#include <Karipon/System/StageParamTable.hpp>
#endif

CometEventKeeper::CometEventKeeper() : mExecutorTimeLimit(nullptr), mScreenFilter(nullptr), mCometName(nullptr), mCometStateIndex(0) {
}

void CometEventKeeper::init() {
    initCometStatus();

#ifdef KRP_CORE
    krp::ByamlIter params;
    bool isFindParams = StageParamTable::findParams(&params, MR::getCurrentStageName(), MR::getCurrentScenarioNo());

    if (isStartEvent("Red") || isStartEvent("Black")) {
        u32 timeLimit = 0;

        if (isFindParams) {
            params.tryGetValueByKey("TimeLimit", &timeLimit);
        }

        mExecutorTimeLimit = new CometEventExecutorTimeLimit(timeLimit);
        mExecutorTimeLimit->initWithoutIter();
        mExecutorTimeLimit->kill();
    }

    const char* cometFilterName = mCometName;

    if (isFindParams) {
        params.tryGetValueByKey("CometFilter", &cometFilterName);
    }

    if (cometFilterName != nullptr) {
        mScreenFilter = new GalaxyCometScreenFilter();
        mScreenFilter->initWithoutIter();
        mScreenFilter->_20 = true;
        mScreenFilter->setCometType(cometFilterName);
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

void CometEventKeeper::initCometStatus() {
    mCometName = MR::makeCurrentGalaxyStatusAccessor().getCometName(MR::getCurrentScenarioNo());
    mCometStateIndex = MR::getGalaxyCometStateIndexInCurrentStage();
}
