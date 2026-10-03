#pragma once

#include <JSystem/JKernel/JKRHeap.hpp>
#include <revolution/types.h>

namespace MR {
    void fastCopy(void* pFrom, void* pTo, u32 size);

    class DataBuffer {
    public:
        DataBuffer() : mBuffer(), mSize() {}

        DataBuffer(u32 size, JKRHeap* pHeap = nullptr) {
            init(size, pHeap);
        }

        ~DataBuffer() {
            if (mBuffer != nullptr) {
                delete[] mBuffer;
            }
        }

        void init(u32 size, JKRHeap* pHeap = nullptr) {
            mSize = size;
            mBuffer = new (pHeap, 0x20) u8[size];
        }

        void copy(u8* pTo) {
            MR::fastCopy(mBuffer, pTo, mSize);
        }

        /* 0x00 */ u8* mBuffer;
        /* 0x04 */ u32 mSize;
    };

    class DataStream {
    public:
        DataStream(u8* pBuffer, u32 size) : mBuffer(pBuffer), mSize(size), mPosition() {}
        DataStream(DataBuffer& rDataBuffer) : mBuffer(rDataBuffer.mBuffer), mSize(rDataBuffer.mSize), mPosition() {}

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

        /* 0x00 */ u8* mBuffer;
        /* 0x04 */ u32 mSize;
        /* 0x08 */ u32 mPosition;
    };
} // namespace MR
