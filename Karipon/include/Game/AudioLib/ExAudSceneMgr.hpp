#pragma once

#include "Karipon/System/ByamlIter.hpp"
#include <Game/AudioLib/AudSceneMgr.hpp>

class JAUSectionHeap;

class ExAudSceneMgr : private AudSceneMgr {
public:
    ExAudSceneMgr(JAUSectionHeap* pSectionHeap);

    void loadStaticResource();
    bool isLoadDoneStaticResource() const;

    void loadStageResource(const char* pStageName);
    bool isLoadDoneStageResource();

    void loadScenarioResource(const char* pStageName, s32 scenarioNo);
    bool isLoadDoneScenarioResource() const;

    void loadWaveSet(const ByamlIter& rIter);
    void eraseWaveSet(const ByamlIter& rIter);
    bool isLoadDoneWaveSet(const ByamlIter& rIter) const;

    s32 findWaveBankNo(const char* pWaveArcName) const;

    ByamlIter getRootIter(const char* pKey) const { return mTableRootIt.getIterByKey(pKey); }

    ByamlIter mTableRootIt;
    ByamlIter mWaveSetStageIt;
    ByamlIter mWaveSetScenarioIt;
};
