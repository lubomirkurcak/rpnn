/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

#if 0
internal void set_values(matrix A, double value)
{
    double *A_values = (double *)A.allocation.memory;
    
    for(int g=0; g<A.features; ++g)
    {
        for(int h=0; h<A.layers; ++h)
        {
            for(int i=0; i<A.height; ++i)
            {
                for(int j=0; j<A.width; ++j)
                {
                    *A_values = value;
                    ++A_values;
                }
            }
        }
    }
    
    Assert(is_well_behaved(A));
}
#endif

// TODO(lubo): Load input/expected output function callbacks?

struct Neural_Network_Hyperparams
{
    double learning_rate;
    double weight_decay;
    double dropout_keep_p;
    double momentum_coefficient;

    optstruct forward_fpopts;
    optstruct backprop_fpopts;
    optstruct update_fpopts;
};

// NOTE(lubo): Default example setting
inline optstruct precision_binary16()
{
    optstruct fpopts = {};
  
    // Set up the parameters for binary16 target format.
    fpopts.precision = 11;                 // Bits in the significand + 1.
    fpopts.emax = 15;                      // The maximum exponent value.
    fpopts.subnormal = CPFLOAT_SUBN_USE;   // Support for subnormals is on.
    fpopts.round = CPFLOAT_RND_TP;        // Round toward +infinity.
    fpopts.flip = CPFLOAT_NO_SOFTERR;      // Bit flips are off.
    fpopts.p = 0;                          // Bit flip probability (not used).
    fpopts.explim = CPFLOAT_EXPRANGE_TARG; // Limited exponent in target format.

    return fpopts;
}

// NOTE(lubo): Another example precision specification (bfloat16)
inline optstruct precision_bfloat16()
{
    optstruct fpopts = {};
  
    fpopts.precision = 8;
    fpopts.emax = 127;
    fpopts.subnormal = CPFLOAT_SUBN_USE;
    fpopts.round = CPFLOAT_RND_SE;
    fpopts.flip = CPFLOAT_NO_SOFTERR;
    fpopts.explim = CPFLOAT_EXPRANGE_TARG;

    return fpopts;
}

// NOTE(lubo): Full precision (no rounding)
inline optstruct precision_full()
{
    optstruct fpopts = {};
    fpopts.round = CPFLOAT_NO_RND;
    fpopts.explim = CPFLOAT_EXPRANGE_TARG;
    return fpopts;
}

inline Neural_Network_Hyperparams default_hyperparams()
{
    Neural_Network_Hyperparams result = {};
    result.learning_rate = 0.01f;
    result.weight_decay = 0.001f;
    result.dropout_keep_p = 1;
    result.momentum_coefficient = 0;
    result.forward_fpopts = precision_full();
    result.backprop_fpopts = precision_full();
    result.update_fpopts = precision_full();

    return result;
}

// NOTE(lubo): Neural network
class Neural_Network : public Function_Approximator
{
public:
    Neural_Network_Hyperparams params;
    
    int layer_count;

    // NOTE(lubo): Last layer can be softmax or no activation
    b32x last_layer_softmax;
    
    list(matrix) activations;
    list(matrix) zs;
    list(matrix) dropouts;
    list(matrix) weights;
    list(matrix) biases;
    list(matrix) del_weights;
    list(matrix) del_biases;
    list(matrix) avg_del_weights;
    list(matrix) avg_del_biases;
    list(matrix) vel_weights;
    list(matrix) vel_biases;

    matrix evaluate_input();
    void learn_from_expected_output(int minibatch_size);
};

// NOTE(lubo): Expect 1 at 'class_index', and 0 everywhere else
inline void set_expected_one_hot(Neural_Network *net, int class_index)
{
    clear(net->expected_output);
    *matrix_access(net->expected_output, class_index) = 1;
}
