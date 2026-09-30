#include "Game/System/GameEmbeddedTable.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapInfo.hpp"

namespace {
    static JMapInfo sGameGalaxyTable;
    static JMapInfo sGameStoryEventTable;
}

namespace GameEmbeddedTable {
    void init() {
        sGameGalaxyTable.attach(MR::receiveFile("/SystemData/GameGalaxyTable.bcsv"));
        sGameStoryEventTable.attach(MR::receiveFile("/SystemData/GameStoryEventTable.bcsv"));
    }

    JMapInfo getGalaxyTable() {
        return sGameGalaxyTable;
    }

    JMapInfoIter getGalaxyEntry(const char* pName) {
        return sGameGalaxyTable.findElement("Name", pName, 0);
    }

    u8 getStoryEventProgress(const char* pName) {
        JMapInfoIter iter = sGameStoryEventTable.findElement("Name", pName, 0);

        u32 progress = 0;
        iter.getValue("Progress", &progress);
        return progress;
    }
}
