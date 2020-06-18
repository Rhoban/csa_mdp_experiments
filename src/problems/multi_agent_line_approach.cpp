#include "problems/multi_agent_line_approach.h"

namespace csa_mdp
{
MultiAgentLineApproach::MultiAgentLineApproach()
  :  // field limits
  field_limit_min(0)
  , field_limit_max(10)
  , nb_static_element(1)
  // robot limits
  , nb_robots(5)
  , nb_robots_policy(3)
  , robot_size(0.2)
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
  updateLimits();
}

std::vector<int> MultiAgentLineApproach::getLearningDimensions() const
{
  std::vector<int> dim;
  for (int i = 0; i <= nb_static_element + nb_robots; i++)
  {
    dim.push_back(i);
  }
  return dim;
}

std::pair<Eigen::VectorXd, Eigen::MatrixXd>
MultiAgentLineApproach::splitMultiAgentState(const Eigen::VectorXd& exhaustive_state) const
{
  // agent sorted
  Eigen::VectorXd world = exhaustive_state.segment(0, nb_static_element);
  int robot_dim = (exhaustive_state.size() - nb_static_element) / nb_robots;
  Eigen::MatrixXd agents(nb_robots, robot_dim);
  for (int i = 0; i < nb_robots; i++)
  {
    agents << exhaustive_state.segment(nb_static_element + robot_dim * i, robot_dim);
  }
  return std::make_pair(world, agents);
}

void MultiAgentLineApproach::updateLimits()
{
  Eigen::MatrixXd state_limits(nb_static_element + nb_robots, 2), action_limits(nb_robots, 2);
  std::vector<std::string> state_names, action_names;
  // update static element -> only in state
  for (int i = 0; i <= nb_static_element; i++)
  {
    state_limits << field_limit_min, field_limit_max;
  }
  state_names.push_back("ball");
  // update agents -> state + action
  for (int i = 0; i <= nb_robots; i++)
  {
    state_limits << field_limit_min, field_limit_max;
    action_limits << -max_robot_speed, max_robot_speed;
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
  Eigen::VectorXd successor(nb_robots + nb_static_element);
  successor = state;

  for (int i = 0; i < nb_robots; i++)
  {
    std::uniform_real_distribution<double> step(action(i) * (1 - walking_noise), action(i) * (1 + walking_noise));

    successor(i + nb_static_element) += step(*engine);
  }

  // 2. move ball
  std::uniform_real_distribution<double> shoot(robot_shoot_distance * (1 - shooting_noise),
                                               robot_shoot_distance * (1 + shooting_noise));
  while (isRobotColliding(state.segment(nb_static_element, nb_robots), state(0)))
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
  std::uniform_real_distribution<double> pos(field_limit_min, field_limit_max);

  Eigen::VectorXd state = Eigen::VectorXd::Zero(nb_static_element + nb_robots);

  // update state with random value on field
  for (int i = 0; i < nb_static_element + nb_robots; i++)
  {
    // TODO : add non collision for begining ?
    double tmp_pos = pos(*engine);
    while (isRobotColliding(state.segment(nb_static_element, i), tmp_pos))
    {
      tmp_pos = pos(*engine);
    }
    state(i) = tmp_pos;
  }

  return state;
}

bool MultiAgentLineApproach::isRobotColliding(const Eigen::VectorXd state, const double pos) const
{
  for (int i = 0; state.size(); i++)
  {
    if (fabs(state(nb_static_element + i) - pos) < robot_size)
      return true;
  }
  return false;
}

bool MultiAgentLineApproach::isSuccess(const Eigen::VectorXd& state) const
{
  return state(0) > field_limit_max;
}

bool MultiAgentLineApproach::isOutOfSpace(const Eigen::VectorXd& state) const
{
  for (int i = 0; i < nb_robots; i++)
  {
    if (state(nb_static_element + i) > field_limit_max || state(nb_static_element + i) < field_limit_min)
      return true;
  }

  return false;
}

bool MultiAgentLineApproach::isColliding(const Eigen::VectorXd& state) const
{
  for (int i = 0; i < nb_robots; i++)
  {
    for (int j = i + 1; j < nb_robots; j++)
    {
      if (fabs(state(nb_static_element + i) - state(nb_static_element + j)) < robot_size)
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
  (void)dir_name;
  // Read internal properties
  rhoban_utils::tryRead(v, "field_limit_min", &field_limit_min);
  rhoban_utils::tryRead(v, "field_limit_max", &field_limit_max);
  rhoban_utils::tryRead(v, "max_robot_speed", &max_robot_speed);
  rhoban_utils::tryRead(v, "nb_robots", &nb_robots);
  rhoban_utils::tryRead(v, "nb_robots_policy", &nb_robots_policy);
  rhoban_utils::tryRead(v, "robot_size", &robot_size);
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
