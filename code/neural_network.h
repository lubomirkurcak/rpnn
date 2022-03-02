/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

// TODO(lubo): There is a lot of loops over all values, maybe that can be abstracted too?
// TODO(lubo): Multithreading API
// TODO(lubo): Transpose flag and when multiplying CHOOSE SMART CACHE AWARE OOGA BOOGA
// NOTE(lubo): Generic matrix
struct matrix
{
    // NOTE(lubo): Indices change from slowest (left) to fastest (right):
    //             [features][layers][height][width]
    int height;
    int width;
    int layers;
    // NOTE(lubo): CNN layers usually have multiple kernel of same
    // size for different "features," and currently this doesn't have
    // meaning in other contexts
    int features;
    buffer allocation;
};

internal void print_matrix(matrix A)
{
    Assert(A.layers == 1);
    Assert(A.features == 1);
    double *A_values = (double *)A.allocation.memory;
    
    for(int i=0; i<A.height; ++i)
    {
        for(int j=0; j<A.width; ++j)
        {
            printf("%3.1lf ", *A_values);
            ++A_values;
        }
        printf("\n");
    }
}

// STUDY NOTE(lubo):
// It would be kinda nice to have a regular matrix struct because it could be used very generally
// For neural nets we need specific extensions (matrix layers + arrays of them)
// so there is huge overlap. Due to this overlap for simplicity we use single unified structure.
// After thinking about relation general matrix is simply this "extended matrix" when
// layers = 1 and features = 1. It would be cool to have a language feature to bake constant values
// to create a new struct without these fields and create function versions for it.
// However maybe we would want to extend matrix instead (OOP actually good? :O)
// How would that work in an ideal language?
// 
// struct convolution_matrix
// {
//     // NOTE(lubo): Indices change from slowest (left) to fastest (right):
//     //             [features][layers][height][width]
//     int height;
//     int width;
//     int layers;
//     // NOTE(lubo): CNN layers usually have multiple kernel of same
//     // size for different "features," and currently this doesn't have
//     // meaning in other contexts
//     int features;
//     buffer allocation;
// }

inline void incinerate(matrix A)
{
    general_deallocate(A.allocation);
    A.allocation = {};
}

inline void *matrix_access(matrix A, int index)
{
    void *result = (double *)A.allocation.memory + index;
    return result;
};

inline void *matrix_access(matrix A, int h, int w, int layer=0, int feature=0)
{
    Assert(0 <= w && w < A.width);
    Assert(0 <= h && w < A.height);
    Assert(0 <= layer && layer < A.layers);
    Assert(0 <= feature && feature < A.features);
    
    void *result = (double *)A.allocation.memory +
        feature * A.layers*A.height*A.width +
        layer * A.height*A.width +
        h * A.width +
        w;
    return result;
};

internal matrix create_matrix(int height, int width, int layers=1, int features=1)
{
    matrix result = {};
    result.height = height;
    result.width = width;
    result.layers = layers;
    result.features = features;
    result.allocation = general_allocate_array(double, features*layers*height*width);
    return result;
}

internal matrix create_matrix(matrix A)
{
    matrix result = create_matrix(A.height, A.width, A.layers, A.features);
    return result;
}

internal void copy_matrix(matrix R, matrix A)
{
    Assert(A.height == R.height);
    Assert(A.width == R.width);
    Assert(A.layers == R.layers);
    Assert(A.features == R.features);
    Assert(A.allocation.size == R.allocation.size);
    
    memcpy(R.allocation.memory, A.allocation.memory, A.allocation.size);
}

internal matrix copy_matrix(matrix A)
{
    matrix result = create_matrix(A);
    copy_matrix(result, A);
    return result;
}

// NOTE(lubo): aka contravariant vector, aka vector
internal matrix create_column_vector(int height)
{
    matrix result = create_matrix(height,1);
    return result;
}

