#include "policies/line_expert.h"
#include "rhoban_csa_mdp/core/agent_selector_factory.h"

namespace csa_mdp
{
LineExpert::LineExpert()
  :  // field limits
  field_limit_min(0)
  , field_limit_max(10)
  // robot limits
  , robot_size(0.2)
  // Walk
  , max_robot_speed(0.2)
  // Shoot
  , robot_shoot_distance(1)
{
}

void LineExpert::init()
{
}

Eigen::VectorXd LineExpert::getRawAction(const Eigen::VectorXd& state)
{
  return getRawAction(state, NULL);
}

Eigen::VectorXd LineExpert::getSingleRawAction(const Eigen::VectorXd& state) const
{
  double ball = state(0);
  double main_agent = state(1);
  Eigen::VectorXd agents = state.segment(2, state.size() - 2);

  // robot behind ball
  if (main_agent - robot_size / 2 < ball)
  {
    if (!agentBetween(main_agent, ball, agents))
      // robot can go to the ball
      return goTo(main_agent, ball);
    else
      // there is another agent between him and the ball, do not move
      return canGo(main_agent, field_limit_min, agents);
  }
  else
  {
    double reception = ball + robot_shoot_distance;

    if (main_agent - robot_size / 2 < reception)
    {
      if (!agentBetween(field_limit_min, ball, agents))
      {
        if (!agentBetween(ball, main_agent, agents))
          return goTo(main_agent, ball);
      }
      return canGo(main_agent, reception, agents);
    }
    else
    {
      if (!agentBetween(reception, main_agent, agents))
        return goTo(main_agent, reception);
      else
        // there is another agent between him and the ball, do not move
        return canGo(main_agent, field_limit_max, agents);
    }
  }
}

Eigen::VectorXd LineExpert::getRawAction(const Eigen::VectorXd& state,
                                         std::default_random_engine* external_engine) const
{
  (void)external_engine;

  std::vector<Eigen::VectorXd> actions;
  for (int i = 0; i < as->getNbAgents(); i++)
  {
    Eigen::VectorXd relevant_state = as->getRelevantState(state, i);
    actions.push_back(getSingleRawAction(relevant_state));
  }

  return this->as->mergeActions(actions);
}

bool LineExpert::agentBetween(double first_pos, double second_pos, const Eigen::VectorXd& agents) const
{
  for (int i = 0; i < agents.size(); i++)
  {
    if (agents(i) - robot_size / 2 < first_pos)
      if (agents(i) + robot_size / 2 > second_pos)
        return true;
  }
  return false;
}

Eigen::VectorXd LineExpert::goTo(double main, double target) const
{
  double dist = std::max(-max_robot_speed, std::min(max_robot_speed, target - main));
  Eigen::VectorXd action(1);
  action << dist;
  return action;
}

Eigen::VectorXd LineExpert::canGo(double main, double target, const Eigen::VectorXd& agents) const
{
  if (main < target)
  {
    if (agentBetween(main, target, agents))
      return goTo(main, target);
  }
  else
  {
    if (agentBetween(target, main, agents))
      return goTo(main, target);
  }

  // maybe get closer to the agents?
  Eigen::VectorXd action(1);
  action << 0.0;
  return action;
}

void LineExpert::fromJson(const Json::Value& v, const std::string& dir_name)
{
  (void)dir_name;
  as = AgentSelectorFactory().read(v, "agent_selector", dir_name);
  rhoban_utils::tryRead(v, "field_limit_min", &field_limit_min);
  rhoban_utils::tryRead(v, "field_limit_max", &field_limit_max);
  rhoban_utils::tryRead(v, "max_robot_speed", &max_robot_speed);
  rhoban_utils::tryRead(v, "robot_shoot_distance", &robot_shoot_distance);
  rhoban_utils::tryRead(v, "robot_size", &robot_size);
}
std::string LineExpert::getClassName() const
{
  return "LineExpert";
}

}  // namespace csa_mdp
