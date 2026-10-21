#include "Game/System/GalaxyNameSortTable.hpp"
#include "Game/Util/JMapInfo.hpp"

#ifdef KRP_CORE
#include <Karipon/System/GameEmbeddedTable.hpp>
#endif

s32 GalaxyNameSortTable::getGalaxySortIndex(const char* pName) {
    JMapInfoIter iter = GameEmbeddedTable::getGalaxyEntry(pName);

    if (!iter.isValid()) {
        return -1;
    }

    return iter.mIndex;
}
