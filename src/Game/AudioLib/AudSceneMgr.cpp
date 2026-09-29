#include "Game/AudioLib/AudSceneMgr.hpp"
#include "Game/AudioLib/AudEffector.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Speaker/SpkSystem.hpp"
#include "Game/Util/ByamlIter.hpp"
#include "Game/Util/ByamlUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "JSystem/JAudio2/JASWaveArcLoader.hpp"
#include <JSystem/JAudio2/JAUSectionHeap.hpp>

AudSceneMgr::AudSceneMgr(JAUSectionHeap* pSectionHeap)
    : mSectionHeap(pSectionHeap), _4(), mStaticResource(), mStageResource(), mWaveSetStage(), mWaveSetScenario(), mPlayerMode(), mPrevPlayerMode(),
      mIsNewPlayerMode(), _1D() {
    void* pFileData = MR::receiveFile("/SystemData/StageWaveTable.byaml");
    ByamlIter root = ByamlUtil::createByamlRoot(static_cast< u8* >(pFileData));

    mStaticResource = root["StaticResource"];
    mStageResource = root["StageResource"];
}

bool AudSceneMgr::isLoadDoneSystemInit() {
    return mSectionHeap->isWaveLoaded(7, 0);
}

void AudSceneMgr::loadStaticResource() {
    loadWaveSet(mStaticResource);
}

bool AudSceneMgr::isLoadDoneStaticResource() {
    return isLoadDoneWaveSet(mStaticResource);
}

void AudSceneMgr::loadStageResource(const char* pSceneName, const char* pStageName) {
    ByamlIter waveSetStage = mStageResource[pStageName]["Common"];

    mIsNewPlayerMode = mPlayerMode != mPrevPlayerMode;

    eraseWaveSet(mWaveSetScenario);

    if (waveSetStage != mWaveSetStage || mIsNewPlayerMode) {
        eraseWaveSet(mWaveSetStage);

        if (mIsNewPlayerMode) {
            loadPlayerResource();
            mPrevPlayerMode = mPlayerMode;
        }

        mWaveSetStage = waveSetStage;
        loadWaveSet(mWaveSetStage);
    }
}

bool AudSceneMgr::isLoadDoneStageResource() {
    if (mIsNewPlayerMode && !isPlayerResourceLoaded()) {
        return false;
    }

    return isLoadDoneWaveSet(mWaveSetStage);
}

void AudSceneMgr::loadScenarioResource(const char* pSceneName, const char* pStageName, s32 scenarioNo) {
    char key[16];
    snprintf(key, sizeof(key), "Scenario%d", scenarioNo);

    mWaveSetScenario = mStageResource[pStageName][key];
    loadWaveSet(mWaveSetScenario);
}

bool AudSceneMgr::isLoadDoneScenarioResource() {
    return isLoadDoneWaveSet(mWaveSetScenario);
}

void AudSceneMgr::loadWaveSet(const ByamlIter& rWaveSet) {
    if (!rWaveSet.isValid()) {
        return;
    }

    s32 size = rWaveSet.getSize();

    for (s32 i = 0; i < size; i++) {
        const char* pName = nullptr;
        rWaveSet.tryGetValueByIndex(&pName, i);

        if (MR::isNullOrEmptyString(pName)) {
            continue;
        }

        s32 bankNo = findWaveBankNo(pName);
        if (bankNo != -1) {
            mSectionHeap->loadWaveArc(bankNo);
            OSReport("[%s] Loaded %s\n", __FILE__, pName);
        } else {
            OSReport("[%s] Failed loading %s\n", __FILE__, pName);
        }
    }
}

void AudSceneMgr::eraseWaveSet(const ByamlIter& rWaveSet) {
    if (!rWaveSet.isValid()) {
        return;
    }

    s32 size = rWaveSet.getSize();

    for (s32 i = 0; i < size; i++) {
        const char* pName = nullptr;
        rWaveSet.tryGetValueByIndex(&pName, i);

        if (MR::isNullOrEmptyString(pName)) {
            continue;
        }

        s32 bankNo = findWaveBankNo(pName);
        if (bankNo != -1) {
            mSectionHeap->eraseWaveArc(bankNo);
            OSReport("[%s] Erased %s\n", __FILE__, pName);
        } else {
            OSReport("[%s] Failed erasing %s\n", __FILE__, pName);
        }
    }
}

bool AudSceneMgr::isLoadDoneWaveSet(const ByamlIter& rWaveSet) const {
    if (!rWaveSet.isValid()) {
        return true;
    }

    s32 size = rWaveSet.getSize();

    for (s32 i = 0; i < size; i++) {
        const char* pName = nullptr;
        rWaveSet.tryGetValueByIndex(&pName, i);

        if (MR::isNullOrEmptyString(pName)) {
            continue;
        }

        s32 bankNo = findWaveBankNo(pName);
        if (bankNo != -1 && !mSectionHeap->isWaveLoaded(bankNo, 0)) {
            return false;
        }
    }

    return true;
}

void AudSceneMgr::startScene() {
    _4 = 0;
    AudWrap::getSystem()->_82A = false;
    AudWrap::getSystem()->_82B = false;
    AudWrap::getSystem()->_82C = false;
    AudWrap::getSystem()->initSceneVolume();
    AudEffector* effector = AudWrap::getSystem()->mAudEffector;
    if (effector != nullptr) {
        effector->initParams();
    }

    _1D = false;
    SpkSystem::reconnect(-1);
}

bool AudSceneMgr::loadPlayerResource() {
    mSectionHeap->eraseWaveArc(34, 2);
    mSectionHeap->eraseWaveArc(34, 4);

    switch (mPlayerMode) {
    case PlayerMode_Mario:
        return mSectionHeap->loadWaveArc(34, 2);
    case PlayerMode_Luigi:
        return mSectionHeap->loadWaveArc(34, 4);
    }

    return false;
}

bool AudSceneMgr::isPlayerResourceLoaded() {
    switch (mPlayerMode) {
    case PlayerMode_Mario:
        return mSectionHeap->isWaveLoaded(34, 1);
    case PlayerMode_Luigi:
        return mSectionHeap->isWaveLoaded(34, 2);
    }

    return true;
}

s32 AudSceneMgr::findWaveBankNo(const char* pWaveArcName) const {
    char filePath[0x200];
    snprintf(filePath, sizeof(filePath), "/AudioRes/Waves/%s", pWaveArcName);

    char filePathLang[0x200];
    MR::makeFileNameConsideringLanguage(filePathLang, sizeof(filePathLang), filePath);

    s32 entryNum = DVDConvertPathToEntrynum(filePathLang);
    if (entryNum == -1) {
        return -1;
    }

    for (u8 bankNo = 0; bankNo < 0xFF; bankNo++) {
        if (!mSectionHeap->getSectionData().registeredWaveBankTables.test(bankNo)) {
            continue;
        }

        JASWaveBank* pWaveBank = mSectionHeap->getWaveBankTable().getWaveBank(bankNo);
        if (pWaveBank == nullptr) {
            continue;
        }

        for (u32 i = 0; i < pWaveBank->getArcCount(); i++) {
            JASWaveArc* pWaveArc = pWaveBank->getWaveArc(i);

            if (pWaveArc->mEntryNum == entryNum) {
                return bankNo;
            }
        }
    }

    return -1;
}
