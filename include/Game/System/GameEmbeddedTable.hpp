#pragma once

#include <revolution/types.h>

class JMapInfo;
class JMapInfoIter;

namespace GameEmbeddedTable {
    void init();
    JMapInfo getGalaxyTable();
    JMapInfoIter getGalaxyIter(const char* pName);
    u8 getStoryEventProgress(const char* pName);
}
