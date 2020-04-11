# CSA MDP EXPERIMENTS

This repository contains various experiments on Continuous-State and Action
Markov Decision Processes (CSA-MDP).


# BINARIES

## `black_box_learning`

Can be used to launch learning experiments, examples of valid examples include:

- `configs/examples/black_box_learner_lppi.json`
- `configs/experimental/ssl_lppi.json`

This type of learning require a blackbox model (i.e. a model which allows to
choose a couple (state, action) and to sample the result).

Running a `black_box_learning` generates multiple files allowing a better
understanding of the learning process:

- `time.csv` contains the time consumption of the different steps of the learning process
- `results.csv` contains the evolution of the reward
- Files specific to the chosen learner:
  - E.g. LPPI creates:
    - `policy_fa.bin`, the best policy found by the learner
    - `value.bin`, the value function approximation associated to the policy

## `learning_machine`

This binary can be used for multiple purposes:

- Launching a learning experiments (no examples available right now)
  - Note: Unlike `black_box_learning`, the `learning_machine` supports
    real-world applications or interface with physical engine (theoretically).
    It uses the notion of `run` with multiple steps and allows to specify a
    `preparation` and an `end` for all the runs.
- Evaluating the average reward of a policy on a given problem `configs/examples/fa_policy_tester.json`
- Acquire samples in order to produce graphs of a policy on a given problem
  `configs/examples/fa_policy_tester.json`

# SCRIPTS

## `mass_bb.sh`

This scripts allows to launch multiple experiments sequentially based on the
following procedure:

- Create a numeroted folder for each experiment (abort if folder already exists)
- Copy configuration files for the blackbox experiment in the folder
- Run a `black_box_learning` process from the experiment folder

This procedure has been thought to allow running simultaneous experiments from
multiple computers using the same network storage.
