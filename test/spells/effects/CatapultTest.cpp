/*
 * CatapultTest.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "EffectFixture.h"

#include <vstd/RNG.h>

#include "../../../lib/mapObjects/CGTownInstance.h"
#include "../../../lib/json/JsonNode.h"


namespace test
{
using namespace ::spells;
using namespace ::spells::effects;
using namespace ::testing;

class CatapultTest : public Test, public EffectFixture
{
public:
	CatapultTest()
		:EffectFixture("core:catapult")
	{
	}

protected:
	void SetUp() override
	{
		EffectFixture::setUp();
		setupEffect(JsonNode());
	}
};

TEST_F(CatapultTest, NotApplicableWithoutTown)
{
	EXPECT_CALL(*battleFake, getDefendedTown()).WillRepeatedly(Return(nullptr));
	EXPECT_CALL(mechanicsMock, adaptProblem(_, _)).WillOnce(Return(false));
	EXPECT_CALL(mechanicsMock, isSmart()).WillRepeatedly(Return(true));
	EXPECT_CALL(*battleFake, getWallState(_)).Times(0);

	EXPECT_FALSE(subject->applicable(problemMock, &mechanicsMock));
}

TEST_F(CatapultTest, NotApplicableInVillage)
{
	auto fakeTown = std::make_shared<CGTownInstance>(nullptr);

	EXPECT_CALL(*battleFake, getDefendedTown()).WillRepeatedly(Return(fakeTown.get()));
	EXPECT_CALL(mechanicsMock, adaptProblem(_, _)).WillOnce(Return(false));
	EXPECT_CALL(mechanicsMock, isSmart()).WillRepeatedly(Return(true));
	EXPECT_CALL(*battleFake, getWallState(_)).Times(0);

	EXPECT_FALSE(subject->applicable(problemMock, &mechanicsMock));
}

TEST_F(CatapultTest, NotApplicableForDefenderIfSmart)
{
	auto fakeTown = std::make_shared<CGTownInstance>(nullptr);
	fakeTown->addBuilding(BuildingID::FORT);
	mechanicsMock.casterSide = BattleSide::DEFENDER;

	EXPECT_CALL(*battleFake, getDefendedTown()).WillRepeatedly(Return(fakeTown.get()));
	EXPECT_CALL(mechanicsMock, adaptProblem(_, _)).WillOnce(Return(false));
	EXPECT_CALL(mechanicsMock, isSmart()).WillRepeatedly(Return(true));
	EXPECT_CALL(*battleFake, getWallState(_)).WillRepeatedly(Return(EWallState::INTACT));

	EXPECT_FALSE(subject->applicable(problemMock, &mechanicsMock));
}

TEST_F(CatapultTest, DISABLED_ApplicableInTown)
{
	auto fakeTown = std::make_shared<CGTownInstance>(nullptr);
	fakeTown->addBuilding(BuildingID::FORT);

	EXPECT_CALL(*battleFake, getDefendedTown()).WillRepeatedly(Return(fakeTown.get()));
	EXPECT_CALL(mechanicsMock, adaptProblem(_, _)).Times(0);
	EXPECT_CALL(mechanicsMock, isSmart()).WillRepeatedly(Return(true));
	EXPECT_CALL(*battleFake, getWallState(_)).WillRepeatedly(Return(EWallState::INTACT));

	EXPECT_TRUE(subject->applicable(problemMock, &mechanicsMock));
}

class CatapultApplyTest : public Test, public EffectFixture
{
public:
	CatapultApplyTest()
		: EffectFixture("core:catapult")
	{
	}

	void setDefaultExpectations()
	{
		EXPECT_CALL(*battleFake, getDefendedTown()).WillRepeatedly(Return(fakeTown.get()));
		EXPECT_CALL(mechanicsMock, isSmart()).WillRepeatedly(Return(true));
		setupDefaultRNG();
	}

protected:
	void SetUp() override
	{
		EffectFixture::setUp();
		fakeTown = std::make_shared<CGTownInstance>(nullptr);
		fakeTown->addBuilding(BuildingID::FORT);
	}
private:
	std::shared_ptr<CGTownInstance> fakeTown;
};

TEST_F(CatapultApplyTest, DISABLED_DamageToIntactPart)
{
	{
		JsonNode config;
		config["targetsToAttack"].Integer() = 1;
		config["chanceToNormalHit"].Integer() = 100;
		EffectFixture::setupEffect(config);
	}

	setDefaultExpectations();

	const EWallPart targetPart = EWallPart::BELOW_GATE;
	auto & actualCaster = unitsFake.add(BattleSide::ATTACKER);

	mechanicsMock.caster = &actualCaster;
	EXPECT_CALL(actualCaster, getCasterUnitId()).WillRepeatedly(Return(-1));
	EXPECT_CALL(mechanicsMock, isMassive()).WillRepeatedly(Return(true));
	EXPECT_CALL(*battleFake, getWallState(_)).WillRepeatedly(Return(EWallState::DESTROYED));
	EXPECT_CALL(*battleFake, getWallState(Eq(targetPart))).WillRepeatedly(Return(EWallState::INTACT));
	EXPECT_CALL(*battleFake, setWallState(Eq(targetPart), Eq(EWallState::DAMAGED))).Times(1);
	EXPECT_CALL(serverMock, apply(Matcher<CatapultAttack *>(_))).Times(1);

    EffectTarget target;
    target.emplace_back();

	subject->apply(&serverMock, &mechanicsMock, target);
}

}
