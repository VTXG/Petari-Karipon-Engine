#pragma once

#include "Game/Util/ByamlIter.hpp"

namespace StageParamTable {
    void init();
    bool findParams(ByamlIter* pIter, const char* pStageName, s32 scenarioNo);
    bool findParamsCommon(ByamlIter* pIter, const char* pStageName);
}
