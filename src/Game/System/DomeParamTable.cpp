#include "Game/System/DomeParamTable.hpp"
#include "Game/Util/ByamlIter.hpp"
#include "Game/Util/ByamlUtil.hpp"
#include "Game/Util/FileUtil.hpp"

namespace {
    static ByamlIter sDomeParamTable;
}

namespace DomeParamTable {
    void init() {
        void* pFileData = MR::receiveFile("/SystemData/DomeParamTable.byaml");
        sDomeParamTable = ByamlUtil::createByamlRoot(static_cast< u8* >(pFileData));
    }

    s32 getDomeNum() {
        return sDomeParamTable.getSize();
    }

    ByamlIter getParam(s32 domeID) {
        return sDomeParamTable[domeID - 1];
    }

    bool isOpenDome(const GameDataHolder* pGameDataHolder, s32 domeID) {
        ByamlIter params = DomeParamTable::getParam(domeID);

        const char* pEventName = nullptr;
        params.tryGetValueByKey("UnlockEventFlag", &pEventName);

        return pEventName == nullptr || pGameDataHolder->isOnGameEventFlag(pEventName);
    }
}
