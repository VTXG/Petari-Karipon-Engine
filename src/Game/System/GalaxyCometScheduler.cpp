#include "Game/System/GalaxyCometScheduler.hpp"
#include "Game/System/GalaxyCometState.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JKernel/JKRDvdRipper.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <cstdio>

#define COMET_CATEGORY_NORMAL 0
#define COMET_CATEGORY_COIN_100 1

namespace {
    static MR::AssignableArray< GalaxyCometTimePaper* > sGalaxyCometTimePaperTable;

    static const char* const cCometCategoryTable[] = {
        /* 0x0 */ "Normal",
        /* 0x1 */ "Coin100",
    };

    static void initGalaxyCometTimePaper(GalaxyCometTimePaper& rPaper, JMapInfo& rInfo, s32 idx) {
        const char* pCategory = nullptr;
        rInfo.getValue(idx, "Name", &rPaper.mGalaxyName);
        rInfo.getValue(idx, "Category", &pCategory);

        rPaper.mCategory = COMET_CATEGORY_NORMAL;

        if (MR::isNullOrEmptyString(pCategory)) {
            return;
        }

        for (s32 category = COMET_CATEGORY_NORMAL; category <= COMET_CATEGORY_COIN_100; category++) {
            if (MR::isEqualString(pCategory, cCometCategoryTable[category])) {
                rPaper.mCategory = category;
                break;
            }
        }
    }

    static void initGalaxyCometTimePaperTable() {
        if (sGalaxyCometTimePaperTable.size() > 0) {
            return;
        }

        GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor("AstroDome");
        s32 tableNum = accessor.getScenarioNum();

        sGalaxyCometTimePaperTable.init(tableNum);

        for (s32 tableIdx = 0; tableIdx < tableNum; tableIdx++) {
            char filePath[0x200];
            snprintf(filePath, sizeof(filePath), "/SystemData/CometCycleTable%d.bcsv", tableIdx + 1);
            void* pFileData = MR::loadToMainRAM(filePath, nullptr, MR::getStationedHeapGDDR3(), JKRDvdRipper::ALLOC_DIRECTION_FORWARD);

            JMapInfo info;
            info.attach(pFileData);

            s32 numEntries = info.getNumEntries();
            GalaxyCometTimePaper* pTable = new GalaxyCometTimePaper[numEntries + 1];

            for (s32 entryIdx = 0; entryIdx < numEntries; entryIdx++) {
                initGalaxyCometTimePaper(pTable[entryIdx], info, entryIdx);
            }

            pTable[numEntries].mGalaxyName = nullptr;
            pTable[numEntries].mCategory = 0;

            sGalaxyCometTimePaperTable[tableIdx] = pTable;
        }
    }

    static const GalaxyCometSerializeInfo cSerializeInfoPurpleCometAppear = {0x0007, 0};

    static const GalaxyCometSerializeInfo cSerializeInfoFirstContact[] = {
        {0x0001, 0}, {0x0000, 0}, {0x0000, 0}, {0x0000, 0}, {0x0000, 0}, {0x0000, 0},
    };
}  // namespace

GalaxyCometTimeTable::GalaxyCometTimeTable(const GalaxyCometTimePaper* pTimePaper)
    : mState(nullptr), mTimePaper(nullptr), mTimePaperPos(0), mIsReady(false), mIsHide(false) {
    mState = new GalaxyCometState();
    mTimePaper = pTimePaper;
    mTimePaperPos = 0;
}

void GalaxyCometTimeTable::update() {
    if (mIsReady) {
        mState->update();
        updateTimePaperPos();
    }
}

void GalaxyCometTimeTable::checkIsReady() {
    s32 pos;
    bool foundSuitableTimePaperPos = findSuitableTimePaperPos(&pos, -1);

    if (mIsReady != foundSuitableTimePaperPos) {
        mIsReady = foundSuitableTimePaperPos;
        if (foundSuitableTimePaperPos) {
            mTimePaperPos = pos;
        }
    }
}

void GalaxyCometTimeTable::serializeStateToGameData(GalaxyCometSerializeInfo* pInfo) {
    s32 stateIndex = mState->getStateIndex();
    pInfo->mStateIndexPacked = (mTimePaperPos << 1) + stateIndex;

    s32 pastSecond = mState->getPastSecond();
    pInfo->mPastSecond = pastSecond;
}

