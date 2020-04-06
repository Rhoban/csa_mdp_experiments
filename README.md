CSA MDP EXPERIMENTS
===================

This repository contains various experiments on Continuous-State and Action
Markov Decision Processes (CSA-MDP).


BINARIES
==========

`learning_machine`
----------------------

This binary can be used for multiple purposes:

- Launching a learning experiments (no examples available right now)
- Evaluating the average reward of a policy on a given problem `configs/examples/fa_policy_tester.json`
- Acquire samples in order to produce graphs of a policy on a given problem
  `configs/examples/fa_policy_tester.json`

`black_box_learning`
------------------------

Can be used to launch learning experiments, examples of valid examples include:

- `configs/examples/black_box_learner_lppi.json`
- `configs/experimental/ssl_lppi.json`

