#include "Game/Scene/StageParamTable.hpp"
#include "Game/Util/ByamlIter.hpp"
#include "Game/Util/ByamlUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include <cstdio>

namespace {
    static ByamlIter sStageParamTable;
}

namespace StageParamTable {
    void init() {
        void* pFileData = MR::receiveFile("/SystemData/StageParamTable.byaml");
        sStageParamTable = ByamlUtil::createByamlRoot(static_cast< u8* >(pFileData));
    }

    bool tryGetParams(ByamlIter* pIter, const char* pStageName, s32 scenarioNo) {
        ByamlIter stageEntry;

        if (sStageParamTable.tryGetIterByKey(pStageName, &stageEntry)) {
            char key[16];
            snprintf(key, sizeof(key), "Scenario%d", scenarioNo);
            return stageEntry.tryGetIterByKey(key, pIter) || stageEntry.tryGetIterByKey("Common", pIter);
        }

        return false;
    }
}  // namespace StageParamTable
