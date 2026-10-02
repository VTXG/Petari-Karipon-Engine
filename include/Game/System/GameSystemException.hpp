#pragma once

#include <revolution/os.h>

class GameSystemException {
public:
    static void init();
    static void handleException(OSError, OSContext*, u32, u32);
    static bool handleExceptionDump(OSContext*);
    static char* printContext(OSContext*, char*, u32);

    static void* sMapFileUsingBuffer;
};