void GalaxyCometTimeTable::deserializeStateFromGameData(const GalaxyCometSerializeInfo* pInfo) {
    u32 stateIndex = pInfo->mStateIndexPacked;
    u32 stateMask = stateIndex >> 31;
    u32 timePaperPos = stateIndex >> 1;

    stateIndex &= 1;
    stateIndex ^= stateMask;
    mTimePaperPos = timePaperPos;

    s32 state = stateIndex - stateMask;
    mState->setStateAndPastSecond(state, pInfo->mPastSecond);
}

bool GalaxyCometTimeTable::isLand() const {
    if (mIsReady == false) {
        return false;
    }
    return mState->isLand();
}

bool GalaxyCometTimeTable::isIncluded(const char* galaxyName) const {
    for (s32 i = 0; mTimePaper[i].mGalaxyName != nullptr; i++) {
        if (MR::isEqualString(galaxyName, mTimePaper[i].mGalaxyName)) {
            return true;
        }
    }
    return false;
}

s32 GalaxyCometTimeTable::getCometCategory() const {
    return mTimePaper[mTimePaperPos].mCategory;
}

void GalaxyCometTimeTable::updateTimePaperPos() {
    if (!mIsHide) {
        if (mState->isHide()) {
            mIsReady = findSuitableTimePaperPos(&mTimePaperPos, mTimePaperPos);
        }
    }
    mIsHide = mState->isHide();
}

bool GalaxyCometTimeTable::findSuitableTimePaperPos(s32* pResultPos, s32 startPos) const {
    s32 timePaperPos = startPos;
    s32 length = calcTimePaperLength();
    for (s32 i = 0; i < length; i++) {
        timePaperPos++;
        if (mTimePaper[timePaperPos].mGalaxyName == nullptr) {
            timePaperPos = 0;
        }

        s32 category = mTimePaper[timePaperPos].mCategory;
        const char* galaxyName = mTimePaper[timePaperPos].mGalaxyName;
        bool canAppear;
        switch (category) {
        case COMET_CATEGORY_NORMAL:
            canAppear = MR::canAppearNormalComet(galaxyName);
            break;
        case COMET_CATEGORY_COIN_100:
            canAppear = MR::canAppearCoin100Comet(galaxyName);
            break;
        default:
            canAppear = false;
        }

        if (canAppear) {
            *pResultPos = timePaperPos;
            return true;
        }
    }

    *pResultPos = timePaperPos;
    return false;
}

s32 GalaxyCometTimeTable::calcTimePaperLength() const {
    s32 length = 0;
    for (; mTimePaper[length].mGalaxyName != nullptr; length++)
        ;
    return length;
}

void GalaxyCometTimeTable::advance() {
    mState->forceToNext();
    updateTimePaperPos();
}

GalaxyCometScheduler::GalaxyCometScheduler() : mTimeTables(), mEnabled(true) {
    initGalaxyCometTimePaperTable();

    mTimeTables.init(sGalaxyCometTimePaperTable.size());

    for (s32 i = 0; i < mTimeTables.size(); i++) {
        mTimeTables[i] = new GalaxyCometTimeTable(sGalaxyCometTimePaperTable[i]);
    }
}

void GalaxyCometScheduler::update() {
    if (mEnabled) {
        for (s32 i = 0; i < mTimeTables.size(); i++) {
            mTimeTables[i]->update();
        }
    }
}

void GalaxyCometScheduler::activate() {
    mEnabled = true;
}

void GalaxyCometScheduler::deactivate() {
    mEnabled = false;
}

void GalaxyCometScheduler::forceToNextState() {
    for (s32 i = 0; i < mTimeTables.size(); i++) {
        mTimeTables[i]->advance();
    }

    if (GameDataFunction::isPowerStarLeftInCometOnly()) {
        setScheduleMostForwardCometAppear();
    }
}

void GalaxyCometScheduler::syncWithFlags() {
    for (s32 i = 0; i < mTimeTables.size(); i++) {
        mTimeTables[i]->checkIsReady();
    }

    if (GameDataFunction::isOnJustGameEventFlag("EventCometStarter")) {
        setScheduleFirstContact();
    }
}

