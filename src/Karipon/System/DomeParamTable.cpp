#include "Karipon/System/DomeParamTable.hpp"
#include "Karipon/Util/ByamlIter.hpp"
#include "Karipon/Util/ByamlUtil.hpp"
#include <Game/System/GameDataHolder.hpp>
#include <Game/Util/FileUtil.hpp>

namespace {
    static krp::ByamlIter sDomeParamTable;
}

namespace DomeParamTable {
    void init() {
        void* fileData = MR::receiveFile("/SystemData/DomeParamTable.byaml");
        sDomeParamTable = krp::createByamlRoot(static_cast< u8* >(fileData));
    }

    s32 getDomeNum() {
        return sDomeParamTable.getSize();
    }

    krp::ByamlIter getParam(s32 domeID) {
        return sDomeParamTable[domeID - 1];
    }

    bool isOpenDome(const GameDataHolder* pGameDataHolder, s32 domeID) {
        krp::ByamlIter params = DomeParamTable::getParam(domeID);

        const char* eventName = nullptr;
        params.tryGetValueByKey("UnlockEventFlag", &eventName);

        return eventName == nullptr || pGameDataHolder->isOnGameEventFlag(eventName);
    }
} // namespace DomeParamTable
