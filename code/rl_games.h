/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2020 All Rights Reserved. $
   ======================================================================== */

enum RL_Games
{
    RL_Game_None,
    
    Hill_Climbing,
    Simple_Gridworld,
    Windy_Gridworld,
    Windy_Gridworld_Q,
    Windy_Gridworld_NN,

    RL_Games_Count,
};

global char *rl_game_names[] =
{
    "No RL Game Selected",
    
    "Hill Climbing",
    "Simple Gridworld",
    "Windy Gridworld",
    "Windy Gridworld Q",
    "Windy Gridworld NN",

    "DONT FORGET TO NAME YOUR GAME XD",
    "DONT FORGET TO NAME YOUR GAME XD",
    "DONT FORGET TO NAME YOUR GAME XD",
    "DONT FORGET TO NAME YOUR GAME XD",
    "DONT FORGET TO NAME YOUR GAME XD",
    "DONT FORGET TO NAME YOUR GAME XD",
    "DONT FORGET TO NAME YOUR GAME XD",
    "DONT FORGET TO NAME YOUR GAME XD",
};

struct RL_Hill_Climbing
{
    //b32x finished;
    float pos;
    float vel;

    int minibatch_size;
    int accumulated_updates;
    Neural_Network actor;
    Neural_Network critic;
};

// TODO(lubo): To do?
struct RL_Hill_Climbing_Experience
{
    
};

internal void rl_hill_climbing_init_nn(RL_Hill_Climbing *state)
{
    state->minibatch_size = 1;
    state->accumulated_updates = 0;

    {
        int layer_sizes[] = {3,8,10,6,1};
        float learning_rate = 0.1f;
        float weight_decay = 0.001f;
        float dropout = 0;//.5f;
        float momentum_coefficient = 0;
    
        Neural_Network *actor = &state->actor;
        *actor = neural_net_parametrized(
            ArrayCount(layer_sizes),
            layer_sizes,
            learning_rate,
            weight_decay,
            dropout,
            momentum_coefficient);
    }

    {
        int layer_sizes[] = {3,8,10,6,1};
        float learning_rate = 0.1f;
        float weight_decay = 0.001f;
        float dropout = 0;//.5f;
        float momentum_coefficient = 0;
    
        Neural_Network *critic = &state->critic;
        *critic = neural_net_parametrized(
            ArrayCount(layer_sizes),
            layer_sizes,
            learning_rate,
            weight_decay,
            dropout,
            momentum_coefficient);
    }
}

internal void rl_hill_climbing(RL_Hill_Climbing *state, b32x input)
{
    float dt = 0.05;
    float stiffness = 20.0f;
    float friction = 0;//0.06f;
    float acceleration = 1.2f;
    float goal_pos = 0.3f;

    float original_pos = state->pos;
    float original_vel = state->vel;
    int original_action = input ? 1 : 0;
    
    float accel = input ? acceleration : 0.0f;
    accel += -friction*state->vel;
    accel += -stiffness*state->pos;
    state->vel += dt*accel;
    state->pos += dt*state->vel;

    // NOTE(lubo): RL Params
    // float alpha = 0.95f;
    float gamma = 0.85f;//1;

    Neural_Network *actor = &state->actor;
    matrix actor_input = actor->input;
    matrix actor_expected_output = actor->expected_output;
    matrix actor_prediction = actor->prediction;

    Neural_Network *critic = &state->critic;
    matrix critic_input = critic->input;
    matrix critic_expected_output = critic->expected_output;
    matrix critic_prediction = critic->prediction;

    float reward = -dt;
    float max_new_q = -float_max;
    if(state->pos >= goal_pos)
    {
        max_new_q = 0;
        reward = 0;
        state->pos = 0;
        state->vel = 0;
    }
    else
    {
        // NOTE(lubo): action == 1 -> accelerate, action == 0 -> dont
        max_new_q = -float_max;
        for(int action=0; action<2; ++action)
        {
            *(float *)matrix_access(critic_input, 0) = (float)state->pos;
            *(float *)matrix_access(critic_input, 1) = (float)state->vel;
            *(float *)matrix_access(critic_input, 2) = (float)(action == 1);
            feedforward(critic);
            float q = *(float *)matrix_access(critic_prediction, 0);
            if(q > max_new_q)
            {
                max_new_q = q;
            }
        }
    }
    
    *(float *)matrix_access(critic_input, 0) = (float)original_pos;
    *(float *)matrix_access(critic_input, 1) = (float)original_vel;
    *(float *)matrix_access(critic_input, 2) = (float)(original_action == 1);
    
    *(float *)matrix_access(critic_expected_output, 0) = reward + gamma*max_new_q;
    
    backprop(critic);
    accumulate_update(critic, state->minibatch_size);
    state->accumulated_updates += 1;
    if(state->accumulated_updates >= state->minibatch_size)
    {
        state->accumulated_updates = 0;
        apply_updates(critic);
    }
}

