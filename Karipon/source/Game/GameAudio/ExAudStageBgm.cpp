#include "Game/GameAudio/ExAudStageBgm.hpp"
#include "Game/Util/ExSingletonHolder.hpp"
#include "Kamek.hpp"
#include "Karipon/System/KariponResourceHolder.hpp"
#include <Game/AudioLib/AudSoundId.hpp>
#include <Game/AudioLib/AudSoundNameConverter.hpp>
#include <Game/AudioLib/AudWrap.hpp>
#include <Game/GameAudio/AudStageBgmWrap.hpp>
#include <Game/Util/EventUtil.hpp>
#include <Game/Util/SceneUtil.hpp>
#include <Game/Util/StringUtil.hpp>
#include <JSystem/JAudio2/JAISound.hpp>

namespace {
    static JAISoundID changeStageNameToSoundIDAudStageBgmWrap(const char* pSceneName, const char* pStageName, s32 scenarioNo) {
        JAISoundID koopaAppearBgm = AudStageBgmWrap::getKoopaFortressAppearBgm(pStageName);
        if (koopaAppearBgm != -1) {
            return koopaAppearBgm;
        }

        if (MR::isEqualString(pStageName, "AstroGalaxy") && scenarioNo == 1 && !MR::isOnGameEventFlagAstroGalaxyBgmBright()) {
            return MBGM_GALAXY_24;
        }

        JAISoundID cometBgm = AudStageBgmWrap::getCometEventBgm(pStageName);

        const StageBgmEntry* pEntry = ExSingletonHolder< KariponResourceHolder >::get()->findStageBgmEntry(pStageName, scenarioNo);

        if (pEntry != nullptr && pEntry->mBgmIDName != nullptr && (cometBgm == -1 || pEntry->mScenarioNo != 0)) {
            return static_cast< u32 >(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pEntry->mBgmIDName));
        }

        return cometBgm;
    }

    static u32 getKoopaFortressBgmId() {
        const char* pBgmName = nullptr;
        ExSingletonHolder< KariponResourceHolder >::get()
            ->getDomeParamIter(MR::getCurrentScenarioNo())
            .tryGetValueByKey(&pBgmName, "KoopaFortressBgm");
        return static_cast< u32 >(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pBgmName));
    }

    static u32 getBgmIdAudStageBgmTable(const char* pStageName, s32 bgmNo) {
        JAISoundID cometBgm = AudStageBgmWrap::getCometEventBgm(pStageName);
        if (cometBgm != -1) {
            return cometBgm;
        }

        const StageBgmSetEntry* pEntry = ExSingletonHolder< KariponResourceHolder >::get()->findStageBgmSetEntry(pStageName);

        if (pEntry != nullptr && bgmNo < pEntry->mChangeBgmIDName.size()) {
            return pEntry->mChangeBgmIDName[bgmNo] != nullptr ?
                       static_cast< u32 >(AudSingletonHolder< AudSoundNameConverter >::get()->getSoundID(pEntry->mChangeBgmIDName[bgmNo])) :
                       -1;
        }

        return 0;
    }

    static u32 getBgmStateAudStageBgmTable(const char* pStageName, s32 stateNo) {
        const StageBgmSetEntry* pEntry = ExSingletonHolder< KariponResourceHolder >::get()->findStageBgmSetEntry(pStageName);

        if (pEntry != nullptr && stateNo < pEntry->mChangeBgmState.size()) {
            return pEntry->mChangeBgmState[stateNo];
        }

        return 0;
    }

    static bool isStartBgmOnWelcome() {
        const StageBgmEntry* pEntry =
            ExSingletonHolder< KariponResourceHolder >::get()->findStageBgmEntry(MR::getCurrentStageName(), MR::getCurrentScenarioNo());
        return pEntry != nullptr && pEntry->mStartType == 1;
    }
} // namespace

extern kmSymbol changeStageNameToSoundID__15AudStageBgmWrapFPCcPCcl;
kmBranch(&changeStageNameToSoundID__15AudStageBgmWrapFPCcPCcl, changeStageNameToSoundIDAudStageBgmWrap);

extern kmSymbol getKoopaFortressAppearBgm__15AudStageBgmWrapFPCc;
kmCall(&getKoopaFortressAppearBgm__15AudStageBgmWrapFPCc + 0x38, getKoopaFortressBgmId);
kmWrite32(&getKoopaFortressAppearBgm__15AudStageBgmWrapFPCc + 0x3C, PPC_MR(0, 3));

extern kmSymbol exeOpen__15MiniatureGalaxyFv;
kmCall(&exeOpen__15MiniatureGalaxyFv + 0x1B0, getKoopaFortressBgmId);
kmWrite32(&exeOpen__15MiniatureGalaxyFv + 0x1B4, PPC_LI(4, 0));
kmCall(&exeOpen__15MiniatureGalaxyFv + 0x1B8, AudWrap::startStageBgm);

extern kmSymbol getBgmId__16AudStageBgmTableFPCcl;
kmBranch(&getBgmId__16AudStageBgmTableFPCcl, getBgmIdAudStageBgmTable);

extern kmSymbol getBgmState__16AudStageBgmTableFPCcl;
kmBranch(&getBgmState__16AudStageBgmTableFPCcl, getBgmStateAudStageBgmTable);

extern kmSymbol isStartBgmOnWelcome__15ScenarioStarterFv;
kmCall(&isStartBgmOnWelcome__15ScenarioStarterFv + 0x38, isStartBgmOnWelcome);
kmWrite32(&isStartBgmOnWelcome__15ScenarioStarterFv + 0x3C, PPC_B(0x6C));
