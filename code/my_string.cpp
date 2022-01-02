/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

// NOTE(lubo): Skips
#define skip_while_true skip_until_false
#define skip_while_false skip_until_true

inline string
skip_until_false(string a, b32x func(u8))
{
    while(a.size && func(*a.memory))
    {
        ++a.memory;
        --a.size;
    }
    return a;
}

inline u8 *
skip_until_false(u8 *a, b32x func(u8))
{
    while(*a && func(*a))
    {
        ++a;
    }
    return a;
}

inline string
skip_until_true(string a, b32x func(u8))
{
    while(a.size && !func(*a.memory))
    {
        ++a.memory;
        --a.size;
    }
    return a;
}

inline u8 *
skip_until_true(u8 *a, b32x func(u8))
{
    while(*a && !func(*a))
    {
        ++a;
    }
    return a;
}

inline string
skip_until_char(string a, u8 c)
{
    while(a.size && *a.memory != c)
    {
        ++a.memory;
        --a.size;
    }
    return a;
}

inline string
skip_until_char(string a, string chars)
{
    while(a.size)
    {
        b32x found = false;
        for(smm char_index=0; char_index<chars.size; ++char_index)
        {
            if(a[0] == chars[char_index])
            {
                found = true;
                break;
            }
        }
        if(found) break;
        
        a = substring(a, 1);
    }
    return a;
}

inline string
skip_after_char(string a, u8 c)
{
    string result = substring(skip_until_char(a, c), 1);
    return result;
}

inline string
skip_from_back_until_false(string a, b32x func(u8))
{
    u8 *at = a.memory + a.size - 1;
    while(a.size && func(*at))
    {
        --at;
        --a.size;
    }
    return a;
}

inline string
skip_from_back_until_true(string a, b32x func(u8))
{
    u8 *at = a.memory + a.size - 1;
    while(a.size && !func(*a.memory))
    {
        --at;
        --a.size;
    }
    return a;
}

inline string
skip_from_back_until_char(string a, u8 c)
{
    u8 *at = a.memory + a.size - 1;
    while(a.size && *at != c)
    {
        --at;
        --a.size;
    }
    return a;
}

inline string
skip_whitespace(string a)
{
    string result = skip_until_false(a, is_whitespace);
    return result;
}

inline u8 *
skip_whitespace(u8 *a)
{
    u8 *result = skip_until_false(a, is_whitespace);
    return result;
}

inline string
skip_whitespace_from_back(string a)
{
    string result = skip_from_back_until_false(a, is_whitespace);
    return result;
}

inline string
strip_whitespace(string a)
{
    string result = skip_whitespace_from_back(skip_whitespace(a));
    return result;
}

inline string
skip_until_whitespace(string a)
{
    string result = skip_until_true(a, is_whitespace);
    return result;
}

inline b32x is_opening_bracket(u8 C)
{
    b32x result = C == '(' || C == '[' || C == '{';
    return result;
}

// Type of bracket determined by a[0]. Supported types: '(', '[', '{'.
inline string skip_after_closing_bracket(string a)
{
    u8 bracket_type = a[0];
    u8 closing_type=0;
    switch(bracket_type)
    {
        case '(': closing_type = ')'; break;
        case '[': closing_type = ']'; break;
        case '{': closing_type = '}'; break;
        InvalidDefaultCase;
    }

    a = substring(a, 1);
    int depth = 1;
    while(depth > 0 && a.size)
    {
        if(a[0] == bracket_type) depth += 1;
        if(a[0] == closing_type) depth -= 1;
        a = substring(a, 1);
    }
    
    return a;
}

inline b32x can_be_eol(u8 C)
{
    b32x result = C == 13 || C == 10;
    return result;
}

inline string
skip_until_end_of_line(string a)
{
    string result = skip_until_true(a, can_be_eol);
    return result;
}

inline u8 *
skip_until_end_of_line(u8 *a)
{
    u8 *result = skip_until_true(a, can_be_eol);
    return result;
}

