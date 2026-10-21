#pragma once

#ifdef KRP_CORE

#include "Karipon/Util/ByamlIter.hpp"

class GameDataHolder;

namespace DomeParamTable {
    void init();
    s32 getDomeNum();
    krp::ByamlIter getParam(s32 domeID);
    bool isOpenDome(const GameDataHolder* pGameDataHolder, s32 domeID);
}; // namespace DomeParamTable

#endif