void GalaxyCometScheduler::updateStateToGameData() {
    GalaxyCometSerializeInfo info;
    for (s32 i = 0; i < mTimeTables.size(); i++) {
        mTimeTables[i]->serializeStateToGameData(&info);
        GameDataFunction::updateGalaxyCometStatus(i + 1, info.mStateIndexPacked, info.mPastSecond);
    }
}

void GalaxyCometScheduler::restoreStateFromGameData() {
    GalaxyCometSerializeInfo info;
    for (s32 i = 0; i < mTimeTables.size(); i++) {
        GameDataFunction::restoreGalaxyCometStatus(i + 1, &info.mStateIndexPacked, &info.mPastSecond);
        mTimeTables[i]->deserializeStateFromGameData(&info);
    }

    if (GameDataFunction::isOnGameEventFlag("Coin100CometEggStarGalaxy")) {
        if (!GameDataFunction::isOnGameEventFlag("SpecialStarCoin100CometRelease")) {
            mTimeTables[0]->deserializeStateFromGameData(&cSerializeInfoPurpleCometAppear);
        }
    }
}

void GalaxyCometScheduler::setScheduleFirstContact() {
    for (s32 i = 0; i < mTimeTables.size(); i++) {
        mTimeTables[i]->deserializeStateFromGameData(&cSerializeInfoFirstContact[i]);
    }
}

void GalaxyCometScheduler::setScheduleMostForwardCometAppear() {
    if (isHideAll()) {
        for (s32 i = 0; i < mTimeTables.size(); i++) {
            GalaxyCometTimeTable* timeTable = mTimeTables[i];
            if (timeTable->mIsReady) {
                if (timeTable->isLand()) {
                    break;
                }
                timeTable->mState->forceToNext();
                timeTable->updateTimePaperPos();
                break;
            }
        }
    }
}

bool GalaxyCometScheduler::isCometLand(const char* pGalaxyName) const {
    for (s32 i = 0; i < mTimeTables.size(); i++) {
        GalaxyCometTimeTable* timeTable = mTimeTables[i];
        if (timeTable->isLand()) {
            const char* name = timeTable->mTimePaper[timeTable->mTimePaperPos].mGalaxyName;
            if (MR::isEqualString(name, pGalaxyName)) {
                return true;
            }
        }
    }
    return false;
}

s32 GalaxyCometScheduler::getEncounterCometPowerStarId(const char* pGalaxyName) const {
    GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor(pGalaxyName);
    GalaxyCometTimeTable* timeTable = findFromGalaxy(pGalaxyName);

    switch (timeTable->getCometCategory()) {
    case COMET_CATEGORY_NORMAL:
        return accessor.getNormalCometScenarioNo();
    case COMET_CATEGORY_COIN_100:
        return accessor.getCoin100CometScenarioNo();
    default:
        return 0;
    }
}

const char* GalaxyCometScheduler::getEncounterCometName(const char* pGalaxyName) const {
    GalaxyStatusAccessor statusAccessor = MR::makeGalaxyStatusAccessor(pGalaxyName);
    s32 id = getEncounterCometPowerStarId(pGalaxyName);
    return statusAccessor.getCometName(id);
}

GalaxyCometTimeTable* GalaxyCometScheduler::findFromGalaxy(const char* pGalaxyName) const {
    for (s32 i = 0; i < mTimeTables.size(); i++) {
        GalaxyCometTimeTable* timeTable = mTimeTables[i];
        s32 pos = timeTable->mTimePaperPos;
        const char* name = timeTable->mTimePaper[pos].mGalaxyName;

        if (MR::isEqualString(name, pGalaxyName)) {
            return timeTable;
        }
    }
    return nullptr;
}

bool GalaxyCometScheduler::isHideAll() const {
    for (s32 i = 0; i < mTimeTables.size(); i++) {
        GalaxyCometTimeTable* timeTable = mTimeTables[i];
        if (timeTable->isLand()) {
            return false;
        }
    }
    return true;
}

s32 GalaxyCometScheduler::getStateValueIncluded(const char* pGalaxyName) {
    for (s32 i = 0; i < mTimeTables.size(); i++) {
        GalaxyCometTimeTable* timeTable = mTimeTables[i];
        if (timeTable->isIncluded(pGalaxyName)) {
            s32 state = timeTable->mState->getStateIndex();
            s32 pos = timeTable->mTimePaperPos;
            return (pos << 1) + state;
        }
    }
    return -1;
}
