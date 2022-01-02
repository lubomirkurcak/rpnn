/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

internal b32x
string_to_float64(string a, float64 *out_result)
{
    string original = a;
    
    float64 whole = 0;
    float64 fractional = 0;
    b32x error = false;
    b32x sign = false;
    
    if(a.size)
    {
        char first_char = a[0];
        if(first_char == '-')
        {
            sign = true;
            a = substring(a, 1);
        }

        float64 decimal_factor = 0.1;
        b32x fractional_part = false;
        for(smm index=0; index<a.size; index+=1)
        {
            char c = a[index];

            if (c == '.' || c == ',')
            {
                if(fractional_part)
                {
                    loginfo("value parser", "Error: Duplicate decimal separators in '%.*s' at position %zd", UnwrapStringToPrintf(original), index);
                    error = true;
                }
                else
                {
                    fractional_part = true;
                }
            }
            else if(is_numeric(c))
            {
                int digit = c - '0';
                if(fractional_part)
                {
                    fractional += decimal_factor*digit;
                    decimal_factor *= 0.1f;
                }
                else
                {
                    whole = 10*whole + (float64)digit;
                }
            }
            else
            {
                loginfo("value parser", "Error: Character '%c' in '%.*s' at position %zd is not valid", c, UnwrapStringToPrintf(original), index);
                error = true;
            }
        };
    }

    float64 result = whole + fractional;
    
    if(sign)
    {
        result = -result;
    }
    
    if(!error)
    {
        *out_result = result;
    }
    
    return !error;
};

internal b32x
string_to_float(string a, float *out_result)
{
    float64 result64;
    b32x success = string_to_float64(a, &result64);
    if(success)
    {
        *out_result = (float)result64;
    }
    return success;
}

internal b32x
string_to_bool(string a, b32x *out_result)
{
    b32x success = false;
    b32x result = false;
                
    if(a.size)
    {
        char first_char = a[0];
        if(first_char == 't' || first_char == 'T')
        {
            success = true;
            result = true;
        }
        else if(first_char == 'f' || first_char == 'F')
        {
            success = true;
            result = false;
        }
        else
        {
            loginfo("value parser", "Error: Boolean must be set to 'true' or 'false' (got '%.*s')",
                    UnwrapStringToPrintf(a));
        }
    }
    else
    {
        result = true;
        success = true;
    }

    if(success)
    {
        *out_result = result;
    }

    return success;
}

