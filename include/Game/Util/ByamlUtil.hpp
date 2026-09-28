#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/gx/GXStruct.h>
#include <revolution/types.h>

class ByamlIter;
class ByamlStringTableIter;

namespace ByamlUtil {
    ByamlIter createByamlRoot(const u8* pData);
    ByamlIter createByamlRootFromFile(const char* pFilePath);
    ByamlStringTableIter getHashKeyTable(const u8* pData);
    ByamlStringTableIter getStringTable(const u8* pData);
    void getColorValue(const ByamlIter& rIter, GXColor* pValue);
    void getVector3Value(const ByamlIter& rIter, TVec3f* pValue);
} // namespace ByamlUtil
