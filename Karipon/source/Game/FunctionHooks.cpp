#include "Game/Util/ExSingletonHolder.hpp"
#include "Kamek.hpp"
#include "Karipon/Network/NetworkSystem.hpp"
#include "Karipon/System/KariponResourceHolder.hpp"
#include <Game/System/GameSystem.hpp>
#include <Game/System/GameSystemObjHolder.hpp>
#include <Game/Util/FileUtil.hpp>
#include <Game/Util/MemoryUtil.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>

namespace {
    static void initGameSystem(GameSystemObjHolder* pGameSystemObjHolder) {
        ExSingletonHolder< KariponResourceHolder >::init();
        ExSingletonHolder< KariponResourceHolder >::get()->init();
        pGameSystemObjHolder->init();
    }

    static void initAfterStationedResourceLoadedGameSystem(GameSystem* pGameSystem) {
        pGameSystem->initAfterStationedResourceLoaded();

        NetworkSystem* pNetworkSystem = ExSingletonHolder< NetworkSystem >::init();
        pNetworkSystem->initSystem(true);

        /*
        OSReport("NetworkSystem %d %d %d %d.%d.%d.%d\n",
            pNetworkSystem->mFd,
            pNetworkSystem->mError,
            pNetworkSystem->mState,
            pNetworkSystem->mIP.mOctets[0],
            pNetworkSystem->mIP.mOctets[1],
            pNetworkSystem->mIP.mOctets[2],
            pNetworkSystem->mIP.mOctets[3]);
        */
    }
} // namespace

extern kmSymbol init__10GameSystemFv;
kmCall(&init__10GameSystemFv + 0xB4, initGameSystem);

extern kmSymbol initAfterStationedResourceLoaded__18GameSystemFunctionFv;
kmBranch(&initAfterStationedResourceLoaded__18GameSystemFunctionFv + 0x4, initAfterStationedResourceLoadedGameSystem);
