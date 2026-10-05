#include "Game/Player/OnlinePlayer.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/System/GameOnlineManager.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "JSystem/JGeometry/TVec.hpp"

extern XanimeGroupInfo marioAnimeTable[];
extern XanimeAuxInfo marioAnimeAuxTable[];
extern XanimeOfsInfo marioAnimeOfsTable[];
extern XanimeBckTable1 singleAnimeTable[];
extern XanimeBckTable2 doubleAnimeTable[];
extern XanimeBckTable3 tripleAnimeTable[];
extern XanimeBckTable4 quadAnimeTable[];
extern XanimeSwapTable luigiAnimeSwapTable[];

OnlinePlayer::OnlinePlayer(const char* pName) : LiveActor(pName), mClient(), mXanimePlayer(), mXanimePlayerUpper(), mHasJetTurtle(), mIsHidden(true) {}

void OnlinePlayer::init(const JMapInfoIter& rIter) {
    mPosition.zero();
    mRotation.zero();
    mScale.set(1.0f);

    initModelManagerWithAnm("GhostMario", nullptr, true);

    XanimeResourceTable* pResourceTable =
        new XanimeResourceTable(MR::getResourceHolder(this), marioAnimeTable, marioAnimeAuxTable, marioAnimeOfsTable,
                                reinterpret_cast< XanimeBckTable* >(singleAnimeTable), doubleAnimeTable, tripleAnimeTable, quadAnimeTable, nullptr);

    mXanimePlayer = new XanimePlayer(MR::getJ3DModel(this), pResourceTable);
    mXanimePlayer->duplicateSimpleGroup();
    mModelManager->mXanimePlayer = mXanimePlayer;
    mXanimePlayer->setDefaultAnimation("基本");
    mXanimePlayer->getCore()->enableJointTransform(MR::getJ3DModelData(this));
    mXanimePlayerUpper = new XanimePlayer(MR::getJ3DModel(this), pResourceTable, mXanimePlayer);
    mXanimePlayerUpper->changeAnimation("基本");

    initHitSensor(1);
    MR::addHitSensorMapObj(this, "body", 32, 100.0f, TVec3f(0.0f, 0.0f, 0.0f));

    MR::initShadowVolumeSphere(this, 50.0f);
    MR::validateShadow(this, nullptr);
    MR::calcGravity(this);
    MR::onCalcShadow(this, nullptr);
    MR::onCalcShadowDropGravity(this, nullptr);

    MR::invalidateClipping(this);
    MR::connectToSceneMapObjStrongLight(this);
    MR::registerDemoSimpleCastAll(this);

    MR::connectToSceneMapObj(this);
    makeActorAppeared();

    // mIsHidden = true;
    // MR::hideModel(this);
    // MR::invalidateHitSensors(this);
    // MR::invalidateShadowAll(this);
}

void OnlinePlayer::movement() {
    /*
    if (mIsHidden) {
        if (mClient->isEqualCurrentStage()) {
            MR::showModel(this);
            MR::validateHitSensors(this);
            MR::validateShadowAll(this);
            mIsHidden = false;
        }
    }
    else if (!mClient->isEqualCurrentStage()) {
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
        MR::invalidateShadowAll(this);
        mIsHidden = true;
    }
    */

    LiveActor::movement();
}

void OnlinePlayer::playAnimation(u32 animHash) {
    mXanimePlayer->changeAnimationByHash(animHash);
    const char* pAnimName = mXanimePlayer->getCurrentAnimationName();

    if (strstr(pAnimName, "水泳ジェット") != nullptr) {
        mHasJetTurtle = true;
    } else if (strstr(pAnimName, "カメ持ち") != nullptr) {
        mHasJetTurtle = true;
    } else if (strstr(pAnimName, "投げ") != nullptr) {
        mHasJetTurtle = false;
        mXanimePlayerUpper->stopAnimation();
        MR::getJoint(this, "Spine1")->setMtxCalc(nullptr);
    }

    if (mHasJetTurtle && (strcmp(pAnimName, "基本") != 0 || strcmp(pAnimName, "ジャンプ") != 0)) {
        mXanimePlayerUpper->changeAnimation("ひろいウエイト");
        mXanimePlayerUpper->overWriteMtxCalc(MR::getJointIndex(this, "PartsControl"));
    }
}

void OnlinePlayer::playAnimationSimple(const char* pAnimName) {
    // TODO figure this out
    MR::startBck(this, pAnimName);
}

void OnlinePlayer::setAnimationFrame(f32 frame) {
    mXanimePlayer->_20->setFrame(frame);
    mXanimePlayer->_20->setRate(0.0f);
    mXanimePlayer->_20->setLoop(J3DFrameCtrl::EMode_RESET);
}
