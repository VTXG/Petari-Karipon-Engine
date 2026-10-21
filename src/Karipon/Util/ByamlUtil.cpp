#include "Karipon/Util/ByamlUtil.hpp"
#include "Karipon/Util/ByamlHeader.hpp"
#include "Karipon/Util/ByamlIter.hpp"

namespace krp {
    ByamlIter createByamlRoot(const u8* pData) {
        const ByamlHeader* header = reinterpret_cast< const ByamlHeader* >(pData);

        if (header->getTag() == 'BY' && (static_cast< s32 >(header->getVersion()) - 1) < 3) {
            return ByamlIter(pData);
        }

        return ByamlIter();
    }

    ByamlStringTableIter getByamlHashKeyTable(const u8* pData) {
        const ByamlHeader* header = reinterpret_cast< const ByamlHeader* >(pData);

        s32 offset = header->getHashKeyTableOffset();
        if (offset == 0) {
            return ByamlStringTableIter();
        }

        return ByamlStringTableIter(&pData[offset]);
    }

    ByamlStringTableIter getByamlStringTable(const u8* pData) {
        const ByamlHeader* header = reinterpret_cast< const ByamlHeader* >(pData);

        s32 offset = header->getStringTableOffset();
        if (offset == 0) {
            return ByamlStringTableIter();
        }

        return ByamlStringTableIter(&pData[offset]);
    }

    void getByamlDataColor8(const ByamlIter& rIter, Color8* pValue) {
        u32 r = 0, g = 0, b = 0, a = 0xFF;
        rIter.tryGetValueByKey("R", &r);
        rIter.tryGetValueByKey("G", &g);
        rIter.tryGetValueByKey("B", &b);
        rIter.tryGetValueByKey("A", &a);
        pValue->r = r;
        pValue->g = g;
        pValue->b = b;
        pValue->a = a;
    }

    void getValueTVec3f(const ByamlIter& rIter, TVec3f* pValue) {
        f32 x = 0.0f, y = 0.0f, z = 0.0f;
        rIter.tryGetValueByKey("X", &x);
        rIter.tryGetValueByKey("Y", &y);
        rIter.tryGetValueByKey("Z", &z);
        pValue->x = x;
        pValue->y = y;
        pValue->z = z;
    }
} // namespace ByamlUtil
