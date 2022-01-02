/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

internal FORT_GAMEMODE_UPDATE(game_2d)
{
    r2 screen_rect = {0, 0, (float)screen_width, (float)screen_height};    
    float dt = Clamp(last_frame_time, 0.001f, 0.033f);
    float unscaled_dt = dt; // TODO(lubo): lulwhatXD?
    dt *= state->timescale;

    float tick_rate = 1;
    static float accumulated_time = 0;
    accumulated_time += tick_rate*dt;
    b32x clock_tick = false;
    if(accumulated_time >= 1)
    {
        clock_tick = true;
        accumulated_time -= 1;
    }

    #if 0
    if(was_pressed(&input->key1))
    {
        state->rl_game_mode = Hill_Climbing;
        state->hill_climbing = {};
        rl_hill_climbing_init_nn(&state->hill_climbing);
        // state->hill_climbing.pos = get_random_float(&state->global_entropy, -0.3f, 0.3f);
        // state->hill_climbing.vel = get_random_float(&state->global_entropy, -0.5f, 0.5f);
    }
    if(was_pressed(&input->key2))
    {
        state->rl_game_mode = Simple_Gridworld;
        state->simple_gridworld = {};
    }
    if(was_pressed(&input->key3))
    {
        state->rl_game_mode = Windy_Gridworld;
        state->windy_gridworld = {};
        state->windy_gridworld.position = V2i(0,3);
    }
    if(was_pressed(&input->key4))
    {
        state->rl_game_mode = Windy_Gridworld_Q;
        state->windy_gridworld = {};
        state->windy_gridworld.position = V2i(0,3);
    }
    if(was_pressed(&input->key5))
    {
        state->rl_game_mode = Windy_Gridworld_NN;
        state->windy_gridworld = {};

        state->windy_gridworld.position = V2i(0,3);
        rl_windy_gridworld_init_nn(&state->windy_gridworld);
    }
    
    switch(state->rl_game_mode)
    {
        case Hill_Climbing:
        {
            RL_Hill_Climbing *hill_climbing = &state->hill_climbing;

            Neural_Network *critic = &hill_climbing->critic;
            matrix critic_input = critic->input;
            matrix critic_expected_output = critic->expected_output;
            matrix critic_prediction = critic->prediction;

            // if(probability(&state->global_entropy, 0.01f))
            // {
            //     critic->learning_rate *= 0.999f;
            // }
            
            b32x pressing_button = false;
            pressing_button = is_pressed(&input->action_down);

            float DEBUG_DISPLAY_QS[2] = {};

            int updates_per_frame = 1;
            for(int update_index=0; update_index<updates_per_frame; ++update_index)
            {
                //if(probability(&state->global_entropy, 0.02f))
                // {
                //     state->hill_climbing.pos = get_random_float(&state->global_entropy, -0.31f, 0.31f);
                //     state->hill_climbing.vel = get_random_float(&state->global_entropy, -1.2f, 1.2f);
                // }
            
                if(GLOBAL_DEBUG_CONTROLLED_BOOL)
                {
                    if(0)
                    {
                        // NOTE(lubo): This is our policy.
                        pressing_button = hill_climbing->vel >= 0;
                    }
                    else
                    {
                        #define ACTION_COUNT 2
                        int policy_action = 0;
                        float epsilon = 0.1f;
                        if(probability(epsilon, &state->global_entropy))
                        {
                            policy_action = get_random_u32_up_to(ACTION_COUNT, &state->global_entropy);
                        }
                        else
                        {
                            float qs[ACTION_COUNT] = {};
                            // float action_probabilities[ACTION_COUNT] = {};
                            float max_q = -float_max;
                            for(int action=0; action<ACTION_COUNT; ++action)
                            {
                                *(float *)matrix_access(critic_input, 0) = (float)hill_climbing->pos;
                                *(float *)matrix_access(critic_input, 1) = (float)hill_climbing->vel;
                                // TODO(lubo): One-hot encoding for actions. We prolly want Q(S) -> {AV1, AV2, AV3} instead of Q(S, A) -> {V}
                                *(float *)matrix_access(critic_input, 2) = (float)(action == 1);
                                feedforward(critic);
                                float q = *(float *)matrix_access(critic_prediction, 0);

                                qs[action] = q;
                                DEBUG_DISPLAY_QS[action] = q;
                            
                                if(q > max_q)
                                {
                                    max_q = q;
                                    policy_action = action;
                                }
                            }

                            // softmax(ACTION_COUNT, qs, action_probabilities);
                            // policy_action = weighted_random(&state->global_entropy, ACTION_COUNT, action_probabilities);
                        }

                        pressing_button = policy_action;
                    }
                }


                if(pressing_button)
                {
                    int aiuwhdkhaiw=0;
                }
            
                rl_hill_climbing(hill_climbing, pressing_button);
            }

            {
                float goal_pos = 0.3f;
    
                v2 halfdim = {50,50};
                r2 rect = rect_center_halfdim(rect_lerp(screen_rect, V2(hill_climbing->pos, 0.0f) + V2(0.5f,0.5f)), halfdim);
                r2 goal_rect = rect_center_halfdim(rect_lerp(screen_rect, V2(goal_pos, 0.0f) + V2(0.5f,0.5f)), halfdim);

                int sprite = pngs_circle_png;
                v4 color = lerp(V4(1,1,1,1), V4(0,1,0,1), inv_lerp(-0.3f, 0.3f, hill_climbing->pos));
                debug_immediate_sprite_ui(rect, sprite, color);
                debug_immediate_sprite_ui(goal_rect, pngs_stars_24px_png, color);
            }

            global float total_min_q = float_max;
            global float total_max_q = -float_max;
            float previous_min_q = total_min_q;
            float previous_max_q = total_max_q;
            float previous_q_range = total_max_q - total_min_q;

            int display_count = 4;
            global int display;
            if(is_pressed(&input->left)) display = modulo(display - 1, display_count);
            if(is_pressed(&input->right)) display = modulo(display + 1, display_count);

            int grid_pos_steps = 64;
            float grid_pos_min = -0.5f;
            float grid_pos_max = 0.5f;
            int grid_vel_steps = 64;
            float grid_vel_min = -1.3f;
            float grid_vel_max = 1.3f;
            total_min_q = float_max;
            total_max_q = -float_max;
            for(float grid_vel=grid_vel_min; grid_vel<grid_vel_max; grid_vel += (grid_vel_max-grid_vel_min)/grid_vel_steps)
            {
                for(float grid_pos=grid_pos_min; grid_pos<grid_pos_max; grid_pos += (grid_pos_max-grid_pos_min)/grid_pos_steps)
                {
                    v2 relp = {inv_lerp(grid_pos_min,grid_pos_max,grid_pos), inv_lerp(grid_vel_min,grid_vel_max,grid_vel)};
                    v2 halfdim = {10,10};
                    v2 offset = {0,0};
                    r2 rect = offset_rect(rect_center_halfdim(rect_lerp(screen_rect, relp), halfdim), offset);
                    
                    int policy_action = 0;
                    float qs[ACTION_COUNT];
                    float action_probabilities[ACTION_COUNT];
                    float max_q = -float_max;
                    for(int action=0; action<2; ++action)
                    {
                        *(float *)matrix_access(critic_input, 0) = (float)grid_pos;
                        *(float *)matrix_access(critic_input, 1) = (float)grid_vel;
                        *(float *)matrix_access(critic_input, 2) = (float)(action == 1);
                        feedforward(critic);
                        float q = *(float *)matrix_access(critic_prediction, 0);
                        qs[action] = q;

                        total_min_q = Min(total_min_q, q);
                        total_max_q = Max(total_max_q, q);
                        
                        if(q > max_q)
                        {
                            max_q = q;
                            policy_action = action;
                        }
                    }

                    // softmax(ACTION_COUNT, qs, action_probabilities);
                    // policy_action = weighted_random(&state->global_entropy, ACTION_COUNT, action_probabilities);
                    
                    v4 color = {0,0,0,1};
                    switch(display)
                    {
                        default:
                        case 0:
                        {
                            color.r = inv_lerp(previous_min_q, previous_max_q, Max(qs[0], qs[1]));
                            color.g = Min(1, Abs(qs[0] - qs[1])/(previous_q_range));
                            color.b = (policy_action == 1) ? 1.0f : 0.0f;
                        } break;
                        case 1:
                        {
                            color.r = inv_lerp(previous_min_q, previous_max_q, Max(qs[0], qs[1]));
                        } break;
                        case 2:
                        {
                            color.b = (policy_action == 1) ? 1.0f : 0.0f;
                        } break;
                        case 3:
                        {
                            color.g = Min(1, Abs(qs[0] - qs[1])/(previous_q_range));
                            color.b = (policy_action == 1) ? 1.0f : 0.0f;
                        } break;
                    }
                    
                    //v4 color = V2i(x,y) == V2i(3,3) ? V4(1,0,0,1) : simple_gridworld->finished ? V4(0,1,0,1) : V4(1,1,1,1);
                    //float state_value = simple_gridworld->value_function[y*4 + x];
                
                    debug_immediate_sprite_ui(rect, pngs_circle_png, color);

                }
            }

            {
                // float difference = total_max_q - total_min_q;
                // total_max_q -= 0.01f*difference;
                // total_min_q += 0.01f*difference;
                Baked_Font *font = &state->debug_font;
                char buffer[32];
                stbsp_snprintf(buffer, ArrayCount(buffer), "%.4f\n%.4f\n%f", previous_min_q, previous_max_q, critic->learning_rate);
                draw_text(font, String(buffer), screen_rect, V2(1,1), V2(1,1));
            }
            
            {
                float grid_pos = hill_climbing->pos;
                float grid_vel = hill_climbing->vel;
                v2 relp = {inv_lerp(grid_pos_min,grid_pos_max,grid_pos), inv_lerp(grid_vel_min,grid_vel_max,grid_vel)};
                v2 halfdim = {10,10};
                if(pressing_button) halfdim = {20,20};
                v2 offset = {0,0};
                r2 rect = offset_rect(rect_center_halfdim(rect_lerp(screen_rect, relp), halfdim), offset);
                //debug_immediate_sprite(rect, pressing_button ? pngs_circle_png : pngs_block_24px_png, V4(1,0,0,1));
                debug_immediate_sprite_ui(rect, pngs_circle_png, V4(1,0,0,1));

                Baked_Font *font = &state->debug_font;
                char buffer[32];
                stbsp_snprintf(buffer, ArrayCount(buffer), "%.4f\n%.4f", DEBUG_DISPLAY_QS[0], DEBUG_DISPLAY_QS[1]);
                draw_text(font, String(buffer), rect);
            }
            
            {
                v2 relp = V2(0.5,0.5)+0.5*input->mouse;
                v2 halfdim = {10,10};
                if(pressing_button) halfdim = {20,20};
                v2 offset = {0,0};
                r2 rect = offset_rect(rect_center_halfdim(rect_lerp(screen_rect, relp), halfdim), offset);
                //debug_immediate_sprite(rect, pressing_button ? pngs_circle_png : pngs_block_24px_png, V4(1,0,0,1));
                debug_immediate_sprite_ui(rect, pngs_circle_png, V4(1,0,0,1));

                float grid_pos = lerp(grid_pos_min,grid_pos_max,relp.x);
                float grid_vel = lerp(grid_vel_min,grid_vel_max,relp.y);
                
                int policy_action = 0;
                float max_q = -float_max;
                float qs[2];
                for(int action=0; action<2; ++action)
                {
                    *(float *)matrix_access(critic_input, 0) = (float)grid_pos;
                    *(float *)matrix_access(critic_input, 1) = (float)grid_vel;
                    *(float *)matrix_access(critic_input, 2) = (float)(action == 1);
                    feedforward(critic);
                    float q = *(float *)matrix_access(critic_prediction, 0);
                    qs[action] = q;

                    total_min_q = Min(total_min_q, q);
                    total_max_q = Max(total_max_q, q);
                        
                    if(q > max_q)
                    {
                        max_q = q;
                        policy_action = action;
                    }
                }

                Baked_Font *font = &state->debug_font;
                char buffer[32];
                stbsp_snprintf(buffer, ArrayCount(buffer), "%.4f\n%.4f", qs[0], qs[1]);
                draw_text(font, String(buffer), screen_rect, V2(0.5,0.5)+0.5*input->mouse, V2(0,0));
            }

        } break;

        case Simple_Gridworld:
        {
            RL_Simple_Gridworld *simple_gridworld = &state->simple_gridworld;
            
            v2i rl_input = {};
            if(was_pressed(&input->left)) rl_input.x -= 1;
            if(was_pressed(&input->right)) rl_input.x += 1;
            if(was_pressed(&input->up)) rl_input.y += 1;
            if(was_pressed(&input->down)) rl_input.y -= 1;

            if(GLOBAL_DEBUG_CONTROLLED_BOOL && clock_tick)
            {
                // NOTE(lubo): epsilon-Greedy policy
                float epsilon = 0.9f;
                if(probability(epsilon, &state->global_entropy))
                {
                    // NOTE(lubo): Greedy policy
                    float max_value = -float_max;
                    v2i max_input = {};
                    for(int action=0; action<4; ++action)
                    {
                        v2i action_input = {};
                        switch(action)
                        {
                            default:
                            case 0:
                            {
                                action_input = V2i(1,0);
                            } break;
                            case 1:
                            {
                                action_input = V2i(0,1);
                            } break;
                            case 2:
                            {
                                action_input = V2i(-1,0);
                            } break;
                            case 3:
                            {
                                action_input = V2i(0,-1);
                            } break;
                        }
                        
                        v2i new_pos = simple_gridworld->position + action_input;
                        new_pos.x = Clamp(new_pos.x, 0, 3);
                        new_pos.y = Clamp(new_pos.y, 0, 3);

                        int new_state = 4*new_pos.y + new_pos.x;
                        float new_state_value = simple_gridworld->value_function[new_state];
                        if(new_state_value > max_value)
                        {
                            max_value = new_state_value;
                            max_input = action_input;
                        }
                    }
                    rl_input = max_input;
                }
                else
                {
                    // NOTE(lubo): Random policy
                    switch(get_random_u32_up_to(4, &state->global_entropy))
                    {
                        default:
                        case 0: rl_input = V2i(1,0); break;
                        case 1: rl_input = V2i(0,1); break;
                        case 2: rl_input = V2i(-1,0); break;
                        case 3: rl_input = V2i(0,-1); break;
                    }
                }
            }

            if(rl_input != V2i(0,0))
            {
                rl_simple_gridworld(simple_gridworld, rl_input);
            }

            for(int y=0; y<4; ++y)
            {
                for(int x=0; x<4; ++x)
                {
                    v2 relp = {x/8.0f, y/8.0f};
                    v2 halfdim = {50,50};
                    v2 offset = {300,300};
                    r2 rect = offset_rect(rect_center_halfdim(rect_lerp(screen_rect, relp), halfdim), offset);

                    //v4 color = V2i(x,y) == V2i(3,3) ? V4(1,0,0,1) : simple_gridworld->finished ? V4(0,1,0,1) : V4(1,1,1,1);
                    float state_value = simple_gridworld->value_function[y*4 + x];
                    v4 color = {1,1,0,1};
                    color.g = Clamp(inv_lerp(-6, 0, state_value),0,1);
                    int sprite = (simple_gridworld->position == V2i(x,y)) ? pngs_texture_24px_png : pngs_checkbox0_png;
            
                    debug_immediate_sprite_ui(rect, sprite, color);

                    Baked_Font *font = &state->debug_font;
                    char buffer[8];
                    stbsp_snprintf(buffer, ArrayCount(buffer), "%.4f", state_value);
                    draw_text(font, String(buffer), rect);
                }
            }

        } break;

        case Windy_Gridworld:
        {
            RL_Windy_Gridworld *windy_gridworld = &state->windy_gridworld;
            
            if(GLOBAL_DEBUG_CONTROLLED_BOOL && clock_tick)
            {
                rl_windy_gridworld(windy_gridworld, &state->global_entropy);
            }

            for(int y=0; y<7; ++y)
            {
                for(int x=0; x<10; ++x)
                {
                    v2 relp = {x/14.0f, y/10.0f};
                    v2 halfdim = {50,50};
                    v2 offset = {300,300};
                    r2 rect = offset_rect(rect_center_halfdim(rect_lerp(screen_rect, relp), halfdim), offset);

                    int best_action = 0;
                    float best_q_value = -float_max;
                    float q_values[4];
                    for(int action=0; action<4; ++action)
                    {
                        q_values[action] = windy_gridworld->q_function[7*10*action + 10*y + x];

                        if(q_values[action] > best_q_value)
                        {
                            best_q_value = q_values[action];
                            best_action = action;
                        }
                    }

                    v4 color = {1,1,0,1};
                    color.g = Clamp(inv_lerp(windy_gridworld->min_q, 0, best_q_value),0,1);
                    color.b = V2i(x,y) == V2i(7,3) ? 1.0f : 0.0f;
                    int sprite = (windy_gridworld->position == V2i(x,y)) ? pngs_texture_24px_png : pngs_checkbox0_png;
            
                    debug_immediate_sprite_ui(rect, sprite, color);

                    v2 padding = {10,10};
                    Baked_Font *font = &state->debug_font;
                    char buffer[64];
                    stbsp_snprintf(buffer, ArrayCount(buffer), "%.2f", q_values[0]);
                    draw_text(font, String(buffer), rect, padding, (best_action == 0) ? V4(0,1,0,1) : V4(1,1,1,1));
                    stbsp_snprintf(buffer, ArrayCount(buffer), "\n%.2f", q_values[1]);
                    draw_text(font, String(buffer), rect, padding, (best_action == 1) ? V4(0,1,0,1) : V4(1,1,1,1));
                    stbsp_snprintf(buffer, ArrayCount(buffer), "\n\n%.2f", q_values[2]);
                    draw_text(font, String(buffer), rect, padding, (best_action == 2) ? V4(0,1,0,1) : V4(1,1,1,1));
                    stbsp_snprintf(buffer, ArrayCount(buffer), "\n\n\n%.2f", q_values[3]);
                    draw_text(font, String(buffer), rect, padding, (best_action == 3) ? V4(0,1,0,1) : V4(1,1,1,1));
                }
            }

        } break;

        case Windy_Gridworld_Q:
        {
            RL_Windy_Gridworld *windy_gridworld = &state->windy_gridworld;

            int user_action = -1;
            if(was_pressed(&input->left)) user_action = 2;
            if(was_pressed(&input->right)) user_action = 0;
            if(was_pressed(&input->up)) user_action = 1;
            if(was_pressed(&input->down)) user_action = 3;

            // NOTE(lubo): User policy
            if(user_action != -1)
            {
                rl_windy_gridworld_q_learning(windy_gridworld, user_action, &state->global_entropy);
            }

            // NOTE(lubo): epsilon-Greedy policy
            if(GLOBAL_DEBUG_CONTROLLED_BOOL && clock_tick)
            {
                int policy_action = 0;
                float epsilon = 0.9f;
                if(probability(epsilon, &state->global_entropy))
                {
                    int max_q_count = 0;
                    float max_q = -float_max;
                    for(int action=0; action<4; ++action)
                    {
                        int q_id = 7*10*action + 10*windy_gridworld->position.y + windy_gridworld->position.x;
                        float q = windy_gridworld->q_function[q_id];
                        if(q > max_q)
                        {
                            max_q_count = 1;
                            max_q = q;
                            policy_action = action;
                        }
                        // NOTE(lubo): Uniform distribution on ties - this is just for fun :)
                        else if(q == max_q)
                        {
                            max_q_count += 1;
                            if(probability(1.0f/max_q_count, &state->global_entropy))
                            {
                                policy_action = action;
                            }
                        }
                    }
                }
                else
                {
                    policy_action = get_random_u32_up_to(4, &state->global_entropy);
                }
                
                rl_windy_gridworld_q_learning(windy_gridworld, policy_action, &state->global_entropy);
            }

            for(int y=0; y<7; ++y)
            {
                for(int x=0; x<10; ++x)
                {
                    v2 relp = {x/14.0f, y/10.0f};
                    v2 halfdim = {50,50};
                    v2 offset = {300,300};
                    r2 rect = offset_rect(rect_center_halfdim(rect_lerp(screen_rect, relp), halfdim), offset);

                    int best_action_count = 0;
                    int chosen_action = 0;
                    float best_q_value = -float_max;
                    float q_values[4];
                    for(int action=0; action<4; ++action)
                    {
                        q_values[action] = windy_gridworld->q_function[7*10*action + 10*y + x];

                        if(q_values[action] > best_q_value)
                        {
                            best_q_value = q_values[action];
                            chosen_action = action;
                            best_action_count = 1;
                        }
                        else if(q_values[action] == best_q_value)
                        {
                            best_action_count += 1;
                        }
                    }

                    if(windy_gridworld->position == V2i(x,y))
                    {
                        debug_immediate_sprite_ui(rect, pngs_circle_png, V4(0,0,1,1));
                    }

                    v4 color = {1,1,0,1};
                    color.g = Clamp(inv_lerp(windy_gridworld->min_q, 0, best_q_value),0,1);
                    color.b = V2i(x,y) == V2i(7,3) ? 1.0f : 0.0f;

                    if(best_action_count == 1)
                    {
                        debug_immediate_sprite_ui(rect, pngs_east_24px_png, color, (Pi32/2)*chosen_action);
                    }
                    else
                    {
                        r2 region = {0.25,0.25,0.75,0.75};
                        debug_immediate_sprite_ui(get_subrect(rect, region), pngs_help_24px_png, color, GLOBAL_DEBUG_ANIMATED_FLOAT_B);
                    }
                    
                    v2 padding = {10,10};
                    
                    Baked_Font *font = &state->debug_font;
                    char buffer[64];
                    stbsp_snprintf(buffer, ArrayCount(buffer), "%.2f", q_values[0]);
                    draw_text(font, String(buffer), rect, padding, (q_values[0] == best_q_value) ? V4(0,1,0,1) : V4(1,1,1,1));
                    stbsp_snprintf(buffer, ArrayCount(buffer), "\n%.2f", q_values[1]);
                    draw_text(font, String(buffer), rect, padding, (q_values[1] == best_q_value) ? V4(0,1,0,1) : V4(1,1,1,1));
                    stbsp_snprintf(buffer, ArrayCount(buffer), "\n\n%.2f", q_values[2]);
                    draw_text(font, String(buffer), rect, padding, (q_values[2] == best_q_value) ? V4(0,1,0,1) : V4(1,1,1,1));
                    stbsp_snprintf(buffer, ArrayCount(buffer), "\n\n\n%.2f", q_values[3]);
                    draw_text(font, String(buffer), rect, padding, (q_values[3] == best_q_value) ? V4(0,1,0,1) : V4(1,1,1,1));
                }
            }

        } break;

        case Windy_Gridworld_NN:
        {
            RL_Windy_Gridworld *windy_gridworld = &state->windy_gridworld;


            
            Neural_Network *net = &windy_gridworld->net;
            matrix net_input = net->input;
            matrix net_expected_output = net->expected_output;
            matrix net_prediction = net->prediction;


            
            int user_action = -1;
            if(was_pressed(&input->left)) user_action = 2;
            if(was_pressed(&input->right)) user_action = 0;
            if(was_pressed(&input->up)) user_action = 1;
            if(was_pressed(&input->down)) user_action = 3;

            // NOTE(lubo): User policy
            if(user_action != -1)
            {
                rl_windy_gridworld_nn(windy_gridworld, user_action, &state->global_entropy);
            }

            // NOTE(lubo): epsilon-Greedy policy
            if(GLOBAL_DEBUG_CONTROLLED_BOOL && clock_tick)
            {
                int policy_action = 0;
                float epsilon = 0.9f;
                if(probability(epsilon, &state->global_entropy))
                {
                    int max_q_count = 0;
                    float max_q = -float_max;
                    for(int action=0; action<4; ++action)
                    {
                        *(float *)matrix_access(net_input, 0) = (float)windy_gridworld->position.x;
                        *(float *)matrix_access(net_input, 1) = (float)windy_gridworld->position.y;
                        *(float *)matrix_access(net_input, 2) = (float)(action == 0);
                        *(float *)matrix_access(net_input, 3) = (float)(action == 1);
                        *(float *)matrix_access(net_input, 4) = (float)(action == 2);
                        *(float *)matrix_access(net_input, 5) = (float)(action == 3);
                        feedforward(net);
                        float q = *(float *)matrix_access(net_prediction, 0);
                        
                        if(q > max_q)
                        {
                            max_q_count = 1;
                            max_q = q;
                            policy_action = action;
                        }
                        // NOTE(lubo): Uniform distribution on ties - this is just for fun :)
                        else if(q == max_q)
                        {
                            max_q_count += 1;
                            if(probability(1.0f/max_q_count, &state->global_entropy))
                            {
                                policy_action = action;
                            }
                        }
                    }
                }
                else
                {
                    policy_action = get_random_u32_up_to(4, &state->global_entropy);
                }
                
                rl_windy_gridworld_nn(windy_gridworld, policy_action, &state->global_entropy);
            }

            for(int y=0; y<7; ++y)
            {
                for(int x=0; x<10; ++x)
            // for(int y=0; y<4; ++y)
            // {
            //     for(int x=0; x<4; ++x)
                {
                    v2 relp = {x/14.0f, y/10.0f};
                    v2 halfdim = {50,50};
                    v2 offset = {300,300};
                    r2 rect = offset_rect(rect_center_halfdim(rect_lerp(screen_rect, relp), halfdim), offset);

                    int best_action_count = 0;
                    int chosen_action = 0;
                    float best_q_value = -float_max;
                    float q_values[4];
                    for(int action=0; action<4; ++action)
                    {
                        *(float *)matrix_access(net_input, 0) = (float)x;
                        *(float *)matrix_access(net_input, 1) = (float)y;
                        *(float *)matrix_access(net_input, 2) = (float)(action == 0);
                        *(float *)matrix_access(net_input, 3) = (float)(action == 1);
                        *(float *)matrix_access(net_input, 4) = (float)(action == 2);
                        *(float *)matrix_access(net_input, 5) = (float)(action == 3);
                        feedforward(net);
                        float q = *(float *)matrix_access(net_prediction, 0);
            
                        q_values[action] = q;

                        if(q_values[action] > best_q_value)
                        {
                            best_q_value = q_values[action];
                            chosen_action = action;
                            best_action_count = 1;
                        }
                        else if(q_values[action] == best_q_value)
                        {
                            best_action_count += 1;
                        }
                    }

                    if(windy_gridworld->position == V2i(x,y))
                    {
                        debug_immediate_sprite_ui(rect, pngs_circle_png, V4(0,0,1,1));
                    }

                    v4 color = {1,1,0,1};
                    color.g = Clamp(inv_lerp(windy_gridworld->min_q, 0, best_q_value),0,1);
                    color.b = V2i(x,y) == V2i(7,3) ? 1.0f : 0.0f;

                    if(best_action_count == 1)
                    {
                        debug_immediate_sprite_ui(rect, pngs_east_24px_png, color, (Pi32/2)*chosen_action);
                    }
                    else
                    {
                        r2 region = {0.25,0.25,0.75,0.75};
                        debug_immediate_sprite_ui(get_subrect(rect, region), pngs_help_24px_png, color, GLOBAL_DEBUG_ANIMATED_FLOAT_B);
                    }
                    
                    v2 padding = {10,10};
                    
                    Baked_Font *font = &state->debug_font;
                    char buffer[64];
                    stbsp_snprintf(buffer, ArrayCount(buffer), "%.2f", q_values[0]);
                    draw_text(font, String(buffer), rect, padding, (q_values[0] == best_q_value) ? V4(0,1,0,1) : V4(1,1,1,1));
                    stbsp_snprintf(buffer, ArrayCount(buffer), "\n%.2f", q_values[1]);
                    draw_text(font, String(buffer), rect, padding, (q_values[1] == best_q_value) ? V4(0,1,0,1) : V4(1,1,1,1));
                    stbsp_snprintf(buffer, ArrayCount(buffer), "\n\n%.2f", q_values[2]);
                    draw_text(font, String(buffer), rect, padding, (q_values[2] == best_q_value) ? V4(0,1,0,1) : V4(1,1,1,1));
                    stbsp_snprintf(buffer, ArrayCount(buffer), "\n\n\n%.2f", q_values[3]);
                    draw_text(font, String(buffer), rect, padding, (q_values[3] == best_q_value) ? V4(0,1,0,1) : V4(1,1,1,1));
                }
            }

        } break;
    };
    #endif
}
