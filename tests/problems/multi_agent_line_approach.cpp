#include <gtest/gtest.h>
#include "problems/multi_agent_line_approach.h"

#define _USE_MATH_DEFINES
#include <cmath>

#define EPSILON std::pow(10, -6)

using namespace csa_mdp;

/*******************************************************
 * Constructor test
 */

TEST(constructor, testSuccess)
{
  csa_mdp::MultiAgentLineApproach multi_approach;
}

TEST(constructor, testRobotSpeed)
{
  std::default_random_engine engine;
  Eigen::VectorXd state, action;
  Problem::Result r;
  MultiAgentLineApproach multi_approach;
  // Disabling noise, choosing dt and ensuring limits are not a problem
  Json::Value no_noise_model;

  no_noise_model["max_robot_speed"] = 0.5;
  no_noise_model["robot_shoot_distance"] = 1;  // [deg/s]
  no_noise_model["walking_noise"] = 0;

  no_noise_model["shooting_noise"] = 0;
  no_noise_model["dt"] = 0.5;
  multi_approach.fromJson(no_noise_model, "");

  // Simple test: stay
  state = Eigen::VectorXd::Zero(4);
  state(0) = 5;  // Ball 1[m] backward
  state(1) = 1;
  state(2) = 3;
  state(3) = 7;

  action = Eigen::VectorXd::Zero(3);

  r = multi_approach.getSuccessor(state, action, &engine);

  EXPECT_NEAR(5, r.successor(0), EPSILON);
  EXPECT_NEAR(1, r.successor(1), EPSILON);
  EXPECT_NEAR(3, r.successor(2), EPSILON);
  EXPECT_NEAR(7, r.successor(3), EPSILON);

  action(0) = 0.2;
  action(1) = 0.5;
  action(2) = -0.4;
  r = multi_approach.getSuccessor(state, action, &engine);
  EXPECT_NEAR(5, r.successor(0), EPSILON);
  EXPECT_NEAR(1.2, r.successor(1), EPSILON);
  EXPECT_NEAR(3.5, r.successor(2), EPSILON);
  EXPECT_NEAR(6.6, r.successor(3), EPSILON);
}
TEST(constructor, testRobotKick)
{
  std::default_random_engine engine;
  Eigen::VectorXd state, action;
  Problem::Result r;
  MultiAgentLineApproach multi_approach;
  // Disabling noise, choosing dt and ensuring limits are not a problem
  Json::Value no_noise_model;

  no_noise_model["max_robot_speed"] = 0.5;
  no_noise_model["robot_shoot_distance"] = 1;  // [deg/s]
  no_noise_model["walking_noise"] = 0;

  no_noise_model["shooting_noise"] = 0;
  no_noise_model["dt"] = 0.5;
  multi_approach.fromJson(no_noise_model, "");

  state = Eigen::VectorXd::Zero(4);
  state(0) = 2;  // Ball 1[m] backward
  state(1) = 2;
  state(2) = 4.1;
  state(3) = 4.9;

  action = Eigen::VectorXd::Zero(3);
  r = multi_approach.getSuccessor(state, action, &engine);
  EXPECT_NEAR(3, r.successor(0), EPSILON);

  state = Eigen::VectorXd::Zero(4);
  state(0) = 2;  // Ball 1[m] backward
  state(1) = 2;

  action = Eigen::VectorXd::Zero(3);
  action(1) = 0.2;
  r = multi_approach.getSuccessor(state, action, &engine);
  EXPECT_NEAR(3, r.successor(0), EPSILON);
  state(0) = 2;  // Ball 1[m] backward
  state(1) = 2;
  state(2) = 2.9;
  state(3) = 4.1;

  r = multi_approach.getSuccessor(state, action, &engine);
  EXPECT_NEAR(5, r.successor(0), EPSILON);
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
