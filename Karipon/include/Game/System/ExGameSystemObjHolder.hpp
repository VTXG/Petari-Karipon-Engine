#include "Karipon/System/SystemDebugMenu.hpp"
#include <Game/System/GameSystemObjHolder.hpp>

class ExGameSystemObjHolder : public GameSystemObjHolder {
public:
    /* 0x3C */ SystemDebugMenu* mSystemDebugMenu;
};
