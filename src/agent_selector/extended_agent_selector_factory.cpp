#include "agent_selector/extended_agent_selector_factory.h"

#include "agent_selector/one_dimension_agent_selector.h"

namespace csa_mdp
{
ExtendedAgentSelectorFactory::ExtendedAgentSelectorFactory()
{
}

void ExtendedAgentSelectorFactory::registerExtraAgentSelector()
{
  // Allowing multiple calls without issues
  static bool performed = false;
  if (performed)
    return;
  performed = true;
}
}  // namespace csa_mdp
