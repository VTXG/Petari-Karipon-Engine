#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/ByamlIter.hpp"

class LiveActorGroup;
class MiniatureGalaxy;

class MiniatureGalaxyHolder : public LiveActor {
public:
    MiniatureGalaxyHolder();

    virtual void init(const JMapInfoIter&);

    void registerActor(LiveActor*, const JMapInfoIter&);
    bool isRegisteredActor(const LiveActor*);
    MiniatureGalaxy* findMiniatureGalaxy(const char*) const;
    void killAllMiniatureGalaxy();
    s32 calcIndex(const LiveActor*) const;
    ByamlIter getOrbitParams(s32 idx);
    void updateCometStatus();

    /* 0x8C */ LiveActorGroup* mMiniatureGalaxyGroup;
    /* 0x90 */ bool _90;
    /* 0x94 */ MiniatureGalaxy* mCometGalaxy;
    /* 0x98 */ s32 mCometID;
    /* 0x9C */ s32 _9C;
    /* 0x100 */ ByamlIter mParamTable;
};

class MiniatureGalaxyFunction {
public:
    static void createHolder();
    static MiniatureGalaxyHolder* getHolder();

    static s32 calcMiniatureGalaxyIndex(const LiveActor* pActor);
    static ByamlIter getMiniatureGalaxyOrbitParams(s32 idx);

    static s32 getMiniatureGalaxyNum();
    static void updateCometStatus();
    static MiniatureGalaxy* getCometLandMiniatureGalaxy();
    static s32 getCometNameId();
    static MiniatureGalaxy* getPointingMiniatureGalaxy();
};
