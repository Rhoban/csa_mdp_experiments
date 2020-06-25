#pragma once

#include "rhoban_csa_mdp/core/policy.h"
#include "rhoban_csa_mdp/core/agent_selector.h"
#include <memory>

namespace csa_mdp
{
class LineExpert : public csa_mdp::Policy
{
public:
  LineExpert();

  void init() override;

  /// Get raw action and update memory_state
  Eigen::VectorXd getRawAction(const Eigen::VectorXd& state) override;
  Eigen::VectorXd getSingleRawAction(const Eigen::VectorXd& state) const;

  /// Get raw action but do not update memory_state
  Eigen::VectorXd getRawAction(const Eigen::VectorXd& state,
                               std::default_random_engine* external_engine) const override;
  bool agentBetween(double first_pos, double second_pos, const Eigen::VectorXd& agents) const;

  Eigen::VectorXd goTo(double main, double target) const;

  Eigen::VectorXd canGo(double main, double target, const Eigen::VectorXd& agents) const;

  void fromJson(const Json::Value& v, const std::string& dir_name) override;
  std::string getClassName() const override;

private:
  // STATE LIMITS
  /// Field limits
  double field_limit_min, field_limit_max;

  // ACTION_LIMITS
  /// The maximal cartesian speed [m/s]
  double max_robot_speed;

  /// kick
  double robot_shoot_distance;

  double robot_size;

  std::shared_ptr<const AgentSelector> as;
};
}  // namespace csa_mdp
