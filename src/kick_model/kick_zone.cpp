#include "kick_model/kick_zone.h"

using namespace rosban_utils;
using namespace xml_tools;

static double deg2rad(double deg) { return M_PI * deg / 180; }
static double rad2deg(double rad) { return 180 * rad / M_PI; }


namespace csa_mdp
{

/**
 * Return the given angle in radian 
 * bounded between -PI and PI
 */
static double normalizeAngle(double angle)
{
  return angle - 2.0*M_PI*std::floor((angle + M_PI)/(2.0*M_PI));
}

KickZone::KickZone()
  : kick_x_min(0.12),
    kick_x_max(0.22),
    kick_y_tol(0.04),
    kick_y_offset(0.08),
    kick_theta_offset(0),
    kick_theta_tol(10 * M_PI/180)
{
}

Eigen::Vector3d KickZone::getWishedPos(bool right_foot) const
{
  Eigen::Vector3d result;
  result(0) = (kick_x_min + kick_x_max) / 2;
  result(1) = right_foot ? -kick_y_offset : kick_y_offset;
  result(2) = right_foot ? -kick_theta_offset : kick_theta_offset;
  return result;
}

double KickZone::getXRange() const
{
  return kick_x_max - kick_x_min;
}

double KickZone::getYRange() const
{
  return kick_y_tol;
}

bool KickZone::isKickable(const Eigen::Vector3d & state) const
{
  return canKickLeftFoot(state) || canKickRightFoot(state);
}

bool KickZone::isKickable(const Eigen::Vector2d & ball_pos,
                          const Eigen::Vector3d & player_state,
                          double kick_dir) const
{
  return isKickable(convertWorldStateToKickState(ball_pos, player_state, kick_dir));
}


bool KickZone::canKick(bool right_foot, const Eigen::Vector3d & state) const
{
  if (right_foot) return canKickRightFoot(state);
  return canKickLeftFoot(state);
}

bool KickZone::canKickLeftFoot(const Eigen::Vector3d & state) const
{
  // Getting explicit names
  double ball_x = state(0);
  double ball_y = state(1);
  double theta  = state(2);
  double kick_err = normalizeAngle(theta + kick_theta_offset);
  // Check validity
  bool x_ok = ball_x > kick_x_min && ball_x < kick_x_max;
  bool y_ok = std::fabs(ball_y - kick_y_offset) < kick_y_tol;
  bool theta_ok = std::fabs(kick_err) < kick_theta_tol;
  bool kick_ok = x_ok && y_ok && theta_ok;
  return kick_ok;
}

bool KickZone::canKickRightFoot(const Eigen::Vector3d & state) const
{
  // Getting explicit names
  double ball_x = state(0);
  double ball_y = state(1);
  double theta  = state(2);
  double kick_err = normalizeAngle(theta - kick_theta_offset);
  // Check validity
  bool x_ok = ball_x > kick_x_min && ball_x < kick_x_max;
  bool y_ok = std::fabs(ball_y + kick_y_offset) < kick_y_tol;
  bool theta_ok = std::fabs(kick_err) < kick_theta_tol;
  bool kick_ok = x_ok && y_ok && theta_ok;
  return kick_ok;
}

Eigen::Vector3d KickZone::convertWorldStateToKickState(
  const Eigen::Vector2d & ball_pos,
  const Eigen::Vector3d & player_state,
  double kick_dir) const
{
  double dx = ball_pos(0) - player_state(0);
  double dy = ball_pos(1) - player_state(1);
  double player_dir = player_state(2);
  Eigen::Vector3d state_in_self;
  state_in_self(0) =   dx * cos(player_dir) + dy * sin(player_dir);
  state_in_self(1) = - dx * sin(player_dir) + dy * cos(player_dir);
  state_in_self(2) = kick_dir - player_dir;
  return state_in_self;
}

void KickZone::to_xml(std::ostream & out) const
{
  xml_tools::write<double>("kick_x_min"       , kick_x_min       , out);
  xml_tools::write<double>("kick_x_max"       , kick_x_max       , out);
  xml_tools::write<double>("kick_y_tol"       , kick_y_tol       , out);
  xml_tools::write<double>("kick_y_offset"    , kick_y_offset    , out);
  // For angles: write human values
  double kick_theta_tol_deg    = rad2deg(kick_theta_tol   );
  double kick_theta_offset_deg = rad2deg(kick_theta_offset);
  xml_tools::write<double>("kick_theta_tol"   , kick_theta_tol_deg   , out);
  xml_tools::write<double>("kick_theta_offset", kick_theta_offset_deg, out);
}

void KickZone::from_xml(TiXmlNode * node)
{
  try_read<double>(node, "kick_x_min"       , kick_x_min       );
  try_read<double>(node, "kick_x_max"       , kick_x_max       );
  try_read<double>(node, "kick_y_tol"       , kick_y_tol       );
  try_read<double>(node, "kick_y_offset"    , kick_y_offset    );
  // For angles: read human values
  double kick_theta_tol_deg    = xml_tools::read<double>(node, "kick_theta_tol");
  double kick_theta_offset_deg = xml_tools::read<double>(node, "kick_theta_offset");
  kick_theta_tol    = deg2rad(kick_theta_tol_deg   );
  kick_theta_offset = deg2rad(kick_theta_offset_deg);
}

std::string KickZone::class_name() const
{
  return "KickZone";
}


}