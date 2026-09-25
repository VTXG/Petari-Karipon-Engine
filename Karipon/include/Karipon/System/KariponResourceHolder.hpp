#pragma once

#include "Game/GameAudio/ExAudStageBgm.hpp"
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

    void initGalaxyResource();
    void initEventResource();
    void initAudioResource();

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
    const StageBgmEntry* findStageBgmEntry(const char* pStageName, s32 scenarioNo);
    const StageBgmSetEntry* findStageBgmSetEntry(const char* pStageName);
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
    MR::AssignableArray< StageBgmEntry > mStageBgmTable;
    MR::AssignableArray< StageBgmSetEntry > mStageBgmSetTable;
    JMapInfo mMultiBgmTable;
    ByamlIter mStageWaveTable;
};
