/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

#define StringPrintfFormat "%.*s"
#define UnwrapStringToPrintf(A) ((int)(A).size), ((A).memory)

// NOTE(lubo): is_something functions
b32x is_whitespace(u8 c)
{
    b32x result =
        c == 0x9 ||
        c == 0xa ||
        c == 0xb ||
        c == 0xc ||
        c == 0xd ||
        c == 0x20 ||
        c == 0x85 ||
        c == 0xa0;
    return result;
}

b32x is_lowercase(u8 c)
{
    b32x result = c >= 'a' && c <= 'z';
    return result;
}

b32x is_numeric(u8 c)
{
    b32x result = c >= '0' && c <= '9';
    return result;
}

b32x is_hex(u8 c)
{
    b32x result = is_numeric(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    return result;
}

b32x is_binary(u8 c)
{
    b32x result = c == '0' || c == '1';
    return result;
}

b32x is_uppercase(u8 c)
{
    b32x result = c >= 'A' && c <= 'Z';
    return result;
}

b32x is_one_of(u8 c, int array_count, char *array)
{
    b32x result = false;
    for(int index=0; index<array_count; ++index)
    {
        if(array[index] == c)
        {
            result = true;
            break;
        }
    }
    return result;
}

b32x is_alphabetical(u8 c)
{
    b32x result = is_uppercase(c) || is_lowercase(c);
    return result;
}

b32x is_alphanumeric(u8 c)
{
    b32x result = is_numeric(c) || is_alphabetical(c);
    return result;
}

b32x is_inputtable(u8 c)
{
    char accepted[] = "!?.";
    b32x result = c == ' ' || is_alphanumeric(c) || is_one_of(c, ArrayCount(accepted), accepted);
    return result;
}

// NOTE(lubomir): basic functions

smm string_length(string a)
{
    smm result = a.size;
    return result;
}

u8 &string::operator[](smm index)
{
    Assert(index >= -size);
    Assert(index < size);

    if(index < 0)
    {
        return memory[size + index];
    }
    else
    {
        return memory[index];
    }
}

#if 0
u8 get_character_at_index(string a, smm index)
{
    Assert(index >= 0);
    Assert(index < a.size);
    u8 result = (index >= 0 && index < a.size) ? a.memory[index] : 0;
    return result;
}

void set_character_at_index(string a, smm index, u8 c)
{
    Assert(index >= 0);
    Assert(index < a.size);
    if(index >= 0 && index < a.size)
    {
        a.memory[index] = c;
    }
}
#endif

b32x is_null(string a)
{
    b32x result = a.memory == 0;
    return result;
}

b32x is_empty(string a)
{
    b32x result = a.size == 0;
    return result;
}

b32x is_null_or_empty(string a)
{
    b32x result = is_null(a) || is_empty(a);
    return result;
}

// NOTE(lubo): I'd like to note that we could easily make non-abc versions but do we care?
//             Naaah.
//             But maybe we should.
string substring(string a, smm start)
{
    //start = (start < 0) ? 0 : (start > a.size) ? a.size : start;
    
    Assert(start >= 0);
    Assert(start <= a.size);
    
    string result;
    result.memory = a.memory + start;
    result.size = a.size - start;
    return result;
}

string end_of_string(string a)
{
    string result = substring(a, a.size);
    return result;
}

string skip_chars(string a, smm count)
{
    Assert(count >= 0);

    string result;
    if(count < a.size)
    {
        result = substring(a, count);
    }
    else
    {
        result = end_of_string(a);
    }
    return result;
}

string substring_from_pointer(string a, u8 *pointer)
{
    string result = substring(a, PointerDifference(a.memory, pointer));
    return result;
}

// TODO(lubo): These rules need to be double checked. Also these magic accessors were disabled in the other substring...
string substring(string a, smm start, smm length)
{
    Assert(start >= 0);
    Assert(start + length <= a.size);

    if(start > a.size)
    {
        start = a.size;
        length = 0;
    }
    
    if(start < 0)
    {
        length -= start;
        start = 0;
    }

    if(length < 0)
    {
        length = 0;
    }
    
    if(start + length > a.size)
    {
        length = a.size - start;
    }
    
    string result;
    result.memory = a.memory + start;
    result.size = length;
    return result;
}

internal string duplicate(string a)
{
    string result = (string)general_allocate(a.size);
    block_copy(result.memory, a.memory, a.size);
    return result;
}

// NOTE(lubo): Deallocate string allocated in duplicate or concatenate
internal void incinerate(string a)
{
    general_deallocate(a);
}

internal string concatenate(string a, string b)
{
    string result = EmptyString;
    if(a.size + b.size > 0)
    {
        result = (string)general_allocate(a.size + b.size);
        block_copy(result.memory, a.memory, a.size);
        block_copy(result.memory + a.size, b.memory, b.size);
    }
    
    return result;
}

internal string null_terminate(string a)
{
    string result = (string)general_allocate(a.size + 1);
    block_copy(result.memory, a.memory, a.size);
    result[result.size - 1] = 0;
    return result;
}

string operator+(string a, string b)
{
    string result = concatenate(a,b);
    return result;
}

string &buffer::operator+=(const string &b)
{
    string a = *this;
    *this = concatenate(a,b);
    if(a.memory || a.size) incinerate(a);
    return *this;
}

int string_compare(string a, string b)
{
    int result = 0;
    smm min_size = Min(a.size, b.size);
    for(smm index=0; index<min_size; ++index)
    {
        if(a[index] < b[index])
        {
            result = -1;
            break;
        }
        else if(a[index] > b[index])
        {
            result = 1;
            break;
        }
    }
    if(result == 0)
    {
        if(a.size < b.size)
        {
            result = -1;
        }
        else if(a.size > b.size)
        {
            result = 1;
        }
    }

    return result;
}

// a == b ?
b32x operator==(string a, string b)
{
    b32x result = a.size == b.size;
    if(result)
    {
        // u8 *_a = a.memory;
        // u8 *_b = b.memory;
        // for(smm index=0; index<a.size; ++index)
        // {
        //     if(*_a++ != *_b++)
        //     {
        //         result = false;
        //         break;
        //     }
        // }
        result = string_compare(a, b) == 0;
    }

    return result;
}

b32x operator!=(string a, string b)
{
    b32x result = !(a == b);
    return result;
}

b32x operator<(string a, string b)
{
    b32x result = string_compare(a, b) < 0;
    return result;
}

b32x operator>(string a, string b)
{
    b32x result = string_compare(a, b) > 0;
    return result;
}

b32x operator<=(string a, string b)
{
    b32x result = !(a > b);
    return result;
}

b32x operator>=(string a, string b)
{
    b32x result = !(a < b);
    return result;
}

b32x operator==(string a, char *b)
{
    b32x result = a == String(b);
    return result;
}
b32x operator==(char *a, string b)
{
    b32x result = String(a) == b;
    return result;
}

// a starts with b ?
b32x starts_with(string a, string b)
{
    b32x result = b.size <= a.size;
    if(result)
    {
        u8 *_a = a.memory;
        u8 *_b = b.memory;
        for(smm index=0; index<b.size; ++index)
        {
            if(*_a++ != *_b++)
            {
                result = false;
                break;
            }
        }
    }

    return result;
}
b32x starts_with(string a, char *b)
{
    b32x result = starts_with(a, String(b));
    return result;
}


// NOTE(lubomir): split string as a basic functionality

struct string_split
{
    string first;
    string remaining;
    // string left;
    // string right;
    smm skipped;
};

string_split split_string(string a, smm index)
{
    string_split result;
    result.first = substring(a, 0, index);
    result.remaining = substring(a, index);
    return result;
}
