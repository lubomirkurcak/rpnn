/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */
#include "shared.h"
#include "idx.h"
#include "cpfloat_binary64.h"
#include "matrix.h"
#include "matrix.cpp"
#include "function_approximator.h"
#include "neural_network.h"
#include "neural_network.cpp"
#include "lookup_table.h"
#include "lookup_table.cpp"

#include "unit_tests.h"

#include "test_mnist.cpp"
//#include "test_factorize.cpp"
#include "test_cartpole.cpp"
#include "test_functionapprox.cpp"


internal void train_mnist_example_config()
{
    // Set neural net parameters
    int layer_sizes[] = {784, 30, 10};
    int layer_count = ArrayCount(layer_sizes);
    int epochs = 30;
    int minibatch_size = 10;

    // Allocate the data structure for target formats and rounding parameters.
    optstruct fpopts = {};
  
    // Set up the parameters for binary16 target format.
    fpopts.precision = 11;                 // Bits in the significand + 1.
    fpopts.emax = 15;                      // The maximum exponent value.
    fpopts.subnormal = CPFLOAT_SUBN_USE;   // Support for subnormals is on.
    fpopts.round = CPFLOAT_RND_TP;        // Round toward +infinity.
    fpopts.flip = CPFLOAT_NO_SOFTERR;      // Bit flips are off.
    fpopts.p = 0;                          // Bit flip probability (not used).
    fpopts.explim = CPFLOAT_EXPRANGE_TARG; // Limited exponent in target format.

    // Validate the parameters in fpopts.
    int retval = cpfloat_validate_optstruct(&fpopts);
    printf("cpfloat_validate_optstruct returned %d\n", retval);

    Neural_Network_Hyperparams params = default_hyperparams();

    int correctly_classified_after_training = 
        train_mnist(params, layer_count, layer_sizes, epochs, minibatch_size);
    printf("correctly_classified_after_training %d\n", correctly_classified_after_training);
}

internal void gym_cartpole_example_setup()
{
    Deep_Q_Network dqn = {};
    gym_cartpole_create(&dqn);
    
    //for(int i=0; i<10000; ++i)
    while(true)
    {
        double observation[] = {get_random_float(), get_random_float(), get_random_float(), get_random_float()};
        double reward = 1.0f;
        b32x done = probability(0.01f);
        int action = gym_cartpole_update(&dqn, observation, reward, done);
        //printf("chose action %d\n", action);
    }
}

internal void sin_approx()
{
    int epochs = 65536;
    //int layer_sizes[] = {1, 64, 32, 16, 1};
    int layer_sizes[] = {1, 16, 16, 16, 1};
    //int layer_sizes[] = {1, 4, 4, 4, 1};
    //int layer_sizes[] = {1, 3, 2, 1};
    Neural_Network_Hyperparams params = default_hyperparams();

    // NOTE(lubo): Approximate the 'sin' function on the interval [-2, 2]
    train_function(params, ArrayCount(layer_sizes), layer_sizes, epochs,
                   -2.0f, 2.0f, sin);
}

int main(int argc, char *argv[])
{
    unit_tests_run();
    
    //train_mnist_example_config();

    gym_cartpole_example_setup();
    
    // sin_approx();

    

    // // TODO(lubo): Test if we get the same distribution for 0 and MAX, and others.
    // float sum = 0;
    // float threshold = 1;
    // while(true)
    // {
    //     sum += get_random_float();
    //     if(sum >=threshold)
    //     {
    //         printf("%f\n", sum);
    //         threshold *= 10;
    //     }
    // }
    
    printf("hi");
}
