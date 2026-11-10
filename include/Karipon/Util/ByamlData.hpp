#pragma once

#include <revolution/types.h>

namespace krp {
    class ByamlHashPair;

    enum ByamlDataType {
        ByamlDataType_Invalid = 0,
        ByamlDataType_String = 0xA0,
        ByamlDataType_Binary = 0xA1,
        ByamlDataType_Array = 0xC0,
        ByamlDataType_Hash = 0xC1,
        ByamlDataType_StringTable = 0xC2,
        ByamlDataType_Bool = 0xD0,
        ByamlDataType_Int = 0xD1,
        ByamlDataType_Float = 0xD2,
        ByamlDataType_UInt = 0xD3,
        ByamlDataType_Long = 0xD4,
        ByamlDataType_ULong = 0xD5,
        ByamlDataType_Double = 0xD6,
        ByamlDataType_Null = 0xFF
    };

    class ByamlHashPair {
    public:
        s32 getKey() const {
            return mData >> 8;
        }
        ByamlDataType getType() const {
            return static_cast< ByamlDataType >(mData & 0xFF);
        }
        s32 getValue() const {
            return mValue;
        }

        const s32 mData;
        const s32 mValue;
    };

    class ByamlData {
    public:
        ByamlData() : mValue(0), mType(ByamlDataType_Invalid) {}

        void set(const ByamlHashPair* pPair) {
            mType = pPair->getType();
            mValue = pPair->getValue();
        }
        void set(ByamlDataType type, u32 val) {
            mType = type;
            mValue = val;
        }
        ByamlDataType getType() const {
            return mType;
        }
        u32 getValue() const {
            return mValue;
        }

        template < typename T >
        T getValue() const {
            u32 val = getValue();
            return *reinterpret_cast< const T* >(&val);
        }

        u32 mValue;
        ByamlDataType mType;
    };

    class ByamlHashIter {
    public:
        ByamlHashIter() : mData(nullptr) {}
        ByamlHashIter(const u8* pData) : mData(pData) {}

        const ByamlHashPair* findPair(s32 key) const;
        bool getDataByIndex(ByamlData* pData, s32 index) const;
        bool getDataByKey(ByamlData* pData, s32 key) const;
        const u8* getOffsetData(u32 off) const {
            return &mData[off];
        }
        const ByamlHashPair* getPairByIndex(s32 index) const;
        const ByamlHashPair* getPairTable() const;
        u32 getSize() const;

        const u8* mData;
    };

    class ByamlArrayIter {
    public:
        ByamlArrayIter() : mData(nullptr) {}
        ByamlArrayIter(const u8* pData) : mData(pData) {}

        bool getDataByIndex(ByamlData* pData, s32 index) const;
        const u32* getDataTable() const;
        const u8* getOffsetData(u32 off) const {
            return &mData[off];
        }
        u32 getSize() const {
            return *reinterpret_cast< const u32* >(mData) & 0xFFFFFF;
        }
        const u8* getTypeTable() const {
            return mData + 4;
        }

        const u8* mData;
    };
}; // namespace krp
