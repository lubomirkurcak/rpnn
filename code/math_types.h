/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

#define Pi32     3.14159265359f
#define HalfPi32 1.57079632679f
#define Tau32    6.28318530718f
#define E_32     2.718281828459045235f
#define Phi32    1.6180339887498948482045868343656381177f
#define Sqrt3Over2_32 0.86602540378443864676f
#define InvSqrt2_32 0.7071067811865475244f
#define Sqrt2_32 1.4142135623730950488f

// NOTE(lubo): Float Vectors and Derivatives

Serializable;
union v2
{
    struct
    {
        float x, y;
    };
    DontSerialize; struct
    {
        float u, v;
    };
    DontSerialize; struct
    {
        float width, height;
    };
    DontSerialize; struct
    {
        float real, imaginary;
    };
    DontSerialize; struct
    {
        float re, im;
    };
    DontSerialize; float E[2];
};

Serializable;
union v3
{
    struct
    {
        float x, y, z;
    };
    DontSerialize; struct
    {
        float u, v;
    };
    DontSerialize; struct
    {
        float r, g, b;
    };
    DontSerialize; struct
    {
        v2 xy;
    };
    DontSerialize; struct
    {
        float _;
        v2 yz;
    };
    DontSerialize; struct
    {
        v2 uv;
    };
    DontSerialize; float E[3];
};

Serializable;
union v4
{
    struct
    {
        float x, y, z, w;
    };
    DontSerialize; struct
    {
        v3 xyz;
        float w;
    };
    DontSerialize; struct
    {
        union
        {
            v3 rgb;
            struct
            {
                float r, g, b;
            };
        };
        
        float a;
    };
    DontSerialize; struct
    {
        v2 xy;
        v2 zw;
    };
    DontSerialize; struct
    {
        float _;
        v2 yz;
    };
    DontSerialize; float E[4];
};

Serializable;
struct r2
{
    v2 min;
    v2 max;
};

Serializable;
struct r3
{
    v3 min;
    v3 max;
};

Serializable;
struct quaternion
{
    union
    {
        DontSerialize; v3 xyz;
        struct
        {
            float x;
            float y;
            float z;
        };
    };
    float w;
};

// NOTE(lubo): We use a system such that we can copy definitions straight from wikipedia ;)

Serializable;
union m2
{
    struct
    {
        float a;
        float b;
        float c;
        float d;
    };
    
    float E[2][2];
};

Serializable;
union m3
{
    struct
    {
        float a;
        float b;
        float c;
        float d;
        float e;
        float f;
        float g;
        float h;
        float i;
    };
    
    float E[3][3];
};

Serializable;
union m4
{
    struct
    {
        float a;
        float b;
        float c;
        float d;
        float e;
        float f;
        float g;
        float h;
        float i;
        float j;
        float k;
        float l;
        float m;
        float n;
        float o;
        float p;
    };
    
    float E[4][4];
};

struct m4_pair
{
    m4 forward;
    m4 inverse;
};

// NOTE(lubo): Integer Vectors and Derivatives

union v2i
{
    struct
    {
        int x, y;
    };
    struct
    {
        int u, v;
    };
    struct
    {
        int width, height;
    };
    int E[2];
};

inline b32x operator==(v2i a, v2i b)
{
    b32x result = a.x == b.x && a.y == b.y;
    return result;
}
inline b32x operator!=(v2i a, v2i b)
{
    b32x result = !(a == b);
    return result;
}

union v3i
{
    struct
    {
        int x, y, z;
    };
    struct
    {
        int u, v;
    };
    struct
    {
        int r, g, b;
    };
    struct
    {
        v2i xy;
    };
    struct
    {
        int _;
        v2i yz;
    };
    struct
    {
        v2i uv;
    };
    int E[3];
};

inline b32x operator==(v3i a, v3i b)
{
    b32x result = a.x == b.x && a.y == b.y && a.z == b.z;
    return result;
}
inline b32x operator!=(v3i a, v3i b)
{
    b32x result = !(a == b);
    return result;
}
inline v3i
operator*(int a, v3i b)
{
    v3i result = {a*b.x, a*b.y, a*b.z};
    return result;
}
inline v3i
operator*(v3i a, int b)
{
    v3i result = b*a;
    return result;
}
inline v3i &
operator*=(v3i &a, int b)
{
    a = a * b;
    return a;
}
inline v3
operator*(float a, v3i b)
{
    v3 result = {a*b.x, a*b.y, a*b.z};
    return result;
}
inline v3
operator*(v3i a, float b)
{
    v3 result = b*a;
    return result;
}
/*inline v3i
operator/(v3i a, int b)
{
    int invb = 1/b;
    v3i result = invb*a;
    return result;
}
inline v3i &
operator/=(v3i &a, int b)
{
    a = a / b;
    return a;
}
*/
inline v3i
operator+(v3i a, v3i b)
{
    v3i result = {a.x+b.x,a.y+b.y,a.z+b.z};
    return result;
}
inline v3i &
operator+=(v3i &a, v3i b)
{
    a = a + b;
    return a;
}
inline v3i
operator-(v3i a)
{
    v3i result = {-a.x,-a.y,-a.z};
    return result;
}
inline v3i
operator-(v3i a, v3i b)
{
    v3i result = a + (-b);
    return result;
}
inline v3i &
operator-=(v3i &a, v3i b)
{
    a = a - b;
    return a;
}

