#pragma once

#include <Game/NameObj/NameObj.hpp>
#include <Game/NameObj/NameObjFactory.hpp>
#include <Game/Util/Array.hpp>

struct ProductMapEntry {
    /* 0x0 */ const char* mModelName;
    /* 0x4 */ const char* mClassName;
};

namespace krp {
    class ProductMapCreator : public NameObj {
    public:
        ProductMapCreator();

        CreatorFuncPtr getCreateFunc(const char* pModelName) const;
        const NameObjFactory::Name2CreateFunc* getCreateEntry(const char* pClassName) const;
        bool isRegisteredObj(const char* pModelName) const;

        MR::AssignableArray< ProductMapEntry > mProductMapTable;
    };

    namespace ProductMapCreatorFunction {
        bool isRegisteredObj(const char* pModelName);
        CreatorFuncPtr getProductMapCreator(const char* pModelName);
    }; // namespace ProductMapCreatorFunction
} // namespace krp
