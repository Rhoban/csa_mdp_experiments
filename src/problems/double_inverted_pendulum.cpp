#include "problems/double_inverted_pendulum.h"

std::vector<double> DoubleInvertedPendulum::max_torque = {5,1};
std::vector<double> DoubleInvertedPendulum::max_axis_vel = {10,30};
double DoubleInvertedPendulum::start_pos_tol = M_PI / 180;// 1 deg of tol
double DoubleInvertedPendulum::start_vel_tol = M_PI / 180;// 1 deg/s of tol

DoubleInvertedPendulum::DoubleInvertedPendulum()
{
  Eigen::MatrixXd state_limits(4,2), action_limits(2,2);
  for (int axis = 0; axis < 2; axis++)
  {
    state_limits(2 * axis, 0) = -M_PI;
    state_limits(2 * axis, 1) =  M_PI;
    state_limits(2 * axis + 1, 0) = -max_axis_vel[axis];
    state_limits(2 * axis + 1, 1) =  max_axis_vel[axis];
    action_limits(axis,0) = -max_torque[axis];
    action_limits(axis,1) =  max_torque[axis];
  }
  setStateLimits(state_limits);
  setActionLimits(action_limits);
}

bool DoubleInvertedPendulum::isTerminal(const Eigen::VectorXd & state) const
{
  if (std::fabs(state(1)) > max_axis_vel[0] ||
      std::fabs(state(3)) > max_axis_vel[1])
    return true;
  return false;
}

double DoubleInvertedPendulum::getReward(const Eigen::VectorXd &state,
                                         const Eigen::VectorXd &action,
                                         const Eigen::VectorXd &dst)
{
  (void) action;
  if (isTerminal(dst) || isTerminal(state)) {
    return -50;
  }
  double pos_cost(0), torque_cost(0);
  for (int axis = 0; axis < 2; axis ++)
  {
    pos_cost += std::fabs(dst(2*axis) / M_PI);
    //torque_cost += std::pow(action(axis) / max_torque[axis], 2);
  }
  return -(pos_cost + torque_cost);
}

Eigen::VectorXd DoubleInvertedPendulum::getSuccessor(const Eigen::VectorXd & state,
                                                     const Eigen::VectorXd & action)
{
  (void) state;
  (void) action;
  throw std::runtime_error("Not implemented");
}

bool DoubleInvertedPendulum::isValidStart(const Eigen::VectorXd &state) const
{
  bool pos_ok = std::fabs(state(0)) > M_PI - start_pos_tol;
  pos_ok = pos_ok && std::fabs(state(2)) < start_pos_tol; 
  bool vel_ok = std::fabs(state(1)) < start_vel_tol;
  vel_ok = vel_ok && std::fabs(state(3)) < start_vel_tol; 
  return pos_ok && vel_ok;
}

Eigen::VectorXd DoubleInvertedPendulum::getResetCmd(const Eigen::VectorXd &state) const
{
  Eigen::VectorXd cmd(2);
  // Force on the opposite of speed down the pendulum
  std::vector<double> gains = {-0.5,-0.1};
  cmd(0) = state(1) * gains[0];
  cmd(1) = state(3) * gains[1];
  // Ensure that pendulum will not stick in stable upward position
  if (std::fabs(state(0)) < M_PI / 10)
  {
    cmd(0) = 0.1;
  }
  if (std::fabs(state(2)) > M_PI - M_PI / 10)
  {
    cmd(1) = 0.1;
  }
  return cmd;
}