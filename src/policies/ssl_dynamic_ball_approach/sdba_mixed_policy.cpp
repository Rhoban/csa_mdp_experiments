#include "policies/ssl_dynamic_ball_approach/sdba_mixed_policy.h"

#include "rhoban_csa_mdp/core/policy_factory.h"
#include "rhoban_csa_mdp/core/problem_factory.h"

using namespace rhoban_fa;

namespace csa_mdp
{
SDBAMixedPolicy::SDBAMixedPolicy()
{
}

SDBAMixedPolicy::~SDBAMixedPolicy()
{
}

Eigen::VectorXd SDBAMixedPolicy::getRawAction(const Eigen::VectorXd& state,
                                              std::default_random_engine* external_engine) const
{
  if (problem->isFinishState(state))
  {
    return finish_policy->getRawAction(state, external_engine);
  }
  else
  {
    return far_policy->getRawAction(state, external_engine);
  }
}

std::string SDBAMixedPolicy::getClassName() const
{
  return "SDBAMixedPolicy";
}

Json::Value SDBAMixedPolicy::toJson() const
{
  Json::Value v = Policy::toJson();
  v["problem"] = problem->toFactoryJson();
  v["far_policy"] = far_policy->toFactoryJson();
  v["finish_policy"] = finish_policy->toFactoryJson();
  return v;
}

void SDBAMixedPolicy::fromJson(const Json::Value& v, const std::string& dir_name)
{
  ProblemFactory problem_factory;
  std::unique_ptr<csa_mdp::Problem> tmp_problem = problem_factory.build(v["problem"], dir_name);
  if (dynamic_cast<SSLDynamicBallApproach*>(tmp_problem.get()) == nullptr)
  {
    throw rhoban_utils::JsonParsingError("Expecting 'problem' of type SSLDynamicApproachProblem");
  }
  problem.reset(dynamic_cast<SSLDynamicBallApproach*>(tmp_problem.release()));

  PolicyFactory policy_factory;
  far_policy = policy_factory.build(v["far_policy"], dir_name);
  finish_policy = policy_factory.build(v["finish_policy"], dir_name);
}

}  // namespace csa_mdp
