#include "Karipon/System/StageParamTable.hpp"
#include "Karipon/Util/ByamlIter.hpp"
#include "Karipon/Util/ByamlUtil.hpp"
#include <Game/Util/FileUtil.hpp>
#include <cstdio>

namespace {
    static krp::ByamlIter sStageParamTable;
}

namespace StageParamTable {
    void init() {
        void* fileData = MR::receiveFile("/SystemData/StageParamTable.byaml");
        sStageParamTable = krp::createByamlRoot(static_cast< u8* >(fileData));
    }

    bool findParams(krp::ByamlIter* pIter, const char* pStageName, s32 scenarioNo) {
        krp::ByamlIter stageEntry;

        if (sStageParamTable.tryGetIterByKey(pStageName, &stageEntry)) {
            char key[16];
            snprintf(key, sizeof(key), "Scenario%d", scenarioNo);
            return stageEntry.tryGetIterByKey(key, pIter) || stageEntry.tryGetIterByKey("Common", pIter);
        }

        return false;
    }

    bool findParamsCommon(krp::ByamlIter* pIter, const char* pStageName) {
        krp::ByamlIter stageEntry;

        if (sStageParamTable.tryGetIterByKey(pStageName, &stageEntry)) {
            return stageEntry.tryGetIterByKey("Common", pIter);
        }

        return false;
    }
}; // namespace StageParamTable