#if 0
// NOTE(lubo): Was character starting at b escaped 
inline b32x
was_escaped(string a, string b, u8 escape_char='\\')
{
}
#endif

// NOTE(lubo): This could be optimized by searching text forward and pattern from back
inline string skip_until_pattern(string text, string pattern)
{
    u8 first_char = pattern[0];
    string remaining = text;

    b32x found=false;
    while(!found)
    {
        found = true;
        remaining = skip_until_char(remaining, first_char);

        if(remaining.size < pattern.size) return end_of_string(remaining);
        
        for(smm i=1; i<pattern.size; ++i)
        {
            if(remaining[i] != pattern[i])
            {
                remaining = substring(remaining, 1);
                found = false;
                break;
            }
        }
    }

    return remaining;
}

inline string skip_until_after_pattern(string text, string pattern)
{
    string remaining = skip_until_pattern(text, pattern);
    if(remaining.size > 0)
    {
        remaining = substring(remaining, pattern.size);
    }
    return remaining;
}

inline string skip_until_one_of_patterns(string text, string *patterns, int pattern_count, int *found_pattern_index)
{
    *found_pattern_index = -1;
    if(pattern_count == 0) return end_of_string(text);
    string first = skip_until_pattern(text, patterns[0]);
    if(first.size > 0) *found_pattern_index = 0;
    
    for(int index=1; index<pattern_count; ++index)
    {
        string find = skip_until_pattern(text, patterns[index]);
        if(find.size > first.size)
        {
            first = find;
            *found_pattern_index = index;
        }
    }

    return first;
}

inline string skip_until_after_one_of_patterns(string text, string *patterns, int pattern_count, int *found_pattern_index)
{
    string remaining = skip_until_one_of_patterns(text, patterns, pattern_count, found_pattern_index);
    if(remaining.size > 0)
    {
        remaining = substring(remaining, patterns[*found_pattern_index].size);
    }
    return remaining;
}

// NOTE(lubo): Splits
inline string_split
split_on_skip(string a, string skip_func(string))
{
    string_split result = {};
    result.remaining = skip_func(a);
    result.skipped = result.remaining.memory - a.memory;
    result.first = substring(a, 0, result.skipped);
    
    return result;
}

inline string_split
split_at_whitespace(string a)
{
    string_split result = split_on_skip(a, skip_until_whitespace);
    return result;
}

inline string_split
split_at_end_of_line(string a)
{
    string_split result = split_on_skip(a, skip_until_end_of_line);
    return result;
}

inline string_split
split_after_closing_bracket(string a)
{
    string_split result = split_on_skip(a, skip_after_closing_bracket);
    return result;
}

inline string_split
split_at_char(string a, u8 c)
{
    string_split result = {};
    result.remaining = skip_until_char(a, c);
    result.skipped = result.remaining.memory - a.memory;
    result.first = substring(a, 0, result.skipped);
    
    return result;
}

inline string_split
split_at_char(string a, string chars)
{
    string_split result = {};
    result.remaining = skip_until_char(a, chars);
    result.skipped = result.remaining.memory - a.memory;
    result.first = substring(a, 0, result.skipped);
    
    return result;
}

inline string_split
split_at_pattern(string text, string pattern)
{
    string_split result = {};
    result.remaining = skip_until_pattern(text, pattern);
    result.skipped = result.remaining.memory - text.memory;
    result.first = substring(text, 0, result.skipped);

    return result;
}

inline string_split
split_after_pattern(string text, string pattern)
{
    string_split result = {};
    result.remaining = skip_until_after_pattern(text, pattern);
    result.skipped = result.remaining.memory - text.memory;
    result.first = substring(text, 0, result.skipped);

    return result;
}

// NOTE(lubo): ultra-simple whitespace tokenizer
inline string_split start_tokenizer(string text)
{
    string_split result = split_at_whitespace(skip_whitespace(text));
    return result;
}

inline string_split advance_tokenizer(string_split split)
{
    string_split result = start_tokenizer(split.remaining);
    return result;
}

inline void advance_tokenizer(string_split *split)
{
    *split = advance_tokenizer(*split);
}
