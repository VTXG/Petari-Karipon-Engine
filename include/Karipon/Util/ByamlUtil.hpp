#pragma once

#include "Karipon/Util/ByamlIter.hpp"
#include <Game/Util/Color.hpp>
#include <JSystem/JGeometry/TVec.hpp>

namespace krp {
    class ByamlIter;
    class ByamlStringTableIter;

    ByamlIter createByamlRoot(const void* pData);
    ByamlStringTableIter getByamlHashKeyTable(const u8* pData);
    ByamlStringTableIter getByamlStringTable(const u8* pData);
    void getByamlDataColor8(const ByamlIter& rIter, Color8* pValue);
    void getByamlDataTVec3f(const ByamlIter& rIter, TVec3f* pValue);
}; // namespace krp
