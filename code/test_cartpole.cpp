/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

struct Cartpole_Experience
{
    double state[4];
    int action;
    int terminal;
    double reward;
    double new_state[4];
};

#define DL_TYPE Cartpole_Experience
#include "dynamic_list_template.h"

struct Reinforcement_Parameters
{
    // NOTE(lubo): Linear epsilon annealing
    float epsilon0;
    float epsilon1;
    int exploration_steps;
    float epsilon;
    
    float gamma;
    float alpha;
    int minibatch_size;

    int target_update_frequency;
    int replay_start_size;

    int action_repeat;
};

inline Reinforcement_Parameters default_reinforcement_params()
{
    Reinforcement_Parameters params = {};

    params.epsilon0 = 1.0f;
    params.epsilon1 = 0.1f;
    params.exploration_steps = 1000000;
    params.epsilon = 1; // NOTE(lubo): We start with random policy
    
    params.gamma = 0.99f;
    params.alpha = 1.0f;

    params.minibatch_size = 32;

    params.target_update_frequency = 10000;
    params.replay_start_size = 50000;
    
    params.action_repeat = 1;
    
    return params;
}

// NOTE(lubo): Inspired by Deep Q-Networks (DQN) from DeepMind
struct Deep_Q_Network
{
    Reinforcement_Parameters params;
    
    Neural_Network active_Q;
    Neural_Network target_Q;
    
    Lookup_Table Q_table;

    int retarget_count;
    int frame_counter;
    
    b32x started;
    double last_observation[4];
    int last_action;

    list(Cartpole_Experience) experiences;
};

internal void gym_cartpole_create(Deep_Q_Network *dqn)
{
    dqn->params = default_reinforcement_params();
    
    int layer_sizes[] = {4, 16, 16, 16, 2};
    auto params = default_hyperparams();
    params.learning_rate = 0.1f;
    params.weight_decay = 0.00001f;
    dqn->active_Q = create_feedforward_net(params, ArrayCount(layer_sizes), layer_sizes);
    dqn->target_Q = create_feedforward_net(params, ArrayCount(layer_sizes), layer_sizes);

    copy_neural_network(&dqn->target_Q, &dqn->active_Q);
    
    system("del   /q /s cartpole_networks");
    system("rmdir /q /s cartpole_networks");
    system("del   /q /s network_images");
    system("rmdir /q /s network_images");
    
    dqn->Q_table = load_lookup_table("cartpole.lt");
    if(!dqn->Q_table.buckets)
    {
        /* NOTE(lubo): Observations (https://github.com/openai/gym/blob/master/gym/envs/classic_control/cartpole.py)
        | Num | Observation           | Min                  | Max                |
        |-----|-----------------------|----------------------|--------------------|
        | 0   | Cart Position         | -4.8                 | 4.8                |
        | 1   | Cart Velocity         | -Inf                 | Inf                |
        | 2   | Pole Angle            | ~ -0.418 rad (-24°)  | ~ 0.418 rad (24°)  |
        | 3   | Pole Angular Velocity | -Inf                 | Inf                |*/
        int buckets[] = {20, 20, 20, 20};
    
        double _ranges[] =
        {
            -5, 5,
            -3, -3,
            -0.5, 0.5,
            -5, 5,
        };
        matrix ranges = {4,2,1,1};
        ranges.allocation.memory = (u8 *)_ranges;
        ranges.allocation.size = sizeof(_ranges);

        int output_count = 2;
        dqn->Q_table = create_lookup_table(buckets, ranges, output_count);
    }
    dqn->Q_table.lerp_value = 0;//.9;
    Assert(dqn->Q_table.buckets);
}

// NOTE(lubo): Fixed-target SARSA step
internal void gym_cartpole_fixed_learn_from_experience(
    Reinforcement_Parameters *params,
    Function_Approximator *Q,
    Function_Approximator *target_Q,
    Cartpole_Experience *exp)
{
    double new_q = 0;
    if(!exp->terminal)
    {
        matrix new_qs = target_Q->evaluate(exp->new_state);
        int new_action = argmax(new_qs);
        if(probability(params->epsilon))
        {
            new_action = get_random_int_up_to(matrix_element_count(new_qs));
        }
        new_q = matrix_get(new_qs, new_action);
    }

    matrix current_qs = Q->evaluate(exp->state);
    double current_q = matrix_get(current_qs, exp->action);
    current_q = current_q + params->alpha*(exp->reward + params->gamma*new_q - current_q);
    matrix_set(current_qs, exp->action, current_q);
    Q->learn(current_qs, params->minibatch_size);
}

