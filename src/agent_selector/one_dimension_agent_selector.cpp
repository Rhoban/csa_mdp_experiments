#include "agent_selector/one_dimension_agent_selector.h"

namespace csa_mdp
{
OneDimensionAgentSelector::OneDimensionAgentSelector() : score_main_agent(20)
{
}

double OneDimensionAgentSelector::getDist(const Eigen::VectorXd agent_1, const Eigen::VectorXd agent_2)
{
  // rewrite for only one dimension agent
  if (agent_1.size() != agent_2.size())
  {
    std::ostringstream oss;
    oss << "AgentSelector::getDist: agent_1.size() != agent_2.size(), " << agent_1.size() << " != " << agent_2.size();
    throw std::runtime_error(oss.str());
  }
  else
  {
    return fabs(agent_1(0) - agent_2(0));
  }
}

Eigen::VectorXd OneDimensionAgentSelector::getRevelantAgents(const Eigen::VectorXd world, const Eigen::MatrixXd agents,
                                                             const int main_agent)
{
  Eigen::VectorXd agent_state(nb_selected_agents);
  Eigen::MatrixXd agents_to_score = removeMainAgent(agents, main_agent);
  // set Points of interests
  int nb_pOI = world.size() + 1;
  Eigen::VectorXd pOI(nb_pOI), score_pOI(nb_pOI);

  pOI << agents.row(main_agent);
  score_pOI << score_main_agent;
  for (int i = 1; i < nb_pOI; i++)
  {
    pOI << world(i - 1);
    score_pOI << score_pOI(i - 1) / 2;
  }

  std::vector<std::pair<double, double>> score_agents = scoringAgent(agents_to_score, pOI, score_pOI);
  // select the closests agents

  for (int i = 0; i < nb_selected_agents; i++)
    agent_state << score_agents.at(i).second;
  return agent_state;
}

std::vector<std::pair<double, double>> OneDimensionAgentSelector::scoringAgent(const Eigen::MatrixXd agents,
                                                                               const Eigen::VectorXd pOI,
                                                                               const Eigen::VectorXd score_pOI)
{
  ///  /!\ missing sorting agent

  // init validate/score
  std::vector<bool> validatePOI;
  for (int i = 0; i < pOI.size(); i++)
  {
    validatePOI.push_back(false);
  }
  std::vector<std::pair<double, double>> score_agents;

  // On one dim, agents is size m X 1
  for (int r = 0; r < agents.rows(); r++)
  {
    double score = 0.0;
    // for each point of interest calculate distance with agent selected
    for (int p = 0; p < pOI.size(); p++)
    {
      if (!validatePOI.at(p) && agents(r, 0) > pOI(p))
      {
        validatePOI.at(p) = true;
        // add bonus if it's closest
        score -= score_pOI(p);
        if (r != 0)
        {
          score_agents.back().first += score_pOI(p);
        }
      }
      Eigen::VectorXd agent(1);
      agent << agents.row(r);

      score += getDist(pOI.segment(p, 1), agent);
    }
    score_agents.push_back(std::make_pair(score, agents(r, 0)));
  }
  for (int p = 0; p < pOI.size(); p++)
  {
    if (!validatePOI.at(p))
    {
      score_agents.back().first -= score_pOI(p);
    }
  }
  std::sort(score_agents.begin(), score_agents.end());
  return score_agents;
}

Json::Value OneDimensionAgentSelector::toJson() const
{
  throw std::logic_error("OneDimensionAgentSelector::toJson: not implemented");
}

void OneDimensionAgentSelector::fromJson(const Json::Value& v, const std::string& dir_name)
{
  AgentSelector::fromJson(v, dir_name);
  // TODO: read Ids
  rhoban_utils::tryRead(v, "score_main_agent", &score_main_agent);
}
std::string OneDimensionAgentSelector::getClassName() const
{
  return "OneDimensionAgentSelector";
}
}  // namespace csa_mdp
