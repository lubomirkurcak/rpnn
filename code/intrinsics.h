/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

#include <math.h>

inline float square_root(float x)
{
    float result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
    return result;
}

inline float reciprocal_square_root(float x)
{
    float result = (1.0f / square_root(x));
    return result;
}

inline float absolute_value(float x)
{
    float result = fabsf(x);
    return result;
}

inline float sign(float x)
{
    float result = 0;
    if(x > 0) result = 1;
    else if(x < 0) result = -1;
    return result;
}

inline float round(float x)
{
    float result = _mm_cvtss_f32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(x), _MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC));
    return result;
}

inline int round_to_int(float x)
{
    s32 result = _mm_cvtss_si32(_mm_set_ss(x));
    return result;
}

inline u32 round_to_u32(float x)
{
    u32 result = (u32)_mm_cvtss_si32(_mm_set_ss(x));
    return result;
}

inline float floor(float x)
{
    float result = _mm_cvtss_f32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(x)));
    return result;
}

inline float ceil(float x)
{
    float result = _mm_cvtss_f32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(x)));
    return result;
}

inline int floor_to_int(float x)
{
    int result = _mm_cvtss_si32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(x)));
    return result;
}

inline int ceil_to_int(float x)
{
    int result = _mm_cvtss_si32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(x)));
    return result;
}

inline int truncate_to_int(float x)
{
    int result = (int)x;
    return result;
}

inline float modulo(float numerator, float denominator)
{
    float result = fmodf(numerator, denominator);
    return result;
}

inline float64 modulo(float64 numerator, float64 denominator)
{
    float64 result = fmod(numerator, denominator);
    return result;
}

inline float sin(float x)
{
    float result = sinf(x);
    return result;
}

inline float cos(float x)
{
    float result = cosf(x);
    return result;
}

inline float tan(float x)
{
    float result = tanf(x);
    return result;
}

inline float atan2(float Y, float X)
{
    float result = atan2f(Y, X);
    return result;
}

inline float arcsin(float x)
{
    float result = asinf(x);
    return result;
}

inline float arccos(float x)
{
    float result = acosf(x);
    return result;
}

// inline float exp(float x)
// {
//     float result = expf(x);
//     return result;
// }

inline float ln(float x)
{
    float result = logf(x);
    return result;
}

inline float log2(float x)
{
    float result = log2f(x);
    return result;
}

inline float exp_with_base(float base, float exponent)
{
    float result = expf(ln(base)*exponent);
    return result;
}

inline u64 round_up_to_power_of_two(u64 value)
{
    u64 result = 1;
    while(result < value)
    {
        result *= 2;
    }

    return result;
}

inline u32 change_endianness(u32 Value)
{
    u32 result =
#if COMPILER_MSVC
        _byteswap_ulong(Value);
#else
        (((value>>24)&0xff)<< 0)|
        (((value>>16)&0xff)<< 8)|
        (((value>> 8)&0xff)<<16)|
        (((value>> 0)&0xff)<<24);
#endif

    return result;
}

inline void change_endianness_inplace(u32 *value)
{
    *value = change_endianness(*value);
}

inline s32 find_least_significant_set_bit(u32 value)
{
    s32 result = -1;

#if COMPILER_MSVC
    _BitScanForward((unsigned long *)&result, value);
#else
    for(s32 index=0; index<32; ++index)
    {
        u32 test = 1 << index;
        if(value & test)
        {
            result = index;
            break;
        }
    }
#endif
    return result;
}

inline s32 find_most_significant_set_bit(u32 value)
{
    s32 result = -1;

#if COMPILER_MSVC
    _BitScanForward((unsigned long *)&result, value);
#else
    for(s32 index=0; index<32; ++index)
    {
        u32 testindex = 31 - index;
        u32 test = 1 << testindex;
        if(value & test)
        {
            result = testindex;
            break;
        }
    }
#endif
    return result;
}
