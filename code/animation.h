/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

inline b32x keyframe_lookup(int count, float *ts, float t, b32x loopable,
                            int *keyframe0, int *keyframe1, float *keyframe_t)
{
    Assert(count > 0);

    b32x animation_ended = true;

    if(loopable)
    {
        // NOTE(lubo): In this case ended means looped around
        animation_ended = t >= ts[count - 1];
        
        for(int i=0; i<count; ++i)
        {
            if(t < ts[i])
            {
                *keyframe0 = modulo(i-1, count);
                *keyframe1 = i;

                if(*keyframe0 == count - 1)
                {
                    Assert(*keyframe1 == 0);
                    *keyframe_t = inv_lerp(0, ts[*keyframe1], t);
                }
                else
                {
                    *keyframe_t = inv_lerp(ts[*keyframe0], ts[*keyframe1], t);
                }
                break;
            }
        }

        if(animation_ended)
        {
            *keyframe0 = *keyframe1 = count - 1;
            *keyframe_t = 0;
        }
    }
    else
    {
        if(t < ts[0])
        {
            animation_ended = false;
            *keyframe0 = *keyframe1 = 0;
            *keyframe_t = 0;
        }
        else
        {
            for(int i=1; i<count; ++i)
            {
                if(t < ts[i])
                {
                    *keyframe0 = i-1;
                    *keyframe1 = i;
                    *keyframe_t = inv_lerp(ts[*keyframe0], ts[*keyframe1], t);
                    animation_ended = false;
                    break;
                }
            }

            if(animation_ended)
            {
                *keyframe0 = *keyframe1 = count - 1;
                *keyframe_t = 0;
            }
        }
    }
    
    return animation_ended;
}


// NOTE(lubo): Functions

enum Interpolation
{
    Interpolation_Linear,
    Interpolation_Cubic,
};

inline float apply_interpolation(int interpolation, float value)
{
    float result = value;

    switch(interpolation)
    {
        case Interpolation_Cubic:
        {
            result = cubic(value);
        } break;
    }
    
    return result;
}

struct Color_Gradient
{
    int keypoints;
    float *t;
    v4 *colors;

    int interpolation;
};

internal v4 get_color_at(Color_Gradient *gradient, float t)
{
    Assert(gradient->t[0] == 0);
    
    v4 result = {};
    for(int keypoint=1; keypoint<gradient->keypoints; ++keypoint)
    {
        if(t < gradient->t[keypoint])
        {
            float _t = inv_lerp(gradient->t[keypoint-1], gradient->t[keypoint], t);
            float __t = apply_interpolation(gradient->interpolation, _t);
            result = lerp(gradient->colors[keypoint-1], gradient->colors[keypoint], __t);
            break;
        }
    }

    return result;
}

// NOTE(lubo): Splines
Serializable;
struct Spline_Point
{
    v3 point;
    v3 tangent_prev;
    v3 tangent_next;
    float t;
};

inline v3 get_position_at(Spline_Point *a, Spline_Point *b, float t)
{
    v3 position =
        hermite00(t)*a->point +
        hermite10(t)*a->tangent_next +
        hermite01(t)*b->point +
        hermite11(t)*b->tangent_prev;
        
    return position;
}

inline v3 get_velocity_at(Spline_Point *a, Spline_Point *b, float t)
{
    v3 velocity =
        dhermite00(t)*a->point +
        dhermite10(t)*a->tangent_next +
        dhermite01(t)*b->point +
        dhermite11(t)*b->tangent_prev;
        
    return velocity;
}

Serializable;
struct Spline
{
    float total_t;
    int point_count;
    b32x cyclical;
    Spline_Point points[8];
};

internal void calc_tangents(Spline *spline, float multiplier=1)
{
    spline->points[0].tangent_prev = {};
    spline->points[0].tangent_next = spline->points[1].point - spline->points[0].point;
    spline->points[spline->point_count-1].tangent_next = {};
    spline->points[spline->point_count-1].tangent_prev = spline->points[spline->point_count-2].point - spline->points[spline->point_count-1].point;

    int index = 1;
    int one_past_max_index = spline->point_count-1;

    if(spline->cyclical)
    {
        index = 0;
        one_past_max_index = spline->point_count;
    }
    
    for(; index<one_past_max_index; ++index)
    {
        int prev_index = index-1; if(prev_index<0) prev_index += spline->point_count;
        int next_index = index+1; if(next_index==spline->point_count) next_index = 0;
        
        v3 prev = spline->points[prev_index].point;
        v3 me = spline->points[index].point;
        v3 next = spline->points[next_index].point;
        v3 dnext = next-me;
        v3 dprev = me-prev;

        // NOTE(lubo): THIS IS BAD
        spline->points[index].tangent_next = {};
        spline->points[index].tangent_prev = {};

        #if 0
        // NOTE(lubo): THIS IS BAD
        spline->points[index].tangent_next = dnext;
        spline->points[index].tangent_prev = dprev;
        #endif

        #if 10
        v3 average = multiplier*lerp(dprev, dnext, 0.5f);
        spline->points[index].tangent_next = average;
        spline->points[index].tangent_prev = average;
        #endif
    }
}

internal v3 get_thingy_at(Spline *spline, float t, b32x velocity, float speed)
{
    v3 result = {};

    if(t < 0)
    {
        if(velocity)
        {
            result = {};
        }
        else
        {
            result = spline->points[0].point;
        }
    }
    else
    {
        Assert(spline->points[0].t == 0);
        float current_t = 0;
        float next_t = spline->points[1].t;
        int index = 0;
        int one_past_max_index = spline->point_count;

        b32x already_got_result = false;
        b32x cyclical_case = false;
        while(1)
        {
            if(t > next_t)
            {
                index += 1;
                if(index == one_past_max_index-1)
                {
                    if(spline->cyclical)
                    {
                        current_t = next_t;
                        next_t = spline->total_t;
                        cyclical_case = true;
                        break;
                    }
                    else
                    {
                        if(velocity)
                        {
                            result = {};
                        }
                        else
                        {
                            result = spline->points[one_past_max_index-1].point;
                        }
                        already_got_result = true;
                        break;
                    }
                }
                else
                {
                    current_t = next_t;
                    next_t = spline->points[index+1].t;
                }
            }
            else
            {
                break;
            }
        }

        if(!already_got_result)
        {
            Spline_Point a = spline->points[index];
            Spline_Point b;
            if(cyclical_case)
            {
                b = spline->points[0];
            }
            else
            {
                b = spline->points[index + 1];
            }

            float c = inv_lerp(current_t, next_t, t);
                
            if(velocity)
            {
                result = speed*(next_t-current_t)*get_velocity_at(&a, &b, c);
            }
            else
            {
                result = get_position_at(&a, &b, c);
            }
        }
    }

    return result;
}

internal v3 get_position_at(Spline *spline, float t)
{
    v3 result = get_thingy_at(spline, t, false, 1);
    return result;
}

internal v3 get_velocity_at(Spline *spline, float t, float speed=1)
{
    v3 result = get_thingy_at(spline, t, true, speed);
    return result;
}

global b32x global_spline_initialized;
global Spline global_spline;
