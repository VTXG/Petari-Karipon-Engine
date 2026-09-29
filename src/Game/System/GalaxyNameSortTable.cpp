#include "Game/System/GalaxyNameSortTable.hpp"
#include "Game/System/GameEmbeddedTable.hpp"
#include "Game/Util/JMapInfo.hpp"

s32 GalaxyNameSortTable::getGalaxySortIndex(const char* pName) {
    JMapInfoIter iter = GameEmbeddedTable::getGalaxyIter(pName);

    if (!iter.isValid()) {
        return -1;
    }

    return iter.mIndex;
}
