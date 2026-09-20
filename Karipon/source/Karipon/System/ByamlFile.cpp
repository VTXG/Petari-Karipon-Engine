#include "Karipon/System/ByamlHeader.hpp"
#include "Karipon/System/ByamlIter.hpp"
#include "Karipon/System/ByamlUtil.hpp"
#include <Game/Util/FileUtil.hpp>
#include <revolution/types.h>

namespace ByamlUtil {
    ByamlIter createByamlRoot(const u8* pData) {
        const ByamlHeader* pHeader = reinterpret_cast<const ByamlHeader*>(pData);

        if (pHeader->getTag() == 'BY' && (static_cast<s32>(pHeader->getVersion()) - 1) < 3) {
            return ByamlIter(pData);
        }

        return ByamlIter();
    }

    ByamlIter createByamlRootFromFile(const char* pFilePath) {
        void* pData = MR::receiveFile(pFilePath);
        return createByamlRoot(static_cast<u8*>(pData));
    }

    ByamlStringTableIter getHashKeyTable(const u8* pData) {
        const ByamlHeader* pHeader = reinterpret_cast<const ByamlHeader*>(pData);

        s32 offset = pHeader->getHashKeyTableOffset();
        if (offset == 0) {
            return ByamlStringTableIter();
        }

        return ByamlStringTableIter(&pData[offset]);
    }

    ByamlStringTableIter getStringTable(const u8* pData) {
        const ByamlHeader* pHeader = reinterpret_cast<const ByamlHeader*>(pData);

        s32 offset = pHeader->getStringTableOffset();
        if (offset == 0) {
            return ByamlStringTableIter();
        }

        return ByamlStringTableIter(&pData[offset]);
    }
} // namespace ByamlUtil
