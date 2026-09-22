#include "Game/MapObj/ExAstroDomeOrbit.hpp"
#include "Kamek.hpp"
#include "Karipon/System/ByamlIter.hpp"
#include "Karipon/System/ByamlUtil.hpp"
#include "Macros.hpp"
#include <Game/AudioLib/AudSoundNameConverter.hpp>
#include <Game/AudioLib/AudWrap.hpp>
#include <Game/Util/SceneUtil.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <cstddef>
#include <revolution/gx/GXStruct.h>
#include <revolution/types.h>

namespace {
    static void getColorValueByKey(const ByamlIter& rIter, GXColor& rColor, const char* pKey) {
        ByamlIter colorIt = rIter.getIterByKey(pKey);
        u32 r = 0;
        u32 g = 0;
        u32 b = 0;
        colorIt.tryGetValueByKey(&r, "R");
        colorIt.tryGetValueByKey(&g, "G");
        colorIt.tryGetValueByKey(&b, "B");
        rColor.r = r;
        rColor.g = g;
        rColor.b = b;
        rColor.a = 0xFF;
    }

    static void getVectorValueByKey(const ByamlIter& rIter, TVec3f& rVec, const char* pKey) {
        ByamlIter vectorIt = rIter.getIterByKey(pKey);
        f32 x = 0.0f;
        f32 y = 0.0f;
        f32 z = 0.0f;
        vectorIt.tryGetValueByKey(&x, "X");
        vectorIt.tryGetValueByKey(&y, "Y");
        vectorIt.tryGetValueByKey(&z, "Z");
        rVec.x = x;
        rVec.y = y;
        rVec.z = z;
    }
} // namespace

namespace {
    static ByamlIter createDomeParamIter(s32 scenarioNo) {
        ByamlIter rootIt = ByamlUtil::createByamlRootFromFile("/SystemData/DomeParamTable.byaml");
        return rootIt.getIterByIndex(scenarioNo - 1);
    }

    static void setupAstroDomeOrbit(ExAstroDomeOrbit* pOrbit, s32 id) {
        ByamlIter orbitIt = createDomeParamIter(MR::getCurrentScenarioNo()).getIterByKey("Orbits").getIterByIndex(id);

        getColorValueByKey(orbitIt, pOrbit->mColor, "Color");
        getColorValueByKey(orbitIt, pOrbit->mBloom, "Bloom");

        f32 radius = 0.0f;
        orbitIt.tryGetValueByKey(&radius, "Radius");
        pOrbit->mOrbitRadius = radius;

        getVectorValueByKey(orbitIt, pOrbit->mRotation, "Rotate");

        pOrbit->mAngle = 230.0f * id; // ::cGalaxyRotateCoordOffset
    }

    static const char* getDomeSkyName(s32 scenarioNo) {
        const char* pSkyName = nullptr;
        createDomeParamIter(scenarioNo).tryGetValueByKey(&pSkyName, "Sky");
        return pSkyName;
    }

    static const char* getSkyNameForAstroList() {
        register s32 scenarioNo;
        GET_REGISTER(scenarioNo, r29);
        return getDomeSkyName(scenarioNo);
    }

    static u32 getKoopaFortressBgmId() {
        const char* pBgmName = nullptr;
        createDomeParamIter(MR::getCurrentScenarioNo()).tryGetValueByKey(&pBgmName, "KoopaFortressBgm");
        return static_cast<u32>(AudSingletonHolder<AudSoundNameConverter>::get()->getSoundID(pBgmName));
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
kmWrite32(&getKoopaFortressAppearBgm__15AudStageBgmWrapFPCc + 0x3C, PPC_MR(0, 3)); // mr r0, r3

extern kmSymbol exeOpen__15MiniatureGalaxyFv;
kmCall(&exeOpen__15MiniatureGalaxyFv + 0x1B0, getKoopaFortressBgmId);
kmWrite32(&exeOpen__15MiniatureGalaxyFv + 0x1B4, PPC_LI(4, 0));
kmCall(&exeOpen__15MiniatureGalaxyFv + 0x1B8, AudWrap::startStageBgm);
