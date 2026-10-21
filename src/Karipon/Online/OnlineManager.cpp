#include "Karipon/Online/OnlineManager.hpp"
#include <Game/LiveActor/Nerve.hpp>

namespace {
    NEW_NERVE(GameOnlineManagerNrvDisconnected, krp::GameOnlineManager, Disconnected);
};

namespace krp {
    GameOnlineManager::GameOnlineManager() : NerveExecutor("GameOnlineManager") {
        initNerve(GET_NERVE_ANON(GameOnlineManagerNrvDisconnected));
    }

    void GameOnlineManager::init() {}

    void GameOnlineManager::close() {}

    void GameOnlineManager::update() {
        updateNerve();
    }

    void GameOnlineManager::exeDisconnected() {}
}; // namespace krp
