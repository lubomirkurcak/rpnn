// Created @ Tue Dec 28 00:01:31 2021
// Lubomir Kurcak, Hamster Floof Inc. (C) Copyright All Rights Reserved.
//
// Do not modify this file! It is automatically generated and will be overwritten.
//

#define Serializable
#define DontSerialize
#define CountGivenBy(...)
#define OffsetOf(type, Member) (umm)&(((type *)0)->Member)
#define TypeOf(Type) type_##Type
#define TypeName(type_id) __Meta_Type_Names[type_id]
#define SizeOf(type_id) __Meta_Type_Sizes[type_id]
#define StructMembersOf(type_id) __Meta_Type_Members[type_id]
#define StructMemberCount(type_id) __Meta_Type_Member_Counts[type_id]

// NOTE(lubo): If you decide to interface with python.h this (and possibly others) may cause collisions
struct Struct_Member
{
    int type;
    int deref_count; // how many asterisks are next to type
    int offset;
    int length; // in case of a static array, how long is it, otherwise 0
    char *name;
    int int_count_offset;
    int expanded;
};

struct Any
{
    int type;
    void *memory;
    int count;
};

char *__Meta_Type_Names[] =
{
"void",
"int",
"float",
"double",
"size_t",
"s8",
"s16",
"s32",
"s64",
"b32",
"b32x",
"u8",
"u16",
"u32",
"u64",
"u32x",
"float64",
"umm",
"smm",
"buffer",
"string",
"v2",
"v3",
"v4",
"r2",
"r3",
"quaternion",
"m2",
"m3",
"m4",
"Transform",
"Spline_Point",
"Spline",
"Particle_System",
"Particle_Emmitter_Shape",
"Particle_Emitter",
"Polyhedron",
"Collider",
"Entity",
};
enum __Meta_Types
{
type_void,
type_int,
type_float,
type_double,
type_size_t,
type_s8,
type_s16,
type_s32,
type_s64,
type_b32,
type_b32x,
type_u8,
type_u16,
type_u32,
type_u64,
type_u32x,
type_float64,
type_umm,
type_smm,
type_buffer,
type_string,
type_v2,
type_v3,
type_v4,
type_r2,
type_r3,
type_quaternion,
type_m2,
type_m3,
type_m4,
type_Transform,
type_Spline_Point,
type_Spline,
type_Particle_System,
type_Particle_Emmitter_Shape,
type_Particle_Emitter,
type_Polyhedron,
type_Collider,
type_Entity,
__Meta_Types_Count,
};
Struct_Member **__Meta_Type_Members;
int *__Meta_Type_Member_Counts;
int *__Meta_Type_Sizes;
