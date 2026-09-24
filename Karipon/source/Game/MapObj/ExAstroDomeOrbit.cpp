#include "Game/MapObj/ExAstroDomeOrbit.hpp"
#include "Game/Util/ExSingletonHolder.hpp"
#include "Kamek.hpp"
#include "Karipon/System/ByamlIter.hpp"
#include "Karipon/System/ByamlUtil.hpp"
#include "Karipon/System/KariponResourceHolder.hpp"
#include "Macros.hpp"
#include <Game/AudioLib/AudSoundNameConverter.hpp>
#include <Game/AudioLib/AudWrap.hpp>
#include <Game/Util/SceneUtil.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <cstddef>
#include <revolution/gx/GXStruct.h>
#include <revolution/types.h>

namespace {
    static void setupAstroDomeOrbit(ExAstroDomeOrbit* pOrbit, s32 id) {
        pOrbit->mAngle = 230.0f * id; // ::cGalaxyRotateCoordOffset

        ByamlIter orbitIt =
            ExSingletonHolder< KariponResourceHolder >::get()->getDomeParamIter(MR::getCurrentScenarioNo()).getIterByKey("Orbits").getIterByIndex(id);

        ByamlUtil::getColorValue(orbitIt.getIterByKey("Color"), &pOrbit->mColor);
        ByamlUtil::getColorValue(orbitIt.getIterByKey("Bloom"), &pOrbit->mBloom);

        f32 radius = 0.0f;
        orbitIt.tryGetValueByKey(&radius, "Radius");
        pOrbit->mOrbitRadius = radius;

        ByamlUtil::getVector3Value(orbitIt.getIterByKey("Rotate"), &pOrbit->mRotation);
    }

    static const char* getDomeSkyName(s32 scenarioNo) {
        const char* pSkyName = nullptr;
        ExSingletonHolder< KariponResourceHolder >::get()->getDomeParamIter(scenarioNo).tryGetValueByKey(&pSkyName, "Sky");
        return pSkyName;
    }

    static const char* getSkyNameForAstroList() {
        register s32 scenarioNo;
        GET_REGISTER(scenarioNo, r29);
        return getDomeSkyName(scenarioNo);
    }

    static u32 getKoopaFortressBgmId() {
        const char* pBgmName = nullptr;
        ExSingletonHolder< KariponResourceHolder >::get()
            ->getDomeParamIter(MR::getCurrentScenarioNo())
            .tryGetValueByKey(&pBgmName, "KoopaFortressBgm");
        return static_cast< u32 >(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pBgmName));
    }
} // namespace

extern kmSymbol init__15MiniatureGalaxyFRC12JMapInfoIter;
kmWrite16(&init__15MiniatureGalaxyFRC12JMapInfoIter + 0x176, sizeof(ExAstroDomeOrbit));

extern kmSymbol setup__14AstroDomeOrbitFl;
kmBranch(&setup__14AstroDomeOrbitFl, setupAstroDomeOrbit);

extern kmSymbol draw__14AstroDomeOrbitCFv;
kmWrite32(&draw__14AstroDomeOrbitCFv + 0x24, PPC_ADDI(4, 31, offsetof(ExAstroDomeOrbit, mColor)));

extern kmSymbol drawBloom__14AstroDomeOrbitCFv;
kmWrite32(&drawBloom__14AstroDomeOrbitCFv + 0x24, PPC_ADDI(4, 31, offsetof(ExAstroDomeOrbit, mBloom)));

extern kmSymbol init__12AstroDomeSkyFRC12JMapInfoIter;
kmWrite32(&init__12AstroDomeSkyFRC12JMapInfoIter + 0x38, PPC_LWZ(3, 8, 1));
kmCall(&init__12AstroDomeSkyFRC12JMapInfoIter + 0x3C, getDomeSkyName);
kmWrite32(&init__12AstroDomeSkyFRC12JMapInfoIter + 0x40, PPC_MR(4, 3));
kmWrite32(&init__12AstroDomeSkyFRC12JMapInfoIter + 0x48, PPC_MR(3, 28));
kmWrite32(&init__12AstroDomeSkyFRC12JMapInfoIter + 0x50, PPC_MR(30, 4));
kmWrite32(&init__12AstroDomeSkyFRC12JMapInfoIter + 0x54, PPC_B(0x8));

extern kmSymbol getModelName__19AstroMapObjFunctionFPCcl;
kmCall(&getModelName__19AstroMapObjFunctionFPCcl + 0x54, getSkyNameForAstroList);
kmWrite32(&getModelName__19AstroMapObjFunctionFPCcl + 0x58, PPC_B(0x54));

extern kmSymbol getKoopaFortressAppearBgm__15AudStageBgmWrapFPCc;
kmCall(&getKoopaFortressAppearBgm__15AudStageBgmWrapFPCc + 0x38, getKoopaFortressBgmId);
kmWrite32(&getKoopaFortressAppearBgm__15AudStageBgmWrapFPCc + 0x3C, PPC_MR(0, 3));

extern kmSymbol exeOpen__15MiniatureGalaxyFv;
kmCall(&exeOpen__15MiniatureGalaxyFv + 0x1B0, getKoopaFortressBgmId);
kmWrite32(&exeOpen__15MiniatureGalaxyFv + 0x1B4, PPC_LI(4, 0));
kmCall(&exeOpen__15MiniatureGalaxyFv + 0x1B8, AudWrap::startStageBgm);
