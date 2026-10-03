#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <nw4r/ut/WideTextWriter.h>

class SystemDebugLayout {
public:
    struct PrintContext {
        PrintContext(SystemDebugLayout* pLayout);

        void prepareDraw(u32 color);
        bool printText(bool isSelectable, const wchar_t* pText);
        bool printTextF(bool isSelectable, const wchar_t* pText, ...);
        void printFillBox(const TVec2f& tl, const TVec2f& br, u32 color);

        /* 0x00 */ SystemDebugLayout* mLayout;
        /* 0x04 */ nw4r::ut::WideTextWriter mWriter;
        /* 0x68 */ nw4r::ut::Rect mDrawRect;
    };

    SystemDebugLayout();

    void draw();

    /* 0x00 */ u32 mPageIndex;
    /* 0x04 */ u32 mSelectIndex;
    /* 0x08 */ u32 mSelectCurrentIndex;
    /* 0x0C */ bool mIsVisible;
};
