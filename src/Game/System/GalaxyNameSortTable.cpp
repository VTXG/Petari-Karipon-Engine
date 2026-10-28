#include "Game/System/GalaxyNameSortTable.hpp"
#include "Game/Util/JMapInfo.hpp"

#ifdef KRP_CORE
#include <Karipon/System/SystemDataHolder.hpp>
#endif

#ifndef KRP_CORE
extern const JMapData GalaxyIDBCSV;
#endif

s32 GalaxyNameSortTable::getGalaxySortIndex(const char* pName) {
#ifdef KRP_CORE
    JMapInfoIter iter = krp::SystemDataHolder::get()->getGalaxyInfo(pName);

    if (!iter.isValid()) {
        return -1;
    }

    return iter.mIndex;
#else
    JMapInfo info;
    info.attach(&GalaxyIDBCSV);

    JMapInfoIter iter = info.findElement("name", pName, 0);

    if (iter == info.end()) {
        return -1;
    }
#endif
}