struct RL_Simple_Gridworld
{
    v2i position;
    // b32x finished;

    float value_function[16];
    int history_length;
    int state_history[8];
};

internal void rl_simple_gridworld(RL_Simple_Gridworld *state, v2i input)
{
    int old_state_id = state->position.y*4 + state->position.x;

    v2i goal = {3,3};

    float reward = 0;
    if(state->position != goal)
    {
        reward = -1;
        
        state->position += input;
        state->position.x = Clamp(state->position.x, 0, 3);
        state->position.y = Clamp(state->position.y, 0, 3);
    }

    int new_state_id = state->position.y*4 + state->position.x;

    float alpha = 1; // TODO(lubo): What's alpha? XD
    float gamma = 1; // TODO(lubo): What's gamma? XD
    
    float old_value = state->value_function[old_state_id];
    float new_value = state->value_function[new_state_id];
    float error = reward + gamma*new_value - old_value;

    float lambda = 0.85f;
    float eligibility = 1-lambda;
    state->value_function[old_state_id] += alpha*eligibility*error;
    for(int i=0; i<state->history_length; ++i)
    {
        eligibility *= lambda;
        int history_state_id = state->state_history[i];
        state->value_function[history_state_id] += alpha*eligibility*error;
    }

    state->history_length = Min(state->history_length + 1, ArrayCount(state->state_history));
    for(int i=state->history_length-1; i>0; --i)
    {
        state->state_history[i] = state->state_history[i-1];
    }
    state->state_history[0] = old_state_id;
    
    if(state->position == goal)
    {
        state->position = {};
        state->history_length = 0;
    }
}

struct RL_Windy_Gridworld
{
    v2i position;
    int action;
    // b32x finished;

    float q_function[7*10*4];

    int history_length;
    int state_history[16];

    Neural_Network net;
    
    float min_q;
};

internal v2i rl_windy_gridworld_simulate_action(v2i position, int action, float *out_reward)
{
    // v2i start = {0,3};
    // v2i goal = {7,3};
    int wind[10] = {0,-1,-1,0,1,1,2,2,1,0};
    //int wind[10] = {0,0,0,0,0,0,0,0,0,0};

    v2i input = {};
    switch(action)
    {
        default:
        case 0: input = V2i(1,0); break;
        case 1: input = V2i(0,1); break;
        case 2: input = V2i(-1,0); break;
        case 3: input = V2i(0,-1); break;
    }

    *out_reward = -1;
    
    v2i new_position = position + input;
    new_position.x = Clamp(new_position.x, 0, 9);
    new_position.y = Clamp(new_position.y + wind[position.x], 0, 6);
    // new_position.x = Clamp(new_position.x, 0, 3);
    // new_position.y = Clamp(new_position.y + wind[position.x], 0, 3);

    return new_position;
}

