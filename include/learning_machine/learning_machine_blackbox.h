#pragma once
#include "learning_machine/learning_machine.h"

#include "problems/blackbox_problem.h"

class LearningMachineBlackBox : public LearningMachine
{
public:
  LearningMachineBlackBox();
  virtual ~LearningMachineBlackBox();

  virtual void prepareRun() override;
  virtual void applyAction(const Eigen::VectorXd &action) override;
  
  virtual void setProblem(std::unique_ptr<csa_mdp::Problem> problem) override;
protected:
  /// Access to another type of problem
  std::shared_ptr<BlackBoxProblem> bb_problem;
};
