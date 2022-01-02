/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

struct Random_Series
{
    u32 state;
};

global Random_Series global_default_random_series;

internal Random_Series create_random_series()
{
    Random_Series result = {(u32)time(0)};
    return result;
}

inline u32 get_random_u32(Random_Series *series=&global_default_random_series)
{
    u32 R = series->state;
    R = R*7001 + 100003;
    R ^= R << 13;
    R ^= R >> 17;
    R ^= R << 5;
    
    series->state = R;

    return R;
}

inline s32 get_random_s32(Random_Series *series=&global_default_random_series)
{
    s32 result = (s32)get_random_u32(series);
    return result;
}

inline float get_random_float(Random_Series *series=&global_default_random_series)
{
    float result = get_random_u32(series) / 4294967296.0f;
    return result;
}

inline float get_random_float_11(Random_Series *series=&global_default_random_series)
{
    float result = get_random_float(series)*2 - 1;
    return result;
}

inline float get_random_float(float min, float max, Random_Series *series=&global_default_random_series)
{
    float result = lerp(min, max, get_random_float(series));
    return result;
}

inline v2 get_random_unit_v2(Random_Series *series=&global_default_random_series)
{
    v2 result = unit_angle(Tau32*get_random_float(series));
    return result;
}

inline v2 get_random_v2_square(Random_Series *series=&global_default_random_series)
{
    v2 result = {get_random_float(series),
                 get_random_float(series),};

    return result;
}

inline v2 get_random_v2_square_11(Random_Series *series=&global_default_random_series)
{
    v2 result = {get_random_float_11(series),
                 get_random_float_11(series),};

    return result;
}

inline v2 get_random_v2_circle(Random_Series *series=&global_default_random_series)
{
    v2 result;
    do
    {
        result = get_random_v2_square_11(series);
    } while(quadrance(result) > 1);

    return result;
}

inline v2 get_random_v2_from_r2(r2 rect, Random_Series *series=&global_default_random_series)
{
    v2 result = {lerp(rect.min.x, rect.max.x, get_random_float(series)),
                 lerp(rect.min.y, rect.max.y, get_random_float(series)),};
    return result;
}

inline v3 get_random_v3_from_r3(r3 rect, Random_Series *series=&global_default_random_series)
{
    v3 result = {lerp(rect.min.x, rect.max.x, get_random_float(series)),
                 lerp(rect.min.y, rect.max.y, get_random_float(series)),
                 lerp(rect.min.z, rect.max.z, get_random_float(series)),};
    return result;
}

// inline r2 get_random_r2(Random_Series *series=&global_default_random_series)
// {
//     v2 a = get_random_v2_square(series);
//     v2 b = get_random_v2_square(series);
// 
//     r2 result = {};
//     result.min.x = Min(a.x, b.x);
//     result.min.y = Min(a.y, b.y);
//     result.max.x = Max(a.x, b.x);
//     result.max.y = Max(a.y, b.y);
// 
//     return result;
// }

inline v3 get_random_v3_cube_11(Random_Series *series=&global_default_random_series)
{
    v3 result = {get_random_float_11(series),
                 get_random_float_11(series),
                 get_random_float_11(series),};

    return result;
}

inline v3 get_random_v3_size(v3 size, Random_Series *series=&global_default_random_series)
{
    v3 result = hadamard(size, get_random_v3_cube_11(series));
    return result;
}

// TODO(lubo): BAD!!!!!
inline v3 get_random_v3_ball(Random_Series *series=&global_default_random_series)
{
    v3 result;
    do
    {
        result = get_random_v3_cube_11(series);
    } while(quadrance(result) > 1);

    return result;
}

inline v3 get_random_v3_cone_X(float cos_theta, Random_Series *series=&global_default_random_series)
{
    v3 result;
    do
    {
        result = get_random_v3_ball(series);
    } while(result.x < cos_theta);

    return result;
}

inline v3 get_random_v3_cone_Z(float cos_theta, Random_Series *series=&global_default_random_series)
{
    v3 result;
    do
    {
        result = get_random_v3_ball(series);
    } while(result.z < cos_theta);

    return result;
}

inline v3 get_random_v3_cone(v3 direction, float cos_theta, Random_Series *series=&global_default_random_series)
{
    v3 result;
    do
    {
        result = get_random_v3_ball(series);
    } while(dot(result, direction) > cos_theta);

    return result;
}

inline v3 get_random_v3_halfball(Random_Series *series=&global_default_random_series)
{
    v3 result;
    do
    {
        result = {get_random_float_11(series),
                  get_random_float_11(series),
                  get_random_float(series),};
    } while(quadrance(result) > 1);

    return result;
}

inline int
get_random_int_up_to(int max, Random_Series *series=&global_default_random_series)
{
    int result = (int)(max * get_random_float(series));
    return result;
}

#define get_random_choice_from_static_array(Array, Series) Array[get_random_int_up_to(ArrayCount(Array), Series)]

