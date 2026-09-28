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
        if (sStageParamTable.isValid()) {
            return;
        }

        void* pFileData = MR::receiveFile("/SystemData/StageParamTable.byaml");
        sStageParamTable = ByamlUtil::createByamlRoot(static_cast< u8* >(pFileData));
    }

    bool tryGetParams(ByamlIter* pIter, const char* pStageName, s32 scenarioNo) {
        ByamlIter stageEntry;

        if (sStageParamTable.tryGetIterByKey(&stageEntry, pStageName)) {
            char key[16];
            snprintf(key, sizeof(key), "Scenario%d", scenarioNo);
            return stageEntry.tryGetIterByKey(pIter, key) || stageEntry.tryGetIterByKey(pIter, "Common");
        }

        return false;
    }
}  // namespace StageParamTable
