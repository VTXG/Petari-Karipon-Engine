#include "Karipon/MapObj/TicoCoin.hpp"
#include "Karipon/System/SystemDataHolder.hpp"
#include "Karipon/Util/ByamlIter.hpp"
#include <Game/LiveActor/LiveActor.hpp>
#include <Game/LiveActor/Nerve.hpp>
#include <Game/System/GameDataFunction.hpp>
#include <Game/System/GameSequenceFunction.hpp>
#include <Game/Util/ActorCameraUtil.hpp>
#include <Game/Util/ActorSensorUtil.hpp>
#include <Game/Util/ActorShadowUtil.hpp>
#include <Game/Util/ActorStateUtil.hpp>
#include <Game/Util/ActorSwitchUtil.hpp>
#include <Game/Util/DemoUtil.hpp>
#include <Game/Util/EventUtil.hpp>
#include <Game/Util/JMapUtil.hpp>
#include <Game/Util/LiveActorUtil.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Util/SceneUtil.hpp>

namespace {
    NEW_NERVE(TicoCoinNrvAppear, krp::TicoCoin, Appear);
    NEW_NERVE(TicoCoinNrvRotate, krp::TicoCoin, Rotate);
    NEW_NERVE(TicoCoinNrvCollect, krp::TicoCoin, Collect);
    NEW_NERVE(TicoCoinNrvDisappear, krp::TicoCoin, Disappear);
}; // namespace

namespace krp {
    TicoCoin::TicoCoin(const char* pName) : LiveActor(pName), mShadowPosition(0.0f, 0.0f, 0.0f), mCameraInfo(), mAppearDemoTime(), mID(-1) {}

    void TicoCoin::init(const JMapInfoIter& rIter) {
        MR::initDefaultPos(this, rIter);
        initModelManagerWithAnm("TicoCoin", nullptr, false);
        MR::connectToSceneNoSilhouettedMapObj(this);

        initHitSensor(1);
        initEffectKeeper(0, "TicoCoin", false);
        initSound(1, true);

        MR::addHitSensorMapObj(this, "Body", 4, 120.0f, TVec3f(0.0f, 215.0f, 0.0f));
        MR::validateClipping(this);
        MR::setClippingTypeSphere(this, 300.0f);
        MR::calcGravity(this);

        initNerve(GET_NERVE_ANON(TicoCoinNrvRotate));

        MR::getJMapInfoArg0NoInit(rIter, &mID);

        if (!isTicoCoinExist()) {
            OSReport("TicoCoin %d doesn't exist\n", mID);
            mID = -1;
        } else {
            OSReport("TicoCoin %d valid\n", mID);
        }

        s32 registerDemo;
        if (MR::getJMapInfoArg1NoInit(rIter, &registerDemo) && registerDemo == 1) {
            MR::registerDemoSimpleCastAll(this);
        }

        if (MR::initMultiActorCamera(this, rIter, &mCameraInfo, "出現デモカメラ")) {
            mAppearDemoTime = MR::getMultiActorCameraFrames(this, mCameraInfo, "出現デモカメラ");

            if (mAppearDemoTime <= 0) {
                mAppearDemoTime = 120;
            }
        }

        MR::startBva(this, isTicoCoinCollected() ? "Empty" : "Normal");

        if (MR::useStageSwitchReadAppear(this, rIter)) {
            MR::syncStageSwitchAppear(this);
            makeActorDead();

            setNerve(GET_NERVE_ANON(TicoCoinNrvAppear));
            MR::invalidateClipping(this);
        } else {
            makeActorAppeared();
        }

        if (mID != -1 && GameSequenceFunction::isTicoCoinCollected(mID)) {
            makeActorDead();
            setNerve(GET_NERVE_ANON(TicoCoinNrvDisappear));
        }
    }

    void TicoCoin::initAfterPlacement() {
        // MR::setClippingRangeIncludeShadow(this, &mShadowPosition, 300.0f);
    }

    void TicoCoin::makeActorAppeared() {
        if (!isNerve(GET_NERVE_ANON(TicoCoinNrvDisappear))) {
            LiveActor::makeActorAppeared();
        }
    }

    void TicoCoin::control() {}

    bool TicoCoin::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
        return MR::isMsgStarPieceReflect(msg);
    }

    bool TicoCoin::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
        if (MR::isMsgItemGet(msg)) {
            return tryCollect();
        }

        return false;
    }

    bool TicoCoin::tryCollect() {
        if (isNerve(GET_NERVE_ANON(TicoCoinNrvRotate))) {
            setNerve(GET_NERVE_ANON(TicoCoinNrvCollect));
            return true;
        }

        return false;
    }

    void TicoCoin::exeAppear() {
        if (MR::tryStartDemo(this, "出現デモ")) {
            MR::startMultiActorCameraTargetSelf(this, mCameraInfo, "出現デモカメラ", -1);
            MR::startAction(this, "Appear");
        }

        if (MR::isGreaterEqualStep(this, mAppearDemoTime)) {
            MR::endMultiActorCamera(this, mCameraInfo, "出現デモカメラ", false, -1);
            MR::endDemo(this, "出現デモ");
            setNerve(GET_NERVE_ANON(TicoCoinNrvRotate));
        }
    }

    void TicoCoin::exeRotate() {
        if (MR::isFirstStep(this)) {
            MR::startAction(this, "Rotate");
        }
    }

    void TicoCoin::exeCollect() {
        if (MR::isFirstStep(this)) {
            MR::startAction(this, "Get");
            MR::hideModel(this);

            if (mID != -1) {
                GameSequenceFunction::onTicoCoinCollected(mID);
            }
        }

        if (MR::isGreaterStep(this, 2)) {
            // MR::explainCometMedalIfAtFirst(this);
            makeActorDead();
            setNerve(GET_NERVE_ANON(TicoCoinNrvDisappear));
        }
    }

    void TicoCoin::exeDisappear() {}

    bool TicoCoin::isTicoCoinExist() const {
        if (mID == -1) {
            return true;
        }

        if (mID < 0) {
            return false;
        }

        ByamlIter params = krp::SystemDataHolder::get()->getStageParams(MR::getCurrentStageName());

        if (params.isValid()) {
            u32 ticoCoinNum = 0;
            params.tryGetValueByKey("TicoCoinNum", &ticoCoinNum);
            return mID < ticoCoinNum;
        }

        return false;
    }

    bool TicoCoin::isTicoCoinCollected() const {
        if (mID == -1) {
            return false;
        }

        return GameDataFunction::isTicoCoinCollected(MR::getCurrentStageName(), mID);
    }
}; // namespace krp
