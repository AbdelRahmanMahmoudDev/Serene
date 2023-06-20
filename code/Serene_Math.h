#if !defined(SERENE_MATH_H)

#define IDENTITY_MAT(x) mat4 x = {}; x.m[0] = 1; x.m[5] = 1; x.m[10] = 1; x.m[15] = 1

// 2 component vector 
// This union gives array access which is important for SIMD
union v2
{
    struct
    {
        f32 x;
        f32 y;
    };
    f32 v2[2];
};

typedef struct v2i
{
    u32 x;
    u32 y;
} v2i;
union v3
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };
    f32 v3[3];
};

union v4 
{
    struct 
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct 
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    f32 v4[4];
};

// TODO(Abdo): Finish this
// a 4x4 matrix is just a 16 float array
typedef struct mat4
{
    float m[16];
} mat4;

#include "Serene_Math.cpp"

#define SERENE_MATH_H
#endif