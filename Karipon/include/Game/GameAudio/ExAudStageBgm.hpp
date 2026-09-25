#pragma once

#include <Game/AudioLib/AudBgmSetting.hpp>
#include <Game/Util/Array.hpp>
#include <revolution/types.h>

struct StageBgmEntry {
    const char* mName;
    s32 mScenarioNo;
    u32 mBgmId;
    s32 mStartType;
};

struct StageBgmSetEntry {
    const char* mName;
    MR::AssignableArray< u32 > mChangeBgmId;
    MR::AssignableArray< s32 > mChangeBgmState;
};

struct MultiBgmSetEntry {
    u32 mMultiId;
    AudBgmSetting::MultiBgmSet mSettings;
};
