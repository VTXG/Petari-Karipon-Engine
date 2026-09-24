#pragma once

#include "Karipon/System/ByamlIter.hpp"
#include <Game/Util/Array.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <cstdio>
#include <revolution/types.h>

struct GameEventFlag;
struct GameEventValue;
struct GalaxyCometTimePaper;

class KariponResourceHolder {
public:
    KariponResourceHolder();

    void init();
    void initGameEventFlags(void* pResource);
    void initGameEventValues(void* pResource);

    // Galaxy
    template < typename T >
    bool tryGetStageParam(T* pOut, const char* pKey, const char* pStageName, s32 scenarioNo) {
        char key[16];
        snprintf(key, sizeof(key), "Scenario%d", scenarioNo);

        ByamlIter paramIt = mStageParamTable.getIterByKey(pStageName).getIterByKey(key);

        if (paramIt.isValid() && paramIt.tryGetValueByKey(pOut, pKey)) {
            return true;
        }

        return false;
    }

    ByamlIter getDomeParamIter(s32 scenarioNo) { return mDomeParamTable.getIterByIndex(scenarioNo - 1); }

    // Event
    u8 getStoryEventProgress(const char* pEventName);

    // Audio
    ByamlIter getStageWaveStaticResourceIter() const { return mStageWaveTable.getIterByKey("StaticResource"); }
    ByamlIter getStageWaveStageResourceIter() const { return mStageWaveTable.getIterByKey("StageResource"); }

    // Galaxy
    void* mGalaxyIDTable;
    ByamlIter mStageParamTable;
    ByamlIter mDomeParamTable;
    MR::AssignableArray< GalaxyCometTimePaper* > mCometCycleTables;

    // Event
    MR::AssignableArray< GameEventFlag > mGameEventFlags;
    MR::AssignableArray< GameEventValue > mGameEventValues;
    JMapInfo mGameStoryEventTable;

    // Audio
    JMapInfo mStageBgmTable;
    JMapInfo mScenarioBgmTable;
    JMapInfo mMultiBgmTable;
    ByamlIter mStageWaveTable;
};
