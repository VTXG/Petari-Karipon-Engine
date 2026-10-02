#pragma once

#include "Game/Util/ByamlData.hpp"
#include <revolution/types.h>

class ByamlContainerHeader;
class ByamlData;
class ByamlHeader;
class ByamlFile;

class ByamlIter {
public:
    ByamlIter() : mData(nullptr), mRootNode(nullptr) {}
    ByamlIter(const u8* pData);
    ByamlIter(const u8* pData, const u8* pRoot) : mData(pData), mRootNode(pRoot) {}

    bool isValid() const { return mData != nullptr; }
    bool isTypeHash() const;
    bool isTypeArray() const;
    bool isTypeContainer() const;
    bool isExistKey(const char* pKey) const;
    s32 getKeyIndex(const char* pKey) const;
    s32 getSize() const;

    ByamlIter getIterByIndex(s32 index) const;
    bool tryGetIterByIndex(s32 index, ByamlIter* pIter) const {
        *pIter = getIterByIndex(index);
        return pIter->isValid();
    }
    bool tryGetByamlDataByIndex(ByamlData* pData, s32 index) const;

    ByamlIter getIterByKey(const char* pKey) const;
    bool tryGetIterByKey(const char* pKey, ByamlIter* pIter) const {
        *pIter = getIterByKey(pKey);
        return pIter->isValid();
    }
    bool tryGetByamlDataByKey(ByamlData* pData, const char* pKey) const;

    template < typename T >
    bool tryGetValueByIndex(s32 index, T* pValue) const {
        ByamlData data;

        if (!tryGetByamlDataByIndex(&data, index)) {
            return false;
        }

        return tryConvertValue< T >(&data, pValue);
    }

    template < typename T >
    bool tryGetValueByKey(const char* pKey, T* pValue) const {
        ByamlData data;

        if (!tryGetByamlDataByKey(&data, pKey)) {
            return false;
        }

        return tryConvertValue< T >(&data, pValue);
    }

    template < typename T >
    bool tryConvertValue(const ByamlData* pData, T* pValue) const;

    ByamlIter operator[](s32 index) const { return getIterByIndex(index); }
    ByamlIter operator[](const char* pKey) const { return getIterByKey(pKey); }
    bool operator==(const ByamlIter& rOther) const { return mData == rOther.mData && mRootNode == rOther.mRootNode; }
    bool operator!=(const ByamlIter& rOther) const { return !(*this == rOther); }

    union {
        /* 0x00 */ const u8* mData;
        /* 0x00 */ const ByamlHeader* mHeader;
    };

    union {
        /* 0x04 */ const u8* mRootNode;
        /* 0x04 */ const ByamlContainerHeader* mContainingHeader;
    };
};