// NOTE(lubo): Sarsa(lambda) algorithm
internal void rl_windy_gridworld(RL_Windy_Gridworld *state, Random_Series *random)
{
    // v2i start = {0,3};
    // v2i goal = {7,3};
    v2i start = {0,0};
    v2i goal = {3,3};
    
    int old_q_id = state->action*7*10 + state->position.y*10 + state->position.x;

    if(state->position == goal)
    {
        // NOTE(lubo): Initialize episode
        state->position = start;
        state->history_length = 0;
    }
    else
    {
        // NOTE(lubo): Take action, observe reward and new state.
        float reward = 0;
        state->position = rl_windy_gridworld_simulate_action(state->position, state->action, &reward);

        // NOTE(lubo): Sample our Q-policy
        {
            // NOTE(lubo): epsilon-Greedy
            float epsilon = 0.9f;
            if(probability(epsilon, random))
            {
                // NOTE(lubo): greedy
                float max_q = -float_max;
                for(int action=0; action<4; ++action)
                {
                    int q_id = 7*10*action + 10*state->position.y + state->position.x;
                    float q = state->q_function[q_id];
                    if(q > max_q)
                    {
                        max_q = q;
                        state->action = action;
                    }
                }
            }
            else
            {
                // NOTE(lubo): random
                state->action = get_random_u32_up_to(4, random);
            }
        }

        float alpha = 1; // TODO(lubo): What's alpha? XD
        float gamma = 1; // TODO(lubo): What's gamma? XD
    
        int new_q_id = state->action*7*10 + state->position.y*10 + state->position.x;

        // NOTE(lubo): Calculate error
        float old_q = state->q_function[old_q_id];
        float new_q = state->q_function[new_q_id];
        float error = reward + gamma*new_q - old_q;

        // NOTE(lubo): Distribute error
        float lambda = 0.85f;
        float eligibility = 1-lambda;
        state->q_function[old_q_id] += alpha*eligibility*error;
        state->min_q = Min(state->min_q, state->q_function[old_q_id]);
        for(int i=0; i<state->history_length; ++i)
        {
            eligibility *= lambda;
            int history_q_id = state->state_history[i];
            state->q_function[history_q_id] += alpha*eligibility*error;
        }

        // NOTE(lubo): Shift eligibility trace array and insert last state
        state->history_length = Min(state->history_length + 1, ArrayCount(state->state_history));
        for(int i=state->history_length-1; i>0; --i)
        {
            state->state_history[i] = state->state_history[i-1];
        }
        state->state_history[0] = old_q_id;
    }
}

// NOTE(lubo): Q-Learning algorithm
internal void rl_windy_gridworld_q_learning(RL_Windy_Gridworld *state, int action, Random_Series *random)
{
    v2i start = {0,3};
    v2i goal = {7,3};
    
    int old_q_id = action*7*10 + state->position.y*10 + state->position.x;

    if(state->position == goal)
    {
        // NOTE(lubo): Initialize episode
        state->position = start;
        // state->position.x = get_random_u32_up_to(random, 10);
        // state->position.y = get_random_u32_up_to(random, 7);
        state->history_length = 0;
    }
    else
    {
        // NOTE(lubo): Take action, observe reward and new state.
        float reward = 0;
        state->position = rl_windy_gridworld_simulate_action(state->position, action, &reward);

        float alpha = 0.95f;//1; // TODO(lubo): What's alpha? XD
        float gamma = 0.95f;//1; // TODO(lubo): What's gamma? XD

        // NOTE(lubo): Calculate error (Optimal greedy policy) (SARSAMAX TD(0))
        float max_new_q = -float_max;
        for(int g_action=0; g_action<4; ++g_action)
        {
            int q_id = 7*10*g_action + 10*state->position.y + state->position.x;
            float q = state->q_function[q_id];
            if(q > max_new_q)
            {
                max_new_q = q;
            }
        }

        float old_q = state->q_function[old_q_id];
        float error = reward + gamma*max_new_q - old_q;
        
        // NOTE(lubo): Distribute error
        state->q_function[old_q_id] = old_q + alpha*error;
        state->min_q = Min(state->q_function[old_q_id], state->min_q);
    }
}

internal void rl_windy_gridworld_init_nn(RL_Windy_Gridworld *state)
{
    int layer_sizes[] = {6,30,6,1};
    float learning_rate = 0.0001f;
    
    Neural_Network *net = &state->net;
    *net = neural_net_parametrized(
        ArrayCount(layer_sizes),
        layer_sizes,
        learning_rate);
}