internal void gym_cartpole_nonfixed_learn_from_experience(
    Reinforcement_Parameters *params,
    Function_Approximator *Q,
    Cartpole_Experience *exp)
{
    gym_cartpole_fixed_learn_from_experience(params, Q, Q, exp);
}

internal void gym_cartpole_learn(Deep_Q_Network *dqn, int batches)
{
    // shuffle(&dqn->experiences);

    // for(int epoch=0; epoch<epochs; ++epoch)
    // {
    //     foreach(auto experience, dqn->experiences)
    //     {
    //         gym_cartpole_fixed_learn_from_experience(
    //             &dqn->params, dqn->active, dqn->target, experience);
    //     }
    // }

    // // NOTE(lubo): Forget some of the experiences
    // dqn->experiences.count = Min(dqn->experiences.count, 4096);

    for(int batch_index=0; batch_index<batches; ++batch_index)
    {
        for(int i=0; i<dqn->params.minibatch_size; ++i)
        {
            auto experience = get_pointer_to_random(&dqn->experiences);
            gym_cartpole_fixed_learn_from_experience(
                &dqn->params, &dqn->active_Q, &dqn->target_Q, experience);
        }

        Assert(dqn->active_Q.minibatch_count == 0);
        Assert(dqn->target_Q.minibatch_count == 0);
    }

    if(dqn->experiences.count > 1000000)
    {
        remove_random(&dqn->experiences);
    }
}

internal int gym_cartpole_update(Deep_Q_Network *dqn, double *observation, double reward, b32x done)
{
    b32x start_phase = dqn->experiences.count < dqn->params.replay_start_size;
    b32x repeat_last_action = (dqn->frame_counter % dqn->params.action_repeat) != 0;
    b32x update_target = (dqn->frame_counter % dqn->params.target_update_frequency) == 0;

    Assert(!repeat_last_action);
    
    // NOTE(lubo): Create memories
    if(dqn->started)
    {
        Cartpole_Experience experience = {};
        block_copy(experience.state, dqn->last_observation, sizeof(double)*4);
        experience.action = dqn->last_action;
        experience.reward = reward;
        experience.terminal = done;
        block_copy(experience.new_state, observation, sizeof(double)*4);
        append_to_list(&dqn->experiences, experience);
        
        gym_cartpole_nonfixed_learn_from_experience(&dqn->params, &dqn->Q_table, &experience);

        if(start_phase)
        {
        }
        else if(!repeat_last_action)
        {
            gym_cartpole_learn(dqn, 1);
            
            if(update_target)
            {
                dqn->params.epsilon = lerp_clamped(dqn->params.epsilon0, dqn->params.epsilon1,
                                                   (float)(dqn->frame_counter-dqn->params.replay_start_size)/(float)dqn->params.exploration_steps);

                Assert(dqn->params.epsilon >= 0);
                Assert(dqn->params.epsilon <= 1);
            
                copy_neural_network(&dqn->target_Q, &dqn->active_Q);
                dqn->retarget_count += 1;
                loginfo("reinforce", "Retarget %d (epsilon = %f) (%d xp)", dqn->retarget_count, dqn->params.epsilon, dqn->experiences.count);



            
                system("mkdir cartpole_networks");
                char save_name[256];
                sprintf(save_name, "cartpole_networks\\%d.nn", dqn->retarget_count);
                save_network(&dqn->active_Q, save_name);
                save_network_weights_images(&dqn->active_Q, dqn->retarget_count);

                save_lookup_table(&dqn->Q_table, "cartpole.lt");
            };
        }
        else
        {
            Assert(!update_target);
        }
    }

    int action = 0;
    if(repeat_last_action)
    {
        action = dqn->last_action;
        Assert(false);
    }
    else
    {
        // NOTE(lubo): Act epsilon-greedy
        if(probability(dqn->params.epsilon))
        {
            action = get_random_int_up_to(2);
        }
        else
        {
            Assert(!start_phase);
            action = argmax(dqn->active_Q.evaluate(observation));
            //action = argmax(dqn->Q_table.evaluate(observation));
        }
    }
    
    // NOTE(lubo): Save last actions
    block_copy(dqn->last_observation, observation, sizeof(double)*4);
    dqn->last_action = action;
    dqn->started = true;
    dqn->frame_counter += 1;

    return action;
}