internal matrix flatten(matrix A)
{
    // TODO(lubo): Make sure we don't overflow?
    smm size = (smm)A.features*(smm)A.layers*(smm)A.height*(smm)A.width;
    Assert(size <= int_max);

    matrix R = A;
    R.height = (int)size; // TODO(lubo): Safecast functions?
    R.width = 1;
    R.layers = 1;
    R.features = 1;

    return R;
}

internal matrix transpose_vector(matrix A)
{
    Assert(A.height == 1 || A.width == 1);
    Assert(A.layers == 1); // NOTE(lubo): Actually this could be >1 but whatever
    Assert(A.features == 1); // NOTE(lubo): Actually this could also be >1 but whatever
    

    matrix result = A;
    result.height = A.width;
    result.width = A.height;

    return result;
}

// NOTE(lubo): Adapted cache-oblivious transpose by mariolpantunes
// from https://stackoverflow.com/questions/5200338/a-cache-efficient-matrix-transpose-program
inline void cachetranspose(int rb, int re, int cb, int ce,
                           double *T_values, double *A_values,
                           int A_rows, int A_columns) {
    int r = re - rb, c = ce - cb;
    if (r <= 16 && c <= 16) {
        for (int i = rb; i < re; i++) {
            for (int j = cb; j < ce; j++) {
                T_values[j * A_rows + i] = A_values[i * A_columns + j];
            }
        }
    } else if (r >= c) {
        cachetranspose(rb, rb + (r / 2), cb, ce, T_values, A_values, A_rows, A_columns);
        cachetranspose(rb + (r / 2), re, cb, ce, T_values, A_values, A_rows, A_columns);
    } else {
        cachetranspose(rb, re, cb, cb + (c / 2), T_values, A_values, A_rows, A_columns);
        cachetranspose(rb, re, cb + (c / 2), ce, T_values, A_values, A_rows, A_columns);
    }
}

internal void transpose(matrix R, matrix A)
{
    Assert(R.width == A.height);
    Assert(R.height == A.width);
    Assert(A.layers == 1);
    Assert(A.features == 1);
    Assert(R.layers == 1);
    Assert(R.features == 1);
    cachetranspose(0, A.height, 0, A.width,
                   (double *)R.allocation.memory, (double *)A.allocation.memory,
                   A.height, A.width);
}

internal matrix transpose(matrix A)
{
    matrix R = create_matrix(A.width, A.height);
    transpose(R, A);
    return R;
};

// NOTE(lubo): Creates a matrix w such that multiplication w*A is
// defined and will produce matrix of the same size as B.
// Useful for creating fully connected layers in neural networks.
// NOTE(lubo): Conformable is the technical term for suitable matrix
// for certain operation
internal matrix create_connection_matrix(matrix A, matrix B)
{
    Assert(A.layers == B.layers);
    Assert(A.features == B.features);
    Assert(A.layers == 1); // NOTE(lubo): Allow this explicitely once there is a need for it
    Assert(A.features == 1); // NOTE(lubo): Allow this explicitely once there is a need for it
    int layers = A.layers;
    int features = A.features;
    matrix result = create_matrix(B.height, A.height, layers, features);
    return result;
}

internal matrix create_matrix_with_dimension_of_product(matrix A, matrix B)
{
    Assert(A.layers == B.layers);
    Assert(A.features == B.features);
    Assert(A.layers == 1); // NOTE(lubo): Allow this explicitely once there is a need for it
    Assert(A.features == 1); // NOTE(lubo): Allow this explicitely once there is a need for it
    matrix result = create_matrix(A.height, B.width);
    return result;
}

