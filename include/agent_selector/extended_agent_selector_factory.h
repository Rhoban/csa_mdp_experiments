#pragma once

#include "rhoban_csa_mdp/core/agent_selector_factory.h"

namespace csa_mdp
{
class ExtendedAgentSelectorFactory : public csa_mdp::AgentSelectorFactory
{
public:
  ExtendedAgentSelectorFactory();

  /// Needs to be called to allow using additionnal Problems
  static void registerExtraAgentSelector();
};

}  // namespace csa_mdp
