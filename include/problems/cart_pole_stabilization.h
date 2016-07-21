#pragma once

#include "problems/blackbox_problem.h"

#include <Eigen/Core>

#include <functional>
#include <random>
#include <vector>

/**
 * The description of this problem is given in the article:
 * "Binary Action Search for Learning Continuous-Action Control Policies"
 * (Pazis & Lagoudakis 2009)
 * The discount rate used in the article is 0.95
 *
 * The overall state space is the following
 * 0 - theta (angular position)
 * 1 - omega (angular speed)
 * 2 - cos(theta)
 * 3 - sin(theta)
 *
 * Two different learning spaces are proposed
 * - Angular: [theta omega]
 * - Cartesian: [cos(theta) sin(theta) omega]
 */

class CartPoleStabilization : public BlackBoxProblem {
public:
  /// cf above
  enum class LearningSpace
  { Angular, Cartesian };

  CartPoleStabilization();

  std::vector<int> getLearningDimensions() const override;

  bool isTerminal(const Eigen::VectorXd & state) const override;

  double getReward(const Eigen::VectorXd & state,
                   const Eigen::VectorXd & action,
                   const Eigen::VectorXd & dst) override;

  Eigen::VectorXd getSuccessor(const Eigen::VectorXd & state,
                               const Eigen::VectorXd & action) override;

  Eigen::VectorXd getStartingState() override;

  void to_xml(std::ostream & out) const override;
  void from_xml(TiXmlNode * node) override;
  std::string class_name() const override;

private:
  std::default_random_engine generator;
  std::uniform_real_distribution<double> noise_distribution;

  LearningSpace learning_space;
  
  //TODO transform those parameters in member variables, accessible through xml

  // Problem properties
  static double integration_step;//[s]
  static double simulation_step; //[s] Also called controlStep
  static double pendulum_mass;   //[kg] Mass of the pendulum
  static double cart_mass;       //[kg] Mass of the cart
  static double pendulum_length; //[m] length of the pendulum
  static double g;//[m/s^2]
  /// State space parameters
  static double theta_max;// Above this values, task is considered as failed
  static double omega_max;
  static double action_max;
  static double noise_max;// Uniform noise in [-noise_max,+noise_max] is applied at each step

  LearningSpace loadLearningSpace(const std::string & str);
};

std::string to_string(CartPoleStabilization::LearningSpace learning_space);
