#include "kick_model/kick_model.h"

namespace csa_mdp
{

KickModel::KickModel()
  : kick_reward(-10)
{
}

const KickZone & KickModel::getKickZone() const
{
  return kick_zone;
}


const Eigen::MatrixXd & KickModel::getParametersLimits() const
{
  return parameters_limits;
}

const std::vector<std::string> & KickModel::getParametersNames() const
{
  return parameters_names;
}


const Eigen::VectorXd & KickModel::getDefaultParameters() const
{
  return default_parameters;
}

double KickModel::getReward() const
{
  return kick_reward;
}

Eigen::Vector2d KickModel::applyKick(const Eigen::Vector2d & ball_pos,
                                     double kick_dir,
                                     std::default_random_engine * engine)const
{
  return applyKick(ball_pos, kick_dir, getDefaultParameters(), engine);
}

void KickModel::setGrassModel(GrassModel grassModel_)
{
  grassModel = grassModel_;
}

void KickModel::toJson(std::ostream & out) const
{
  out << "<kick_zone>";
  kick_zone.toJson(out);
  out << "</kick_zone>";
  rhoban_utils::xml_tools::write<double>("kick_reward", kick_reward, out);
}

void KickModel::fromJson(TiXmlNode * node)
{
  kick_zone.read(node, "kick_zone");
  rhoban_utils::xml_tools::try_read<double>(node, "kick_reward", kick_reward);  
}

}
