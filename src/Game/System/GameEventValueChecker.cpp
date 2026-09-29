#include "Game/System/GameEventValueChecker.hpp"
#include "Game/Util/Array.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

namespace {
    static MR::AssignableArray< GameEventValue > sGameEventValueTable;

    static void initGameEventValue(GameEventValue& rValue, JMapInfo& rInfo, s32 idx) {
        u32 defaultValue;
        rInfo.getValue(idx, "Name", &rValue.mName);
        rInfo.getValue(idx, "DefaultValue", &defaultValue);
        rValue.mDefaultValue = defaultValue;
    }

    static void initGameEventValueTable() {
        if (sGameEventValueTable.size() > 0) {
            return;
        }

        void* pFileData = MR::receiveFile("/SystemData/GameEventValueTable.bcsv");

        JMapInfo info;
        info.attach(pFileData);

        s32 numEntries = info.getNumEntries();
        sGameEventValueTable.init(numEntries);

        for (s32 i = 0; i < numEntries; i++) {
            initGameEventValue(sGameEventValueTable[i], info, i);
        }
    }

    static s32 findIndex(const char* pName) {
        for (s32 idx = 0; idx < sGameEventValueTable.mMaxSize; idx++) {
            if (MR::isEqualString(pName, ::sGameEventValueTable[idx].mName)) {
                return idx;
            }
        }

        return -1;
    }

    static s32 findIndexFromHashCode(u16 hash) {
        for (s32 idx = 0; idx < sGameEventValueTable.mMaxSize; idx++) {
            if (hash == static_cast< u16 >(MR::getHashCode(::sGameEventValueTable[idx].mName))) {
                return idx;
            }
        }

        return -1;
    }
}  // namespace

GameEventValueChecker::GameEventValueChecker() : mValues(), mNumValues() {
    initGameEventValueTable();
    mNumValues = sGameEventValueTable.mMaxSize;
    mValues = new u16[mNumValues];
    initializeData();
}

u32 GameEventValueChecker::getValue(const char* pName) const {
    return mValues[findIndex(pName)];
}

void GameEventValueChecker::setValue(const char* pName, u16 value) {
    mValues[findIndex(pName)] = value;
}

u32 GameEventValueChecker::makeHeaderHashCode() const {
    return getSignature();
}

u32 GameEventValueChecker::getSignature() const {
    return 'VLE1';
}

s32 GameEventValueChecker::serialize(u8* pData, u32 maxBufferSize) const {
    JSUMemoryOutputStream stream(pData, maxBufferSize);

    s32 hash;

    for (s32 idx = 0; idx < mNumValues; idx++) {
        hash = MR::getHashCode(::sGameEventValueTable[idx].mName);
        const u16 value = mValues[idx];

        stream.writeU16(hash);
        stream.writeU16(value);
    }

    return stream.mPosition;
}

s32 GameEventValueChecker::deserialize(const u8* pData, u32 maxBufferSize) {
    s32 readError = 0;

    JSUMemoryInputStream stream(pData, maxBufferSize);

    s32 numEntries = static_cast< s32 >(maxBufferSize) / 2;

    for (s32 idx = 0; idx < numEntries; idx++) {
        u16 hash = stream.readU16();
        u16 value = stream.readU16();
        s32 valueIndex = findIndexFromHashCode(hash);

        if (valueIndex >= 0) {
            mValues[valueIndex] = value;
        } else {
            readError = 1;
        }
    }

    if (stream.mState != JSUIosBase::IO_OK && stream.getState(JSUIosBase::IO_MEMORY_ERROR) == 0) {
        stream.mState = JSUIosBase::IO_OK;
    }

    return readError ? 1 : 0;
}

void GameEventValueChecker::initializeData() {
    for (s32 idx = 0; idx < mNumValues; idx++) {
        mValues[idx] = ::sGameEventValueTable[idx].mDefaultValue;
    }
}
