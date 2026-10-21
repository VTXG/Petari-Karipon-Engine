#pragma once

#include "Game/LiveActor/LiveActor.hpp"

#ifdef KRP_CORE
#include <Karipon/Util/ByamlIter.hpp>
#endif

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
#ifdef KRP_CORE
    krp::ByamlIter getOrbitParams(s32 idx);
#endif
    void updateCometStatus();

    /* 0x8C */ LiveActorGroup* mMiniatureGalaxyGroup;
    /* 0x90 */ bool _90;
    /* 0x94 */ MiniatureGalaxy* mCometGalaxy;
    /* 0x98 */ s32 mCometID;
    /* 0x9C */ s32 _9C;
#ifdef KRP_CORE
    /* 0x100 */ krp::ByamlIter mParamTable;
#endif
};

class MiniatureGalaxyFunction {
public:
    static void createHolder();
    static MiniatureGalaxyHolder* getHolder();

    static s32 calcMiniatureGalaxyIndex(const LiveActor* pActor);
#ifdef KRP_CORE
    static krp::ByamlIter getMiniatureGalaxyOrbitParams(s32 idx);
#endif

    static s32 getMiniatureGalaxyNum();
    static void updateCometStatus();
    static MiniatureGalaxy* getCometLandMiniatureGalaxy();
    static s32 getCometNameId();
    static MiniatureGalaxy* getPointingMiniatureGalaxy();
};
