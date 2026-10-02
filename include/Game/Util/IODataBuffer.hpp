#pragma once

#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>
#include <revolution/types.h>

namespace MR {
    void fastCopy(void* pFrom, void* pTo, u32 size);
}

class IODataBuffer {
public:
    IODataBuffer() : mBuffer(), mSize() {
    }

    IODataBuffer(u32 size) {
        init(size);
    }

    ~IODataBuffer() {
        if (mBuffer != nullptr) {
            delete[] mBuffer;
        }
    }

    void init(u32 size) {
        mSize = size;
        mBuffer = new u8[size];
    }

    void resize(u32 size) {
        if (size <= mSize) {
            return;
        }

        u8* tmp = new u8[size];
        MR::fastCopy(tmp, mBuffer, mSize);
        mBuffer = tmp;
        mSize = size;
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
