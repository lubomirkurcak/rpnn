/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

#define DRAW_FUNCTION_CHAR_WIDTH 80
#define DRAW_FUNCTION_CHAR_HEIGHT 20

internal void draw_function(int *y_values)
{
    for(int y=DRAW_FUNCTION_CHAR_HEIGHT-1; y>=0; --y)
    {
        for(int x=0; x<DRAW_FUNCTION_CHAR_WIDTH; ++x)
        {
            if(y == y_values[x])
            {
                printf("x");
            }
            else
            {
                printf(" ");
            }
        }
        printf("|\n");
    }
}

inline void draw_function(r2 domain, float f(float), Neural_Network *net=0)
{
    int y_values[DRAW_FUNCTION_CHAR_WIDTH];
    for(int x=0; x<DRAW_FUNCTION_CHAR_WIDTH; ++x)
    {
        float xt = (float)x/(float)(DRAW_FUNCTION_CHAR_WIDTH-1);
        float x_cont = lerp(domain.min.x, domain.max.x, xt);

        float y_cont;
        if(f)
        {
            y_cont = f(x_cont);
        }
        else
        {
            double x_cont_double = (double)x_cont;
            y_cont = (float)matrix_get(net->evaluate(x_cont_double), 0);
        }
        
        float yt = inv_lerp(domain.min.y, domain.max.y, y_cont);
        y_values[x] = (int)(DRAW_FUNCTION_CHAR_HEIGHT*yt);
    }

    draw_function(y_values);
}

internal void train_function(
    Neural_Network_Hyperparams params,
    int layer_count, int *layer_sizes,
    int epochs = 65536,
    float range_min = -2,
    float range_max = 2,
    float f1(float) = 0,
    float f2(float,float) = 0)
{
    Assert(layer_sizes[0] == 1);
    Assert(layer_sizes[layer_count - 1] == 1);
    
    Neural_Network _net = create_feedforward_net(params, layer_count, layer_sizes);
    Neural_Network *net = &_net;

    r2 domain = {};
    domain.min.x = range_min;
    domain.max.x = range_max;
    domain.min.y = -1;
    domain.max.y = 1;
    
    //draw_function(domain, f1);
    
    for(int epoch=0; epoch<epochs; ++epoch)
    {
        double value = (double)lerp(range_min, range_max, get_random_float());
        double expected = (double)f1((float)value);
        
        matrix predicted = net->evaluate(value);
        net->learn(expected, 1);

        b32x print_this_batch = (epoch % 16384) == 0;
        if(print_this_batch)
        {
            double current_cost = calculate_cost(net);
            #ifdef PY_SSIZE_T_CLEAN
            PySys_WriteStdout("Epoch %d/%d Cost %f LR %f\n", epoch, epochs, current_cost, net->params.learning_rate);
            #endif
            loginfo("neural_net", "Epoch %d/%d Cost %f LR %f", epoch, epochs, current_cost, net->params.learning_rate);
            draw_function(domain, 0, net);
        }
    }
    
    {
        double current_cost = calculate_cost(net);
#ifdef PY_SSIZE_T_CLEAN
        PySys_WriteStdout("Final Cost %f\n", current_cost);
#endif
        loginfo("neural_net", "Final Cost %f", current_cost);
        draw_function(domain, 0, net);

        // NOTE(lubo): 3000+ runs converged using this threshold
        Assert(current_cost < 0.01f);
    }
}