// TODO(lubomir): Support hexadecimal and binary later?
internal b32x
string_to_integer(string a, void *out_result, int bytes, b32x is_signed)
{
    b32x error = false;
    b32x sign = false;

    string original = a;
    u64 accumulator = 0;
        
    if(a.size)
    {
        char first_char = a[0];
        if(first_char == '-')
        {
            sign = true;
            a = substring(a, 1);

            if(!is_signed)
            {
                error = true;
                loginfo("value parser", "Error: Encountered minus sign '-' for *unsigned* %d-byte integer in '%.*s'. Unsigned integers cannot be negative.",
                        bytes, UnwrapStringToPrintf(original));
            }
        }

        
        for(smm index=0; index<a.size; ++index)
        {
            char c = a[index];
            if(is_numeric(c))
            {
                int digit = c - '0';

                // TODO(lubo): We won't catch if 64bit number is too big. Or really any reliably.
                accumulator = 10*accumulator + digit;
            }
            else
            {
                error = true;
                loginfo("value parser", "Error: Invalid digit '%c' in '%.*s' at position %zd",
                        c, UnwrapStringToPrintf(original), index);
            }
        }
    }

    b32x too_big = false;
    b32x too_small = false;
    
    if(is_signed)
    {
        if(sign)
        {
            if(accumulator > (((u64)(s64_max))+1))
            {
                error = true;
                too_small = true;
            }
        }
        else
        {
            if(accumulator > ((u64)s64_max))
            {
                error = true;
                too_big = true;
            }
        }
    }

    s64 signed_accumulator = (s64)accumulator;
    if(sign) signed_accumulator = -signed_accumulator;
    
    if(!error)
    {
        switch(bytes)
        {
            case 1:
            {
                if(is_signed)
                {
                    if(signed_accumulator > s8_max) too_big = true;
                    else if(signed_accumulator < s8_min) too_small = true;
                    else *(s8 *)out_result = (s8)signed_accumulator;
                }
                else
                {
                    if(accumulator > u8_max) too_big = true;
                    else *(u8 *)out_result = (u8)accumulator;
                }
            } break;
            case 2:
            {
                if(is_signed)
                {
                    if(signed_accumulator > s16_max) too_big = true;
                    else if(signed_accumulator < s16_min) too_small = true;
                    else *(s16 *)out_result = (s16)signed_accumulator;
                }
                else
                {
                    if(accumulator > u16_max) too_big = true;
                    else *(u16 *)out_result = (u16)accumulator;
                }
            } break;
            case 4:
            {
                if(is_signed)
                {
                    if(signed_accumulator > s32_max) too_big = true;
                    else if(signed_accumulator < s32_min) too_small = true;
                    else *(s32 *)out_result = (s32)signed_accumulator;
                }
                else
                {
                    if(accumulator > u32_max) too_big = true;
                    else *(u32 *)out_result = (u32)accumulator;
                }
            } break;
            case 8:
            {
                // TODO(lubo): Are bounds checking working when accumulator is 64-bit?
                if(is_signed)
                {
                    if(signed_accumulator > s64_max) too_big = true;
                    else if(signed_accumulator < s64_min) too_small = true;
                    else *(s64 *)out_result = (s64)signed_accumulator;
                }
                else
                {
                    if(accumulator > u64_max) too_big = true;
                    else *(u64 *)out_result = (u64)accumulator;
                }
            } break;
            default:
            {
                error = true;
                loginfo("value parser", "Error: Number of bytes must be 1, 2, 4 or 8. Got '%d' while parsing '%.*s'",
                        bytes, UnwrapStringToPrintf(original));
            } break;
        }
    }

    if(too_big)
    {
        error = true;
        loginfo("value parser", "Error: Value '%.*s' is too big for %d-byte %s integer",
                UnwrapStringToPrintf(original), bytes, is_signed ? "signed" : "unsigned");
    }

    if(too_small)
    {
        error = true;
        loginfo("value parser", "Error: Value '%.*s' is too small for %d-byte %s integer",
                UnwrapStringToPrintf(original), bytes, is_signed ? "signed" : "unsigned");
    }
    
    return !error;
}

internal b32x
string_to_int(string a, int *out_result)
{
    b32x success = string_to_integer(a, out_result, sizeof(int), true);
    return success;
}

internal b32x
string_to_s8(string a, s8 *out_result)
{
    b32x success = string_to_integer(a, out_result, sizeof(s8), true);
    return success;
}
internal b32x
string_to_s16(string a, s16 *out_result)
{
    b32x success = string_to_integer(a, out_result, sizeof(s16), true);
    return success;
}
internal b32x
string_to_s32(string a, s32 *out_result)
{
    b32x success = string_to_integer(a, out_result, sizeof(s32), true);
    return success;
}
internal b32x
string_to_s64(string a, s64 *out_result)
{
    b32x success = string_to_integer(a, out_result, sizeof(s64), true);
    return success;
}

internal b32x
string_to_u8(string a, u8 *out_result)
{
    b32x success = string_to_integer(a, out_result, sizeof(u8), false);
    return success;
}
internal b32x
string_to_u16(string a, u16 *out_result)
{
    b32x success = string_to_integer(a, out_result, sizeof(u16), false);
    return success;
}
internal b32x
string_to_u32(string a, u32 *out_result)
{
    b32x success = string_to_integer(a, out_result, sizeof(u32), false);
    return success;
}
internal b32x
string_to_u64(string a, u64 *out_result)
{
    b32x success = string_to_integer(a, out_result, sizeof(u64), false);
    return success;
}

inline b32x is_vector_field_delimiter(u8 c)
{
    b32x result = c == ' ' || c == ',';
    return result;
}

