/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

#define TypeOf(type) Type_##type

#define _MetaCreateTypeName(type) String(#type)
internal string _MetaTypeNames[] =
{
    _MetaCreateTypeName(void),
    _MetaCreateTypeName(s8),
    _MetaCreateTypeName(s16),
    _MetaCreateTypeName(s32),
    _MetaCreateTypeName(s64),
    _MetaCreateTypeName(u8),
    _MetaCreateTypeName(u16),
    _MetaCreateTypeName(u32),
    _MetaCreateTypeName(u64),
    _MetaCreateTypeName(b32),
    _MetaCreateTypeName(b32x),
    _MetaCreateTypeName(float),
    _MetaCreateTypeName(float64),
    _MetaCreateTypeName(string),
};
#undef _TypeName

#define TypeName(type) _MetaTypeNames[type]

typedef enum Types
{
    TypeOf(void),
    TypeOf(int),
    TypeOf(s8),
    TypeOf(s16),
    TypeOf(s32),
    TypeOf(s64),
    TypeOf(u8),
    TypeOf(u16),
    TypeOf(u32),
    TypeOf(u64),
    TypeOf(b32),
    TypeOf(b32x),
    TypeOf(float),
    TypeOf(float64),
    TypeOf(string),
    TypeOf(v2),
    TypeOf(v3),
    TypeOf(v4),
    // ...
} Types;

#if 0 
typedef struct Any
{
    Types type;
    void *memory;
} Any;
#endif
