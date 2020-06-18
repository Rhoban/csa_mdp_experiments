#pragma once

#include <Eigen/Core>

#include <functional>
#include <random>
#include <vector>
#include "rhoban_csa_mdp/core/agent_selector.h"

namespace csa_mdp
{
class OneDimensionAgentSelector : public AgentSelector
{
public:
  OneDimensionAgentSelector();

  double getDist(const Eigen::VectorXd agent_1, const Eigen::VectorXd agent_2) override;

  Eigen::MatrixXd getRevelantAgent(const Eigen::VectorXd world, const Eigen::MatrixXd agents, const int main_agent,
                                   int nb_selected_agents) override;

  std::vector<std::pair<double, double>> scoringAgent(const Eigen::MatrixXd agents, const Eigen::VectorXd pOI,
                                                      const Eigen::VectorXd score_pOI);

  Json::Value toJson() const override;
  void fromJson(const Json::Value& v, const std::string& dir_name) override;
  std::string getClassName() const override;

private:
  // scores in parameters
  double score_main_agent;
};

}  // namespace csa_mdp
