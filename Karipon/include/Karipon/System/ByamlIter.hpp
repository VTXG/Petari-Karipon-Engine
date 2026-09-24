#pragma once

#include "Karipon/System/ByamlData.hpp"
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
    bool getByamlDataByIndex(ByamlData* pData, s32 index) const;

    ByamlIter getIterByKey(const char* pKey) const;
    bool getByamlDataByKey(ByamlData* pData, const char* pKey) const;

    template < typename T >
    bool tryGetValueByIndex(T* pValue, s32 index) const {
        ByamlData data;

        if (!getByamlDataByIndex(&data, index)) {
            return false;
        }

        return tryConvertValue< T >(pValue, &data);
    }

    template < typename T >
    bool tryGetValueByKey(T* pValue, const char* pKey) const {
        ByamlData data;

        if (!getByamlDataByKey(&data, pKey)) {
            return false;
        }

        return tryConvertValue< T >(pValue, &data);
    }

    template < typename T >
    bool tryConvertValue(T* pValue, const ByamlData* pData) const;

    bool operator==(const ByamlIter& rOther) const { return mData == rOther.mData && mRootNode == rOther.mRootNode; }
    bool operator!=(const ByamlIter& rOther) const { return !(*this == rOther); }

    union {
        const u8* mData;
        const ByamlHeader* mHeader;
    };

    union {
        const u8* mRootNode;
        const ByamlContainerHeader* mContainingHeader;
    };
};
