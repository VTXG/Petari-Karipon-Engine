#pragma once

#ifdef KRP_ONLINE

#include <Game/System/NerveExecutor.hpp>
#include <JSystem/JGeometry/TVec.hpp>

namespace krp {
    /// @brief Multiplayer player state.
    struct GameOnlinePlayerState {
    public:
        char mStageName[32];
        s32 mCurrentScenarioNo;

        TVec3f mPosition;
        TVec3f mRotation;
        TVec3f mScale;
    };

    /// @brief Multiplayer session manager.
    class GameOnlineManager : public NerveExecutor {
    public:
        /// @brief Constructs a new `GameOnlineManager`.
        GameOnlineManager();

        /// @brief Initializes the underlying network system.
        void init();

        /// @brief Closes the underlying network system.
        void close();

        /// @brief Updates the manager state.
        void update();

        /// @brief Disconnected nerve update.
        void exeDisconnected();
    };
}; // namespace krp

#endif