union v4i
{
    struct
    {
        union
        {
            v3i xyz;
            struct
            {
                int x, y, z;
            };
        };
        
        int w;
    };
    struct
    {
        union
        {
            v3i rgb;
            struct
            {
                int r, g, b;
            };
        };
        
        int a;
    };
    struct
    {
        v2i xy;
        v2i zw;
    };
    struct
    {
        int _;
        v2i yz;
    };
    int E[4];
};

struct r2i
{
    v2i min;
    v2i max;
};

inline b32x operator==(r2i a, r2i b)
{
    b32x result = a.min == b.min && a.max == b.max;
    return result;
}

struct r3i
{
    v3i min;
    v3i max;
};


// Constructors
inline v2 V2(float a) {return {a,a};}
inline v2 V2(float x, float y) {return {x,y};}
inline v2 V2(v2i v) {return {(float)v.x,(float)v.y};}
inline v3 V3(float a) {return {a,a,a};}
inline v3 V3(float x, float y, float z) {return {x,y,z};}
inline v3 V3(v2 xy, float z) {return {xy.x,xy.y,z};}
//inline v3 V3(v2i xy, float z) {return {(float)xy.x,(float)xy.y,z};}
inline v3 V3(v3i v) {return {(float)v.x,(float)v.y,(float)v.z};}
inline v4 V4(float a) {return {a,a,a,a};}
inline v4 V4(float x, float y, float z, float w) {return {x,y,z,w};}
inline v4 V4(v3 xyz, float w) {return {xyz.x,xyz.y,xyz.z,w};}
inline v4 V4(v4i v) {return {(float)v.x,(float)v.y,(float)v.z,(float)v.w};}

inline v2i V2i(int x, int y) {return {x,y};}
inline v2i V2i(v2 v) {return {(int)v.x,(int)v.y};}
inline v2i V2i_floor(v2 v) {return {floor_to_int(v.x),floor_to_int(v.y)};}
inline v3i V3i(int x, int y, int z) {return {x,y,z};}
inline v3i V3i(v2i xy, int z) {return {xy.x,xy.y,z};}
inline v3i V3i(v3 v) {return {(int)v.x,(int)v.y,(int)v.z};}
inline v4i V4i(int x, int y, int z, int w) {return {x,y,z,w};}
inline v4i V4i(v3i xyz, int w) {return {xyz.x,xyz.y,xyz.z,w};}
inline v4i V4i(v4 v) {return {(int)v.x,(int)v.y,(int)v.z,(int)v.w};}

inline r2 R2(float minx, float miny, float maxx, float maxy) {return {minx, miny, maxx, maxy};}
inline r2 R2(v2 min, v2 max) {return {min, max};}
inline r2 R2(r2i r){return {(float)r.min.x, (float)r.min.y, (float)r.max.x, (float)r.max.y};}

// Operators
inline b32x
operator==(v2 a, v2 b)
{
    b32x result =
        a.x == b.x &&
        a.y == b.y;
    return result;
}
inline b32x
operator!=(v2 a, v2 b)
{
    b32x result = !(a == b);
    return result;
}
inline v2
operator*(float a, v2 b)
{
    v2 result = {a*b.x, a*b.y};
    return result;
}
inline v2
operator*(v2 a, float b)
{
    v2 result = b*a;
    return result;
}
inline v2 &
operator*=(v2 &a, float b)
{
    a = a * b;
    return a;
}
inline v2
operator/(v2 a, float b)
{
    float invb = 1/b;
    v2 result = invb*a;
    return result;
}
inline v2 &
operator/=(v2 &a, float b)
{
    a = a / b;
    return a;
}
inline v2
operator/(float a, v2 b)
{
    v2 result = {};
    result.x = a/b.x;
    result.y = a/b.y;
    return result;
}
inline v2
operator+(v2 a, v2 b)
{
    v2 result = {a.x+b.x,a.y+b.y};
    return result;
}
inline v2 &
operator+=(v2 &a, v2 b)
{
    a = a + b;
    return a;
}
inline v2
operator-(v2 a)
{
    v2 result = {-a.x,-a.y};
    return result;
}
inline v2
operator-(v2 a, v2 b)
{
    v2 result = a + (-b);
    return result;
}
inline v2 &
operator-=(v2 &a, v2 b)
{
    a = a - b;
    return a;
}

// NOTE(lubo): x - real, y - y
inline v2
complex_multiply(v2 a, v2 b)
{
    v2 result = {a.x*b.x - a.y*b.y,
                 a.x*b.y + a.y*b.x};
    return result;
}

