#pragma once

#ifdef KRP_CORE

#include "Karipon/Util/ByamlIter.hpp"

namespace StageParamTable {
    void init();
    bool findParams(krp::ByamlIter* pIter, const char* pStageName, s32 scenarioNo);
    bool findParamsCommon(krp::ByamlIter* pIter, const char* pStageName);
}

#endif
