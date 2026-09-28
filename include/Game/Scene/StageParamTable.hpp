#pragma once

#include "Game/Util/ByamlIter.hpp"

namespace StageParamTable {
    void init();
    bool tryGetParams(ByamlIter* pIter, const char* pStageName, s32 scenarioNo);
}
