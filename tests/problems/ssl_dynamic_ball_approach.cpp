#include <gtest/gtest.h>
#include <problems/ssl_dynamic_ball_approach.h>

#define _USE_MATH_DEFINES
#include <cmath>

#define EPSILON std::pow(10, -6)

using namespace csa_mdp;

/*******************************************************
 * Constructor test
 */

TEST(constructor, testSuccess)
{
  csa_mdp::SSLDynamicBallApproach ball_approach;
}

TEST(constructor, testRobotSpeed)
{
  std::default_random_engine engine;
  Eigen::VectorXd state, action;
  Problem::Result r;
  SSLDynamicBallApproach ball_approach;
  // Disabling noise, choosing dt and ensuring limits are not a problem
  Json::Value no_noise_model;
  no_noise_model["max_robot_speed"] = 1;
  no_noise_model["max_robot_speed_theta"] = 360;  // [deg/s]
  no_noise_model["cart_stddev"] = 0;
  no_noise_model["angular_stddev"] = 0;
  no_noise_model["dt"] = 0.5;
  ball_approach.fromJson(no_noise_model, "");
  // Simple test: positive rotation (no speed for ball)
  state = Eigen::VectorXd::Zero(11);
  state(0) = -1;    // Ball 1[m] backward
  state(2) = 1;     // Target 1[m] backward
  state(6) = M_PI;  // Speed: pi/2 [rad/s]
  action = Eigen::VectorXd::Zero(4);
  r = ball_approach.getSuccessor(state, action, &engine);
  EXPECT_NEAR(0, r.successor(0), EPSILON);
  EXPECT_NEAR(1, r.successor(1), EPSILON);
  EXPECT_NEAR(0, r.successor(2), EPSILON);
  EXPECT_NEAR(-1, r.successor(3), EPSILON);
  EXPECT_NEAR(M_PI, r.successor(6), EPSILON);
  EXPECT_NEAR(M_PI / 2, r.successor(10), EPSILON);
  // Simple test: pure translation along y-axis
  state = Eigen::VectorXd::Zero(11);
  state(0) = -1;  // Ball 1[m] backward
  state(2) = 1;   // Target 1[m] backward
  state(5) = 1;   // Speed: 1[m/s] left
  action = Eigen::VectorXd::Zero(4);
  r = ball_approach.getSuccessor(state, action, &engine);
  EXPECT_NEAR(-1, r.successor(0), EPSILON);
  EXPECT_NEAR(-0.5, r.successor(1), EPSILON);
  EXPECT_NEAR(1, r.successor(2), EPSILON);
  EXPECT_NEAR(-0.5, r.successor(3), EPSILON);
  EXPECT_NEAR(0, r.successor(6), EPSILON);
  EXPECT_NEAR(0, r.successor(10), EPSILON);
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