internal int parse_float_list(string input, int max_count, float *result)
{
    int results_parsed = 0;
    
    string_split split = split_at_char(input, ',');
    if(split.remaining.size == 0)
    {
        // NOTE(lubo): space delimited
        split = split_at_whitespace(input);
        
        for(int insert_index=0; insert_index<max_count; ++insert_index)
        {
            string current = split.first;
            results_parsed += string_to_float(current, result + insert_index);
            if(split.remaining.size == 0) break;
            
            split = split_at_whitespace(skip_whitespace(split.remaining));
        }
    }
    else
    {
        // NOTE(lubo): comma delimited
        for(int insert_index=0; insert_index<max_count; ++insert_index)
        {
            string current = split.first;
            results_parsed += string_to_float(current, result + insert_index);
            if(split.remaining.size == 0) break;
            
            split = split_at_char(skip_whitespace(substring(split.remaining, 1)), ',');
        }
    }

    return results_parsed;
}

internal b32x parse_value(Any value, string input)
{
    Assert(value.count == 0, "Arrays need to be parsed one by one"); // TODO(lubo): Make it not so?
    
    b32x success = false;
    switch(value.type)
    {
        case TypeOf(void):
            loginfo("value parser", "Error: Cannot parse into type '%s' (while trying to parse '%.*s')",
                    TypeName(value.type), UnwrapStringToPrintf(input));
            break;
        
        // case TypeOf(int): success = string_to_integer(input, (int *)value.memory, sizeof(int), true); break;
            
        // case TypeOf(s8):  success = string_to_integer(input, (s8  *)value.memory, sizeof(s8), true); break;
        // case TypeOf(s16): success = string_to_integer(input, (s16 *)value.memory, sizeof(s16), true); break;
        // case TypeOf(s32): success = string_to_integer(input, (s32 *)value.memory, sizeof(s32), true); break;
        // case TypeOf(s64): success = string_to_integer(input, (s64 *)value.memory, sizeof(s64), true); break;
            
        // case TypeOf(u8):  success = string_to_integer(input, (u8  *)value.memory, sizeof(u8), false); break;
        // case TypeOf(u16): success = string_to_integer(input, (u16 *)value.memory, sizeof(u16), false); break;
        // case TypeOf(u32): success = string_to_integer(input, (u32 *)value.memory, sizeof(u32), false); break;
        // case TypeOf(u64): success = string_to_integer(input, (u64 *)value.memory, sizeof(u64), false); break;
        
        case TypeOf(int): success = string_to_int(input, (int *)value.memory); break;
            
        case TypeOf(s8):  success = string_to_s8(input, (s8  *)value.memory); break;
        case TypeOf(s16): success = string_to_s16(input, (s16 *)value.memory); break;
        case TypeOf(s32): success = string_to_s32(input, (s32 *)value.memory); break;
        case TypeOf(s64): success = string_to_s64(input, (s64 *)value.memory); break;
            
        case TypeOf(u8):  success = string_to_u8(input, (u8  *)value.memory); break;
        case TypeOf(u16): success = string_to_u16(input, (u16 *)value.memory); break;
        case TypeOf(u32): success = string_to_u32(input, (u32 *)value.memory); break;
        case TypeOf(u64): success = string_to_u64(input, (u64 *)value.memory); break;
            
        case TypeOf(b32):
            success = string_to_bool(input, (b32 *)value.memory);
            break;
        case TypeOf(b32x):
            success = string_to_bool(input, (b32x *)value.memory);
            break;
            
        case TypeOf(float):
            success = string_to_float(input, (float *)value.memory);
            break;
            
        case TypeOf(float64):
            success = string_to_float64(input, (float64 *)value.memory);
            break;
            
        case TypeOf(string):
            success = true;
            string copy = duplicate(input);
            *(string *)value.memory = copy;
            break;
            
        case TypeOf(v2):
        {
            int parsed = parse_float_list(input, 2, (float *)value.memory);
            success = parsed == 2;
        } break;
            
        case TypeOf(v3):
        {
            int parsed = parse_float_list(input, 3, (float *)value.memory);
            success = parsed == 3;
        } break;
            
        case TypeOf(v4):
        {
            int parsed = parse_float_list(input, 4, (float *)value.memory);
            success = parsed == 4;
        } break;
            
        default:
        {
            loginfo("value parser", "Error: Unknown type! (while trying to parse '%.*s')", UnwrapStringToPrintf(input));
        } break;
    }
    
    return success;
}

