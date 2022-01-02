/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

#define var auto

inline int argmin(int count, float *values)
{
    Assert(count>0);
    int arg = 0;
    float min = values[0];
    for(int i=1; i<count; ++i)
    {
        if(values[i] < min)
        {
            arg = i;
            min = values[i];
        }
    }

    return arg;
}

inline int modulo(int value, int mod)
{
    int result = value % mod;
    if(result < 0)
    {
        result += mod;
        Assert(result >= 0);
        Assert(result < mod);
    }
    
    return result;
}

// NOTE(lubo): Examples
//   6 / 5 = 1
//   5 / 5 = 1
//   4 / 5 = 0
//   0 / 5 = 0
//  -1 / 5 = -1
//  -5 / 5 = -1
//  -6 / 5 = -2
inline int down_div(int top, int bottom)
{
    Assert(bottom != 0);
    
    if(bottom < 0)
    {
        top = -top;
        bottom = -bottom;
    }
    
    int result = top / bottom;
    if(top < 0)
    {
        result = (top - bottom + 1) / bottom;
    }

    return result;
}

inline v2i index_to_xy(int width, int index)
{
    v2i result = {};
    result.x = modulo(index, width);
    result.y = down_div(index, width);
    return result;
}

inline int index_from_xy(int width, v2i coords)
{
    int result = width*coords.y + coords.x;
    return result;
}

inline b32x approximately(float a, float b, float tolerance=0.00001f)
{
    b32x result = absolute_value(a-b) <= tolerance;
    return result;
}
inline b32x approximately(v3 a, v3 b, float tolerance=0.00001f)
{
    b32x result =
        approximately(a.x, b.x, tolerance) &&
        approximately(a.y, b.y, tolerance) &&
        approximately(a.z, b.z, tolerance);
    return result;
}

inline int square(int x)
{
    var result = x*x;
    return result;
}

// Scalar
inline float square(float x)
{
    var result = x*x;
    return result;
}
inline float64 square(float64 x)
{
    var result = x*x;
    return result;
}

inline float clamp01(float x)
{
    float result = x;
    if(x > 1) result = 1;
    if(x < 0) result = 0;
    return result;
}

inline float ratio(float top, float bottom)
{
    float result = top/bottom;
    return result;
}

inline float ratio(int top, int bottom)
{
    float result = ratio((float)top, (float)bottom);
    return result;
}

inline float safe_ratio_n(float top, float bottom, float default_value)
{
    var result = default_value;
    if(bottom != 0)
    {
        result = top/bottom;
    }
    return result;
}
inline float safe_ratio_0(float top, float bottom)
{
    var result = safe_ratio_n(top, bottom, 0);
    return result;
}
inline float safe_ratio_1(float top, float bottom)
{
    var result = safe_ratio_n(top, bottom, 1);
    return result;
}

inline s8 clamp_to_s8(float value)
{
    s8 result = 0;
    if(value >= s8_max) result = s8_max;
    else if(value <= s8_min) result = s8_min;
    else result = (s8)floor_to_int(value);
    return result;
}

inline float to_range_11(float x)
{
    float result = 2*x - 1;
    return result;
}

inline float to_range_01(float x)
{
    float result = 0.5f*x + 0.5f;
    return result;
}

inline float lerp(float a, float b, float t)
{
    float result = (1-t)*a + t*b;
    return result;
}

inline float inv_lerp(float min, float max, float value)
{
    float result = 0;
    float range = max-min;
    if(absolute_value(range) > 0.00001f)
    {
        result = (value-min)/range;
    }
    return result;
}

inline int array_lerp(int count, float t)
{
    int index = Clamp(floor_to_int(t*count), 0, count-1);
    return index;
}

// NOTE(lubo): linearly remaps value from range [a,b] to [A,B]
inline float remap(float a, float b, float A, float B, float value)
{
    float result = lerp(A, B, inv_lerp(a, b, value));
    return result;
}

// NOTE(lubo): linearly remaps value from range [a,b] to [A,B]
inline float remap_clamped(float a, float b, float A, float B, float value)
{
    float result = lerp(A, B, clamp01(inv_lerp(a, b, value)));
    return result;
}

inline float cubic_interpolation(float t)
{
    var result = -2*t*t*t + 3*t*t;
    return result;
}

inline float cubic_interpolation(float min, float max, float t)
{
    var result = lerp(min, max, cubic_interpolation(t));
    return result;
}

inline float cosine_interpolation(float t)
{
    var result = -0.5f*cos(Pi32*t) + 0.5f;
    return result;
}

inline float quadratic_smooth_end(float t)
{
    var result = -square(t-1) + 1;
    return result;
}
inline float quadratic(float t)
{
    var result = -4*square(t-0.5f) + 1;
    return result;
}

inline float smoothstep5(float t)
{
    var result = t*t*t*(t*(t*6 - 15) + 10);
    return result;
};

float smoothstep5(float min, float max, float t)
{
    float result = lerp(min, max, smoothstep5(t));
    return result;
}

#define cubic hermite01
inline float hermite00(float t){return 2*t*t*t - 3*t*t + 1;}
inline float hermite10(float t){return t*t*t - 2*t*t + t;}
inline float hermite01(float t){return -2*t*t*t + 3*t*t;}
inline float hermite11(float t){return t*t*t - t*t;}
inline float dhermite00(float t){return 3*2*t*t - 2*3*t;}
inline float dhermite10(float t){return 3*t*t - 2*2*t + 1;}
inline float dhermite01(float t){return -2*3*t*t + 2*3*t;}
inline float dhermite11(float t){return 3*t*t - 2*t;}

inline float triangle_function(float x)
{
    var result = 2*Min(x,1-x);
    return result;
}

inline float parabolic_hump(float x)
{
    var result = -4*x*x+4*x;
    return result;
}

// NOTE(lubo):
// f is non-negative and quadratic function
// f(0) = 0.5
// f(1) = 1
// there exists a number z from (0, 1) such that f(z) = 0
inline float staff_function(float x)
{
    var result = (3.0f/2.0f + Sqrt2_32)*x*x + (0.5f - (3.0f/2.0f + Sqrt2_32))*x + 0.5f;
    return result;
}

// NOTE(lubo):
// f is cubic
// f(0) = 0.5
// f'(0) = 0
// f(1) = 1
// there exists a number z from (0, 1) such that f(z) = 0
inline float staff_function2(float x)
{
    // a(2/3 - 1/(3a))^3 + (0.5 - a)(2/3 - 1/(3a))^2 + 0.5 = 0
    float a = 4.721767800796626041000553f;
    float result = a*x*x*x + (0.5f-a)*x*x + 0.5f;
    return result;
}

// NOTE(lubo): Linearly (0,0) -> (a,1) -> (b,1) -> (1,0)
inline float trapezoid_function(float a, float b, float x)
{
    float result = 1;
    if(x < a) result = lerp(0, 1, inv_lerp(0, a, x));
    if(x > b) result = lerp(1, 0, inv_lerp(b, 1, x));
    return result;
}

inline float isosceles_trapezoid_function(float width, float x)
{
    var result = trapezoid_function(lerp(0.5,0,width), lerp(0.5,1,width), x);
    return result;
}

inline float flat_hump(float x, int flatness)
{
    Assert(flatness >= 0);
    
    var result = (1-parabolic_hump(x));
    while(flatness--)
    {
        result *= result;
    }
    result = 1-result;
    return result;
}

inline float move_towards(float target, float position, float step)
{
    float result = target;
    float distance = target - position;
    if(absolute_value(distance) > step)
    {
        if(distance < 0)
        {
            result = position - step;
        }
        else
        {
            result = position + step;
        }
    }
    return result;
}

inline float relu(float x)
{
    var result = x>0 ? x : 0;
    return result;
}

inline float to_radians(float degrees)
{
    float result = degrees * (HalfPi32/90);
    return result;
}

inline float to_degrees(float theta)
{
    float result = theta * (90/HalfPi32);
    return result;
}

inline float sin_4_lines(float theta)
{
    float norm = theta/Tau32 + 0.125f;
    int period = (int)(norm);
    float norm_phase = norm - period;

    float x = norm_phase;
    if(norm_phase < 0)
    {
        x = (1 + norm_phase);
    }

    float result = -1;
    if(x < 0.25)
    {
        result = -1 + 8*x;
    }
    else if(x < 0.5)
    {
        result = 1;
    }
    else if(x < 0.75)
    {
        result = 1 - (x-0.5f)*8;
    }

    return result;
}

// Vector 2D
inline v2 perp_left(v2 a)
{
    v2 result = {-a.y, a.x};
    return result;
}

inline v2 perp_right(v2 a)
{
    v2 result = {a.y, -a.x};
    return result;
}

inline v2 perp(v2 a)
{
    v2 result = perp_left(a);
    return result;
}

inline float dot(v2 a, v2 b)
{
    float result = a.x*b.x + a.y*b.y;
    return result;
}

inline float dot(v3 a, v3 b)
{
    float result = a.x*b.x + a.y*b.y + a.z*b.z;
    return result;
}

