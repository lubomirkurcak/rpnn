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
#include "neural_network.h"
#include "neural_network.cpp"

global buffer global_debug_mnist_test_bitmap_buffer;
global Loaded_Bitmap global_debug_mnist_test_bitmap;

internal int neural_net_test(
    optstruct *fpopts,
    int layer_count, int *layer_sizes,
    int epochs = 30,
    int minibatch_size = 10,
    float learning_rate = 0.1f,
    float weight_decay = 0,
    float dropout = 0,
    float momentum_coefficient = 0)
{
    buffer train_set_file = read_entire_file(String("data\\mnist\\train-images.idx3-ubyte"));
    buffer train_labels_file = read_entire_file(String("data\\mnist\\train-labels.idx1-ubyte"));
    buffer test_set_file = read_entire_file(String("data\\mnist\\t10k-images.idx3-ubyte"));
    buffer test_labels_file = read_entire_file(String("data\\mnist\\t10k-labels.idx1-ubyte"));
    
    LK_IDX_File train_set = process_idx_file(train_set_file);
    LK_IDX_File train_labels = process_idx_file(train_labels_file);
    LK_IDX_File test_set = process_idx_file(test_set_file);
    LK_IDX_File test_labels = process_idx_file(test_labels_file);

    // NOTE(lubo): Load one of the pictures into displayable texture
    {
        int display_element_id = 123;
        void *display_element_data = idx_get_element(train_set, display_element_id);
    
        global_debug_mnist_test_bitmap.width = train_set.dimension_sizes[1];
        global_debug_mnist_test_bitmap.height = train_set.dimension_sizes[2];
        global_debug_mnist_test_bitmap.channels = 4;
        global_debug_mnist_test_bitmap_buffer = general_allocate(global_debug_mnist_test_bitmap.width*global_debug_mnist_test_bitmap.height*global_debug_mnist_test_bitmap.channels);
        global_debug_mnist_test_bitmap.data = global_debug_mnist_test_bitmap_buffer.memory;
    
        u32 *write = (u32 *)global_debug_mnist_test_bitmap.data;
        for(int y=0; y<global_debug_mnist_test_bitmap.height; ++y)
        {
            u8 *read = (u8 *)display_element_data + (global_debug_mnist_test_bitmap.height-y-1)*train_set.type_size*global_debug_mnist_test_bitmap.width;
            for(int x=0; x<global_debug_mnist_test_bitmap.width; ++x)
            {
                u32 value = *read;
                #if 0
                *write = (value<<24 | value<<16 | value<<8 | value<<0);
                #else
                *write = (255<<24 | value<<16 | value<<8 | value<<0);
                #endif

                read++;
                write++;
            }
        }
    }

    #if false
    // NOTE(lubo): These settings get about 94.5% on MNIST
    //int layer_sizes[] = {train_set.element_size, 30, 10};
    int epochs = 30;
    int minibatch_size = 10;
    float learning_rate = 0.1f;    
    float weight_decay = 5.0f/train_set.elements;
    float dropout = 0;
    float momentum_coefficient = 0;

    epochs = 1;
    #endif
    
    
    // learning_rate = 0.01f;
    // epochs = 60;
    // momentum_coefficient = 0.05f;
    
    #if false
    // NOTE(lubo): Precision reduce options
    optstruct *fpopts = init_optstruct();
  
    // Set up the parameters for binary16 target format.
    fpopts->precision = 11;                 // Bits in the significand + 1.
    fpopts->emax = 15;                      // The maximum exponent value.
    fpopts->subnormal = CPFLOAT_SUBN_USE;   // Support for subnormals is on.
    fpopts ->round = CPFLOAT_RND_TP;        // Round toward +infinity.
    fpopts->flip = CPFLOAT_NO_SOFTERR;      // Bit flips are off.
    fpopts->p = 0;                          // Bit flip probability (not used).
    fpopts->explim = CPFLOAT_EXPRANGE_TARG; // Limited exponent in target format.
  
    // Validate the parameters in fpopts.
    int retval = cpfloat_validate_optstruct(fpopts);
    PySys_WriteStdout("The validation function returned %d.\n", retval);
    #endif

    optstruct *forward_fpopts = fpopts;
    optstruct *backprop_fpopts = fpopts;
    optstruct *update_fpopts = fpopts;
    
    Neural_Network _net = neural_net_parametrized(layer_count, layer_sizes, learning_rate, weight_decay, dropout, /*idx.elements, */momentum_coefficient,
                                                  forward_fpopts, backprop_fpopts, update_fpopts);
    Neural_Network *net = &_net;

    //save(net);
    
    int correctly_classified_after_training = train_idx(net,
              train_set, train_labels,
              test_set, test_labels,
              epochs, minibatch_size);

    incinerate(net);
              
    return correctly_classified_after_training;
}

#include "unit_tests.h"

int main(int argc, char *argv[])
{
    unit_tests_run();
    
    // Set neural net parameters
    int layer_sizes[] = {784, 30, 10};
    int layer_count = ArrayCount(layer_sizes);
    int epochs = 30;
    int minibatch_size = 10;
    float learning_rate = 0.1f;    
    float weight_decay = 0.0001f; //5.0f/train_set.elements;
    float dropout = 0;
    float momentum_coefficient = 0;

    // Allocate the data structure for target formats and rounding parameters.
    optstruct *fpopts = init_optstruct();
  
    // Set up the parameters for binary16 target format.
    fpopts->precision = 11;                 // Bits in the significand + 1.
    fpopts->emax = 15;                      // The maximum exponent value.
    fpopts->subnormal = CPFLOAT_SUBN_USE;   // Support for subnormals is on.
    fpopts->round = CPFLOAT_RND_TP;        // Round toward +infinity.
    fpopts->flip = CPFLOAT_NO_SOFTERR;      // Bit flips are off.
    fpopts->p = 0;                          // Bit flip probability (not used).
    fpopts->explim = CPFLOAT_EXPRANGE_TARG; // Limited exponent in target format.

    // Validate the parameters in fpopts.
    int retval = cpfloat_validate_optstruct(fpopts);
    printf("cpfloat_validate_optstruct returned %d\n", retval);
  

    int correctly_classified_after_training = 
        neural_net_test(fpopts, layer_count, layer_sizes, epochs, minibatch_size, learning_rate, weight_decay, dropout, momentum_coefficient);
    printf("correctly_classified_after_training %d\n", correctly_classified_after_training);

    free_optstruct(fpopts);
}
