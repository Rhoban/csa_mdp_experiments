#include "problems/multi_agent_line_approach.h"
#include <iostream>

namespace csa_mdp
{
MultiAgentLineApproach::MultiAgentLineApproach()
  :  // field limits
  field_limit_min(0)
  , field_limit_max(10)
  // Walk
  , max_robot_speed(0.2)
  , walking_noise(0.1)
  // Shoot
  , robot_shoot_distance(1)
  , shooting_noise(0.1)
  // Time
  , dt(0.033)
  // Reward
  , collision_reward(-50)
  , out_of_space_reward(-200)
{
  nb_agents = 3;
  nb_static_element = 1;
  agent_size = 0.2;

  updateLimits();
}

std::vector<int> MultiAgentLineApproach::getLearningDimensions() const
{
  std::vector<int> dim;
  for (int i = 0; i <= nb_static_element + nb_agents; i++)
  {
    dim.push_back(i);
  }
  return dim;
}

void MultiAgentLineApproach::updateLimits()
{
  Eigen::MatrixXd state_limits(nb_static_element + nb_agents, 2);
  Eigen::MatrixXd action_limits(nb_agents, 2);

  std::vector<std::string> state_names, action_names;
  // update static element -> only in state
  for (int i = 0; i < nb_static_element; i++)
  {
    state_limits(i, 0) = field_limit_min;
    state_limits(i, 1) = field_limit_max;
  }

  state_names.push_back("ball");
  // update agents -> state + action
  for (int i = 0; i < nb_agents; i++)
  {
    state_limits(i + nb_static_element, 0) = field_limit_min;

    state_limits(i + nb_static_element, 1) = field_limit_max;
    action_limits(i, 0) = -max_robot_speed;
    action_limits(i, 1) = max_robot_speed;

    state_names.push_back("robot_" + std::to_string(i));
    action_names.push_back("speed_robot_" + std::to_string(i));
  }

  setStateLimits(state_limits);
  setActionLimits({ action_limits });
  setStateNames(state_names);
  setActionsNames({ action_names });
}

bool MultiAgentLineApproach::isTerminal(const Eigen::VectorXd& state) const
{
  return isSuccess(state) || isOutOfSpace(state) || isColliding(state);
}

double MultiAgentLineApproach::getReward(const Eigen::VectorXd& state, const Eigen::VectorXd& action,
                                         const Eigen::VectorXd& dst) const
{
  (void)state;
  (void)action;
  if (isSuccess(dst))
  {
    return 0;
  }
  if (isColliding(dst))
  {
    return collision_reward;
  }
  if (isOutOfSpace(dst))
  {
    return out_of_space_reward;
  }
  return -dt;  // Default reward
}

Problem::Result MultiAgentLineApproach::getSuccessor(const Eigen::VectorXd& state, const Eigen::VectorXd& action,
                                                     std::default_random_engine* engine) const
{
  // successor ;
  // 1. move robot
  Eigen::VectorXd successor(nb_agents + nb_static_element);
  successor = state;

  for (int i = 0; i < nb_agents; i++)
  {
    std::uniform_real_distribution<double> step(action(i + 1) * (1 - walking_noise),
                                                action(i + 1) * (1 + walking_noise));

    successor(i + nb_static_element) += step(*engine);
  }

  // 2. move ball
  std::uniform_real_distribution<double> shoot(robot_shoot_distance * (1 - shooting_noise),
                                               robot_shoot_distance * (1 + shooting_noise));
  while (isRobotColliding(state, successor(0)))
  {
    successor(0) += shoot(*engine);
  }
  Problem::Result result;
  result.successor = successor;
  result.reward = getReward(state, action, successor);
  result.terminal = isTerminal(successor);
  return result;
}

Eigen::VectorXd MultiAgentLineApproach::getStartingState(std::default_random_engine* engine) const
{
  // init staring state
  // Creating the distribution
  std::uniform_real_distribution<double> pos(field_limit_min + agent_size, field_limit_max - agent_size);

  Eigen::VectorXd state = Eigen::VectorXd::Zero(nb_static_element + nb_agents);

  // update state with random value on field
  for (int i = 0; i < nb_static_element + nb_agents; i++)
  {
    // TODO : add non collision for begining ?
    double tmp_pos = pos(*engine);
    while (isRobotColliding(state.segment(0, i), tmp_pos, 1))
    {
      tmp_pos = pos(*engine);
    }
    state(i) = tmp_pos;
  }

  return state;
}

bool MultiAgentLineApproach::isRobotColliding(const Eigen::VectorXd state, const double pos, int twoRobots) const
{
  for (int i = 0; i < state.size() - 1; i++)
  {
    if (std::abs(state(nb_static_element + i) - pos) <= agent_size / (2 - twoRobots))
    {
      return true;
    }
  }
  return false;
}

bool MultiAgentLineApproach::isSuccess(const Eigen::VectorXd& state) const
{
  return state(0) > field_limit_max;
}

bool MultiAgentLineApproach::isOutOfSpace(const Eigen::VectorXd& state) const
{
  for (int i = 0; i < nb_agents; i++)
  {
    if (state(nb_static_element + i) > field_limit_max || state(nb_static_element + i) < field_limit_min)
      return true;
  }

  return false;
}

bool MultiAgentLineApproach::isColliding(const Eigen::VectorXd& state) const
{
  for (int i = 0; i < nb_agents; i++)
  {
    for (int j = i + 1; j < nb_agents; j++)
    {
      if (fabs(state(nb_static_element + i) - state(nb_static_element + j)) < agent_size)
        return true;
    }
  }
  return false;
}

Json::Value MultiAgentLineApproach::toJson() const
{
  throw std::logic_error("MultiAgentLineApproach::toJson: not implemented");
}

void MultiAgentLineApproach::fromJson(const Json::Value& v, const std::string& dir_name)
{
  // Calling parent implementation
  Problem::fromJson(v, dir_name);
  // Read internal properties
  rhoban_utils::tryRead(v, "field_limit_min", &field_limit_min);
  rhoban_utils::tryRead(v, "field_limit_max", &field_limit_max);
  rhoban_utils::tryRead(v, "max_robot_speed", &max_robot_speed);
  rhoban_utils::tryRead(v, "walking_noise", &walking_noise);
  rhoban_utils::tryRead(v, "robot_shoot_distance", &robot_shoot_distance);
  rhoban_utils::tryRead(v, "shooting_noise", &shooting_noise);
  rhoban_utils::tryRead(v, "collision_reward", &collision_reward);
  rhoban_utils::tryRead(v, "out_of_space_reward", &out_of_space_reward);
  rhoban_utils::tryRead(v, "dt", &dt);

  // Update limits according to the new parameters
  updateLimits();
}

std::string MultiAgentLineApproach::getClassName() const
{
  return "MultiAgentLineApproach";
}
}  // namespace csa_mdp
