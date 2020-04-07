CSA MDP EXPERIMENTS
===================

This repository contains various experiments on Continuous-State and Action
Markov Decision Processes (CSA-MDP).


BINARIES
==========

`black_box_learning`
------------------------

Can be used to launch learning experiments, examples of valid examples include:

- `configs/examples/black_box_learner_lppi.json`
- `configs/experimental/ssl_lppi.json`

This type of learning require a blackbox model (i.e. a model which allows to
choose a couple (state, action) and to sample the result).

`learning_machine`
----------------------

This binary can be used for multiple purposes:

- Launching a learning experiments (no examples available right now)
  - Note: Unlike `black_box_learning`, the `learning_machine` supports
    real-world applications or interface with physical engine (theoretically).
    It uses the notion of `run` with multiple steps and allows to specify a
    `preparation` and an `end` for all the runs.
- Evaluating the average reward of a policy on a given problem `configs/examples/fa_policy_tester.json`
- Acquire samples in order to produce graphs of a policy on a given problem
  `configs/examples/fa_policy_tester.json`


