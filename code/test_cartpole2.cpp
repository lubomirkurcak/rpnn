/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

// NOTE(lubo): Inspired by Deep Q-Networks (DQN) from DeepMind
struct Dual_Q_Network
{
    Reinforcement_Parameters params;

    Neural_Network Q_0;
    Neural_Network Q_1;
    Neural_Network *target;
    Neural_Network *active;

    int swap_counter;
    int current_counter;
    
    b32x started;
    double last_observation[4];
    int last_action;

    int max_experiences_count;
    list(Cartpole_Experience) experiences;
};

internal Dual_Q_Network gym_cartpole_create()
{
    Dual_Q_Network dqn = {};

    dqn.params = default_reinforcement_params();
    
    int layer_sizes[] = {4, 16, 16, 16, 2};
    auto params = default_hyperparams();
    dqn.Q_0 = create_feedforward_net(params, ArrayCount(layer_sizes), layer_sizes);
    dqn.Q_1 = create_feedforward_net(params, ArrayCount(layer_sizes), layer_sizes);
    dqn.active = &dqn.Q_0;
    dqn.target = &dqn.Q_1;

    dqn.swap_counter = 0;
    dqn.current_counter = 0;

    dqn.max_experiences_count = 4096;

    return dqn;
}

internal void gym_cartpole_learn(Dual_Q_Network *dqn, int epochs)
{
    shuffle(&dqn->experiences);

    for(int epoch=0; epoch<epochs; ++epoch)
    {
        foreach(auto experience, dqn->experiences)
        {
            gym_cartpole_sarsa_step(&dqn->params, &dqn->Q_table, experience);
        }
    }

    qn->experiences.count = Min(dqn->experiences.count, dqn->max_experiences_count);
}
