#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class ByamlIter;
class ByamlStringTableIter;
class Color8;

namespace ByamlUtil {
    ByamlIter createByamlRoot(const u8* pData);
    ByamlStringTableIter getHashKeyTable(const u8* pData);
    ByamlStringTableIter getStringTable(const u8* pData);
    void getValueColor8(const ByamlIter& rIter, Color8* pValue);
    void getValueTVec3f(const ByamlIter& rIter, TVec3f* pValue);
} // namespace ByamlUtil