/*inline v2 &
operator*=(v2 &a, v2 b)
{
    a = a * b;
    return a;
    }*/

inline b32x
operator==(v3 a, v3 b)
{
    b32x result =
        a.x == b.x &&
        a.y == b.y &&
        a.z == b.z;
    return result;
}
inline v3
operator*(float a, v3 b)
{
    v3 result = {a*b.x, a*b.y, a*b.z};
    return result;
}
inline v3
operator*(v3 a, float b)
{
    v3 result = b*a;
    return result;
}
inline v3 &
operator*=(v3 &a, float b)
{
    a = a * b;
    return a;
}
inline v3
operator/(v3 a, float b)
{
    float invb = 1/b;
    v3 result = invb*a;
    return result;
}
inline v3 &
operator/=(v3 &a, float b)
{
    a = a / b;
    return a;
}
inline v3
operator/(float a, v3 b)
{
    v3 result = {};
    result.x = a/b.x;
    result.y = a/b.y;
    result.z = a/b.z;
    return result;
}
inline v3
operator+(v3 a, v3 b)
{
    v3 result = {a.x+b.x,a.y+b.y,a.z+b.z};
    return result;
}
inline v3 &
operator+=(v3 &a, v3 b)
{
    a = a + b;
    return a;
}
inline v3
operator-(v3 a)
{
    v3 result = {-a.x,-a.y,-a.z};
    return result;
}
inline v3
operator-(v3 a, v3 b)
{
    v3 result = a + (-b);
    return result;
}
inline v3 &
operator-=(v3 &a, v3 b)
{
    a = a - b;
    return a;
}

inline b32x
operator==(v4 a, v4 b)
{
    b32x result =
        a.x == b.x &&
        a.y == b.y &&
        a.z == b.z &&
        a.w == b.w;
    return result;
}
inline v4
operator*(float a, v4 b)
{
    v4 result = {a*b.x, a*b.y, a*b.z, a*b.w};
    return result;
}
inline v4
operator*(v4 a, float b)
{
    v4 result = b*a;
    return result;
}
inline v4 &
operator*=(v4 &a, float b)
{
    a = a * b;
    return a;
}
inline v4
operator/(v4 a, float b)
{
    float invb = 1/b;
    v4 result = invb*a;
    return result;
}
inline v4 &
operator/=(v4 &a, float b)
{
    a = a / b;
    return a;
}
inline v4
operator/(float a, v4 b)
{
    v4 result = {};
    result.x = a/b.x;
    result.y = a/b.y;
    result.z = a/b.z;
    result.w = a/b.w;
    return result;
}
inline v4
operator+(v4 a, v4 b)
{
    v4 result = {a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w};
    return result;
}
inline v4 &
operator+=(v4 &a, v4 b)
{
    a = a + b;
    return a;
}
inline v4
operator-(v4 a)
{
    v4 result = {-a.x,-a.y,-a.z,-a.w};
    return result;
}
inline v4
operator-(v4 a, v4 b)
{
    v4 result = a + (-b);
    return result;
}
inline v4 &
operator-=(v4 &a, v4 b)
{
    a = a - b;
    return a;
}

inline v2i
operator*(s32 a, v2i b)
{
    v2i result = {a*b.x, a*b.y};
    return result;
}
inline v2i
operator*(v2i a, s32 b)
{
    v2i result = b*a;
    return result;
}
inline v2i &
operator*=(v2i &a, s32 b)
{
    a = a * b;
    return a;
}
inline v2i
operator+(v2i a, v2i b)
{
    v2i result = {a.x+b.x,a.y+b.y};
    return result;
}
inline v2i &
operator+=(v2i &a, v2i b)
{
    a = a + b;
    return a;
}
inline v2i
operator-(v2i a)
{
    v2i result = {-a.x,-a.y};
    return result;
}
inline v2i
operator-(v2i a, v2i b)
{
    v2i result = a + (-b);
    return result;
}
inline v2i &
operator-=(v2i &a, v2i b)
{
    a = a - b;
    return a;
}

inline quaternion
operator+(quaternion A, quaternion B)
{
    quaternion Q;
    Q.xyz = A.xyz + B.xyz;
    Q.w = A.w + B.w;
    return Q;
};
inline quaternion
operator*(float S, quaternion A)
{
    quaternion Q;
    Q.xyz = S * A.xyz;
    Q.w = S * A.w;
    return Q;
}

struct Ray
{
    v3 eye;
    v3 dir;
};

Serializable;
struct Transform
{
    quaternion orientation;
    v3 pos;
    // b32x uniform_scale;
    // float scale;
    v3 scale_axes; // NOTE(lubo): gltf forced this upon us ;_;
};

inline Transform default_transform()
{
    Transform t;
    t.pos = {};
    t.orientation = {0,0,0,1};
    //t.scale = 1;
    t.scale_axes = {1,1,1};

    return t;
}