// IMPORTANT NOTE(lubo): padding, stride, dilation, etc only affect 2D
// layers as that is the expectation for usual neural nets
// TODO(lubo): Utility function to automatically choose padding that preserves dimensions
internal matrix create_convolution_result_matrix(matrix A, matrix B, int padding=0, int stride=1, int dilation=1)
{
    Assert(A.features == 1);
    
    int source_h = A.height + 2*padding;
    int source_w = A.width + 2*padding;
    int source_d = A.layers;
    int kernel_h = B.height + (B.height - 1)*(dilation - 1);
    int kernel_w = B.width + (B.width - 1)*(dilation - 1);
    int kernel_d = B.layers;
    
    Assert(source_h >= kernel_h);
    Assert(source_w >= kernel_w);
    Assert(source_d == kernel_d);
    
    Assert((((source_h - kernel_h)%stride) == 0));
    Assert((((source_w - kernel_w)%stride) == 0));

    int height = (source_h - kernel_h)/stride + 1;
    int width = (source_w - kernel_w)/stride + 1;
    int layers = B.features;
    
    matrix result = create_matrix(height, width, layers);
    return result;
}

internal matrix create_pooling_result_matrix(matrix A, int size=2)
{
    Assert((A.height % size) == 0);
    Assert((A.width % size) == 0);
    Assert(A.features == 1);
    
    matrix result = create_matrix(A.height/size, A.width/size, A.layers);
    return result;
}

internal b32x is_well_behaved(matrix A)
{
    b32x well_behaved = true;
    
    double *A_values = (double *)A.allocation.memory;
    for(int g=0; g<A.features; ++g)
    {
        for(int h=0; h<A.layers; ++h)
        {
            for(int i=0; i<A.height; ++i)
            {
                for(int j=0; j<A.width; ++j)
                {
                    double value = *A_values;

                    well_behaved &= value == value;
                    well_behaved &= -float64_max <= value && value <= float64_max;
            
                    ++A_values;
                }
            }
        }
    }

    Assert((smm)A.features * (smm)A.layers * (smm)A.height * (smm)A.width * (smm)sizeof(double) == A.allocation.size);
    
    return well_behaved;
}

internal int
argmax(matrix A)
{
    Assert(A.features == 1);
    Assert(A.layers == 1);
    
    double *A_values = (double *)A.allocation.memory;

    int argmax = 0;
    double max = -float64_max;

    int arg = 0;
    for(int i=0; i<A.height; ++i)
    {
        for(int j=0; j<A.width; ++j)
        {
            if(*A_values > max)
            {
                max = *A_values;
                argmax = arg;
            }

            ++arg;
            ++A_values;
        }
    }

    return argmax;
}

// TODO(lubo): Move this to math?
inline double move_towards_zero(double x, double step)
{
    double result = 0;
    if(x > step)
    {
        result = x - step;
    }
    else if(x < -step)
    {
        result = x + step;
    }

    return result;
}

#if 0
inline double reduce_precision(float value)
{
    float precision_multiplier = powf(0.5f, 10);
    float inv_precision_multiplier = 1/precision_multiplier;
    float result = inv_precision_multiplier*round(precision_multiplier*value);
    return result;
}

// NOTE(lubo): Latex equation: \exp\left(\frac{\operatorname{round}\left(2^{a}\ln x\right)}{2^{a}}\right)
inline float reduce_precision_log(float value, float precision_multiplier)
{
    float64 precision_multiplier64 = (float64)precision_multiplier;
    float64 inv_precision_multiplier64 = 1/precision_multiplier64;
    
    float result = 0;
    if(value == 0)
    {
        result = 0;
    }
    else if(value > 0)
    {
        result = (float)exp2(inv_precision_multiplier64*round(precision_multiplier64*log2((double)value)));
    }
    else
    {
        result = -(float)exp2(inv_precision_multiplier64*round(precision_multiplier64*log2(-(double)value)));
    }
    
    return result;
}
#endif

enum Matrix_Ops
{
    MOP_SET,
    MOP_ADD,
    MOP_SUBTRACT,
    MOP_MULTIPLY,
    MOP_DIVIDE,
    MOP_MODULO,
    MOP_MAX,
    MOP_MIN,
    MOP_GT,
    MOP_LT,
    MOP_EQ,
    MOP_GTE,
    MOP_LTE,
    MOP_ABS,

    MOP_EXP,
    MOP_LN,
    
