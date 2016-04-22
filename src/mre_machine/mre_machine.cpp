#include "mre_machine/mre_machine.h"

#include "problems/problem_factory.h"

#include "rosban_csa_mdp/core/history.h"
#include "rosban_csa_mdp/core/policy_factory.h"

#include "rosban_utils/benchmark.h"

using csa_mdp::History;
using csa_mdp::Problem;
using csa_mdp::MRE;

using rosban_utils::Benchmark;

using csa_mdp::Policy;

MREMachine::Config::Config()
  : mode(MREMachine::Mode::exploration)
{
}

std::string MREMachine::Config::class_name() const
{
  return "Config";
}

void MREMachine::Config::to_xml(std::ostream &out) const
{
  rosban_utils::xml_tools::write<std::string>("mode", to_string(mode), out);
  rosban_utils::xml_tools::write<int>("nb_runs", nb_runs, out);
  rosban_utils::xml_tools::write<int>("nb_steps", nb_steps, out);
  rosban_utils::xml_tools::write<std::string>("problem", problem, out);
  switch(mode)
  {
    case MREMachine::Mode::evaluation:
      policy->write("policy", out);
      break;
    case MREMachine::Mode::exploration:
      mre_config.write("mre", out);
      break;
    case MREMachine::Mode::full:
      throw std::runtime_error("MREMachine does not implement mode 'full' yet");
  }
}

void MREMachine::Config::from_xml(TiXmlNode *node)
{
  std::string mode_str = rosban_utils::xml_tools::read<std::string>(node, "mode");
  mode = loadMode(mode_str);
  nb_runs  = rosban_utils::xml_tools::read<int>(node, "nb_runs");
  nb_steps = rosban_utils::xml_tools::read<int>(node, "nb_steps");
  problem  = rosban_utils::xml_tools::read<std::string>(node, "problem");
  switch(mode)
  {
    case MREMachine::Mode::evaluation:
    {
      TiXmlNode * policy_node = node->FirstChild("policy");
      if(!policy_node) throw std::runtime_error("Failed to find node 'policy'");
      policy = std::unique_ptr<Policy>(PolicyFactory().build(policy_node));
      break;
    }
    case MREMachine::Mode::exploration:
      mre_config.read(node, "mre");
      rosban_utils::xml_tools::try_read<std::string>(node, "seed_path", seed_path);
      break;
    case MREMachine::Mode::full:
      throw std::runtime_error("MREMachine does not implement mode 'full' yet");      
  }
}

MREMachine::MREMachine(std::shared_ptr<Config> config_)
  : config(config_), run(1), step(0), nb_updates(0), next_update(1)
{
  // Generating problem
  problem = std::shared_ptr<Problem>(ProblemFactory().build(config->problem));
  // Applying problem Limits
  config->mre_config.mrefpf_conf.setStateLimits(problem->getStateLimits());
  config->mre_config.mrefpf_conf.setActionLimits(problem->getActionLimits());
  // Initalizing mre
  if ( config->mode != MREMachine::Mode::evaluation )
  {
    mre = std::unique_ptr<MRE>(new MRE(config->mre_config,
                                       [this](const Eigen::VectorXd &state)
    {
      return this->problem->isTerminal(state);
    }
                                 ));
  }
  // Setting action space for policy
  if ( config->mode == MREMachine::Mode::evaluation )
  {
    config->policy->setActionLimits(problem->getActionLimits());
  }
  // Opening log files
  run_logs.open("run_logs.csv");
  time_logs.open("time_logs.csv");
  reward_logs.open("reward_logs.csv");
}

MREMachine::~MREMachine()
{
  run_logs.close();
  time_logs.close();
  reward_logs.close();
}

void MREMachine::execute()
{
  init();
  while(alive() && run <= config->nb_runs)
  {
    doRun();
    run++;
  }
}

void MREMachine::doRun()
{
  Benchmark::open("preparation");
  prepareRun();
  writeTimeLog("preparation", Benchmark::close());
  Benchmark::open("simulation");
  while (alive() && step <= config->nb_steps && !problem->isTerminal(current_state))
  {
    doStep();
    step++;
  }
  writeTimeLog("simulation", Benchmark::close());
  endRun();
}

