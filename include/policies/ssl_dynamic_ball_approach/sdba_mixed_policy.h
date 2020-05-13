#pragma once

#include "rhoban_csa_mdp/core/policy.h"
#include "problems/ssl_dynamic_ball_approach.h"

namespace csa_mdp
{
/// A state-less policy for the SSL Dynamic Ball Approach problem based on applying one of two policies
/// - Far policy: while robot has not properly aligned with the ball
/// - Finish policy: Once robot has a state considered as
class SDBAMixedPolicy : public csa_mdp::Policy
{
public:
  SDBAMixedPolicy();
  ~SDBAMixedPolicy();

  /// Get raw action but do not update memory_state
  Eigen::VectorXd getRawAction(const Eigen::VectorXd& state,
                               std::default_random_engine* external_engine) const override;

  std::string getClassName() const override;
  Json::Value toJson() const override;
  void fromJson(const Json::Value& v, const std::string& dir_name) override;

private:
  std::unique_ptr<csa_mdp::Policy> far_policy;
  std::unique_ptr<csa_mdp::Policy> finish_policy;

  std::unique_ptr<csa_mdp::SSLDynamicBallApproach> problem;
};

}  // namespace csa_mdp