    MOP_FUNCTION,
    MOP_FUNCTION_PARAM,

    MOP_MOVE_TOWARDS_ZERO,

    MOP_L1,
    MOP_L2,
    MOP_SUM,

    #if 0
    // NOTE(lubo): Simulates reduced precision
    MOP_CHOP_ROUND,
    MOP_CHOP_CEIL,
    MOP_CHOP_FLOOR,
    MOP_CHOP_TRUNCATE,
    MOP_CHOP_STOCHASTIC_PROPORTIONAL,
    MOP_CHOP_STOCHASTIC_EQUAL,
    #endif
};

double default_function(double x) {return x;}
//double default_function2(double x, double y) {return x+y;}
double default_function_param(double x, double param) {return x;}

internal void
scalar_op(matrix R, matrix A, int op, double param=0,
          double function(double)=default_function,
          double function_param(double,double)=default_function_param) //, double function2(double, double)=default_function2)
{
    Assert(A.features == 1);
    Assert(A.layers == 1);
    Assert(R.features == 1);
    Assert(R.layers == 1);
    Assert(A.height == R.height);
    Assert(A.width == R.width);
    
    double *R_values = (double *)R.allocation.memory;
    double *A_values = (double *)A.allocation.memory;

    if(op == MOP_DIVIDE)
    {
        op = MOP_MULTIPLY;
        param = 1/param;
    }
    
    for(int i=0; i<R.height; ++i)
    {
        for(int j=0; j<R.width; ++j)
        {
            switch(op)
            {
                case MOP_SET: *R_values = param; break;
                case MOP_ADD: *R_values = *A_values + param; break;
                case MOP_SUBTRACT: *R_values = *A_values - param; break;
                case MOP_MULTIPLY: *R_values = *A_values * param; break;
                case MOP_DIVIDE: InvalidCodePath;
                case MOP_MODULO: *R_values = modulo(*A_values, param); break;
                case MOP_MAX: *R_values = Max(*A_values, param); break;
                case MOP_MIN: *R_values = Min(*A_values, param); break;
                case MOP_GT: *R_values = (*A_values > param) ? (double)1 : (double)0; break;
                case MOP_LT: *R_values = (*A_values < param) ? (double)1 : (double)0; break;
                case MOP_EQ: *R_values = (*A_values == param) ? (double)1 : (double)0; break;
                case MOP_GTE: *R_values = (*A_values >= param) ? (double)1 : (double)0; break;
                case MOP_LTE: *R_values = (*A_values <= param) ? (double)1 : (double)0; break;
                case MOP_ABS: *R_values = Abs(*A_values); break;

                case MOP_EXP: *R_values = exp(*A_values); break;
                case MOP_LN: *R_values = log(*A_values); break;
                    
                case MOP_FUNCTION: *R_values = function(*A_values); break;
                case MOP_FUNCTION_PARAM: *R_values = function_param(*A_values, param); break;

                case MOP_MOVE_TOWARDS_ZERO: *R_values = move_towards_zero(*A_values, param); break;

                // case MOP_CHOP: *R_values = *A_values
                    
                InvalidDefaultCase;
            }

            ++R_values;
            ++A_values;
        }
    }

    Assert(is_well_behaved(R));
}

internal void
elementwise_op(matrix R, matrix A, matrix B, int op)
{
    Assert(A.features == 1);
    Assert(A.layers == 1);
    Assert(R.features == 1);
    Assert(R.layers == 1);
    Assert(A.height == B.height);
    Assert(A.width == B.width);
    Assert(A.height == R.height);
    Assert(A.width == R.width);

    double *R_values = (double *)R.allocation.memory;
    double *A_values = (double *)A.allocation.memory;
    double *B_values = (double *)B.allocation.memory;
    
    for(int i=0; i<R.height; ++i)
    {
        for(int j=0; j<R.width; ++j)
        {
            switch(op)
            {
                case MOP_ADD: *R_values = *A_values + *B_values; break;
                case MOP_SUBTRACT: *R_values = *A_values - *B_values; break;
                case MOP_MULTIPLY: *R_values = *A_values * *B_values; break;
                case MOP_DIVIDE: *R_values = *A_values / *B_values; break;
                case MOP_MAX: *R_values = Max(*A_values, *B_values); break;
                case MOP_MIN: *R_values = Min(*A_values, *B_values); break;

                InvalidDefaultCase;
            }

            ++R_values;
            ++A_values;
            ++B_values;
        }
    }
    
    Assert(is_well_behaved(R));
}

