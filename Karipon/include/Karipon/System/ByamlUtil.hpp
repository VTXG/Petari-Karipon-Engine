#pragma once

#include <revolution/types.h>

class ByamlIter;
class ByamlStringTableIter;

namespace ByamlUtil {
    ByamlIter createByamlRoot(const u8* pData);
    ByamlIter createByamlRootFromFile(const char* pFilePath);
    ByamlStringTableIter getHashKeyTable(const u8* pData);
    ByamlStringTableIter getStringTable(const u8* pData);
} // namespace ByamlUtil
