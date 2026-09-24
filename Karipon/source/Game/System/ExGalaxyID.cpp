#include "Game/Util/ExSingletonHolder.hpp"
#include "Kamek.hpp"
#include "Karipon/System/KariponResourceHolder.hpp"
#include <Game/Util/FileUtil.hpp>

namespace {
    static void* getGalaxyIDData() {
        return ExSingletonHolder< KariponResourceHolder >::get()->mGalaxyIDTable;
    }
} // namespace

extern kmSymbol initPaneCtrlPointing__9GalaxyMapFv;
kmCall(&initPaneCtrlPointing__9GalaxyMapFv + 0x30, getGalaxyIDData);
kmWrite32(&initPaneCtrlPointing__9GalaxyMapFv + 0x34, PPC_MR(4, 3));
kmWrite32(&initPaneCtrlPointing__9GalaxyMapFv + 0x38, PPC_ADDI(3, 1, 0x1C));

extern kmSymbol isComplete__17GalaxyMapDomeIconCFv;
kmCall(&isComplete__17GalaxyMapDomeIconCFv + 0x1C, getGalaxyIDData);
kmWrite32(&isComplete__17GalaxyMapDomeIconCFv + 0x20, PPC_MR(4, 3));
kmWrite32(&isComplete__17GalaxyMapDomeIconCFv + 0x24, PPC_ADDI(3, 1, 0x28));

extern kmSymbol hasNewGalaxy__17GalaxyMapDomeIconCFv;
kmCall(&hasNewGalaxy__17GalaxyMapDomeIconCFv + 0x1C, getGalaxyIDData);
kmWrite32(&hasNewGalaxy__17GalaxyMapDomeIconCFv + 0x20, PPC_MR(4, 3));
kmWrite32(&hasNewGalaxy__17GalaxyMapDomeIconCFv + 0x24, PPC_ADDI(3, 1, 0x28));

extern kmSymbol getGalaxySortIndex__19GalaxyNameSortTableFPCc;
kmCall(&getGalaxySortIndex__19GalaxyNameSortTableFPCc + 0x20, getGalaxyIDData);
kmWrite32(&getGalaxySortIndex__19GalaxyNameSortTableFPCc + 0x24, PPC_MR(4, 3));
kmWrite32(&getGalaxySortIndex__19GalaxyNameSortTableFPCc + 0x28, PPC_ADDI(3, 1, 0x2C));

extern kmSymbol getPowerStarNumToOpenGalaxy__13GameDataConstFPCc;
kmCall(&getPowerStarNumToOpenGalaxy__13GameDataConstFPCc + 0x1C, getGalaxyIDData);
kmWrite32(&getPowerStarNumToOpenGalaxy__13GameDataConstFPCc + 0x20, PPC_MR(4, 3));
kmWrite32(&getPowerStarNumToOpenGalaxy__13GameDataConstFPCc + 0x24, PPC_ADDI(3, 1, 0x1C));

extern kmSymbol getIncludedGrandGalaxyId__13GameDataConstFPCc;
kmCall(&getIncludedGrandGalaxyId__13GameDataConstFPCc + 0x1C, getGalaxyIDData);
kmWrite32(&getIncludedGrandGalaxyId__13GameDataConstFPCc + 0x20, PPC_MR(4, 3));
kmWrite32(&getIncludedGrandGalaxyId__13GameDataConstFPCc + 0x24, PPC_ADDI(3, 1, 0x1C));

extern kmSymbol getGalaxyDependedFlags__18GameEventFlagTableFPPCciPCc;
kmCall(&getGalaxyDependedFlags__18GameEventFlagTableFPPCciPCc + 0x24, getGalaxyIDData);
kmWrite32(&getGalaxyDependedFlags__18GameEventFlagTableFPPCciPCc + 0x28, PPC_MR(4, 3));
kmWrite32(&getGalaxyDependedFlags__18GameEventFlagTableFPPCciPCc + 0x2C, PPC_ADDI(3, 1, 0x30));
