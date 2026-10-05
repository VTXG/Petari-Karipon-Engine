#pragma once

#include <JSystem/JKernel/JKRHeap.hpp>
#include <revolution/types.h>

namespace MR {
    void fastCopy(void* pFrom, void* pTo, u32 size);

    class DataBuffer {
    public:
        DataBuffer() : mData(), mSize() {}

        DataBuffer(u32 capacity, JKRHeap* pHeap = nullptr) {
            init(capacity, pHeap);
        }

        ~DataBuffer() {
            if (mData != nullptr) {
                delete[] mData;
            }
        }

        void init(u32 capacity, JKRHeap* pHeap = nullptr) {
            mSize = capacity;
            mData = new (pHeap, 0x20) u8[capacity];
        }

        void copy(u8* pTo) {
            MR::fastCopy(mData, pTo, mSize);
        }

        /* 0x00 */ u8* mData;
        /* 0x04 */ u32 mSize;
    };

    class DataStream {
    public:
        DataStream(u8* pBuffer, u32 size) : mData(pBuffer), mSize(size), mPosition() {}
        DataStream(DataBuffer& rDataBuffer) : mData(rDataBuffer.mData), mSize(rDataBuffer.mSize), mPosition() {}

        bool readData(void* pData, s32 size);
        bool writeData(const void* pData, s32 size);

        template < typename T >
        bool read(T* pData) {
            return readData(pData, sizeof(*pData));
        }

        template < typename T >
        bool write(const T* pData) {
            return writeData(pData, sizeof(*pData));
        }

        template < typename T >
        T as() {
            return reinterpret_cast< T >(mData + mPosition);
        }

        u32 adjust() {
            return mSize = mPosition;
        }

        /* 0x00 */ u8* mData;
        /* 0x04 */ u32 mSize;
        /* 0x08 */ u32 mPosition;
    };
} // namespace MR