internal string format_value(buffer output, Any value, smm starting_index=0)
{
    char *M = (char *)output.memory + starting_index;
    int S = (int)output.size;
    int N = -1;

    b32x success = true;
    switch(value.type)
    {
        case TypeOf(void):
            success = false;
            loginfo("value formatter", "Error: Cannot format type '%s'", TypeName(value.type));
            break;
        
        case TypeOf(s8):  
            N = stbsp_snprintf(M, S, "%d", *(s8 *)value.memory); break;
        case TypeOf(s16):
            N = stbsp_snprintf(M, S, "%d", *(s16 *)value.memory); break;
        case TypeOf(int):
            N = stbsp_snprintf(M, S, "%d", *(int *)value.memory); break;
        case TypeOf(s32):
            N = stbsp_snprintf(M, S, "%d", *(s32 *)value.memory); break;
        case TypeOf(s64):
            N = stbsp_snprintf(M, S, "%lld", *(s64 *)value.memory); break;
            
        case TypeOf(u8):
            N = stbsp_snprintf(M, S, "%u", *(u8 *)value.memory); break;
        case TypeOf(u16):
            N = stbsp_snprintf(M, S, "%u", *(u16 *)value.memory); break;
        case TypeOf(u32):
            N = stbsp_snprintf(M, S, "%u", *(u32 *)value.memory); break;
        case TypeOf(u64):
            N = stbsp_snprintf(M, S, "%llu", *(u64 *)value.memory); break;
            
        case TypeOf(b32):
            N = stbsp_snprintf(M, S, "%s", (*(b32 *)value.memory) ? "true" : "false"); break;
            break;
        case TypeOf(b32x):
            N = stbsp_snprintf(M, S, "%s", (*(b32x *)value.memory) ? "true" : "false"); break;
            break;
            
        case TypeOf(float):
            N = stbsp_snprintf(M, S, "%f", *(float *)value.memory); break;
            break;
            
        case TypeOf(float64):
            N = stbsp_snprintf(M, S, "%f", *(float64 *)value.memory); break;
            break;
            
        case TypeOf(string):
            N = stbsp_snprintf(M, S, "%.*s", UnwrapStringToPrintf(*(string *)value.memory)); break;
            break;
            
        case TypeOf(v2):
        {
            N = stbsp_snprintf(M, S, "%f %f", ((float *)value.memory)[0], ((float *)value.memory)[1]);
        } break;
            
        case TypeOf(v3):
        {
            N = stbsp_snprintf(M, S, "%f %f %f", ((float *)value.memory)[0], ((float *)value.memory)[1], ((float *)value.memory)[2]);
        } break;
            
        case TypeOf(v4):
        {
            N = stbsp_snprintf(M, S, "%f %f %f %f", ((float *)value.memory)[0], ((float *)value.memory)[1], ((float *)value.memory)[2], ((float *)value.memory)[3]);
        } break;
            
        default:
        {
            success = false;
            if(value.type < __Meta_Types_Count)
            {
                loginfo("value formatter", "Error: Cannot format type '%s' (type ID: %d)", TypeName(value.type), value.type);
            }
            else
            {
                loginfo("value formatter", "Error: Cannot format unknown type! (type ID: %d)", value.type);
            }
        } break;
    }

    string result = {};
    if(success)
    {
        result.memory = output.memory;
        result.size = starting_index + N;
    }
    else
    {
        result = EmptyString;
    }
    
    return result;
}


