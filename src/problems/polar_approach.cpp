#include "problems/polar_approach.h"

#include "rosban_utils/xml_tools.h"

#include <cmath>

namespace csa_mdp
{

// State limits
double PolarApproach::min_step_x     = -0.02;
double PolarApproach::max_step_x     =  0.04;
double PolarApproach::max_step_y     =  0.02;
double PolarApproach::max_step_theta =  0.3 ;
// Action limits
double PolarApproach::max_step_x_diff     = 0.02;
double PolarApproach::max_step_y_diff     = 0.01;
double PolarApproach::max_step_theta_diff = 0.15;
// Step noise
double PolarApproach::step_x_noise     = 0.02;
double PolarApproach::step_y_noise     = 0.02;
double PolarApproach::step_theta_noise = 5 * M_PI / 180;
// Kick
double PolarApproach::kick_x_min     = 0.15   ;
double PolarApproach::kick_x_max     = 0.25   ;
double PolarApproach::kick_y_tol     = 0.06   ;
double PolarApproach::kick_theta_tol = 10 * M_PI/180;
double PolarApproach::kick_reward    = 0;
// Viewing the ball
double PolarApproach::viewing_angle  = 2*M_PI/3;
double PolarApproach::no_view_reward = 0       ;
// Collision
double PolarApproach::collision_x_front =  0.15;
double PolarApproach::collision_x_back  =  0.20;
double PolarApproach::collision_y       =  0.25;
double PolarApproach::collision_reward  = -3;
// Misc
double PolarApproach::out_of_space_reward = -100;
double PolarApproach::step_reward         = -1;
double PolarApproach::init_min_dist = 0.4;
double PolarApproach::init_max_dist = 0.95;

/**
 * Return the given angle in radian 
 * bounded between -PI and PI
 */
static double normalizeAngle(double angle)
{
  return angle - 2.0*M_PI*std::floor(
    (angle + M_PI)/(2.0*M_PI));
}

PolarApproach::PolarApproach()
  : max_dist(1.0)
{
  updateLimits();

  setStateNames({"ball_dist", "ball_dir", "target_angle", "step_x", "step_y", "step_theta"});
  setActionNames({"d_step_x","d_step_y","d_step_theta"});
}

void PolarApproach::updateLimits()
{
  Eigen::MatrixXd state_limits(6,2), action_limits(3,2);
  state_limits <<
    0, max_dist,
    -M_PI, M_PI,
    -M_PI, M_PI,
    min_step_x, max_step_x,
    -max_step_y, max_step_y,
    -max_step_theta, max_step_theta;
  action_limits <<
    -max_step_x_diff, max_step_x_diff,
    -max_step_y_diff, max_step_y_diff,
    -max_step_theta_diff, max_step_theta_diff;
  setStateLimits(state_limits);
  setActionLimits(action_limits);
}

void PolarApproach::setMaxDist(double dist)
{
  max_dist = dist;
  updateLimits();
}
  
void PolarApproach::setOdometry(const Eigen::MatrixXd& model)
{
  if (model.rows() != 3 && model.cols() != 4) {
    throw std::logic_error(
      "PolarApproach::setOdometry Invalid format");
  }
  odometry_coefficients = model;
}

bool PolarApproach::isTerminal(const Eigen::VectorXd & state) const
{
  return isOutOfSpace(state);
}

double  PolarApproach::getReward(const Eigen::VectorXd & state,
                                 const Eigen::VectorXd & action,
                                 const Eigen::VectorXd & dst) const
{
  (void)state;(void)action;
  if (isKickable(dst)  ) return kick_reward;
  if (isColliding(dst) ) return collision_reward;
  if (isOutOfSpace(dst)) return out_of_space_reward;
  double reward = step_reward;
  if (!seeBall(dst)    ) reward += no_view_reward;
  return reward;
}

Eigen::VectorXd PolarApproach::getSuccessor(const Eigen::VectorXd & state,
                                            const Eigen::VectorXd & action,
                                            std::default_random_engine * engine) const
{
  /// Initialize noise distributions
  std::uniform_real_distribution<double> step_x_noise_distrib    (-step_x_noise,
                                                                  step_x_noise);
  std::uniform_real_distribution<double> step_y_noise_distrib    (-step_y_noise,
                                                                  step_y_noise);
  std::uniform_real_distribution<double> step_theta_noise_distrib(-step_theta_noise,
                                                                  step_theta_noise);
  // Get the step which will be applied
  Eigen::VectorXd next_cmd(3);
  for (int dim = 0; dim < 3; dim++)
  {
    // Ensuring that acceleration is in the bounds
    const Eigen::MatrixXd & action_limits = getActionLimits();
    double min_acc = action_limits(dim, 0);
    double max_acc = action_limits(dim, 1);
    double bounded_action = std::min(max_acc, std::max(min_acc, action(dim)));
    // Action applies a delta on step
    next_cmd(dim) = bounded_action + state(dim + 3);
    // Ensuring that final action is inside of the bounds
    const Eigen::MatrixXd & limits = getStateLimits();
    double min_cmd = limits(dim + 3, 0);
    double max_cmd = limits(dim + 3, 1);
    next_cmd(dim) = std::min(max_cmd, std::max(min_cmd, next_cmd(dim)));
  }
  // Apply a linear modification (from theory to 'reality')
  Eigen::VectorXd predicted_move = predictMotion(next_cmd);
  // Apply noise to get the real move
  Eigen::VectorXd real_move(3);
  real_move(0) = predicted_move(0) + step_x_noise_distrib(*engine);
  real_move(1) = predicted_move(1) + step_y_noise_distrib(*engine);
  real_move(2) = predicted_move(2) + step_theta_noise_distrib(*engine);
  // Apply the real move
  Eigen::VectorXd next_state = state;
  // Apply rotation first
  double delta_theta = real_move(2);
  next_state(2) = normalizeAngle(state(2) - delta_theta);
  next_state(1) = normalizeAngle(state(1) - delta_theta);
  // Then, apply translation
  double ball_x = getBallX(next_state) - real_move(0);
  double ball_y = getBallY(next_state) - real_move(1);
  double new_dist = std::sqrt(ball_x * ball_x + ball_y * ball_y);
  double new_dir = atan2(ball_y, ball_x);
  next_state(0) = new_dist;
  next_state(1) = new_dir;
  // Update cmd
  next_state.segment(3,3) = next_cmd;
  return next_state;
}

Eigen::VectorXd PolarApproach::getStartingState()
{
  Eigen::VectorXd state = Eigen::VectorXd::Zero(6);
  // Creating the distribution
  std::uniform_real_distribution<double> dist_distrib(init_min_dist, init_max_dist);
  std::uniform_real_distribution<double> angle_distrib(-M_PI, M_PI);
  // Generating random values
  double dist = dist_distrib(random_engine);
  double ball_theta = angle_distrib(random_engine);
  double target_theta = angle_distrib(random_engine);
  // Updating state
  state(0) = dist;
  state(1) = ball_theta;
  state(2) = target_theta;

  return state;
}

bool PolarApproach::isKickable(const Eigen::VectorXd & state) const
{
  double ball_x = getBallX(state);
  double ball_y = getBallY(state);
  double theta  = state(2);
  bool x_ok = ball_x > kick_x_min && ball_x < kick_x_max;
  bool y_ok = std::fabs(ball_y) < kick_y_tol;
  bool theta_ok = - kick_theta_tol < theta && theta < kick_theta_tol;
  return x_ok && y_ok && theta_ok;
}

bool PolarApproach::isColliding(const Eigen::VectorXd & state) const
{
  double ball_x = getBallX(state);
  double ball_y = getBallY(state);
  bool x_ko = ball_x > - collision_x_back && ball_x < collision_x_front;
  bool y_ko = std::fabs(ball_y) < collision_y;
  return x_ko && y_ko;
}

bool PolarApproach::isOutOfSpace(const Eigen::VectorXd & state) const
{
  const Eigen::MatrixXd & space_limits = getStateLimits();
  for (int dim = 0; dim < state.rows(); dim++)
  {
    if (state(dim) < space_limits(dim, 0) || state(dim) > space_limits(dim, 1))
    {
      return true;
    }
  }
  return false;
}

bool PolarApproach::seeBall(const Eigen::VectorXd & state) const
{
  double angle = state(1);
  return angle < viewing_angle && angle > -viewing_angle;
}

void PolarApproach::to_xml(std::ostream & out) const {(void)out;}

void PolarApproach::from_xml(TiXmlNode * node)
{
  std::vector<double> odometry_coefficients_read;
  rosban_utils::xml_tools::try_read_vector<double>(node,
                                                   "odometry_coefficients",
                                                   odometry_coefficients_read);

  // If coefficients have been properly read, use them
  if (odometry_coefficients_read.size() == 12)
  {
    odometry_coefficients = Eigen::Map<Eigen::MatrixXd>(odometry_coefficients_read.data(), 3, 4);
  }
  else if (odometry_coefficients_read.size() != 0)
  {
    std::ostringstream oss;
    oss << "PolarApproach::from_xml: "
        << "invalid number of coefficients for 'odometry_coefficients'. "
        << "read: " << odometry_coefficients_read.size() << " expecting 12.";
    throw std::runtime_error(oss.str());
  }
}

std::string PolarApproach::class_name() const
{
  return "polar_approach";
}

double PolarApproach::getBallX(const Eigen::VectorXd & state)
{
  return cos(state(1)) * state(0);
}

double PolarApproach::getBallY(const Eigen::VectorXd & state)
{
  return sin(state(1)) * state(0);
}

Eigen::VectorXd PolarApproach::predictMotion(const Eigen::VectorXd & walk_orders) const
{
  // In theory, robot makes 2 steps
  Eigen::VectorXd theoric_move = 2 * walk_orders;
  // If no odometry has been loaded, return walk orders
  if (odometry_coefficients.rows() <= 0)
  {
    return theoric_move;
  }

  // [1, walk_x, walk_y, walk_theta]
  Eigen::VectorXd augmented_state(4);
  augmented_state(0) = 1;
  augmented_state.segment(1,3) = theoric_move;
  // Apply odometry on augmented state
  return odometry_coefficients * augmented_state;
}

}
