#include "Game/System/GameStoryEventTable.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapInfo.hpp"

namespace {
    static void* sGameStoryEventTable;
}

namespace GameStoryEventTable {
    void init() {
        if (sGameStoryEventTable != nullptr) {
            return;
        }

        sGameStoryEventTable = MR::receiveFile("/SystemData/GameStoryEventTable.bcsv");
    }

    u8 getStoryEventProgress(const char* pName) {
        JMapInfo info;
        info.attach(sGameStoryEventTable);

        JMapInfoIter iter = info.findElement("Name", pName, 0);

        u32 progress = 0;
        iter.getValue("Progress", &progress);
        return progress;
    }
}