#define VECTOR_MATH(vec)                                                \
                                                                        \
    inline float quadrance(vec a)                                       \
    {                                                                   \
        float result = dot(a, a);                                       \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline float magnitude(vec a)                                       \
    {                                                                   \
        float result = square_root(quadrance(a));                       \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline float distance_sqr(vec a, vec b)                             \
    {                                                                   \
        float result = quadrance(b-a);                                  \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline float distance(vec a, vec b)                                 \
    {                                                                   \
        float result = magnitude(b-a);                                  \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec lerp(vec a, vec b, float t)                              \
    {                                                                   \
        vec result = (1-t)*a + t*b;                                     \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline b32x approximately_zero(vec a)                               \
    {                                                                   \
        b32x result = quadrance(a) < square(0.0001f);                   \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec normalize(vec a)                                         \
    {                                                                   \
        vec result = a / magnitude(a);                                  \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec noz(vec a)                                               \
    {                                                                   \
        vec result = {};                                                \
        if(!approximately_zero(a))                                      \
        {                                                               \
            result = normalize(a);                                      \
        }                                                               \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline float units_along_basis(vec a, vec b)                        \
    {                                                                   \
        float result = dot(a,b)/quadrance(b);                           \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec project_to_line(vec dir, vec p)                          \
    {                                                                   \
        vec result = dir * units_along_basis(p,dir);                    \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec project_to_line(vec a, vec b, vec p)                     \
    {                                                                   \
        vec result = a + project_to_line(b-a, p-a);                     \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec distance_to_line(vec a, vec b, vec p)                    \
    {                                                                   \
        vec result = project_to_line(a,b,p) - p;                        \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline float quadrance_to_line(vec a, vec b, vec p)                 \
    {                                                                   \
        float result = quadrance(distance_to_line(a,b,p));              \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec project_to_plane(vec a, vec plane)                       \
    {                                                                   \
        vec result = a - plane * units_along_basis(a,plane);            \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec project_to_normalized_plane(vec a, vec plane)            \
    {                                                                   \
        vec result = a - plane * dot(a,plane);                          \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec project_to_plane(vec a, vec plane, vec plane_origin)     \
    {                                                                   \
        vec result = project_to_plane(a - plane_origin, plane) + plane_origin; \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec project_to_normalized_plane(vec a, vec plane, vec plane_origin) \
    {                                                                   \
        vec result = project_to_normalized_plane(a - plane_origin, plane) + plane_origin; \
        return result;                                                  \
    }                                                                   \
                                                                        \
    inline vec distance_to_plane(vec a, vec plane, vec plane_origin)    \
    {                                                                   \
        vec result = project_to_plane(a, plane, plane_origin) - a;      \
        return result;                                                  \
    }

VECTOR_MATH(v3);
VECTOR_MATH(v2);

//#define length_sq quadrance

// NOTE(lubo): this sucks lol
inline float spread(v2 a, v2 b)
{
    float result = square(a.x*b.y - a.y*b.x)/(quadrance(a)*quadrance(b));
    return result;
}

inline float cross(v2 a, v2 b)
{
    float result = a.x*b.y - b.x*a.y;
    return result;
}

inline v2 hadamard(v2 a, v2 b)
{
    v2 result;
    result.x = a.x*b.x;
    result.y = a.y*b.y;
    return result;
}

inline v3 hadamard(v3 a, v3 b)
{
    v3 result;
    result.x = a.x*b.x;
    result.y = a.y*b.y;
    result.z = a.z*b.z;
    return result;
}

inline v4 hadamard(v4 a, v4 b)
{
    v4 result;
    result.x = a.x*b.x;
    result.y = a.y*b.y;
    result.z = a.z*b.z;
    result.w = a.w*b.w;
    return result;
}

// inline v2 smooth_normalize(v2 a)
// {
//     v2 result = a;
//     if(quadrance(a) > 1)
//     {
//         result = normalize(a);
//     }

//     return result;
// }

inline v2 clamp_vector(v2 a, float max_magnitude)
{
    v2 result = a;
    float length_sq = quadrance(a);
    if(length_sq > square(max_magnitude))
    {
        float length = square_root(length_sq);
        result = (max_magnitude/length)*a;
    }

    return result;
}

inline float angle(v2 a, v2 b)
{
    float cos_theta_squared = square(dot(a,b))/(quadrance(a)*quadrance(b));
    float result = 0.5f*arccos(2*cos_theta_squared - 1);
    
    return result;
}

inline v2 unit_angle(float theta)
{
    v2 result = {cos(theta), sin(theta)};
    return result;
}

inline float atan2(v2 a)
{
    float result = atan2(a.y, a.x);
    return result;
}

inline int lock_into_4_directions(v2 dir)
{
    int result = 0;
    
    v2 top_right = {1,1};
    v2 top_left = {-1,1};

    if(dot(dir, top_right) > 0)
    {
        if(dot(dir, top_left) > 0)
        {
            result = 1; // top
        }
        else
        {
            result = 0; // right
        }
    }
    else
    {
        if(dot(dir, top_left) > 0)
        {
            result = 2; // left
        }
        else
        {
            result = 3; // down
        }
    }

    return result;
}

// NOTE(lubo): Inspired by https://mathworld.wolfram.com/QuadraticEquation.html
// NOTE(lubo): Solution with -sqrt(D) is in .x, solution with +sqrt(D) is in .y
inline v2 stable_solve_quadratic_equation(float a, float b, float c)
{
    float d = square_root(b*b - 4*a*c);

    float q = -0.5f*(b + d);
    v2 result = {q/a, c/q};
    if(b < 0)
    {
        float q = -0.5f*(b - d);
        result = {c/q, q/a};
    }

    return result;
}

// NOTE(lubo): Modified https://www.gamasutra.com/blogs/ScottLembcke/20180508/316662/Shooting_a_Moving_Target.php
inline v2 aim(v2 sp, v2 sv, v2 dp, v2 dv, float projectile_speed, float projectile_duration)
{
    v2 delta = dp - sp;
    v2 rel_v = dv - sv;

    float a = quadrance(rel_v) - square(projectile_speed);
    float b = 2*dot(rel_v, delta);
    float c = quadrance(delta);

    float d = square(b) - 4*a*c;

    v2 result = sp;
    
    if(d >= 0)
    {
        v2 ts = stable_solve_quadratic_equation(a, b, c);

        float t = ts.x;
        if(t < 0) t = ts.y;

        if(t >= 0 && t <= projectile_duration)
        {
            result = dp + dv*t;
        }
    }
    #if 0
    else
    {
        // NOTE(lubo): Desperate shot. Target is on safe trajectory. We select time that where derivative is 0.
        float t = (-b)/(2*a);
        
        if(t >= 0 && t <= projectile_duration)
        {
            result = dp + dv*t;
        }
    }
    #endif

    return result;
}

inline v2 to_range_11(v2 v)
{
    v2 result = {};
    result.x = to_range_11(v.x);
    result.y = to_range_11(v.y);
    return result;
}

inline v2 to_range_01(v2 v)
{
    v2 result = {};
    result.x = to_range_01(v.x);
    result.y = to_range_01(v.y);
    return result;
}

inline v2 mouse_input_to_aspect(v2 v, float aspect)
{
    v2 result;
    result.x = aspect*v.x;
    result.y = v.y;
    return result;
}

inline v2 mouse_input_to_11(v2 v, float aspect)
{
    v2 result;
    result.x = (1/aspect)*v.x;
    result.y = v.y;
    return result;
}

// Vector 3D
inline b32x is_zero(v3 a)
{
    var result = a.x == 0 && a.y == 0 && a.z == 0;
    return result;
}

inline v3 perp_any(v3 a)
{
    v3 result = V3(a.z, 0, -a.x);
    if(a.z == 0)
    {
        result = V3(a.y, -a.x, 0);
    }
    return result;
}
// NOTE(lubo): This was determined not needed
/*
inline v3 perp_any_prefer_non_zero(v3 a)
{
    v3 result = V3(a.z, 0, a.x);
    if(a.z == 0)
    {
        result = V3(a.y, -a.x, 0);
    }
    return result;
}

inline v3 perp_any(v3 a)
{
    v3 result = perp_any_prefer_non_zero(a);
    return result;
}
*/

inline v3 cross(v3 a, v3 b)
{
    v3 result;
    
    result.x = a.y*b.z - a.z*b.y;
    result.y = a.z*b.x - a.x*b.z;
    result.z = a.x*b.y - a.y*b.x;
    
    return result;
}

inline float inv_lerp(v3 a, v3 b, v3 p)
{
    v3 d = b-a;
    v3 v = p-a;
    
    float result = dot(v,d)/quadrance(d);
    return result;
}

// inline v3 smooth_normalize(v3 a)
// {
//     var result = a;
//     if(quadrance(a) > 1)
//     {
//         result = normalize(a);
//     }

//     return result;
// }

inline v3 clamp_vector(v3 a, float max_magnitude)
{
    v3 result = a;
    float length_sq = quadrance(a);
    if(length_sq > square(max_magnitude))
    {
        float length = square_root(length_sq);
        result = (max_magnitude/length)*a;
    }

    return result;
}

inline float angle(v3 a, v3 b)
{
    float cos_theta_squared = square(dot(a,b))/(quadrance(a)*quadrance(b));
    float result = 0.5f*arccos(2*cos_theta_squared - 1);
    
    return result;
}

inline float greatest_magnitude_component(v3 a)
{
    float result = absolute_value(a.x);
    if(absolute_value(a.y) > result)
    {
        result = absolute_value(a.y);
    }
    if(absolute_value(a.z) > result)
    {
        result = absolute_value(a.z);
    }
    return result;
}

struct Plane
{
    v3 origin;
    v3 normal;
};

inline v3 plane_normal_from_points(v3 a, v3 b, v3 c)
{
    v3 result = cross(b-a, c-a);
    return result;
}
    
inline Plane plane_from_points(v3 a, v3 b, v3 c)
{
    Plane result = {};
    result.origin = a;
    result.normal = plane_normal_from_points(a, b, c);

    return result;
}

inline v3 plane_ray_intersection(Ray ray, v3 plane, v3 plane_origin)
{
    float d = dot(plane, ray.dir);
    float t = dot(plane, plane_origin - ray.eye)/d;
    v3 result = ray.eye + t*ray.dir;

    if(t < 0)
    {
        result.x = ratio(0,0);
        result.y = ratio(0,0);
        result.z = ratio(0,0);
    }
    
    return result;
}

// Vector 4D
inline v4 lerp(v4 a, v4 b, float t)
{
    var result = (1-t)*a + t*b;
    return result;
}

// Vector 2Di
inline s64 quadrance(v2i a)
{
    s64 result = (s64)a.x*(s64)a.x + (s64)a.y*(s64)a.y;
    return result;
}


// Rectangle 2D Constructors
inline r2 rect_atlas(int width, int height, int index, float padding=0)
{
    int x = index % width;
    int y = height - 1 - (index/width);

    r2 result = {};
    result.min.x = ratio((float)x,(float)width) + padding;
    result.min.y = ratio((float)y,(float)height) + padding;
    result.max.x = ratio((float)(x + 1),(float)width) - padding;
    result.max.y = ratio((float)(y + 1),(float)height) - padding;
    return result;
}

inline r2
rect_min_max(v2 min, v2 max)
{
    r2 result = {min, max};
    return result;
}

inline r2
rect_min_max(float minx, float miny, float maxx, float maxy)
{
    r2 result = rect_min_max(V2(minx, miny), V2(maxx, maxx));
    return result;
}

inline r2
rect_min_dim(v2 min, v2 dim)
{
    r2 result = {min, min+dim};
    return result;
}

inline r2
rect_min_dim(float minx, float miny, float dimx, float dimy)
{
    r2 result = rect_min_dim(V2(minx, miny), V2(dimx, dimx));
    return result;
}

inline r2
rect_center_halfdim(v2 center, v2 halfdim)
{
    r2 result;
    result.min = center - halfdim;
    result.max = center + halfdim;

    return result;
}

inline r2
rect_center_dim(v2 center, v2 dim)
{
    r2 result = rect_center_halfdim(center, 0.5f*dim);
    return result;
}

inline r2
rect_anchor_dim(v2 anchor, v2 alignment, v2 size)
{
    v2 min = anchor - hadamard(alignment, size);
    r2 result = rect_min_dim(min, size);
    return result;
}

// Rectangle 2D Operations
inline float
get_width(r2 a)
{
    float result = a.max.x - a.min.x;
    return result;
}

inline float
get_height(r2 a)
{
    float result = a.max.y - a.min.y;
    return result;
}

inline float
get_aspect_ratio(r2 a)
{
    float result = get_width(a)/get_height(a);
    return result;
}

inline r2
normalize(r2 a)
{
    r2 result = {};
    result.min.x = Min(a.min.x, a.max.x);
    result.min.y = Min(a.min.y, a.max.y);
    result.max.x = Max(a.min.x, a.max.x);
    result.max.y = Max(a.min.y, a.max.y);
    return result;
}

#define get_dim get_size
inline v2
get_size(r2 a)
{
    v2 result;
    result.x = get_width(a);
    result.y = get_height(a);
    return result;
}

inline v2
get_bottom_left(r2 rect)
{
    v2 result = rect.min;
    return result;
}
inline v2
get_top_left(r2 rect)
{
    v2 result = {rect.min.x, rect.max.y};
    return result;
}
inline v2
get_bottom_right(r2 rect)
{
    v2 result = {rect.max.x, rect.min.y};
    return result;
}
inline v2
get_top_right(r2 rect)
{
    v2 result = rect.max;
    return result;
}

#define get_point_in_rect rect_lerp

inline v2 rect_lerp(r2 rect, v2 t)
{
    v2 result;
    result.x = lerp(rect.min.x, rect.max.x, t.x);
    result.y = lerp(rect.min.y, rect.max.y, t.y);
    return result;
}

inline v2 rect_inv_lerp(r2 rect, v2 point)
{
    v2 result;
    result.x = inv_lerp(rect.min.x, rect.max.x, point.x);
    result.y = inv_lerp(rect.min.y, rect.max.y, point.y);
    return result;
}

inline v2 get_center(r2 rect)
{
    v2 result = rect_lerp(rect, V2(0.5, 0.5));
    return result;
}

inline r2 get_subrect(r2 rect, r2 region)
{
    r2 result = rect_min_max(rect_lerp(rect, region.min), rect_lerp(rect, region.max));
    return result;
}

inline r2 scale_rect(r2 rect, float scale)
{
    r2 result = rect_center_dim(get_center(rect), scale*get_size(rect));
    return result;
}

inline r2 offset_rect(r2 rect, v2 translation)
{
    r2 result;
    result.min = rect.min + translation;
    result.max = rect.max + translation;
    return result;
}

inline r2 offset_rect_relative(r2 rect, v2 translation_percentage)
{
    v2 translation = hadamard(translation_percentage, get_size(rect));
    r2 result = offset_rect(rect, translation);
    return result;
}

inline r2 pad_rect(r2 rect, v2 padding)
{
    r2 result;
    result.min = rect.min - padding;
    result.max = rect.max + padding;
    return result;
}

inline r2 pad_rect(r2 rect, float padding)
{
    r2 result = pad_rect(rect, V2(padding, padding));
    return result;
}

// inline r2
// pad_rect(r2 rect, r2 padding)
// {
//     r2 result;
//     result.min = rect.min + padding.min;
//     result.max = rect.max + padding.max;
//     return result;
// }

inline r2 clip_to_grid(r2 rect)
{
    r2 result;
    result.min.x = floor(rect.min.x);
    result.min.y = floor(rect.min.y);
    result.max.x = ceil(rect.max.x);
    result.max.y = ceil(rect.max.y);

    return result;
}

inline r2 rect_union_identity()
{
    r2 result;
    result.min.x = ratio(1,0);
    result.min.y = ratio(1,0);
    result.max.x = ratio(-1,0);
    result.max.y = ratio(-1,0);
    return result;
}

inline r2 rect_union(r2 a, r2 b)
{
    r2 result;
    result.min.x = Min(a.min.x, b.min.x);
    result.min.y = Min(a.min.y, b.min.y);
    result.max.x = Max(a.max.x, b.max.x);
    result.max.y = Max(a.max.y, b.max.y);
    return result;
}

struct r2_pair
{
    r2 one;
    r2 other;
};

inline r2_pair split_rect_x(r2 rect, float x)
{
    r2 one = rect;
    r2 other = rect;

    one.max.x = x;
    other.min.x = x;

    r2_pair result;
    result.one = one;
    result.other = other;
    return result;
}

inline r2_pair split_rect_y(r2 rect, float y)
{
    r2 one = rect;
    r2 other = rect;

    one.max.y = y;
    other.min.y = y;

    r2_pair result;
    result.one = one;
    result.other = other;
    return result;
}

inline r2_pair split_rect_x_percent(r2 rect, float p)
{
    float x = rect.min.x + p*get_width(rect);
    r2_pair result = split_rect_x(rect, x);
    return result;
}

inline r2_pair split_rect_y_percent(r2 rect, float p)
{
    float y = rect.min.y + p*get_height(rect);
    r2_pair result = split_rect_y(rect, y);
    return result;
}

inline r2 rect_grid_cell(r2 rect, v2i size, v2i pos)
{
    r2 result = {};
    result.min.x = lerp(rect.min.x, rect.max.x, ratio(pos.x, size.x));
    result.min.y = lerp(rect.min.y, rect.max.y, ratio(pos.y, size.y));
    result.max.x = lerp(rect.min.x, rect.max.x, ratio(pos.x + 1, size.x));
    result.max.y = lerp(rect.min.y, rect.max.y, ratio(pos.y + 1, size.y));

    return result;
}

inline r2 rect_grid_cell(r2 rect, v2i size, int index)
{
    v2i pos = index_to_xy(size.x, index);
    r2 result = rect_grid_cell(rect, size, pos);
    return result;
}

inline r2 rect_grid_square_cell_scrolldown(r2 rect, int column_count, int index)
{
    v2i pos = index_to_xy(column_count, index);
    
    r2 result = {};
    result.min.x = lerp(rect.min.x, rect.max.x, ratio(pos.x, column_count));
    result.max.x = lerp(rect.min.x, rect.max.x, ratio(pos.x + 1, column_count));

    float cell_size = result.max.x - result.min.x;

    result.max.y = rect.max.y - cell_size*pos.y;
    result.min.y = rect.max.y - cell_size*(pos.y + 1);

    return result;
}
    
// inline r2 rect_grid_square_cell_scrolldown(float top_y, float min_x, float max_x, int column_count, int index)
// {
//     v2i pos = index_to_xy(column_count, index);

//     float cell_size = ratio(max_x-min_x, (float)column_count);
    
//     r2 result = {};
//     result.min.x = pos.x*cell_size;
//     result.max.x = (pos.x + 1)*cell_size;

//     result.max.y = top_y - cell_size*pos.y;
//     result.min.y = top_y - cell_size*(pos.y + 1);

//     return result;
// }

// Rectangle 3D constructors
inline r3
rect_min_max(v3 min, v3 max)
{
    r3 result = {min, max};
    return result;
}

inline r3
rect_center_halfdim(v3 center, v3 halfdim)
{
    r3 result;
    result.min = center - halfdim;
    result.max = center + halfdim;

    return result;
}

inline r3
rect_center_dim(v3 center, v3 dim)
{
    r3 result = rect_center_halfdim(center, 0.5f*dim);
    return result;
}

// Rectangle 3D Operations
inline v3
rect_lerp(r3 rect, v3 point)
{
    v3 result;
    result.x = lerp(rect.min.x, rect.max.x, point.x);
    result.y = lerp(rect.min.y, rect.max.y, point.y);
    result.z = lerp(rect.min.z, rect.max.z, point.z);
    return result;
}

inline v3
get_center(r3 rect)
{
    v3 result = rect_lerp(rect, V3(0.5, 0.5, 0.5));
    return result;
}

inline r3 rect_scale(r3 rect, float scale)
{
    r3 result;
    result.min.x = scale*rect.min.x;
    result.min.y = scale*rect.min.y;
    result.min.z = scale*rect.min.z;
    result.max.x = scale*rect.max.x;
    result.max.y = scale*rect.max.y;
    result.max.z = scale*rect.max.z;

    return result;
}

inline r3
pad_rect(r3 rect, v3 padding)
{
    r3 result;
    result.min = rect.min - padding;
    result.max = rect.max + padding;
    return result;
}

inline r3
pad_rect(r3 rect, float padding)
{
    r3 result = pad_rect(rect, {padding,padding,padding});
    return result;
}

inline r3 rect_union(r3 a, r3 b)
{
    r3 result;
    result.min.x = Min(a.min.x, b.min.x);
    result.min.y = Min(a.min.y, b.min.y);
    result.min.z = Min(a.min.z, b.min.z);
    result.max.x = Max(a.max.x, b.max.x);
    result.max.y = Max(a.max.y, b.max.y);
    result.max.z = Max(a.max.z, b.max.z);
    return result;
}

// Integer Rectangle 2D constructors
inline r2i
rect_min_max(v2i min, v2i max)
{
    r2i result = {min, max};
    return result;
}

inline r2i
rect_min_max(s32 minx, s32 miny, s32 maxx, s32 maxy)
{
    r2i result = rect_min_max(V2i(minx, miny), V2i(maxx, maxx));
    return result;
}

inline r2i
rect_min_dim(v2i min, v2i dim)
{
    r2i result = {min, min+dim};
    return result;
}

inline r2i
rect_min_dim(s32 minx, s32 miny, s32 dimx, s32 dimy)
{
    r2i result = rect_min_dim(V2i(minx, miny), V2i(dimx, dimx));
    return result;
}

// Integer Rectangle 2D Operations
inline s32
get_width(r2i a)
{
    s32 result = a.max.x - a.min.x;
    return result;
}

inline s32
get_height(r2i a)
{
    s32 result = a.max.y - a.min.y;
    return result;
}

inline v2i
get_size(r2i a)
{
    v2i result;
    result.x = get_width(a);
    result.y = get_height(a);
    return result;
}

inline r2 rect_scale(r2i rect, float scale)
{
    r2 result;
    result.min.x = scale*rect.min.x;
    result.min.y = scale*rect.min.y;
    result.max.x = scale*rect.max.x;
    result.max.y = scale*rect.max.y;

    return result;
}

//

// Collision

inline b32x
collides(v2 point, r2 rect)
{
    b32x result =
        (point.x > rect.min.x && point.x < rect.max.x) &&
        (point.y > rect.min.y && point.y < rect.max.y);
    
    return result;
}

// NOTE(lubo): Point must already collide with rect. Kinda weird.
inline v2
collision_resolve(v2 point, r2 rect, v2 *vel=0)
{
    float distancetoleft = point.x - rect.min.x;
    float distancetoright = rect.max.x - point.x;
    float distancetobottom = point.y - rect.min.y;
    float distancetotop = rect.max.y - point.y;

    float minx = Min(distancetoleft, distancetoright);
    float miny = Min(distancetotop, distancetobottom);

    v2 result = point;
    
    if(minx < miny)
    {
        if(distancetoleft < distancetoright)
        {
            result.x = rect.min.x;
            if(vel && vel->x > 0) vel->x = 0;
        }
        else
        {
            result.x = rect.max.x;
            if(vel && vel->x < 0) vel->x = 0;
        }
    }
    else
    {
        if(distancetobottom < distancetotop)
        {
            result.y = rect.min.y;
            if(vel && vel->y > 0) vel->y = 0;
        }
        else
        {
            result.y = rect.max.y;
            if(vel && vel->y < 0) vel->y = 0;
        }
    }

    return result;
}

inline float get_acceptance_dot(float angle)
{
    float result = cos(angle/2);
    return result;
}

inline b32x collides_cone(v2 center, v2 dir, float range, float angle, v2 p)
{
    b32x angle_check = dot(noz(p - center), noz(dir)) >= cos(angle/2);
    b32x range_check = distance(center, p) <= range;
    b32x result = angle_check && range_check;
    return result;
}

// NOTE(lubo): Point must already collide with rect. Kinda weird.
/*
inline v2
collision_resolve_diamond(v2 point, r2 rect, v2 *vel=0)
{
    v2 result = point;
    v2 center = get_center(rect);

    v2 up = get_point_in_rect(rect, V2(0.5f, 1));
    v2 down = get_point_in_rect(rect, V2(0.5f, 0));
    v2 left = get_point_in_rect(rect, V2(0, 0.5f));
    v2 right = get_point_in_rect(rect, V2(1, 0.5f));
    
    if(point.x < center.x)
    {
        if(point.y < center.y)
        {
            project_to_line_if_on_the_left(left, down, point);
        }
        else
        {
        }
    }
    
    float distancetoleft = point.x - rect.min.x;
    float distancetoright = rect.max.x - point.x;
    float distancetobottom = point.y - rect.min.y;
    float distancetotop = rect.max.y - point.y;

    float minx = Min(distancetoleft, distancetoright);
    float miny = Min(distancetotop, distancetobottom);

    v2 result = point;
    
    if(minx < miny)
    {
        if(distancetoleft < distancetoright)
        {
            result.x = rect.min.x;
            if(vel && vel->x > 0) vel->x = 0;
        }
        else
        {
            result.x = rect.max.x;
            if(vel && vel->x < 0) vel->x = 0;
        }
    }
    else
    {
        if(distancetobottom < distancetotop)
        {
            result.y = rect.min.y;
            if(vel && vel->y > 0) vel->y = 0;
        }
        else
        {
            result.y = rect.max.y;
            if(vel && vel->y < 0) vel->y = 0;
        }
    }

    return result;
}
*/

inline b32x
collides(float amin, float amax, float bmin, float bmax)
{
    b32x result = (amax > bmin) && (bmax > amin);
    return result;
}

inline b32x
collides(r2 a, r2 b)
{
    b32x result =
        collides(a.min.x, a.max.x, b.min.x, b.max.x) &&
        collides(a.min.y, a.max.y, b.min.y, b.max.y);
    
    return result;
}


// NOTE(lubo): Point must already collide with rect. Kinda weird.
inline v2
collision_resolve(r2 a, r2 b, v2 *vel=0)
{
    b.max += get_size(a);
    v2 result = collision_resolve(a.min, b, vel);
    return result;
}

inline b32x
collides(r3 a, r3 b)
{
    b32x result =
        collides(a.min.x, a.max.x, b.min.x, b.max.x) &&
        collides(a.min.y, a.max.y, b.min.y, b.max.y) &&
        collides(a.min.z, a.max.z, b.min.z, b.max.z);
    
    return result;
}

inline b32x
collides(v3i v, r3i r)
{
    b32x result =
        v.x >= r.min.x && v.x <= r.max.x &&
        v.y >= r.min.y && v.y <= r.max.y &&
        v.z >= r.min.z && v.z <= r.max.z;
    return result;
}

// NOTE(lubo): Is 'a' a subset of 'b'? (does b fully contain a?)
inline b32x subset(float amin, float amax, float bmin, float bmax)
{
    b32x result = (bmin <= amin) && (amax <= bmax);
    return result;
}

inline float subset_resolve(float amin, float amax, float bmin, float bmax)
{
    float adelta = 0;
    
    float out_from_left = bmin - amin;
    float out_from_right = amax - bmax;

    if(out_from_left > 0)
    {
        if(out_from_right > 0)
        {
            // NOTE(lubo): a can not fit into b
            Assert(false);
        }
        else
        {
            adelta = out_from_left;
        }
    }
    else if(out_from_right > 0)
    {
        adelta = -out_from_right;
    }
    else
    {
        // NOTE(lubo): No offset needed
    }

    return adelta;
}

inline b32x subset(r2 a, r2 b)
{
    b32x result =
        subset(a.min.x, a.max.x, b.min.x, b.max.x) &&
        subset(a.min.y, a.max.y, b.min.y, b.max.y);
    
    return result;
}

inline void subset_resolve(r2 a, r2 b)
{
}

// Color

inline v4
color_from_u32(u32 Color)
{
    v4 Result;
    Result.a = ((Color>>24)&0xFF)/255.0f;
    Result.b = ((Color>>16)&0xFF)/255.0f;
    Result.g = ((Color>> 8)&0xFF)/255.0f;
    Result.r = ((Color>> 0)&0xFF)/255.0f;
    return Result;
}

inline u32
color_to_u32(v4 Color)
{
    u32 Result =
        (round_to_int(255*Color.a)<<24)|
        (round_to_int(255*Color.b)<<16)|
        (round_to_int(255*Color.g)<< 8)|
        (round_to_int(255*Color.r)<< 0);
    return Result;
}

#define GAMMA 2.2f
internal float gamma_to_linear(float Value)
{
    float Result = exp_with_base(Value, GAMMA);
    return Result;
}
internal float
linear_to_gamma(float value)
{
    float result = exp_with_base(value, 1/GAMMA);
    return result;
}
internal v4
color_to_gamma(v4 color)
{
    v4 result;
    result.r = linear_to_gamma(color.r);
    result.g = linear_to_gamma(color.g);
    result.b = linear_to_gamma(color.b);
    result.a = color.a;
    return result;
}
internal v4
color_to_linear(v4 color)
{
    v4 result;
    result.r = gamma_to_linear(color.r);
    result.g = gamma_to_linear(color.g);
    result.b = gamma_to_linear(color.b);
    result.a = color.a;
    return result;
}

// Quaternion Constructors
inline quaternion identity_quaternion()
{
    quaternion Q = {0,0,0,1};
    return Q;
}

inline quaternion
operator*(quaternion a, quaternion b)
{
    quaternion q;
    q.xyz = cross(a.xyz,b.xyz) + a.w*b.xyz + b.w*a.xyz;
    q.w = a.w*b.w - dot(a.xyz,b.xyz);
    // Q.x = A.y*B.z - A.z*B.y + A.w*B.x + B.w*A.x;
    // Q.y = A.z*B.x - A.x*B.z + A.w*B.y + B.w*A.y;
    // Q.z = A.x*B.y - A.y*B.x + A.w*B.z + B.w*A.z;
    // Q.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;

    return q;
}

inline quaternion quaternion_axis_theta(v3 v, float theta)
{
    quaternion Q;
    Q.xyz = v * sin(theta/2);
    Q.w = cos(theta/2);
    return Q;
};

// yaw (Z), pitch (Y), roll (X)
inline quaternion quaternion_from_angles(float yaw, float pitch, float roll)
{
    float cy = cos(0.5f*yaw);
    float sy = sin(0.5f*yaw);
    float cp = cos(0.5f*pitch);
    float sp = sin(0.5f*pitch);
    float cr = cos(0.5f*roll);
    float sr = sin(0.5f*roll);

    quaternion q;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    q.w = cr * cp * cy + sr * sp * sy;

    return q;
}

inline quaternion quaternion_from_vectors(v3 A, v3 B)
{
    quaternion Q;
    Q.xyz = cross(A, B);
    Q.w = dot(A, B);
    return Q;
}

// Quaternion Operations

inline quaternion conjugate(quaternion A)
{
    quaternion Q;
    Q.xyz = A.xyz;
    Q.w = -A.w;
    return Q;
}

inline float
inner(quaternion A, quaternion B)
{
    float Q = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
    return Q;
}

inline float
norm(quaternion A)
{
    float Q = inner(A, A);
    return Q;
}

inline quaternion
inverse(quaternion A)
{
    quaternion Q = conjugate(A);
    return Q;
}

inline quaternion
lerp(quaternion A, quaternion B, float t)
{
    if(inner(A, B) < 0)
    {
        A = -1*A;
    }
    quaternion Q = (1-t)*A + t*B;
    return Q;
}

inline quaternion
normalize(quaternion A)
{
    quaternion Q = (1/square_root(norm(A)))*A;
    return Q;
}

inline quaternion
nlerp(quaternion A, quaternion B, float t)
{
    quaternion Q = normalize(lerp(A, B, t));
    return Q;
}

// TODO(lubo): THIS IS PROBABLY WRONG!
#if 0
inline quaternion
slerp(quaternion a, quaternion b, float t)
{
    t /= 2;
    float theta = absolute_value(arccos(inner(a, b)));
    float sin_theta = sin(theta);
    float c0 = sin((1-t)*theta) / sin_theta;
    float c1 = sin(t*theta) / sin_theta;
    
    quaternion result = normalize(c0*a + c1*b);
    return result;
}
#endif

// Matrix 2x2
inline m2
operator*(float s, m2 A)
{
    m2 R =
    {
        s*A.E[0][0], s*A.E[0][1],
        s*A.E[1][0], s*A.E[1][1],
    };
    
    return R;
}

inline m3
operator*(float s, m3 A)
{
    m3 R =
    {
        s*A.E[0][0], s*A.E[0][1], s*A.E[0][2],
        s*A.E[1][0], s*A.E[1][1], s*A.E[1][2],
        s*A.E[2][0], s*A.E[2][1], s*A.E[2][2],
    };
    
    return R;
}

inline v2
operator*(m2 A, v2 v)
{
    v2 result =
    {
        v.x*A.E[0][0] + v.y*A.E[0][1],
        v.x*A.E[1][0] + v.y*A.E[1][1],
    };
    
    return result;
}

inline v2 rot45(v2 a)
{
    m2 R =
    {
        InvSqrt2_32, -InvSqrt2_32,
        InvSqrt2_32,  InvSqrt2_32,
    };
    
    v2 result = R*a;
    return result;
}

inline v2 rot_neg45(v2 a)
{
    m2 R =
    {
         InvSqrt2_32, InvSqrt2_32,
        -InvSqrt2_32, InvSqrt2_32,
    };
    
    v2 result = R*a;
    return result;
}

inline v3
operator*(m3 A, v3 v)
{
    v3 result =
    {
        v.x*A.E[0][0] + v.y*A.E[0][1] + v.z*A.E[0][2],
        v.x*A.E[1][0] + v.y*A.E[1][1] + v.z*A.E[1][2],
        v.x*A.E[2][0] + v.y*A.E[2][1] + v.z*A.E[2][2],
    };
    
    return result;
}

inline float determinant(m2 M)
{
    float result = M.a*M.d - M.b*M.c;
    return result;
}

inline float determinant(m3 M)
{
    float result = M.a*M.e*M.i + M.b*M.f*M.g + M.c*M.d*M.h - M.c*M.e*M.g - M.b*M.d*M.i - M.a*M.f*M.h;
    return result;
}

inline m2 inverse(m2 A)
{
    m2 B =
    {
        A.d, -A.b,
        -A.c, A.a,
    };

    m2 R = (1/determinant(A)) * B;
    return R;
}

inline m2 rotation_matrix(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    
    m2 R =
    {
        c,-s,
        s, c,
    };

    return R;
}

inline v2 rotate(v2 a, float angle)
{
    m2 R = rotation_matrix(angle);
    v2 result = R * a;
    return result;
}

// NOTE(lubo): Works best with small angles!
inline v2 rotate_imprecise(v2 a, float angle)
{
    // NOTE(lubo): Taylor expansion with n=1
    float c = 1;
    float s = angle;
    
    m2 R =
    {
        c,-s,
        s, c,
    };
    
    v2 result = R * a;
    return result;
}
inline v2 rotate_imprecise2(v2 a, float angle)
{
    // NOTE(lubo): Taylor expansion with n=2
    float c = 1 - square(angle)/2;
    float s = angle;
    
    m2 R =
    {
        c,-s,
        s, c,
    };
    
    v2 result = R * a;
    return result;
}
inline v2 rotate_imprecise3(v2 a, float angle)
{
    // NOTE(lubo): Taylor expansion with n=3
    float a2 = angle*angle;
    float c = 1 - a2/2;
    float s = angle - angle*a2/6;
    
    m2 R =
    {
        c,-s,
        s, c,
    };
    
    v2 result = R * a;
    return result;
}

inline m3 inverse(m3 A)
{
    float a =  (A.e*A.i - A.f*A.h);
    float b = -(A.d*A.i - A.f*A.g);
    float c =  (A.d*A.h - A.e*A.g);
    float d = -(A.b*A.i - A.c*A.h);
    float e =  (A.a*A.i - A.c*A.g);
    float f = -(A.a*A.h - A.b*A.g);
    float g =  (A.b*A.f - A.c*A.e);
    float h = -(A.a*A.f - A.c*A.d);
    float i =  (A.a*A.e - A.b*A.d);
    
    m3 B =
    {
        a, d, g,
        b, e, h,
        c, f, i,
    };

    m3 R = (1/determinant(A)) * B;
    return R;
}

// Matrix 4x4 Constructors

inline m4 identity_matrix()
{
    m4 I =
    {
        {1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1},
    };
    
    return I;
}

inline m4 scale_matrix(float scale)
{
    float s = scale;
    
    m4 S =
    {
        {s, 0, 0, 0,
         0, s, 0, 0,
         0, 0, s, 0,
         0, 0, 0, 1},
    };
    
    return S;
}

inline m4 scale_matrix(v3 scale)
{
    m4 S =
    {
        {scale.x, 0, 0, 0,
         0, scale.y, 0, 0,
         0, 0, scale.z, 0,
         0, 0, 0, 1},
    };
    
    return S;
}

inline m4
translation_matrix(v3 t)
{
#if 0
    m4 T =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        t.x, t.y, t.z, 1,
    };
#else
    m4 T =
    {
        1, 0, 0, t.x,
        0, 1, 0, t.y,
        0, 0, 1, t.z,
        0, 0, 0, 1,
    };
#endif
    
    return T;
}

// NOTE(lubomir): These two assume up is (0,0,1)
inline v3 forward_direction_from_pitch_and_yaw(float pitch, float yaw)
{
    v3 result;
    result.x = cos(pitch) * -sin(yaw);
    result.y = cos(pitch) * cos(yaw);
    result.z = sin(pitch);

    return result;
}
inline v3 right_direction_from_and_yaw(float yaw)
{
    v3 result;
    result.x = cos(yaw);
    result.y = sin(yaw);
    result.z = 0;
    return result;
}

inline m4_pair look_from_base_vectors(v3 eye, v3 X, v3 Y, v3 Z)
{
    v3 P = eye;

    v3 MP;
    MP.x = -(X.x*P.x + X.y*P.y + X.z*P.z);
    MP.y = -(Y.x*P.x + Y.y*P.y + Y.z*P.z);
    MP.z = -(Z.x*P.x + Z.y*P.y + Z.z*P.z);
    
    m4 M = identity_matrix();
    M.E[0][0] = X.x;
    M.E[0][1] = X.y;
    M.E[0][2] = X.z;
    M.E[1][0] = Y.x;
    M.E[1][1] = Y.y;
    M.E[1][2] = Y.z;
    M.E[2][0] = Z.x;
    M.E[2][1] = Z.y;
    M.E[2][2] = Z.z;
    M.E[0][3] = MP.x;
    M.E[1][3] = MP.y;
    M.E[2][3] = MP.z;

    m4 N = identity_matrix();
    N.E[0][0] = X.x;
    N.E[1][0] = X.y;
    N.E[2][0] = X.z;
    N.E[0][1] = Y.x;
    N.E[1][1] = Y.y;
    N.E[2][1] = Y.z;
    N.E[0][2] = Z.x;
    N.E[1][2] = Z.y;
    N.E[2][2] = Z.z;
    /**/
    N.E[0][3] = -(X.x*MP.x + Y.x*MP.y + Z.x*MP.z);
    N.E[1][3] = -(X.y*MP.x + Y.y*MP.y + Z.y*MP.z);
    N.E[2][3] = -(X.z*MP.x + Y.z*MP.y + Z.z*MP.z);
    
    m4_pair result;
    result.forward = M;
    result.inverse = N;
    return result;
}

inline m4_pair look_in_direction(v3 eye, v3 dir, v3 up)
{
    v3 Z = -dir;
    v3 X = normalize(cross(dir, up));
    v3 Y = cross(Z, X);
    m4_pair M = look_from_base_vectors(eye, X, Y, Z);
    return M;
}

inline m4_pair look_at(v3 eye, v3 target, v3 up)
{
    m4_pair M = look_in_direction(eye, normalize(target - eye), up);
    return M;
}

internal m4_pair orthographic(float inv_vertical_size, float aspect, float z_near, float z_far)
{
    float a = inv_vertical_size;
    float b = inv_vertical_size*aspect;
    float n = z_near;
    float f = z_far;
    float d = 2.0f / (n - f);
    float e = (n + f) / (n - f);

    m4_pair result =
    {
        {a,  0,  0,  0,
         0,  b,  0,  0,
         0,  0,  d,  e,
         0,  0,  0,  1},

        {1/a,   0,   0,    0,
           0, 1/b,   0,    0,
           0,   0, 1/d, -e/d,
           0,   0,   0,    1},
    };
    
    return result;
}

internal m4_pair perspective(float fovy, float aspect, float _near, float _far)
{
    //Assert(aspect != 0);
    Assert(_far != _near);

    float rad = fovy;
    float tanHalfFovy = tan(rad/2);

    float d = -(_far + _near) / (_far - _near);
    float e = -(2*_far*_near) / (_far - _near);
    
    m4 F = {};
    F.E[0][0] = 1 / (aspect * tanHalfFovy);
    F.E[1][1] = 1 / (tanHalfFovy);
    F.E[2][2] = d;
    F.E[3][2] = -1;
    F.E[2][3] = e;

    m4 R = {};
    R.E[0][0] = aspect * tanHalfFovy;
    R.E[1][1] = tanHalfFovy;
    R.E[3][2] = 1/e;
    R.E[2][3] = -1;
    R.E[3][3] = d/e;

    m4_pair result;
    result.forward = F;
    result.inverse = R;
    
    return result;
}

inline m3 m3_from_quaternion(quaternion Q)
{
    float N = norm(Q);
    float s = N > 0 ? 2/N : 0;
    float xs = Q.x * s;
    float ys = Q.y * s;
    float zs = Q.z * s;
    
    float wx = Q.w * xs;
    float wy = Q.w * ys;
    float wz = Q.w * zs;

    float x2 = Q.x * xs;
    float y2 = Q.y * ys;
    float z2 = Q.z * zs;

    float xy = Q.x * ys;
    float yz = Q.y * zs;
    float zx = Q.z * xs;

    float a = 1 - (y2 + z2);
    float b = xy + wz;
    float c = zx - wy;
    float d = xy - wz;
    float e = 1 - (x2 + z2);
    float f = yz + wx;
    float g = zx + wy;
    float h = yz - wx;
    float i = 1 - (x2 + y2);

#if 0
    m3 R =
    {
        a, d, g,
        b, e, h,
        c, f, i,
    };
#else
    m3 R =
    {
        a, b, c,
        d, e, f,
        g, h, i,
    };
#endif

    return R;
}
inline v3
operator*(quaternion q, v3 v)
{
    v3 _result = 2.0f * dot(q.xyz, v) * q.xyz
          + (q.w*q.w - dot(q.xyz, q.xyz)) * v
          + 2.0f * q.w * cross(q.xyz, v);

    v3 result = m3_from_quaternion(q)*v;
    
    return result;
}

inline m4 matrix_from_quaternion(quaternion Q)
{
    float N = norm(Q);
    float s = N > 0 ? 2/N : 0;
    float xs = Q.x * s;
    float ys = Q.y * s;
    float zs = Q.z * s;
    
    float wx = Q.w * xs;
    float wy = Q.w * ys;
    float wz = Q.w * zs;

    float x2 = Q.x * xs;
    float y2 = Q.y * ys;
    float z2 = Q.z * zs;

    float xy = Q.x * ys;
    float yz = Q.y * zs;
    float zx = Q.z * xs;

    float a = 1 - (y2 + z2);
    float b = xy + wz;
    float c = zx - wy;
    float d = xy - wz;
    float e = 1 - (x2 + z2);
    float f = yz + wx;
    float g = zx + wy;
    float h = yz - wx;
    float i = 1 - (x2 + y2);

#if 0
    m4 R =
    {
        a, d, g, 0,
        b, e, h, 0,
        c, f, i, 0,
        0, 0, 0, 1,
    };
#else
    m4 R =
    {
        a, b, c, 0,
        d, e, f, 0,
        g, h, i, 0,
        0, 0, 0, 1,
    };
#endif

    return R;
}

// Matrix 4x4 Operations
inline v3
get_column(m4 A, u32 c)
{
    v3 result = {A.E[0][c], A.E[1][c], A.E[2][c]};
    return result;
}

inline v3
get_row(m4 A, u32 r)
{
    v3 result = {A.E[r][0], A.E[r][1], A.E[r][2]};
    return result;
}

#if 1
inline m4
transpose(m4 A)
{
    m4 M;
    
    for(int j = 0; j < 4; ++j)
    {
        for(int i = 0; i < 4; ++i)
        {
            M.E[j][i] = A.E[i][j];
        }
    }
    
    return M;
}
#endif

inline m3
operator*(m3 A, m3 B)
{
    m3 R;

#if 1
    R.E[0][0] = A.E[0][0]*B.E[0][0] + A.E[0][1]*B.E[1][0] + A.E[0][2]*B.E[2][0];
    R.E[1][0] = A.E[1][0]*B.E[0][0] + A.E[1][1]*B.E[1][0] + A.E[1][2]*B.E[2][0];
    R.E[2][0] = A.E[2][0]*B.E[0][0] + A.E[2][1]*B.E[1][0] + A.E[2][2]*B.E[2][0];
    R.E[0][1] = A.E[0][0]*B.E[0][1] + A.E[0][1]*B.E[1][1] + A.E[0][2]*B.E[2][1];
    R.E[1][1] = A.E[1][0]*B.E[0][1] + A.E[1][1]*B.E[1][1] + A.E[1][2]*B.E[2][1];
    R.E[2][1] = A.E[2][0]*B.E[0][1] + A.E[2][1]*B.E[1][1] + A.E[2][2]*B.E[2][1];
    R.E[0][2] = A.E[0][0]*B.E[0][2] + A.E[0][1]*B.E[1][2] + A.E[0][2]*B.E[2][2];
    R.E[1][2] = A.E[1][0]*B.E[0][2] + A.E[1][1]*B.E[1][2] + A.E[1][2]*B.E[2][2];
    R.E[2][2] = A.E[2][0]*B.E[0][2] + A.E[2][1]*B.E[1][2] + A.E[2][2]*B.E[2][2];
#else    
    R.E[0][0] = A.E[0][0]*B.E[0][0] + A.E[0][1]*B.E[1][0] + A.E[0][2]*B.E[2][0];
    R.E[0][1] = A.E[1][0]*B.E[0][0] + A.E[1][1]*B.E[1][0] + A.E[1][2]*B.E[2][0];
    R.E[0][2] = A.E[2][0]*B.E[0][0] + A.E[2][1]*B.E[1][0] + A.E[2][2]*B.E[2][0];
    R.E[1][0] = A.E[0][0]*B.E[0][1] + A.E[0][1]*B.E[1][1] + A.E[0][2]*B.E[2][1];
    R.E[1][1] = A.E[1][0]*B.E[0][1] + A.E[1][1]*B.E[1][1] + A.E[1][2]*B.E[2][1];
    R.E[1][2] = A.E[2][0]*B.E[0][1] + A.E[2][1]*B.E[1][1] + A.E[2][2]*B.E[2][1];
    R.E[2][0] = A.E[0][0]*B.E[0][2] + A.E[0][1]*B.E[1][2] + A.E[0][2]*B.E[2][2];
    R.E[2][1] = A.E[1][0]*B.E[0][2] + A.E[1][1]*B.E[1][2] + A.E[1][2]*B.E[2][2];
    R.E[2][2] = A.E[2][0]*B.E[0][2] + A.E[2][1]*B.E[1][2] + A.E[2][2]*B.E[2][2];
#endif
    
    return R;
}

// NOTE(lubo): Look at this big boy!
inline m4
operator*(m4 A, m4 B)
{
    m4 R;

#if 1
    R.E[0][0] = A.E[0][0]*B.E[0][0] + A.E[0][1]*B.E[1][0] + A.E[0][2]*B.E[2][0] + A.E[0][3]*B.E[3][0];
    R.E[1][0] = A.E[1][0]*B.E[0][0] + A.E[1][1]*B.E[1][0] + A.E[1][2]*B.E[2][0] + A.E[1][3]*B.E[3][0];
    R.E[2][0] = A.E[2][0]*B.E[0][0] + A.E[2][1]*B.E[1][0] + A.E[2][2]*B.E[2][0] + A.E[2][3]*B.E[3][0];
    R.E[3][0] = A.E[3][0]*B.E[0][0] + A.E[3][1]*B.E[1][0] + A.E[3][2]*B.E[2][0] + A.E[3][3]*B.E[3][0];
    R.E[0][1] = A.E[0][0]*B.E[0][1] + A.E[0][1]*B.E[1][1] + A.E[0][2]*B.E[2][1] + A.E[0][3]*B.E[3][1];
    R.E[1][1] = A.E[1][0]*B.E[0][1] + A.E[1][1]*B.E[1][1] + A.E[1][2]*B.E[2][1] + A.E[1][3]*B.E[3][1];
    R.E[2][1] = A.E[2][0]*B.E[0][1] + A.E[2][1]*B.E[1][1] + A.E[2][2]*B.E[2][1] + A.E[2][3]*B.E[3][1];
    R.E[3][1] = A.E[3][0]*B.E[0][1] + A.E[3][1]*B.E[1][1] + A.E[3][2]*B.E[2][1] + A.E[3][3]*B.E[3][1];
    R.E[0][2] = A.E[0][0]*B.E[0][2] + A.E[0][1]*B.E[1][2] + A.E[0][2]*B.E[2][2] + A.E[0][3]*B.E[3][2];
    R.E[1][2] = A.E[1][0]*B.E[0][2] + A.E[1][1]*B.E[1][2] + A.E[1][2]*B.E[2][2] + A.E[1][3]*B.E[3][2];
    R.E[2][2] = A.E[2][0]*B.E[0][2] + A.E[2][1]*B.E[1][2] + A.E[2][2]*B.E[2][2] + A.E[2][3]*B.E[3][2];
    R.E[3][2] = A.E[3][0]*B.E[0][2] + A.E[3][1]*B.E[1][2] + A.E[3][2]*B.E[2][2] + A.E[3][3]*B.E[3][2];
    R.E[0][3] = A.E[0][0]*B.E[0][3] + A.E[0][1]*B.E[1][3] + A.E[0][2]*B.E[2][3] + A.E[0][3]*B.E[3][3];
    R.E[1][3] = A.E[1][0]*B.E[0][3] + A.E[1][1]*B.E[1][3] + A.E[1][2]*B.E[2][3] + A.E[1][3]*B.E[3][3];
    R.E[2][3] = A.E[2][0]*B.E[0][3] + A.E[2][1]*B.E[1][3] + A.E[2][2]*B.E[2][3] + A.E[2][3]*B.E[3][3];
    R.E[3][3] = A.E[3][0]*B.E[0][3] + A.E[3][1]*B.E[1][3] + A.E[3][2]*B.E[2][3] + A.E[3][3]*B.E[3][3];
#else    
    R.E[0][0] = A.E[0][0]*B.E[0][0] + A.E[0][1]*B.E[1][0] + A.E[0][2]*B.E[2][0] + A.E[0][3]*B.E[3][0];
    R.E[0][1] = A.E[1][0]*B.E[0][0] + A.E[1][1]*B.E[1][0] + A.E[1][2]*B.E[2][0] + A.E[1][3]*B.E[3][0];
    R.E[0][2] = A.E[2][0]*B.E[0][0] + A.E[2][1]*B.E[1][0] + A.E[2][2]*B.E[2][0] + A.E[2][3]*B.E[3][0];
    R.E[0][3] = A.E[3][0]*B.E[0][0] + A.E[3][1]*B.E[1][0] + A.E[3][2]*B.E[2][0] + A.E[3][3]*B.E[3][0];
    R.E[1][0] = A.E[0][0]*B.E[0][1] + A.E[0][1]*B.E[1][1] + A.E[0][2]*B.E[2][1] + A.E[0][3]*B.E[3][1];
    R.E[1][1] = A.E[1][0]*B.E[0][1] + A.E[1][1]*B.E[1][1] + A.E[1][2]*B.E[2][1] + A.E[1][3]*B.E[3][1];
    R.E[1][2] = A.E[2][0]*B.E[0][1] + A.E[2][1]*B.E[1][1] + A.E[2][2]*B.E[2][1] + A.E[2][3]*B.E[3][1];
    R.E[1][3] = A.E[3][0]*B.E[0][1] + A.E[3][1]*B.E[1][1] + A.E[3][2]*B.E[2][1] + A.E[3][3]*B.E[3][1];
    R.E[2][0] = A.E[0][0]*B.E[0][2] + A.E[0][1]*B.E[1][2] + A.E[0][2]*B.E[2][2] + A.E[0][3]*B.E[3][2];
    R.E[2][1] = A.E[1][0]*B.E[0][2] + A.E[1][1]*B.E[1][2] + A.E[1][2]*B.E[2][2] + A.E[1][3]*B.E[3][2];
    R.E[2][2] = A.E[2][0]*B.E[0][2] + A.E[2][1]*B.E[1][2] + A.E[2][2]*B.E[2][2] + A.E[2][3]*B.E[3][2];
    R.E[2][3] = A.E[3][0]*B.E[0][2] + A.E[3][1]*B.E[1][2] + A.E[3][2]*B.E[2][2] + A.E[3][3]*B.E[3][2];
    R.E[3][0] = A.E[0][0]*B.E[0][3] + A.E[0][1]*B.E[1][3] + A.E[0][2]*B.E[2][3] + A.E[0][3]*B.E[3][3];
    R.E[3][1] = A.E[1][0]*B.E[0][3] + A.E[1][1]*B.E[1][3] + A.E[1][2]*B.E[2][3] + A.E[1][3]*B.E[3][3];
    R.E[3][2] = A.E[2][0]*B.E[0][3] + A.E[2][1]*B.E[1][3] + A.E[2][2]*B.E[2][3] + A.E[2][3]*B.E[3][3];
    R.E[3][3] = A.E[3][0]*B.E[0][3] + A.E[3][1]*B.E[1][3] + A.E[3][2]*B.E[2][3] + A.E[3][3]*B.E[3][3];
#endif
    
    return R;
}

internal float trace(m4 A)
{
    float result = A.E[0][0] + A.E[1][1] + A.E[2][2] + A.E[3][3];
    return result;
}

internal v4
transform(m4 A, v4 P)
{
    // NOTE(casey): This is written to be instructive, not optimal!
    // NOTE(lubo): Thank you casey
    
    v4 R;
    
    R.x = P.x*A.E[0][0] + P.y*A.E[0][1] + P.z*A.E[0][2] + P.w*A.E[0][3];
    R.y = P.x*A.E[1][0] + P.y*A.E[1][1] + P.z*A.E[1][2] + P.w*A.E[1][3];
    R.z = P.x*A.E[2][0] + P.y*A.E[2][1] + P.z*A.E[2][2] + P.w*A.E[2][3];
    R.w = P.x*A.E[3][0] + P.y*A.E[3][1] + P.z*A.E[3][2] + P.w*A.E[3][3];
    
    return R;
}

inline v3
operator*(m4 A, v3 P)
{
    v3 R = transform(A, V4(P, 1.0f)).xyz;
    return R;
}

inline v4
operator*(m4 A, v4 P)
{
    v4 R = transform(A, P);
    return R;
}

// Composite problems

// NOTE(lubo): Scale, rotate, translate
inline v3 transform(Transform *transform, v3 a)
{
    // v3 result = (transform->orientation * (transform->scale * a)) + transform->pos;
    v3 result = (transform->orientation * (hadamard(transform->scale_axes, a))) + transform->pos;
    return result;
}

inline v3 inverse_transform(Transform *transform, v3 a)
{
    // v3 result = (inverse(transform->orientation) * (a - transform->pos))/(transform->scale);
    v3 result = hadamard((inverse(transform->orientation) * (a - transform->pos)), 1/(transform->scale_axes));
    return result;
}

// NOTE(lubo): Scale, rotate
inline v3 transform_vel(Transform *transform, v3 a)
{
    // v3 result = transform->orientation * (transform->scale * a);
    v3 result = transform->orientation * (hadamard(transform->scale_axes, a));
    return result;
}

inline v3 inverse_transform_vel(Transform *transform, v3 a)
{
    // v3 result = (inverse(transform->orientation) * a)/transform->scale;
    v3 result = hadamard((inverse(transform->orientation) * a), 1/(transform->scale_axes));
    return result;
}

// NOTE(lubo): Just rotate
inline v3 transform_normal(Transform *transform, v3 n)
{
    v3 result = transform->orientation * n;
    return result;
}

inline v3 inverse_transform_normal(Transform *transform, v3 n)
{
    v3 result = inverse(transform->orientation) * n;
    return result;
}

// TODO(lubo): There should be a more efficient way than multiply 3 4x4 matrices.
internal m4 compute_transform_matrix(Transform *transform)
{
    // m4 M =
    //     translation_matrix(transform->pos) *
    //     matrix_from_quaternion(transform->orientation) *
    //     scale_matrix(transform->scale_axes);

    m4 M = matrix_from_quaternion(transform->orientation);
    v3 s = transform->scale_axes;
    v3 t = transform->pos;
    M.a *= s.x;
    M.e *= s.x;
    M.i *= s.x;
    M.b *= s.y;
    M.f *= s.y;
    M.j *= s.y;
    M.c *= s.z;
    M.g *= s.z;
    M.k *= s.z;
    M.d = t.x;
    M.h = t.y;
    M.l = t.z;
    
    return M;
}

internal m4 compute_inverse_transform_matrix(Transform *transform)
{
    m4 M =
        scale_matrix(1/transform->scale_axes) *
        matrix_from_quaternion(inverse(transform->orientation)) *
        translation_matrix(-transform->pos);

    return M;
}

#if 0
// NOTE(lubo): Individual scale axes make transform not behave as a
// group under composition.  My lightly experimentally tested theory
// is that uniform-scale-only keeps the group property.  This could
// simplify composition.  Not sure what we want, for now we're
// suspending this and work with full matrices when composing.

// NOTE(lubo): result = a * b
// That means result represent combined transformation of:
//  1. scale, rotate, translate by 'b'
//  2. scale, rotate, translate by 'a'
inline void combine_transforms(Transform *result, Transform *a, Transform *b)
{
    result->pos = transform(a, b->pos);
    result->orientation = a->orientation * b->orientation;
    //result->scale = a->scale * b->scale;

    // NOTE(lubo): This is wrong. Rotations with individual scale can introduce shear.
    result->scale_axes = hadamard(a->scale_axes, b->scale_axes);
}
#endif

inline Ray transform_ray(Transform *t, Ray ray)
{
    Ray result = {};
    result.eye = transform(t, ray.eye);
    result.dir = transform_normal(t, ray.dir);
    return result;
}

inline Ray inverse_transform_ray(Transform *transform, Ray ray)
{
    Ray result = {};
    result.eye = inverse_transform(transform, ray.eye);
    result.dir = inverse_transform_normal(transform, ray.dir);
    return result;
}


// NOTE(lubo): Barycentric coordinates
// Gives coordinates l0, l1, l2, ...
//   of a point p
//   in terms of triangle bases: AB, AC, AD, ...
//   such that p = l0*AB + l1*AC + l2*AD + ...
// 1D version degenerates into inverse lerp

// STUDY(lubo): Advantages - we always only need one division and we
// can generalize to arbitrary number of dimensions.  Disadvantage -
// we need dimensional correspondence - for example we would like to
// get coordinates inside a triangle in 3D space and without
// projecting and reprojecting a triangle will only accept v2's.
//
// Solution: In the end we didn't use this for gjk closest feature
// and instead used multidimensional inv_lerp to get distances along each axis
// 
inline float get_barycentric_coordinates(float a, float b, float p)
{
    float T = b - a;
    float result = (1/T) * (p-a);
    //float result = (p-a)/(b-a);
    return result;
}

inline v2 get_barycentric_coordinates(v2 a, v2 b, v2 c, v2 p)
{
    m2 T =
    {
        b.x-a.x, c.x-a.x,
        b.y-a.y, c.y-a.y,
    };

    v2 result = inverse(T) * (p-a);
    return result;
}

inline v3 get_barycentric_coordinates(v3 a, v3 b, v3 c, v3 d, v3 p)
{
    m3 T =
    {
        b.x-a.x, c.x-a.x, d.x-a.x,
        b.y-a.y, c.y-a.y, d.y-a.y,
        b.z-a.z, c.z-a.z, d.z-a.z,
    };

    v3 result = inverse(T) * (p-a);
    return result;
}

#if 0

// NOTE(lubo): These simple projections will not work for closest simplex feature
// becase we would need normals on faces/edges to. HOLD ON A SECOND I GOT AN IDEA
inline v2 project_to_plane_reduce(v3 a, v3 b, v3 c, v3 p)
{
    // a -> (0,0)
    // b -> (1,0)
    // c -> (0,1)

    m3 A =
    {
        a.x, b.x, c.x,
        a.y, b.y, c.y,
        a.z, b.z, c.z,
    };

    A = inverse(A);

    v2 result = {};
    result.x = A.d*p.x + A.e*p.y + A.f*p.z;
    result.y = A.g*p.x + A.h*p.y + A.i*p.z;

    return result;
}

// NOTE(lubo): After all this has not been needed.
// NOTE(lubo):
// ABC is the triangle we have
// D is the new point
// this algorithm replaces one of the points from ABC with D, giving modified (ABC)'
// this algorithm also says which part of (ABC)' is closest to origin
inline void gjk_distance_extension_closest_new_triangle_problem(v3 a, v3 b, v3 c, v3 d)
{
    v3 origin = {0,0,0};
    v3 new_point = d;
    v3 lambda = get_barycentric_coordinates(new_point, a, b, c, origin);

    if(lambda.x < 0)
    {
        if(lambda.y < 0)
        {
            if(lambda.z < 0)
            {
                // A, replace B or C or D
            }
            else
            {
                // AD, replace B or C
            }
        }
        else
        {
            if(lambda.z < 0)
            {
                // AC, replace B or D
            }
            else
            {
                // ACD, replace B
            }
        }
    }
    else
    {
        if(lambda.y < 0)
        {
            if(lambda.z < 0)
            {
                // AB, replace C or D
            }
            else
            {
                // ABD, replace C
            }
        }
        else
        {
            if(lambda.z < 0)
            {
                // ABC, replace D
            }
            else
            {
                // NOTE(lubo): New point should not be able to enclose the origin at this point (calculating distance to closest triangle in GJK algorithm)
                // ABCD
                InvalidCodePath;
            }
        }
    }
}
#endif
