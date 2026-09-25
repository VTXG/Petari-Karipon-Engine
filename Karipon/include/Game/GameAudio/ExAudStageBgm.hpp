#pragma once

#include <Game/Util/Array.hpp>
#include <revolution/types.h>

struct StageBgmEntry {
    const char* mName;
    s32 mScenarioNo;
    const char* mBgmIDName;
    s32 mStartType;
};

struct StageBgmSetEntry {
    const char* mName;
    MR::AssignableArray< const char* > mChangeBgmIDName;
    MR::AssignableArray< s32 > mChangeBgmState;
};
