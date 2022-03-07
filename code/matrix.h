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

#define DL_TYPE matrix
#include "dynamic_list_template.h"

enum Matrix_Ops
{
    MOP_SET,
    MOP_ADD,
    MOP_SUBTRACT,
    MOP_MULTIPLY,
    MOP_DIVIDE,
    MOP_MODULO,
    MOP_MAX,
    //MOP_ARGMAX, // TODO(lubo): reduce returns double. could we make reduce polymorphic??
    MOP_MIN,
    MOP_GT,
    MOP_LT,
    MOP_EQ,
    //MOP_GTE,
    MOP_LTE,
    MOP_ABS,

    MOP_EXP,
    MOP_LN,
    MOP_DROPOUT,
    
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

inline double *matrix_access(matrix A, smm index)
{
    double *result = (double *)A.allocation.memory + index;
    return result;
};

inline void *matrix_access(matrix A, int h, int w, int layer=0, int feature=0)
{
    Assert(0 <= w && w < A.width);
    Assert(0 <= h && w < A.height);
    Assert(0 <= layer && layer < A.layers);
    Assert(0 <= feature && feature < A.features);

    smm index =
        (smm)feature * (smm)A.layers*(smm)A.height*(smm)A.width +
        (smm)layer * (smm)A.height*(smm)A.width +
        (smm)h * (smm)A.width +
        (smm)w;

    void *result = matrix_access(A, index);
    return result;
};

inline double matrix_get(matrix A, smm index)
{
    double result = *matrix_access(A, index);
    return result;
};

inline void matrix_set(matrix A, smm index, double value)
{
    *matrix_access(A, index) = value;
};

inline double matrix_get(matrix A, int h, int w)
{
    double result = *(double *)matrix_access(A, h, w);
    return result;
};

internal matrix create_stack_matrix(double *data, int height, int width, int layers=1, int features=1)
{
    matrix result = {};
    result.height = height;
    result.width = width;
    result.layers = layers;
    result.features = features;
    result.allocation.memory = (u8 *)data;
    result.allocation.size = sizeof(double)*features*layers*height*width;
    return result;
}

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

double default_function(double x) {return x;}
//double default_function2(double x, double y) {return x+y;}
double default_function_param(double x, double param) {return x;}

internal void
scalar_op(matrix R, matrix A, int op, double param=0,
          double function(double)=default_function,
          double function_param(double,double)=default_function_param)
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
                case MOP_DIVIDE: InvalidCodePath; break;
                case MOP_MODULO: *R_values = modulo(*A_values, param); break;
                case MOP_MAX: *R_values = Max(*A_values, param); break;
                case MOP_MIN: *R_values = Min(*A_values, param); break;
                case MOP_GT:  *R_values = (*A_values >  param) ? 1.0 : 0.0; break;
                case MOP_LT:  *R_values = (*A_values <  param) ? 1.0 : 0.0; break;
                case MOP_EQ:  *R_values = (*A_values == param) ? 1.0 : 0.0; break;
                    //case MOP_GTE: *R_values = (*A_values >= param) ? 1.0 : 0.0; break;
                case MOP_LTE: *R_values = (*A_values <= param) ? 1.0 : 0.0; break;
                case MOP_ABS: *R_values = Abs(*A_values); break;

                case MOP_EXP: *R_values = exp(*A_values); break;
                case MOP_LN:  *R_values = log(*A_values); break;

                case MOP_DROPOUT: *R_values = probability(param) ? (1/param) : 0.0; break;
                    
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
        case MOP_MULTIPLY: result = 1; break;
    };
    
    
    for(int i=0; i<A.height; ++i)
    {
        for(int j=0; j<A.width; ++j)
        {
            double a = *A_values++;
            
            switch(op)
            {
                case MOP_MULTIPLY: result *= a; break;
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
