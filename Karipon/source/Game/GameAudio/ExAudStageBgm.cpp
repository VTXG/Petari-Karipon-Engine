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

        if (pEntry != nullptr && (cometBgm == -1 || pEntry->mScenarioNo != 0)) {
            return pEntry->mBgmId;
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

        if (pEntry != nullptr && bgmNo < pEntry->mChangeBgmId.size()) {
            return pEntry->mChangeBgmId[bgmNo];
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

    static u32 getSeqIdForMultiBgm(u32 id) {
        return ExSingletonHolder< KariponResourceHolder >::get()->findMultiBgmSetEntry(id)->mSettings.mSeqId;
    }

    static u32 getStreamIdForMultiBgm(u32 id) {
        return ExSingletonHolder< KariponResourceHolder >::get()->findMultiBgmSetEntry(id)->mSettings.mStreamId;
    }

    static f32 getBeatMulForMultiBgm(u32 id) {
        return ExSingletonHolder< KariponResourceHolder >::get()->findMultiBgmSetEntry(id)->mSettings.mBeatMul;
    }

    static u32 getIntroBeatsForMultiBgm(u32 id) {
        return ExSingletonHolder< KariponResourceHolder >::get()->findMultiBgmSetEntry(id)->mSettings.mIntroBeats;
    }

    static u32 getLoopBeatsForMultiBgm(u32 id) {
        return ExSingletonHolder< KariponResourceHolder >::get()->findMultiBgmSetEntry(id)->mSettings.mLoopBeats;
    }

    static u32 getLoopStartSamplesForMultiBgm(u32 id) {
        return ExSingletonHolder< KariponResourceHolder >::get()->findMultiBgmSetEntry(id)->mSettings.mLoopStartSamples;
    }

    static u32 getLoopEndSamplesForMultiBgm(u32 id) {
        return ExSingletonHolder< KariponResourceHolder >::get()->findMultiBgmSetEntry(id)->mSettings.mLoopEndSamples;
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

extern kmSymbol getSeqIdForMultiBgm__13AudBgmSettingFUl;
kmBranch(&getSeqIdForMultiBgm__13AudBgmSettingFUl, getSeqIdForMultiBgm);

extern kmSymbol getStreamIdForMultiBgm__13AudBgmSettingFUl;
kmBranch(&getStreamIdForMultiBgm__13AudBgmSettingFUl, getStreamIdForMultiBgm);

extern kmSymbol getBeatMulForMultiBgm__13AudBgmSettingFUl;
kmBranch(&getBeatMulForMultiBgm__13AudBgmSettingFUl, getBeatMulForMultiBgm);

extern kmSymbol getIntroBeatsForMultiBgm__13AudBgmSettingFUl;
kmBranch(&getIntroBeatsForMultiBgm__13AudBgmSettingFUl, getIntroBeatsForMultiBgm);

extern kmSymbol getLoopBeatsForMultiBgm__13AudBgmSettingFUl;
kmBranch(&getLoopBeatsForMultiBgm__13AudBgmSettingFUl, getLoopBeatsForMultiBgm);

extern kmSymbol getLoopStartSamplesForMultiBgm__13AudBgmSettingFUl;
kmBranch(&getLoopStartSamplesForMultiBgm__13AudBgmSettingFUl, getLoopStartSamplesForMultiBgm);

extern kmSymbol getLoopEndSamplesForMultiBgm__13AudBgmSettingFUl;
kmBranch(&getLoopEndSamplesForMultiBgm__13AudBgmSettingFUl, getLoopEndSamplesForMultiBgm);
