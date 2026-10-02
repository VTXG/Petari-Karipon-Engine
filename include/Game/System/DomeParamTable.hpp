#pragma once

#include "Game/System/GameDataHolder.hpp"
#include "Game/Util/ByamlIter.hpp"

namespace DomeParamTable {
    void init();
    s32 getDomeNum();
    ByamlIter getParam(s32 domeID);
    bool isOpenDome(const GameDataHolder* pGameDataHolder, s32 domeID);
}