// NOTE(lubo): Neural net algorithm
internal void rl_windy_gridworld_nn(RL_Windy_Gridworld *state, int action, Random_Series *random)
{
    v2i start = {0,3};
    v2i goal = {7,3};

    if(probability(0.1f, random))
    {
        state->position.x = get_random_u32_up_to(10, random);
        state->position.y = get_random_u32_up_to(7, random);
        state->history_length = 0;
    }

    Neural_Network *net = &state->net;
    matrix net_input = net->input;
    matrix net_expected_output = net->expected_output;
    matrix net_prediction = net->prediction;
    
    //float alpha = 0.95f;//1; // TODO(lubo): What's alpha? XD
    float gamma = 0.85f;//1; // TODO(lubo): What's gamma? XD

            
    if(state->position == goal)
    // if(state->position == goal_bl ||
    //    state->position == goal_br ||
    //    state->position == goal_tr ||
    //    state->position == goal_tl)
    {
        {
                    

            *(float *)matrix_access(net_input, 0) = (float)goal.x;
            *(float *)matrix_access(net_input, 1) = (float)goal.y;
            *(float *)matrix_access(net_input, 2) = (float)(action == 0);
            *(float *)matrix_access(net_input, 3) = (float)(action == 1);
            *(float *)matrix_access(net_input, 4) = (float)(action == 2);
            *(float *)matrix_access(net_input, 5) = (float)(action == 3);
        
            *(float *)matrix_access(net_expected_output, 0) = 0;
        
            backprop(net);
            accumulate_update(net, 1);
            apply_updates(net);
            
            //float cost = calculate_cost(net);
        
        
        
            // loginfo("neural_net", "Iter %d Cost %f LR %f", iter, cost, learning_rate);

            // for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
            // {
            //     matrix weights = get_by_index(&net->weights, layer_index);
            //     matrix biases = get_by_index(&net->biases, layer_index);
            //     matrix del_weights = get_by_index(&net->del_weights, layer_index);
            //     matrix del_biases = get_by_index(&net->del_biases, layer_index);
        
            //     multiply(del_weights, -learning_rate);
            //     multiply(del_biases, -learning_rate);
            //     add(weights, del_weights);
            //     add(biases, del_biases);
            // }

        }
        
        
        
        // NOTE(lubo): Initialize episode
        state->position = start;
        // state->position.x = get_random_u32_up_to(random, 10);
        // state->position.y = get_random_u32_up_to(random, 7);
        state->history_length = 0;
    }
    else
    {
        // NOTE(lubo): Take action, observe reward and new state.
        v2i original_pos = state->position;
        int original_action = action;
        float reward = 0;
        state->position = rl_windy_gridworld_simulate_action(state->position, action, &reward);

        // TODO(lubo): Reduce dimensionality in one-hot encoding by 1
        float max_new_q = -float_max;
        for(int g_action=0; g_action<4; ++g_action)
        {
            *(float *)matrix_access(net_input, 0) = (float)state->position.x;
            *(float *)matrix_access(net_input, 1) = (float)state->position.y;
            *(float *)matrix_access(net_input, 2) = (float)(g_action == 0);
            *(float *)matrix_access(net_input, 3) = (float)(g_action == 1);
            *(float *)matrix_access(net_input, 4) = (float)(g_action == 2);
            *(float *)matrix_access(net_input, 5) = (float)(g_action == 3);
            feedforward(net);
            float q = *(float *)matrix_access(net_prediction, 0);
            if(q > max_new_q)
            {
                max_new_q = q;
            }
        }

        // NOTE(lubo): Update previous state with bootstrapping to new state

        *(float *)matrix_access(net_input, 0) = (float)original_pos.x;
        *(float *)matrix_access(net_input, 1) = (float)original_pos.y;
        *(float *)matrix_access(net_input, 2) = (float)(original_action == 0);
        *(float *)matrix_access(net_input, 3) = (float)(original_action == 1);
        *(float *)matrix_access(net_input, 4) = (float)(original_action == 2);
        *(float *)matrix_access(net_input, 5) = (float)(original_action == 3);
        
        *(float *)matrix_access(net_expected_output, 0) = reward + gamma*max_new_q;
        
        backprop(net);
        accumulate_update(net, 1);
        apply_updates(net);
        //float cost = calculate_cost(net);
        
        // loginfo("neural_net", "Iter %d Cost %f LR %f", iter, cost, learning_rate);

        // for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
        // {
        //     matrix weights = get_by_index(&net->weights, layer_index);
        //     matrix biases = get_by_index(&net->biases, layer_index);
        //     matrix del_weights = get_by_index(&net->del_weights, layer_index);
        //     matrix del_biases = get_by_index(&net->del_biases, layer_index);
        
        //     multiply(del_weights, -learning_rate);
        //     multiply(del_biases, -learning_rate);
        //     add(weights, del_weights);
        //     add(biases, del_biases);
        // }

        
        {
            *(float *)matrix_access(net_input, 0) = (float)original_pos.x;
            *(float *)matrix_access(net_input, 1) = (float)original_pos.y;
            *(float *)matrix_access(net_input, 2) = (float)(original_action == 0);
            *(float *)matrix_access(net_input, 3) = (float)(original_action == 1);
            *(float *)matrix_access(net_input, 4) = (float)(original_action == 2);
            *(float *)matrix_access(net_input, 5) = (float)(original_action == 3);
            feedforward(net);
            float q = *(float *)matrix_access(net_prediction, 0);
            state->min_q = Min(q, state->min_q);
        }
    }
}
