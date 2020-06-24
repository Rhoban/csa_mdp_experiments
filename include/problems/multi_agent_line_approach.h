#pragma once

#include "rhoban_csa_mdp/core/black_box_problem.h"

#include <Eigen/Core>

#include <functional>
#include <random>
#include <vector>

namespace csa_mdp
{
/// This problem consists in developping a simple exemple of multi-robot cooperation.
/// Define on one single dimension, robots have to kick a ball toward the field.
///
/// The state space is the following (all data are provided in field referential):
/// - state[0] -> position of the ball (x)
/// - state[1:n] -> position of the robots (x)
///
/// Ball and robots must be initialized in the field

class MultiAgentLineApproach : public BlackBoxProblem
{
public:
  MultiAgentLineApproach();

  std::vector<int> getLearningDimensions() const override;

  bool isTerminal(const Eigen::VectorXd& state) const;

  double getReward(const Eigen::VectorXd& state, const Eigen::VectorXd& action, const Eigen::VectorXd& dst) const;

  Problem::Result getSuccessor(const Eigen::VectorXd& state, const Eigen::VectorXd& action,
                               std::default_random_engine* engine) const override;

  Eigen::VectorXd getStartingState(std::default_random_engine* engine) const override;

  /// Is current state a success (target reached)
  bool isSuccess(const Eigen::VectorXd& state) const;

  /// Is the robot colliding with the ball
  bool isColliding(const Eigen::VectorXd& state) const;
  /// Is the ball outside of the given limits
  bool isOutOfSpace(const Eigen::VectorXd& state) const;

  Json::Value toJson() const override;
  void fromJson(const Json::Value& v, const std::string& dir_name) override;
  std::string getClassName() const override;

  /// Ensure that limits are consistent with the parameters
  void updateLimits();
  /// Does the element (pos) is colliding with one in the state
  bool isRobotColliding(const Eigen::VectorXd state, const double pos) const;

protected:
  // STATE LIMITS
  /// Field limits
  double field_limit_min, field_limit_max;

  // ACTION_LIMITS
  /// The maximal cartesian speed [m/s]
  double max_robot_speed;
  double walking_noise;

  /// kick
  double robot_shoot_distance;
  double shooting_noise;

  // REWARDS

  /// The time step for control [s]
  double dt;

  /// The reward received when robots collide
  double collision_reward;

  /// The reward received when getting out of space
  double out_of_space_reward;
};

}  // namespace csa_mdp