internal void
add(matrix R, matrix A, matrix B)
{
    elementwise_op(R, A, B, MOP_ADD);
}

internal void
subtract(matrix R, matrix A, matrix B)
{
    elementwise_op(R, A, B, MOP_SUBTRACT);
}

internal void
hadamard(matrix R, matrix A, matrix B)
{
    elementwise_op(R, A, B, MOP_MULTIPLY);
}

internal void
hadamard(matrix R, matrix A)
{
    hadamard(R, R, A);
};

internal void
add(matrix A, matrix B)
{
    add(A, A, B);
}

internal matrix
operator+(matrix A, matrix B)
{
    matrix R = create_matrix(A.height, A.width);
    add(R, A, B);

    return R;
}

#if 0
internal void
reduce_precision_log(matrix R, matrix A, double precision_param)
{
    scalar_op(R, A, MOP_FUNCTION_PARAM, precision_param, 0, reduce_precision_log);
}
#endif

internal void
multiply(matrix R, matrix A, double scalar)
{
    scalar_op(R, A, MOP_MULTIPLY, scalar);
}

internal void
multiply(matrix A, double scalar)
{
    multiply(A, A, scalar);
}

internal void
multiply(matrix R, matrix A, matrix B)
{
    Assert(A.features == 1);
    Assert(A.layers == 1);
    Assert(B.features == 1);
    Assert(B.layers == 1);
    Assert(R.features == 1);
    Assert(R.layers == 1);
    Assert(A.width == B.height);
    int dim = A.width;

    double *R_values = (double *)R.allocation.memory;
    double *A_values = (double *)A.allocation.memory;
    double *B_values = (double *)B.allocation.memory;
    
    for(int i=0; i<A.height; ++i)
    {
        for(int j=0; j<B.width; ++j)
        {
            R_values[i*R.width + j] = 0;
            for(int d=0; d<dim; ++d)
            {
                R_values[i*R.width + j] += A_values[i*A.width + d]*B_values[d*B.width + j];
            }
        }
    }
    
    Assert(is_well_behaved(R));
}

internal void
convolution(matrix R, matrix A, matrix B, int padding=0, int stride=1, int dilation=1)
{
    Assert(A.features == 1);
    Assert(R.features == 1);
    Assert(A.height >= B.height);
    Assert(A.width >= B.width);
    Assert((A.height - B.height)%stride == 0);
    Assert((A.width - B.width)%stride == 0);
    Assert(R.height == (A.height - B.height)/stride + 1);
    Assert(R.width == (A.width - B.width)/stride + 1);
    Assert(A.layers == B.layers);
    Assert(R.layers == B.features);

    double *R_values = (double *)R.allocation.memory;
    double *A_values = (double *)A.allocation.memory;
    
    for(int h=0; h<R.layers; ++h)
    {
        for(int i=0; i<R.height; ++i)
        {
            for(int j=0; j<R.width; ++j)
            {
                *R_values = 0;

                int feature_index = h;
                double *B_values = (double *)B.allocation.memory + feature_index*B.layers*B.height*B.width;
                for(int z=0; z<B.layers; ++z)
                {
                    for(int y=0; y<B.height; ++y)
                    {
                        for(int x=0; x<B.width; ++x)
                        {
                            int A_y = stride*i + dilation*y - padding;
                            int A_x = stride*j + dilation*x - padding;
                            int A_z = z;

                            double A_value = 0;
                            if(A_y >= 0 && A_x >= 0 && A_y < A.height && A_x < A.width)
                            {
                                A_value = A_values[A_z*A.height*A.width + A_y*A.width + A_x];
                            }
                    
                            *R_values += A_value * *B_values;
                    
                    
                            ++B_values;
                        }
                    }
                }
            
                ++R_values;
            }
        }
    }
}

