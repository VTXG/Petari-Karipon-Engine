#include "Game/NPC/CometEventKeeper.hpp"
#include "Game/NPC/CometEventExecutorTimeLimit.hpp"
#include "Game/Scene/StageParamTable.hpp"
#include "Game/Screen/GalaxyCometScreenFilter.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

CometEventKeeper::CometEventKeeper() : mExecutorTimeLimit(nullptr), mScreenFilter(nullptr), mCometName(nullptr), mCometStateIndex(0) {
}

void CometEventKeeper::init() {
    initCometStatus();

    ByamlIter params;
    bool isExistParams = StageParamTable::tryGetParams(&params, MR::getCurrentStageName(), MR::getCurrentScenarioNo());

    if (isStartEvent("Red") || isStartEvent("Black")) {
        u32 timeLimit = 0;

        if (isExistParams) {
            params.tryGetValueByKey(&timeLimit, "TimeLimit");
        }

        mExecutorTimeLimit = new CometEventExecutorTimeLimit(timeLimit);
        mExecutorTimeLimit->initWithoutIter();
        mExecutorTimeLimit->kill();
    }

    const char* pCometFilterName = mCometName;

    if (isExistParams) {
        params.tryGetValueByKey(&pCometFilterName, "CometFilter");
    }

    if (pCometFilterName != nullptr) {
        mScreenFilter = new GalaxyCometScreenFilter();
        mScreenFilter->initWithoutIter();
        mScreenFilter->_20 = true;
        mScreenFilter->setCometType(pCometFilterName);
    }
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
