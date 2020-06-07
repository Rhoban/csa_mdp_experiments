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

## ssl_experiment (optional)

This binary integrates two [SSL](https://ssl.robocup.org/) libraries of the team [NAMeC](https://namec.fr) ([core](https://gitlab.namec.fr/ssl/software/backend/libs/core) and [base](https://gitlab.namec.fr/ssl/software/backend/libs/base)) with the **learning_machine** in order to launch real-world experimentations.
Currently, it's possible to run experiment under the official SSL simulator named **grSim**. ([download link](https://gitlab.namec.fr/ssl/software/external/grSim)).

Note: You should check [core](https://gitlab.namec.fr/ssl/software/backend/libs/core) and [base](https://gitlab.namec.fr/ssl/software/backend/libs/base) README.md files to be able to build the `ssl_experiment` binary.

Build command under rhoban's workspace:
```bash
  ./workspace build -Dbuild_ssl_experiment=ON
```

# SCRIPTS

## mass_bb

This scripts allows to launch multiple experiments sequentially based on the
following procedure:

- Create a numeroted folder for each experiment (abort if folder already exists)
- Copy configuration files for the blackbox experiment in the folder
- Run a `black_box_learning` process from the experiment folder

It requires the following elements in the `base` folder for the experiments:

- A `black_box_learner.json` file containing the configuration of the experiment
- All the configuration files at the same path as `json` files.
  E.g. a file named `problem.json` which describes the problem and is referenced
  by the learner

This procedure has been thought to allow running simultaneous experiments from
multiple computers using the same network storage.

An example to run the experiments on two computers is:

- On computer `1`: `mass_bb.sh <base> 10` which runs experiments 1 to 10
- On computer `2`: `mass_bb.sh <base> 20 11` which runs experiments 11 to 20

Since tasks are likely to be long to execute, it is highly recommended to use
`nohup` or something similar to use them and to remember.

The line above for computer `1` would therefore be replaced by:
`nohup mass_bb.sh <base> 10 > learning_1_10 &`

# Plot scripts

## Introduction

One of the main purposes of this repository is to allow running large numbers of
learning experiments based on the `rhoban_csa_mdp` environment, in order to
improve the understanding of the effect the different parameters of a parameter
or a problem.

Visualization of the learning data through different kind of plots is essential
to acquire insights on how to improve algorithms or solve problems.

The folder `plots` contains scripts written in `R` allowing to
interpret data. Some scripts are generic to all the learning experiments while
some are specific to problems (e.g. displaying a 'run').

DISCLAIMER: Currently, the folder only contains some rudimentary scripts with
a lot of unused functions, it has been extracted from

## Generic scripts

### `analyze_score_evolution.r`

This script takes as input a folder containing multiple configurations, each of
them containing various experiments produced by [mass_bb](#mass_bb).
An example of tree of the `root` folder is the following :
```bash
root
├── config1
│   ├── 001
│   │   ├── results.csv
│   │   └── ...
│   ├── 002
│   │   ├── results.csv
│   │   └── ...
│   ├── ...
├── config2
│   ├── 001
│   │   ├── results.csv
│   │   └── ...
│   ├── 002
│   │   ├── results.csv
│   │   └── ...
│   ├── ...
└── ...
```
It displays the evolution of the reward through time, with a color based on the
category and display the general trend of the score evolution for each category.
Running `Rscript analyze_score_evolution.r root` produces the file
`score_evolution.png` directly in the `root` folder.
