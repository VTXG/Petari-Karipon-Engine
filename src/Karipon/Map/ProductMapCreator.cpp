#include "Karipon/Map/ProductMapCreator.hpp"
#include <Game/Map/Air.hpp>
#include <Game/Map/Sky.hpp>
#include <Game/MapObj/AnmModelObj.hpp>
#include <Game/MapObj/AssemblyBlock.hpp>
#include <Game/MapObj/HipDropMoveObj.hpp>
#include <Game/MapObj/ManholeCover.hpp>
#include <Game/MapObj/MiniatureGalaxy.hpp>
#include <Game/MapObj/RailMoveObj.hpp>
#include <Game/MapObj/RevolvingWay.hpp>
#include <Game/MapObj/RotateMoveObj.hpp>
#include <Game/MapObj/SimpleClipPartsObj.hpp>
#include <Game/MapObj/SimpleFloaterObj.hpp>
#include <Game/MapObj/SimpleMapObj.hpp>
#include <Game/MapObj/SimpleTimerObj.hpp>
#include <Game/MapObj/SurprisedGalaxy.hpp>
#include <Game/Scene/SceneObjHolder.hpp>
#include <Game/Util/FileUtil.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Util/StringUtil.hpp>
#include <JSystem/JKernel/JKRMemArchive.hpp>

namespace {
    const NameObjFactory::Name2CreateFunc cProductMapClassTable[] = {
        {"AnmModelSwitchMove", createNameObj< AnmModelSwitchMove >, nullptr},
        {"AnmModelGroundOnMove", createNameObj< AnmModelGroundOnMove >, nullptr},
        {"AnmModelBindMove", createNameObj< AnmModelBindMove >, nullptr},
        {"AnmModelSwitchMoveEventCamera", createNameObj< AnmModelSwitchMoveEventCamera >, nullptr},
        {"Air", createNameObj< Air >, nullptr},
        {"ProjectionMapAir", createNameObj< ProjectionMapAir >, nullptr},
        {"Sky", createNameObj< Sky >, nullptr},
        {"ProjectionMapSky", createNameObj< ProjectionMapSky >, nullptr},
        {"AssemblyBlock", createNameObj< AssemblyBlock >, nullptr},
        {"HipDropMoveObj", createNameObj< HipDropMoveObj >, nullptr},
        {"ManholeCover", createNameObj< ManholeCover >, nullptr},
        {"MiniatureGalaxy", createNameObj< MiniatureGalaxy >, nullptr},
        {"SurprisedGalaxy", createNameObj< SurprisedGalaxy >, "MiniSurprisedGalaxy"},
        {"RevolvingWay", createNameObj< RevolvingWay >, nullptr},
        {"RailMoveObj", createNameObj< RailMoveObj >, nullptr},
        {"RotateMoveObj", createNameObj< RotateMoveObj >, nullptr},
        {"SimpleClipPartsObj", createNameObj< SimpleClipPartsObj >, nullptr},
        {"SimpleFloaterObj", createNameObj< SimpleFloaterObj >, nullptr},
        {"SimpleMapObj", createNameObj< SimpleMapObj >, nullptr},
        {"SimpleEnvironmentObj", createNameObj< SimpleEnvironmentObj >, nullptr},
        {"SimpleMapObjFarMax", createNameObj< SimpleMapObjFarMax >, nullptr},
        {"SimpleMapObjPush", createNameObj< SimpleMapObjPush >, nullptr},
        {"SimpleMapObjNoSilhouetted", createNameObj< SimpleMapObjNoSilhouetted >, nullptr},
        {"SimpleSeesawObj", createNameObj< SimpleSeesawObj >, nullptr},
        {"SimpleMirrorObj", createNameObj< SimpleMirrorObj >, nullptr},
        {"SimpleMirrorReflectionObj", createNameObj< SimpleMirrorReflectionObj >, nullptr},
        {"SimpleTextureSwitchChangeObj", createNameObj< SimpleTextureSwitchChangeObj >, nullptr},
        {"SimpleTimerObj", createNameObj< SimpleTimerObj >, nullptr},
    };
};

namespace krp {
    ProductMapCreator::ProductMapCreator() : NameObj("ProductMapCreator") {
        JKRMemArchive* archive = MR::receiveArchive("/ObjectData/ProductMapObjDataTable.arc");

        JMapInfo info;
        info.attach(archive->getResource("ProductMapObjDataTable.bcsv"));

        int entryNum = info.getNumEntries();
        mProductMapTable.init(entryNum);

        for (s32 i = 0; i < entryNum; i++) {
            ProductMapEntry* entry = &mProductMapTable[i];
            MR::getCsvDataStr(&entry->mModelName, &info, "ModelName", i);
            MR::getCsvDataStr(&entry->mClassName, &info, "ClassName", i);
        }
    }

    CreatorFuncPtr ProductMapCreator::getCreateFunc(const char* pModelName) const {
        for (s32 i = 0; i < mProductMapTable.size(); i++) {
            const ProductMapEntry* entry = &mProductMapTable[i];

            if (MR::isEqualString(entry->mModelName, pModelName)) {
                return getCreateEntry(entry->mClassName)->mCreateFunc;
            }
        }

        return nullptr;
    }

    const NameObjFactory::Name2CreateFunc* ProductMapCreator::getCreateEntry(const char* pClassName) const {
        for (s32 i = 0; i < ARRAY_SIZE(cProductMapClassTable); i++) {
            const NameObjFactory::Name2CreateFunc* entry = &cProductMapClassTable[i];

            if (MR::isEqualString(entry->mName, pClassName)) {
                return entry;
            }
        }

        return nullptr;
    }

    bool ProductMapCreator::isRegisteredObj(const char* pModelName) const {
        for (s32 i = 0; i < mProductMapTable.size(); i++) {
            const ProductMapEntry* entry = &mProductMapTable[i];

            if (MR::isEqualString(entry->mModelName, pModelName)) {
                return true;
            }
        }

        return false;
    }

    bool ProductMapCreatorFunction::isRegisteredObj(const char* pModelName) {
        return MR::getSceneObj< ProductMapCreator >(SceneObj_ProductMapCreator)->isRegisteredObj(pModelName);
    }

    CreatorFuncPtr ProductMapCreatorFunction::getProductMapCreator(const char* pModelName) {
        return MR::getSceneObj< ProductMapCreator >(SceneObj_ProductMapCreator)->getCreateFunc(pModelName);
    }
} // namespace krp
