#pragma once

#include <JSystem/JKernel/JKRHeap.hpp>
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>
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

        template < typename T >
        T* as(u32 offset = 0) {
            return reinterpret_cast< T* >(mBuffer + offset);
        }

        JSUMemoryInputStream createInputStream() const {
            return JSUMemoryInputStream(mBuffer, mSize);
        }

        JSUMemoryOutputStream createOutputStream() const {
            return JSUMemoryOutputStream(mBuffer, mSize);
        }

        /* 0x00 */ u8* mBuffer;
        /* 0x04 */ u32 mSize;
    };
}  // namespace MR
