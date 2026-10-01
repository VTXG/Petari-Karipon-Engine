#include "Game/System/GameSystemException.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <JSystem/JUtility/JUTAssert.hpp>
#include <JSystem/JUtility/JUTConsole.hpp>
#include <JSystem/JUtility/JUTDirectPrint.hpp>
#include <JSystem/JUtility/JUTException.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>
#include <cstdio>

extern "C" void __OSStopAudioSystem(void);

void* GameSystemException::sMapFileUsingBuffer;

namespace {
    static char sFileBuff[0x800] ATTRIBUTE_ALIGN(32);

    bool isBootWPAD() {
        GameSystemObjHolder* pObjHolder = SingletonHolder< GameSystem >::get()->mObjHolder;

        return pObjHolder != nullptr && pObjHolder->mWPadHolder != nullptr;
    }
};  // namespace

void GameSystemException::init() {
    JUTDirectPrint* pDirectPrint = JUTDirectPrint::start();

    JUTAssertion::create();
    JUTAssertion::changeDisplayTime(600);
    JUTAssertion::changeDevice(3);
    JUTConsoleManager::createManager(nullptr);
    JUTException::create(pDirectPrint);
    JUTException::createConsole(new u8[0x24FC], 0x24FC);
    JUTException::setPreUserCallback(GameSystemException::handleException);
    JUTException::sErrorManager->mPrintWaitTime1 = 0;
    JUTException::sErrorManager->mPrintWaitTime0 = 0;

    JUTConsole* pConsole = JUTException::getConsole();
    pConsole->mPositionX = 0xF;
    pConsole->mPositionY = 0x30;
    pConsole = JUTException::getConsole();
    pConsole->mHeight = 0x16;

    if (pConsole->mHeight > pConsole->mMaxLines) {
        pConsole->mHeight = pConsole->mMaxLines;
    }

    GameSystemException::sMapFileUsingBuffer = new u8[0x10];
}

void GameSystemException::handleException(OSError error, OSContext* pContext, u32 dsisr, u32 dar) {
    if (JUTVideo::getManager() == nullptr) {
        JUTException::sConsole->mOutput = 2;
        JUTException::sConsole->mVisible = false;
        JUTAssertion::setVisible(false);
    }

    VIFlush();
    JUTAssertion::flushMessage();

    if (::isBootWPAD()) {
        for (s32 chan = WPAD_CHAN0; chan < WPAD_CHAN0 + MR::getWPadMaxCount(); chan++) {
            WPADControlMotor(chan, WPAD_MOTOR_STOP);
        }

        JUTException* exception = JUTException::sErrorManager;
        exception->mGamePad = reinterpret_cast< JUTGamePad* >(0xFFFFFFFF);
        exception->mGamePadPort = JUTGamePad::Port_Unknown;
    }

    AIRegisterDMACallback(nullptr);
    __OSStopAudioSystem();

    BOOL interrupts = OSEnableInterrupts();

    if (!handleExceptionDump(pContext)) {
        OSReport("[%s:%d] Failed to dump exception\n", __FILE__, __LINE__);
    }

    OSRestoreInterrupts(interrupts);
}

bool GameSystemException::handleExceptionDump(OSContext* pContext) {
    OSCalendarTime time;
    OSTicksToCalendarTime(OSGetTime(), &time);

    char fileName[40];
    snprintf(fileName, sizeof(fileName), "Exception_%04d%02d%02d_%02d%02d%02d.txt", time.year, time.mon, time.mday, time.hour, time.min, time.sec);

    if (NANDCreate(fileName, NAND_PERM_RWALL, 0) != NAND_RESULT_OK) {
        return false;
    }

    NANDFileInfo fileHandle;
    if (NANDOpen(fileName, &fileHandle, NAND_ACCESS_WRITE) != NAND_RESULT_OK) {
        return false;
    }

    char* pFileEnd = printContext(pContext, sFileBuff, sizeof(sFileBuff));
    s32 len = pFileEnd - sFileBuff;
    return NANDWrite(&fileHandle, sFileBuff, len) == len;
}

char* GameSystemException::printContext(OSContext* pContext, char* pBuffer, u32 bufferSize) {
    char* pCursor = pBuffer;
    pCursor += snprintf(pCursor, bufferSize, "GPR: \n");

    for (u32 i = 0; i < ARRAY_SIZE(pContext->gpr); i += 4) {
        pCursor += snprintf(pCursor, bufferSize, "  R%02d:0x%08X R%02d:0x%08X R%02d:0x%08X R%02d:0x%08X\n", i, pContext->gpr[i], i + 1,
                            pContext->gpr[i + 1], i + 2, pContext->gpr[i + 2], i + 3, pContext->gpr[i + 3]);
    }

    pCursor += snprintf(pCursor, bufferSize, "FPR: \n");
    for (u32 i = 0; i < ARRAY_SIZE(pContext->fpr); i += 4) {
        pCursor += snprintf(pCursor, bufferSize, "  F%02d:%+.3E F%02d:%+.3E F%02d:%+.3E F%02d:%+.3E\n", i, pContext->fpr[i], i + 1,
                            pContext->fpr[i + 1], i + 2, pContext->fpr[i + 2], i + 3, pContext->fpr[i + 3]);
    }

    pCursor += snprintf(pCursor, bufferSize, "STACK TRACE:\n");
    const u32* pStack = reinterpret_cast< u32* >(pContext->gpr[1]);
    for (u32 i = 0; (pStack != nullptr) && (pStack != reinterpret_cast< u32* >(0xFFFFFFFF)) && (i++ < 0x10);) {
        pCursor += snprintf(pCursor, bufferSize, "  %08X %08X %08X\n", pStack, pStack[0], pStack[1]);
        pStack = reinterpret_cast< u32* >(pStack[0]);
    }

    pCursor += snprintf(pCursor, bufferSize, "SRR0:  0x%08X\n", pContext->srr0);
    pCursor += snprintf(pCursor, bufferSize, "SRR1:  0x%08X\n", pContext->srr1);
    pCursor += snprintf(pCursor, bufferSize, "MODE:  0x%04X\n", pContext->mode);
    pCursor += snprintf(pCursor, bufferSize, "STATE: 0x%04X\n", pContext->state);
    pCursor += snprintf(pCursor, bufferSize, "CR:    0x%08X\n", pContext->cr);
    pCursor += snprintf(pCursor, bufferSize, "LR:    0x%08X\n", pContext->lr);
    pCursor += snprintf(pCursor, bufferSize, "CTR:   0x%08X\n", pContext->ctr);
    pCursor += snprintf(pCursor, bufferSize, "XER:   0x%08X\n", pContext->xer);
    pCursor += snprintf(pCursor, bufferSize, "FPSCR: 0x%08X\n", pContext->fpscr);
    return pCursor;
}
