#include "kick_model/kick_model_collection.h"

#include "kick_model/kick_model_factory.h"

using namespace rhoban_utils::xml_tools;

namespace csa_mdp
{

KickModelCollection::KickModelCollection() {}

const KickModel & KickModelCollection::getKickModel(const std::string & name) const
{
  try {
    return *(models.at(name));
  }
  catch (const std::out_of_range & exc) {
    throw std::logic_error("Cannot find '" + name + "' in KickModelCollection");
  }
}

std::vector<std::string> KickModelCollection::getKickNames() const
{
  std::vector<std::string> names;
  for (const auto & entry : models) {
    names.push_back(entry.first);
  }
  return names;
}

void KickModelCollection::toJson(std::ostream & out) const
{
  (void) out;
  throw std::logic_error("KickModelCollection::toJson: not implemented");
}

void KickModelCollection::fromJson(TiXmlNode * node)
{
  KickModelFactory kmf;
  models = read_map<std::unique_ptr<KickModel>>(node, "map", 
                                                [&kmf](TiXmlNode * node) {
                                                  return kmf.build(node);
                                                });

  grassModel.read(node, "grassModel");
  for (auto & entry : models) {
    entry.second->setGrassModel(grassModel);
  }
}

void KickModelCollection::setGrassConeOffset(double offset)
{
  grassModel.setConeOffset(offset);
  for (auto & entry : models) {
    entry.second->setGrassModel(grassModel);
  }
}

std::string KickModelCollection::getClassName() const
{
  return "KickModelCollection";
}

}
