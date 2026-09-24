#include "Game/System/ExMessageHolder.hpp"
#include "Kamek.hpp"
#include <Game/Screen/MessageTagSkipTagProcessor.hpp>
#include <Game/System/GameSystemObjHolder.hpp>
#include <Game/Util/FileUtil.hpp>
#include <Game/Util/MemoryUtil.hpp>
#include <Game/Util/SceneUtil.hpp>
#include <Game/Util/SystemUtil.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <cstdio>

MessageData* ExMessageSystem::getZoneMessageData(s32 id) {
    ExMessageHolder* pMessageHolder = static_cast< ExMessageHolder* >(MR::getGameSystemObjHolder()->mMessageHolder);

    if (pMessageHolder->mZoneMessageData == nullptr || id < 0 || id >= pMessageHolder->mZoneMessageCount) {
        return pMessageHolder->mGameMessageData;
    }

    MessageData* pData = pMessageHolder->mZoneMessageData[id];
    return pData ? pData : pMessageHolder->mGameMessageData;
}

namespace {
    static ExMessageHolder* ctorExtMessageHolder(ExMessageHolder* pMessageHolder) {
        pMessageHolder->resetZoneMessageData();
        return pMessageHolder;
    }

    static void initSceneData(ExMessageHolder* pMessageHolder) {
        pMessageHolder->mZoneMessageCount = MR::getZoneNum();
        pMessageHolder->mZoneMessageData = new MessageData*[pMessageHolder->mZoneMessageCount];

        for (s32 i = 0; i < pMessageHolder->mZoneMessageCount; i++) {
            const char* pZoneName = MR::getZoneNameFromZoneId(i);

            char filePath[0x100];
            snprintf(filePath, sizeof(filePath), "/MessageData/%s.arc", pZoneName);

            if (MR::isFileExist(filePath, true)) {
                MR::mountArchive(filePath, MR::getSceneHeapGDDR3());
                pMessageHolder->mZoneMessageData[i] = new MessageData(filePath);
            } else {
                pMessageHolder->mZoneMessageData[i] = nullptr;
            }
        }
    }

    static void destroySceneData(ExMessageHolder* pMessageHolder) {
        if (pMessageHolder->mZoneMessageData == nullptr) {
            return;
        }

        for (s32 i = 0; i < pMessageHolder->mZoneMessageCount; i++) {
            if (pMessageHolder->mZoneMessageData[i] != nullptr) {
                delete pMessageHolder->mZoneMessageData[i];
            }
        }

        pMessageHolder->resetZoneMessageData();
    }

    static MessageData* initTalkNodeCtrl(ExTalkNodeCtrl* pTalkNodeCtrl, TalkMessageCtrl* pMsgCtrl) {
        return pTalkNodeCtrl->mMessageData = ExMessageSystem::getZoneMessageData(pMsgCtrl->mZoneID);
    }

    static TalkNode* getNextNodeTalkNodeCtrl(ExTalkNodeCtrl* pTalkNodeCtrl) {
        TalkNode* node = pTalkNodeCtrl->mCurrentNode;
        u16 idx;

        if (node == nullptr) {
            return nullptr;
        }

        if (node->mNodeType == 1) {
            idx = node->mNextIdx;

            if (idx != 0xFFFF) {
                return pTalkNodeCtrl->mMessageData->getNode(idx);
            }
        } else if (node->mNodeType == 3) {
            idx = node->mIndex;

            if (pTalkNodeCtrl->mMessageData->isValidBranchNode(idx)) {
                return pTalkNodeCtrl->mMessageData->getBranchNode(idx = node->mIndex);
            }
        }

        return nullptr;
    }

