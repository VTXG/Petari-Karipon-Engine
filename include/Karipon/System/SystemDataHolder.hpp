#pragma once

#include "Karipon/Util/ByamlIter.hpp"
#include <Game/Util/Array.hpp>
#include <Game/Util/JMapInfo.hpp>

class GalaxyCometTimePaper;
class GameEventFlag;
class GameEventValue;
class JKRMemArchive;

namespace krp {
    class SystemDataHolder {
    public:
        SystemDataHolder();

        void init();
        void initCometTable();
        void initEventFlagTable(const JMapInfo* pInfo);
        void initEventValueTable(const JMapInfo* pInfo);

        JMapInfoIter getGalaxyInfo(const char* pName);
        u8 getStoryEventProgress(const char* pName);
        krp::ByamlIter getStageParams(const char* pName, s32 scenarioNo = 0);
        krp::ByamlIter getDomeParams(s32 scenarioNo);

        static SystemDataHolder* get();
        // static JMapInfo getGalaxyTable();
        // static JMapInfoIter getGalaxyTableIter(const char* pName);
        // static MR::AssignableArray< GalaxyCometTimePaper* >& getCometTable();
        // static MR::AssignableArray< GameEventFlag >& getEventFlagTable();
        // static MR::AssignableArray< GameEventValue >& getEventValueTable();
        // static u8 getStoryEventProgress(const char* pName);
        // static krp::ByamlIter getStageParamsIter(const char* pName, s32 scenarioNo = 0);

        JKRMemArchive* mArchive;
        JMapInfo mGalaxyInfoTable;
        MR::AssignableArray< GalaxyCometTimePaper* > mCometTable;
        MR::AssignableArray< GameEventFlag > mEventFlagTable;
        MR::AssignableArray< GameEventValue > mEventValueTable;
        JMapInfo mStoryEventTable;
        krp::ByamlIter mStageParamTable;
        krp::ByamlIter mDomeParamTable;
    };
}; // namespace krp