internal matrix
convolution(matrix A, matrix B, int padding=0, int stride=1, int dilation=1)
{
    matrix R = create_convolution_result_matrix(A, B, padding, stride, dilation);
    convolution(R, A, B, padding, stride, dilation);
    return R;
}

// TODO(lubo): Option to choose random value instead of max
internal void
pooling(matrix R, matrix A, int size=2)
{
    Assert(size*R.height == A.height);
    Assert(size*R.width == A.width);
    Assert(R.layers == A.layers);
    Assert(A.features == 1);
    Assert(R.features == 1);

    double *R_values = (double *)R.allocation.memory;
    
    for(int h=0; h<R.layers; ++h)
    {
        int layer = h;
        double *A_values = (double *)A.allocation.memory + layer*A.height*A.width;
        for(int i=0; i<R.height; ++i)
        {
            for(int j=0; j<R.width; ++j)
            {
                double value = -float64_max;
            
                for(int y=0; y<size; ++y)
                {
                    for(int x=0; x<size; ++x)
                    {
                        value = Max(value, A_values[(size*i + y)*A.width + size*j + x]);
                    }
                }
            
                *R_values++ = value;
            }
        }
    }
}

internal matrix
pooling(matrix A, int size=2)
{
    matrix R = create_pooling_result_matrix(A, size);
    pooling(R, A, size);
    return R;
}

internal matrix
operator*(matrix A, matrix B)
{
    matrix R = create_matrix(A.height, B.width);
    multiply(R, A, B);

    return R;
}

enum Initialization_Techniques
{
    NNIT_Sigmoid,
    NNIT_ReLU,
    NNIT_Tanh,
};

internal void randomize_values(Random_Series *series, matrix A, int initialization_technique=NNIT_ReLU)
{
    double *A_values = (double *)A.allocation.memory;

    double sigma = 1;
    int input_neurons = A.width;
    switch(initialization_technique)
    {
        default:
        {
            sigma = square_root(1/(float)input_neurons);
        } break;
            
        case NNIT_ReLU:
        {
            // NOTE(lubo): ReLU activation according to He et al., 2015
            // This should make output layer variances approximately 1,
            // which mitigates chances for exploding/vanishing gradients
            sigma = square_root(2/(float)input_neurons);
        } break;
    }
    
    for(int g=0; g<A.features; ++g)
    {
        for(int h=0; h<A.layers; ++h)
        {
            for(int i=0; i<A.height; ++i)
            {
                for(int j=0; j<A.width; ++j)
                {
                    // TODO(lubo): We can afford to initialize with more precise function
                    *A_values = (double)normal_distribution((float)sigma, 0, series);
            
                    ++A_values;
                }
            }
        }
    }
    
    Assert(is_well_behaved(A));
}

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

internal void clear(matrix A)
{
    memset(A.allocation.memory, 0, A.allocation.size);
}

internal b32x
is_row_all_zeroes(matrix R, int row_index)
{
    Assert(R.features == 1);
    Assert(R.layers == 1);
    Assert(0 <= row_index && row_index < R.height);

    b32x result = true;
    
    for(int i=0; i<R.width; ++i)
    {
        double *value = (double *)matrix_access(R, row_index, i);
        result &= *value == 0;
    }

    return result;
}

internal b32x
is_column_all_zeroes(matrix R, int column_index)
{
    Assert(R.features == 1);
    Assert(R.layers == 1);
    Assert(0 <= column_index && column_index < R.width);

    b32x result = true;
    
    for(int i=0; i<R.height; ++i)
    {
        double *value = (double *)matrix_access(R, i, column_index);
        result &= *value == 0;
    }

    return result;
}