    static const wchar_t* getSubMessageTalkNodeCtrl(ExTalkNodeCtrl* pTalkNodeCtrl) {
        const TalkMessageInfo& rMessageInfo = pTalkNodeCtrl->mMessageInfo;

        if (*reinterpret_cast< const wchar_t* >(rMessageInfo._0) != 0x1A) {
            return nullptr;
        }

        MessageEditorMessageTag messageTag(reinterpret_cast< const wchar_t* >(&rMessageInfo._0[2]));

        if (reinterpret_cast< const char* >(messageTag.mMessage)[1] != 8 || messageTag.mMessage[1] != 0) {
            return nullptr;
        }

        TalkMessageInfo subMessageInfo;
        u16 param = messageTag.getParam32(0);

        if (pTalkNodeCtrl->mMessageData->getMessage(&subMessageInfo, 0, param)) {
            return reinterpret_cast< const wchar_t* >(subMessageInfo._0);
        }

        return nullptr;
    }
} // namespace

extern kmSymbol initMessageResource__19GameSystemObjHolderFv;
kmWrite16(&initMessageResource__19GameSystemObjHolderFv + 0xA2, sizeof(ExMessageHolder));

extern kmSymbol createMessage__15TalkMessageCtrlFRC12JMapInfoIterPCc;
kmWrite16(&createMessage__15TalkMessageCtrlFRC12JMapInfoIterPCc + 0x22, sizeof(ExTalkNodeCtrl));

extern kmSymbol createMessageDirect__15TalkMessageCtrlFRC12JMapInfoIterPCc;
kmWrite16(&createMessageDirect__15TalkMessageCtrlFRC12JMapInfoIterPCc + 0x22, sizeof(ExTalkNodeCtrl));

extern kmSymbol __ct__13MessageHolderFv;
kmBranch(&__ct__13MessageHolderFv + 0x10, ctorExtMessageHolder);

extern kmSymbol initSceneData__13MessageHolderFv;
kmBranch(&initSceneData__13MessageHolderFv, initSceneData);

extern kmSymbol destroySceneData__13MessageHolderFv;
kmBranch(&destroySceneData__13MessageHolderFv, destroySceneData);

extern kmSymbol createFlowNodeDirect__12TalkNodeCtrlFP15TalkMessageCtrlRC12JMapInfoIterPCcPP15ActorCameraInfo;
kmCall(&createFlowNodeDirect__12TalkNodeCtrlFP15TalkMessageCtrlRC12JMapInfoIterPCcPP15ActorCameraInfo + 0x28, initTalkNodeCtrl);
kmWrite32(&createFlowNodeDirect__12TalkNodeCtrlFP15TalkMessageCtrlRC12JMapInfoIterPCcPP15ActorCameraInfo + 0x80, PPC_LWZ(3, 0x48, 27));

extern kmSymbol forwardFlowNode__12TalkNodeCtrlFv;
kmWrite32(&forwardFlowNode__12TalkNodeCtrlFv + 0x34, PPC_LWZ(3, 0x48, 30));
kmWrite32(&forwardFlowNode__12TalkNodeCtrlFv + 0x54, PPC_LWZ(3, 0x48, 30));

extern kmSymbol getNextNode__12TalkNodeCtrlCFv;
kmBranch(&getNextNode__12TalkNodeCtrlCFv, getNextNodeTalkNodeCtrl);

extern kmSymbol updateMessage__12TalkNodeCtrlFv;
kmWrite32(&updateMessage__12TalkNodeCtrlFv + 0x44, PPC_LWZ(3, 0x48, 28));

extern kmSymbol getSubMessage__12TalkNodeCtrlCFv;
kmBranch(&getSubMessage__12TalkNodeCtrlCFv, getSubMessageTalkNodeCtrl);

extern kmSymbol forwardCurrentBranchNode__12TalkNodeCtrlFb;
kmWrite32(&forwardCurrentBranchNode__12TalkNodeCtrlFb + 0x28, PPC_LWZ(3, 0x48, 30));
kmWrite32(&forwardCurrentBranchNode__12TalkNodeCtrlFb + 0x40, PPC_LWZ(3, 0x48, 30));
