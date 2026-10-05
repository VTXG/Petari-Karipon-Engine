#include "Game/Player/OnlinePlayer.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/ModelManager.hpp"
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

OnlinePlayer::OnlinePlayer(const char* pName) : LiveActor(pName), mResourceTable(), mXanimePlayer(), mXanimePlayerUpper(), mHasJetTurtle() {}

void OnlinePlayer::init(const JMapInfoIter& rIter) {
    mPosition.zero();
    mRotation.zero();
    mScale.set(1.0f);

    initModelManagerWithAnm("GhostMario", nullptr, true);
    MR::initDLMakerFog(this, true);
    MR::newDifferedDLBuffer(this);

    mResourceTable =
        new XanimeResourceTable(MR::getResourceHolder(this), marioAnimeTable, marioAnimeAuxTable, marioAnimeOfsTable,
                                reinterpret_cast< XanimeBckTable* >(singleAnimeTable), doubleAnimeTable, tripleAnimeTable, quadAnimeTable, nullptr);

    mXanimePlayer = new XanimePlayer(MR::getJ3DModel(this), mResourceTable);
    mXanimePlayer->duplicateSimpleGroup();
    mModelManager->mXanimePlayer = mXanimePlayer;
    mXanimePlayer->setDefaultAnimation("基本");
    mXanimePlayer->getCore()->enableJointTransform(MR::getJ3DModelData(this));
    mXanimePlayerUpper = new XanimePlayer(MR::getJ3DModel(this), mResourceTable, mXanimePlayer);
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
}

void OnlinePlayer::playAnimation(u32 animHash) {
    mXanimePlayer->changeAnimationByHash(animHash);

    const char* pCurrentAnimName = mXanimePlayer->getCurrentAnimationName();

    if (strstr(pCurrentAnimName, "水泳ジェット") != nullptr) {
        mHasJetTurtle = true;
    } else if (strstr(pCurrentAnimName, "カメ持ち") != nullptr) {
        mHasJetTurtle = true;
    } else if (strstr(pCurrentAnimName, "投げ") != nullptr) {
        mHasJetTurtle = false;
        mXanimePlayerUpper->stopAnimation();
        MR::getJoint(this, "Spine1")->setMtxCalc(nullptr);
    }

    if (mHasJetTurtle && (strcmp(pCurrentAnimName, "基本") != 0 || strcmp(pCurrentAnimName, "ジャンプ") != 0)) {
        mXanimePlayerUpper->changeAnimation("ひろいウエイト");
        mXanimePlayerUpper->overWriteMtxCalc(MR::getJointIndex(this, "PartsControl"));
    }
}

void OnlinePlayer::playAnimationSimple(const char* pAnimName) {
    mXanimePlayer->changeAnimationBck(pAnimName);
}

void OnlinePlayer::setAnimationFrame(f32 frame) {
    mXanimePlayer->_20->setFrame(frame);
    mXanimePlayer->_20->setRate(0.0f);
    mXanimePlayer->_20->setLoop(J3DFrameCtrl::EMode_RESET);
}