inline u32
get_random_u32_up_to(u32 max, Random_Series *series=&global_default_random_series)
{
    u32 result = (u32)(max * get_random_float(series));
    return result;
}

inline s32
get_random_s32_up_to(s32 max, Random_Series *series=&global_default_random_series)
{
    s32 result = (s32)(max * get_random_float(series));
    return result;
}

inline s32
get_random_s32_from_range(s32 min, s32 max, Random_Series *series=&global_default_random_series)
{
    s32 result = (s32)lerp((float)min, (float)max, get_random_float(series));
    return result;
}

inline u32
choose_group_based_on_weights(u32 groups, u32 *weights, Random_Series *series=&global_default_random_series)
{
    u32 sum = 0;
    
    for(u32 group_index=0; group_index<groups; ++group_index)
    {
        sum += weights[group_index];
    }

    u32 roll = get_random_u32_up_to(sum, series);

    u32 result = 0;
    for(u32 group_index=0; group_index<groups; ++group_index)
    {
        if(roll < weights[group_index])
        {
            result = group_index;
            break;
        }

        roll -= weights[group_index];
    }

    return result;
}

inline b32x
probability(float p, Random_Series *series=&global_default_random_series)
{
    float roll = get_random_float(series);
    b32 result = roll < p;
    return result;
}

#if 0
// TODO(lubo): What is this??????
inline void softmax(int count, float *values, float *results, float temperature=1)
{
    float inv_temperature = 1/temperature;
    float sum = 0;
    
    for(int i=0; i<count; ++i)
    {
        float value = expf(inv_temperature*values[i]);
        results[i] = value;
        sum += value;
    }

    float inv_sum = 1/sum;
    
    for(int i=0; i<count; ++i)
    {
        results[i] *= inv_sum;
    }
}
#endif

inline int weighted_random(int count, float *weights, Random_Series *series=&global_default_random_series)
{
    float total_weight = 0;
    for(int i=0; i<count; ++i)
    {
        total_weight += weights[i];
    }
    
    float remaining = total_weight*get_random_float(series);

    int result = 0;
    while(result < count && remaining >= weights[result])
    {
        remaining -= weights[result];
        result += 1;
    }

    return result;
}

inline v4
get_random_color(Random_Series *series=&global_default_random_series)
{
    v4 result = {1,1,1,1};
    result.r = get_random_float(series);
    result.g = get_random_float(series);
    result.b = get_random_float(series);
    return result;
}

// NOTE(lubo): Inverse cumulative gaussian distribution. Thanks to Peter John Acklam
#define  A1  (-3.969683028665376e+01f)
#define  A2   2.209460984245205e+02f
#define  A3  (-2.759285104469687e+02f)
#define  A4   1.383577518672690e+02f
#define  A5  (-3.066479806614716e+01f)
#define  A6   2.506628277459239e+00f

#define  B1  (-5.447609879822406e+01f)
#define  B2   1.615858368580409e+02f
#define  B3  (-1.556989798598866e+02f)
#define  B4   6.680131188771972e+01f
#define  B5  (-1.328068155288572e+01f)

#define  C1  (-7.784894002430293e-03f)
#define  C2  (-3.223964580411365e-01f)
#define  C3  (-2.400758277161838e+00f)
#define  C4  (-2.549732539343734e+00f)
#define  C5   4.374664141464968e+00f
#define  C6   2.938163982698783e+00f

#define  D1   7.784695709041462e-03f
#define  D2   3.224671290700398e-01f
#define  D3   2.445134137142996e+00f
#define  D4   3.754408661907416e+00f

#define P_LOW   0.02425
/* P_high = 1 - p_low*/
#define P_HIGH  0.97575

float normsinv(float p)
{
    float result = 0;
    if(0 < p && p < P_LOW)
    {
        float q = square_root(-2*ln(p));
        result = (((((C1*q+C2)*q+C3)*q+C4)*q+C5)*q+C6) / ((((D1*q+D2)*q+D3)*q+D4)*q+1);
    }
    else if(p <= P_HIGH)
    {
        float q = p - 0.5f;
        float r = q*q;
        result = (((((A1*r+A2)*r+A3)*r+A4)*r+A5)*r+A6)*q /(((((B1*r+B2)*r+B3)*r+B4)*r+B5)*r+1);
    }
    else if(p < 1)
    {
        float q = square_root(-2*ln(1-p));
        result = -(((((C1*q+C2)*q+C3)*q+C4)*q+C5)*q+C6) / ((((D1*q+D2)*q+D3)*q+D4)*q+1);
    }
    else
    {
        // NOTE(lubo): This is not defined but it doesn't matter all that much, just return 0
        //InvalidCodePath;
        result = 0;
    }

    return result;
}

// NOTE(lubo): Advanced random functions, may have dependencies on math/colors, etc. Maybe this could be split into a separate file, something like advanced_random.h?


inline float
normal_distribution(float sigma=1, float mu=0, Random_Series *series=&global_default_random_series)
{
    float result = sigma*normsinv(get_random_float(series)) + mu;
    return result;
}
