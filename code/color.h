/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2020 All Rights Reserved. $
   ======================================================================== */

inline v4 darken(v4 color, float factor=0.5)
{
    v4 result = {};
    result.xyz = (1-factor)*color.xyz;
    result.a = color.a;
    return result;
}

internal v4 hsv_to_rgb(v4 hsv)
{
    v4 result = {};
    result.a = hsv.a;

    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;
    
    int segment = (int)(6*h);
    float within_segment = h - (segment/6.0f);
    
    float ff = within_segment;
    float p = v * (1 - s);
    float q = v * (1 - (s * ff));
    float t = v * (1 - (s * (1 - ff)));

    switch(segment)
    {
        default:
        case 0: // NOTE(lubo): red->yellow
        {
            result.r = v;
            result.g = t;
            result.b = p;
        } break;
        case 1: // NOTE(lubo): yellow->green
        {
            result.r = q;
            result.g = v;
            result.b = p;
        } break;
        case 2: // NOTE(lubo): green->cyan
        {
            result.r = p;
            result.g = v;
            result.b = t;
        } break;
        case 3: // NOTE(lubo): cyan->blue
        {
            result.r = p;
            result.g = q;
            result.b = v;
        } break;
        case 4: // NOTE(lubo): blue->magenta
        {
            result.r = t;
            result.g = p;
            result.b = v;
        } break;
        case 5: // NOTE(lubo): magenta->red
        {
            result.r = v;
            result.g = p;
            result.b = q;
        } break;
    }
    
    return result;     
}
