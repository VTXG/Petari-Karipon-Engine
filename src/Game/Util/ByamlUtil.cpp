#include "Game/Util/ByamlUtil.hpp"
#include "Game/Util/ByamlHeader.hpp"
#include "Game/Util/ByamlIter.hpp"
#include <revolution/types.h>

namespace ByamlUtil {
    ByamlIter createByamlRoot(const u8* pData) {
        const ByamlHeader* pHeader = reinterpret_cast< const ByamlHeader* >(pData);

        if (pHeader->getTag() == 'BY' && (static_cast< s32 >(pHeader->getVersion()) - 1) < 3) {
            return ByamlIter(pData);
        }

        return ByamlIter();
    }

    ByamlStringTableIter getHashKeyTable(const u8* pData) {
        const ByamlHeader* pHeader = reinterpret_cast< const ByamlHeader* >(pData);

        s32 offset = pHeader->getHashKeyTableOffset();
        if (offset == 0) {
            return ByamlStringTableIter();
        }

        return ByamlStringTableIter(&pData[offset]);
    }

    ByamlStringTableIter getStringTable(const u8* pData) {
        const ByamlHeader* pHeader = reinterpret_cast< const ByamlHeader* >(pData);

        s32 offset = pHeader->getStringTableOffset();
        if (offset == 0) {
            return ByamlStringTableIter();
        }

        return ByamlStringTableIter(&pData[offset]);
    }

    void getColorValue(const ByamlIter& rIter, GXColor* pValue) {
        u32 r = 0, g = 0, b = 0, a = 0xFF;
        rIter.tryGetValueByKey(&r, "R");
        rIter.tryGetValueByKey(&g, "G");
        rIter.tryGetValueByKey(&b, "B");
        rIter.tryGetValueByKey(&a, "A");
        pValue->r = r;
        pValue->g = g;
        pValue->b = b;
        pValue->a = a;
    }

    void getVector3Value(const ByamlIter& rIter, TVec3f* pValue) {
        f32 x = 0.0f, y = 0.0f, z = 0.0f;
        rIter.tryGetValueByKey(&x, "X");
        rIter.tryGetValueByKey(&y, "Y");
        rIter.tryGetValueByKey(&z, "Z");
        pValue->x = x;
        pValue->y = y;
        pValue->z = z;
    }
} // namespace ByamlUtil