internal b32x
is_any_row_all_zeroes(matrix R)
{
    Assert(R.features == 1);
    Assert(R.layers == 1);

    b32x result = false;
    
    for(int row_index=0; row_index<R.height; ++row_index)
    {
        result |= is_row_all_zeroes(R, row_index);
    }

    return result;
}

internal b32x
is_any_column_all_zeroes(matrix R)
{
    Assert(R.features == 1);
    Assert(R.layers == 1);

    b32x result = false;
    
    for(int column_index=0; column_index<R.width; ++column_index)
    {
        result |= is_column_all_zeroes(R, column_index);
    }

    return result;
}

internal b32x
is_any_column_or_row_all_zeroes(matrix R)
{
    Assert(R.features == 1);
    Assert(R.layers == 1);

    b32x result = is_any_row_all_zeroes(R) || is_any_column_all_zeroes(R);
    return result;
}

internal b32x
is_zeroes(matrix R)
{
    Assert(R.features == 1);
    Assert(R.layers == 1);

    b32x result = true;
    double *R_values = (double *)R.allocation.memory;
    for(int i=0; i<R.height; ++i)
    {
        for(int j=0; j<R.width; ++j)
        {
            result &= *R_values == 0;

            ++R_values;
        }
    }

    return result;
}

internal double reduce(matrix A, int op)
{
    Assert(A.features == 1);
    Assert(A.layers == 1);
    
    double *A_values = (double *)A.allocation.memory;

    double result = 0;
    switch(op)
    {
        case MOP_MAX: result = -float64_max; break;
        case MOP_MIN: result = float64_max; break;
    };
    
    
    for(int i=0; i<A.height; ++i)
    {
        for(int j=0; j<A.width; ++j)
        {
            double a = *A_values++;
            
            switch(op)
            {
                case MOP_SUM: result += a; break;
                case MOP_L1: result += Abs(a); break;
                case MOP_L2: result += a*a; break;
                case MOP_MAX: result = Max(result, a); break;
                case MOP_MIN: result = Min(result, a); break;

                InvalidDefaultCase;
            }
        }
    }

    return result;
}

internal double matrix_max(matrix A)
{
    double result = reduce(A, MOP_MAX);
    return result;
}

internal double matrix_sum(matrix A)
{
    double result = reduce(A, MOP_SUM);
    return result;
}

internal bool matrices_are_equal(matrix A, matrix B)
{
    bool result =
        A.width == B.width &&
        A.height == B.height &&
        A.layers == B.layers &&
        A.features == B.features;
    
    if(result)
    {
        Assert(A.allocation.size == B.allocation.size);
        result = (memcmp(A.allocation.memory, B.allocation.memory, A.allocation.size) == 0);
    }
    else
    {
        Assert(A.allocation.size != B.allocation.size);
    }

    return result;
}

internal double matrix_l2(matrix A)
{
    double result = reduce(A, MOP_L2);
    return result;
}

internal void matrix_normalize(matrix A)
{
    double s = matrix_sum(A);
    multiply(A, 1/s);
}

internal void relu(matrix R, matrix A)
{
    scalar_op(R, A, MOP_MAX, 0);
}

internal void relu(matrix A)
{
    relu(A, A);
}

internal void drelu(matrix R, matrix A)
{
    scalar_op(R, A, MOP_GTE, 0);
}

internal void drelu(matrix A)
{
    drelu(A, A);
}

internal void absolute_value(matrix R, matrix A)
{
    scalar_op(R, A, MOP_ABS);
}

internal void absolute_value(matrix A)
{
    absolute_value(A, A);
}

// internal double sigmoid(double x)
// {
//     double result = 1/(1+exp(-x));
//     return result;
// }

// internal double dsigmoid(double x)
// {
//     double s = sigmoid(x);
//     double result = s*(1-s);
//     return result;
// }

// internal void sigmoid(matrix R, matrix A)
// {
//     scalar_op(R, A, MOP_FUNCTION, 0, sigmoid);
// }