internal b32x click_value(Any value, b32x left_click=true)
{
    b32x action_needed = false;
    switch(value.type)
    {
        case TypeOf(void):
            //success = false;
            loginfo("value formatter", "Error: Cannot format type '%s'", TypeName(value.type));
            break;
        
        case TypeOf(s8):  
            *(s8 *)value.memory += 1; break;
        case TypeOf(s16):
            *(s16 *)value.memory += 1; break;
        case TypeOf(int):
            *(int *)value.memory += 1; break;
        case TypeOf(s32):
            *(s32 *)value.memory += 1; break;
        case TypeOf(s64):
            *(s64 *)value.memory += 1; break;

        case TypeOf(u8):
            *(u8 *)value.memory += 1; break;
        case TypeOf(u16):
            *(u16 *)value.memory += 1; break;
        case TypeOf(u32):
            *(u32 *)value.memory += 1; break;
        case TypeOf(u64):
            *(u64 *)value.memory += 1; break;
            
        case TypeOf(b32):
            *(b32 *)value.memory = !(*(b32 *)value.memory); break;
        case TypeOf(b32x):
            *(b32x *)value.memory = !(*(b32x *)value.memory); break;
            
        case TypeOf(float):
            action_needed = true;
            //N = stbsp_snprintf(M, S, "%f", *(float *)value.memory); break;
            break;
            
          #if 0
        case TypeOf(float64):
            N = stbsp_snprintf(M, S, "%f", *(float64 *)value.memory); break;
            break;
            
        case TypeOf(string):
            N = stbsp_snprintf(M, S, "%.*s", UnwrapStringToPrintf(*(string *)value.memory)); break;
            break;
            
        case TypeOf(v2):
        {
            N = stbsp_snprintf(M, S, "%f %f", ((float *)value.memory)[0], ((float *)value.memory)[1]);
        } break;
            
        case TypeOf(v3):
        {
            N = stbsp_snprintf(M, S, "%f %f %f", ((float *)value.memory)[0], ((float *)value.memory)[1], ((float *)value.memory)[2]);
        } break;
            
        case TypeOf(v4):
        {
            N = stbsp_snprintf(M, S, "%f %f %f %f", ((float *)value.memory)[0], ((float *)value.memory)[1], ((float *)value.memory)[2], ((float *)value.memory)[3]);
        } break;
            #endif
        
        default:
        {
            //success = false;
            if(value.type < __Meta_Types_Count)
            {
                loginfo("value formatter", "Error: Cannot format type '%s' (type ID: %d)", TypeName(value.type), value.type);
            }
            else
            {
                loginfo("value formatter", "Error: Cannot format unknown type! (type ID: %d)", value.type);
            }
        } break;
    }

    return action_needed;
}


internal float hold_value(Any value, float change, b32x float_grid)
{
    switch(value.type)
    {
        case TypeOf(s8): *(s8 *)value.memory += (s8)round(change); change -= (s8)round(change); break;
        case TypeOf(s16): *(s16 *)value.memory += (s16)round(change); change -= (s16)round(change); break;
        case TypeOf(int): *(int *)value.memory += (int)round(change); change -= (int)round(change); break;
        case TypeOf(s32): *(s32 *)value.memory += (s32)round(change); change -= (s32)round(change); break;
        case TypeOf(s64): *(s64 *)value.memory += (s64)round(change); change -= (s64)round(change); break;

        case TypeOf(u8): *(u8 *)value.memory += (u8)round(change); change -= (u8)round(change); break;
        case TypeOf(u16): *(u16 *)value.memory += (u16)round(change); change -= (u16)round(change); break;
        case TypeOf(u32): *(u32 *)value.memory += (u32)round(change); change -= (u32)round(change); break;
        case TypeOf(u64): *(u64 *)value.memory += (u64)round(change); change -= (u64)round(change); break;

        case TypeOf(float):
        {
            if(float_grid)
            {
                float old_value = *(float *)value.memory;
                float new_value = round(old_value + change);
                change -= new_value - old_value;
                *(float *)value.memory = new_value;
            }
            else
            {
                *(float *)value.memory += change; change = 0;
            }
        } break;
        case TypeOf(float64):
        {
            *(float64 *)value.memory += change; change = 0;
        } break;
    }

    return change;
}

