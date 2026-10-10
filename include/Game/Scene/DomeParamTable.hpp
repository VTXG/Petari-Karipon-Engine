#pragma once

#include "Game/Util/ByamlIter.hpp"

class GameDataHolder;

namespace DomeParamTable {
    void init();
    s32 getDomeNum();
    ByamlIter getParam(s32 domeID);
    bool isOpenDome(const GameDataHolder* pGameDataHolder, s32 domeID);
}