// internal void sigmoid(matrix A)
// {
//     sigmoid(A, A);
// }

// internal void dsigmoid(matrix R, matrix A)
// {
//     scalar_op(R, A, MOP_FUNCTION, 0, dsigmoid);
// }

// internal void dsigmoid(matrix A)
// {
//     sigmoid(A, A);
// }

internal void softmax(matrix R, matrix A)
{
    // double temperature = 1;
    // scalar_op(R, A, MOP_MULTIPLY, 1/temperature);
    
    double maximum = matrix_max(A);
    scalar_op(R, A, MOP_SUBTRACT, maximum);
    scalar_op(R, R, MOP_EXP);
    matrix_normalize(R);
}

internal void softmax(matrix A)
{
    softmax(A, A);
}

internal void decay_l1(matrix A, double step)
{
    scalar_op(A, A, MOP_MOVE_TOWARDS_ZERO, step);
}

internal void decay_l2(matrix A, double step)
{
    multiply(A, 1-step);
}

enum Cost_Function
{
    CF_Quadratic,
    CF_CrossEntropy,
    CF_LogLikelihood,
};

internal double cost(matrix A, matrix B, int activation=CF_Quadratic)
{
    Assert(A.features == 1);
    Assert(A.layers == 1);
    Assert(B.features == 1);
    Assert(B.layers == 1);
    
    Assert(A.height == B.height);
    Assert(A.width == B.width);
    
    double *A_values = (double *)A.allocation.memory;
    double *B_values = (double *)B.allocation.memory;

    double result = 0;
    
    for(int i=0; i<A.height; ++i)
    {
        for(int j=0; j<A.width; ++j)
        {
            double a = *A_values++;
            double y = *B_values++;
            
            switch(activation)
            {
                default:
                {
                    result += square(a - y);
                } break;
                
                case CF_CrossEntropy:
                {
                    result += -y*log(a) - (1-y)*log(1-a);
                } break;
                
                case CF_LogLikelihood:
                {
                    if(y > 0.00001)
                    {
                        result += -y*log(a);
                    }
                } break;
            }
        }
    }

    return result;
}

internal void dcost(matrix R, matrix A, matrix B)
{
    subtract(R, A, B);
}

internal void reduce_precision(matrix R, matrix A, optstruct *fpopts)
{
    Assert(A.features == 1);
    Assert(A.layers == 1);
    Assert(R.features == 1);
    Assert(R.layers == 1);
    Assert(A.height == R.height);
    Assert(A.width == R.width);

    cpfloat((double *)R.allocation.memory, (double *)A.allocation.memory, A.width*A.height, fpopts);
}

internal void reduce_precision(matrix A, optstruct *fpopts)
{
    reduce_precision(A, A, fpopts);
}

#define DL_TYPE matrix
#include "dynamic_list_template.h"

// TODO(lubo): Load input/expected output function callbacks?

struct Neural_Network_Hyperparams
{
    Random_Series random_series;
    double learning_rate;
    double weight_decay;
    double dropout;
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
    result.random_series = create_random_series();
    result.learning_rate = 0.1f;
    result.forward_fpopts = precision_full();
    result.backprop_fpopts = precision_full();
    result.update_fpopts = precision_full();

    return result;
}

// NOTE(lubo): Neural network
struct Neural_Network
{
    // TODO(lubo): __restrict probably non-standard. Sadly, there's no compiler option to ALWAYS assume non-aliased pointers.
    Neural_Network_Hyperparams *__restrict params;
    
    int layer_count;
    
    // IDX_File train_set;
    // IDX_File train_labels;
    // IDX_File test_set;
    // IDX_File test_labels;

    // NOTE(lubo): Last layer can be softmax or no activation
    b32x last_layer_softmax;
    
    matrix expected_output;
    matrix prediction; // NOTE(lubo): Points to last activation layer, just for convenience
    matrix input; // NOTE(lubo): Points to first activation layer, just for convenience
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
};
