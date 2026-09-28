#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <nw4r/ut/WideTextWriter.h>

class SystemDebugLayout {
public:
    SystemDebugLayout();

    void draw();
    void initWriter();
    void initDraw(u32 color);
    bool printText(bool selectable, const wchar_t* pText);
    bool printTextF(bool selectable, const wchar_t* pText, ...);
    void printFillBox(const TVec2f& tl, const TVec2f& br, u32 color);

    /* 0x00 */ nw4r::ut::WideTextWriter mWriter;
    /* 0x64 */ u32 mPageIndex;
    /* 0x68 */ u32 mSelectIndex;
    /* 0x6C */ u32 mSelectCurrentIndex;
    /* 0x70 */ bool mIsVisible;
};