void MREMachine::doStep()
{
  Eigen::VectorXd cmd;
  switch(config->mode)
  {
    case MREMachine::Mode::exploration:
      cmd = mre->getAction(current_state);
      break;
    case MREMachine::Mode::evaluation:
      cmd = config->policy->getAction(current_state);
      break;
    case MREMachine::Mode::full:
      throw std::runtime_error("MREMachine doest not implement mode 'full' yet");      
  }
  Eigen::VectorXd last_state = current_state;
  applyAction(cmd);
  writeRunLog(run_logs, run, step, last_state, cmd, current_reward);
  if (config->mode == MREMachine::Mode::exploration)
  {
    csa_mdp::Sample new_sample(last_state,
                               cmd,
                               current_state,
                               current_reward);
    // Add new sample
    mre->feed(new_sample);
  }
  trajectory_reward += current_reward;
}

void MREMachine::init()
{
  writeRunLogHeader(run_logs);
  time_logs << "run,type,time" << std::endl;
  reward_logs << "run,reward" << std::endl;
  // Preload some experiment
  if (config->mode == MREMachine::Mode::exploration && config->seed_path != "")
  {
    std::vector<History> histories = History::readCSV(config->seed_path,
                                                      problem->getStateLimits().rows(),
                                                      problem->getActionLimits().rows());
    std::vector<csa_mdp::Sample> samples = History::getBatch(histories);
    for (const csa_mdp::Sample & s : samples)
    {
      mre->feed(s);
    }
    mre->updatePolicy();
  }
}

bool MREMachine::alive()
{
  return true;
}

void MREMachine::prepareRun()
{
  step = 0;
  trajectory_reward = 0;
  current_reward = 0;
}

void MREMachine::endRun()
{
  // If the maximal step has not been reached, it mean we reached a final state
  if (step <= config->nb_steps)
  {
    int u_dim = problem->getActionLimits().rows();
    writeRunLog(run_logs, run, step, current_state, Eigen::VectorXd::Zero(u_dim), 0);
  }
  reward_logs << run << "," << trajectory_reward << std::endl;
  if (config->mode == MREMachine::Mode::exploration &&
      run < config->nb_runs && run == next_update)
  {
    mre->updatePolicy();
    nb_updates++;
    writeTimeLog("qValueTS", mre->getQValueTrainingSetTime());
    writeTimeLog("qValueET", mre->getQValueExtraTreesTime());
    writeTimeLog("policyTS", mre->getPolicyTrainingSetTime());
    writeTimeLog("policyET", mre->getPolicyExtraTreesTime());
    next_update = pow(nb_updates + 1, 2);
  }
  if (config->mode == MREMachine::Mode::evaluation)
  {
    config->policy->init();
  }
}

void MREMachine::writeRunLogHeader(std::ostream &out)
{
  out << "run,step,";
  // State information
  for (const std::string & name : problem->getStateNames())
  {
    out << name << ",";
  }
  // Commands
  for (const std::string & name : problem->getActionNames())
  {
    out << name << ",";
  }
  out << "reward" << std::endl;
}

void MREMachine::writeTimeLog(const std::string &type, double time)
{
  time_logs << run << "," << type << "," << time << std::endl;
}

void MREMachine::writeRunLog(std::ostream &out, int run, int step,
                             const Eigen::VectorXd &state,
                             const Eigen::VectorXd &action,
                             double reward)
{
  out << run << "," << step << ",";

  for (int i = 0; i < state.rows(); i++)
  {
    out << state(i) << ",";
  }
  for (int i = 0; i < action.rows(); i++)
  {
    out << action(i) << ",";
  }
  out << reward << std::endl;
}

std::string to_string(MREMachine::Mode mode)
{
  switch (mode)
  {
    case MREMachine::Mode::exploration: return "exploration";
    case MREMachine::Mode::evaluation:  return "evaluation";
    case MREMachine::Mode::full:        return "full";
  }
  throw std::runtime_error("Unknown MREMachine::Mode type in to_string(Type)");
}

MREMachine::Mode loadMode(const std::string &mode)
{
  if (mode == "exploration")
  {
    return MREMachine::Mode::exploration;
  }
  if (mode == "evaluation")
  {
    return MREMachine::Mode::evaluation;
  }
  if (mode == "full")
  {
    return MREMachine::Mode::full;
  }
  throw std::runtime_error("Unknown MREMachine::Mode: '" + mode + "'");
}