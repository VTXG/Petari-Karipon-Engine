#include <Game/MapObj/AstroDomeOrbit.hpp>
#include <revolution/gx/GXStruct.h>

class ExAstroDomeOrbit : public AstroDomeOrbit {
public:
    GXColor mColor;
    GXColor mBloom;
    f32 mSpeed;
};
