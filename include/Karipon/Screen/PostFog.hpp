#pragma once

#include <Game/Screen/ImageEffectBase.hpp>

class ResourceHolder;
class JUTTexture;

class PostFog : public ImageEffectBase {
public:
    PostFog(const char* pName);

    virtual void draw() const;

    void capture() const;
    void drawFog(s16 left, s16 top, s16 right, s16 bottom, bool) const;
    void createFogTexture();
    void loadGradientTexture();
    JUTTexture* getGradientTexture() const;

    /* 0x14 */ JUTTexture* mDepthTexture;
    /* 0x18 */ u32 _1C;  // Unknown
    /* 0x1C */ s32 mInnerRadius;
    /* 0x20 */ s32 mOuterRadius;
    /* 0x24 */ u8 mA;
    /* 0x25 */ u8 mR;
    /* 0x26 */ u8 mG;
    /* 0x27 */ u8 mB;
    /* 0x28 */ s32 mGradientType;
    /* 0x2C */ JUTTexture** mFogGradTexture;
    /* 0x30 */ s32 mFogGradNum;
    /* 0x34 */ ResourceHolder* mFogGradArc;
};
