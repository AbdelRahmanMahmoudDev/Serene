#if !defined(SERENE_MATH_H)

// 2 component vector 
// This union gives array access which is important for SIMD
#if 0
union v2
{
    struct
    {
        f32 x;
        f32 y;
    }
    f32 v2[2];
}
#else
struct v2
{
    f32 x;
    f32 y;
};
#endif

internal v2
V2(f32 A, f32 B)
{
    v2 Result = {};

    Result.x = A;
    Result.y = B;

    return Result;
}

internal v2
operator+(v2 A, v2 B)
{
    v2 Result = {};

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return Result;
}

internal v2
operator-(v2 A, v2 B)
{
    v2 Result = {};

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;

    return Result;
}

internal v2
operator-(v2 &A)
{
    A.x = -A.x;
    A.y = -A.y;
    return A;
}

internal v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;

    return A; 
}

internal v2 &
operator-=(v2 &A, v2 B)
{
    A = A - B;

    return A; 
}

internal v2
operator*(v2 A, f32 scalar)
{
    v2 Result = {};

    Result.x = A.x * scalar;
    Result.y = A.y * scalar;

    return Result;
}

internal v2
operator*(f32 scalar, v2 A)
{
    v2 Result = {};

    Result.x = A.x * scalar;
    Result.y = A.y * scalar;

    return Result;
}

internal v2
operator*=(v2 &A, f32 scalar)
{
    A.x = A.x * scalar;
    A.y = A.y * scalar;

    return A;
}

struct v3
{
    f32 x;
    f32 y;
    f32 z;
};

internal v3
V3(f32 A, f32 B, f32 C)
{
    v3 Result = {};

    Result.x = A;
    Result.y = B;
    Result.z = C;

    return Result;
}

internal v3
operator+(v3 A, v3 B)
{
    v3 Result = {};

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return Result;
}

internal v3
operator-(v3 A, v3 B)
{
    v3 Result = {};

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;

    return Result;
}

internal v3
operator-(v3 &A)
{
    A.x = -A.x;
    A.y = -A.y;
    A.z = -A.z;
    return A;
}

internal v3 &
operator+=(v3 &A, v3 B)
{
    A = A + B;

    return A; 
}

internal v3 &
operator-=(v3 &A, v3 B)
{
    A = A - B;

    return A; 
}

internal v3
operator*(v3 A, f32 scalar)
{
    v3 Result = {};

    Result.x = A.x * scalar;
    Result.y = A.y * scalar;
    Result.z = A.z * scalar;

    return Result;
}

internal v3
operator*(f32 scalar, v3 A)
{
    v3 Result = {};

    Result.x = A.x * scalar;
    Result.y = A.y * scalar;
    Result.z = A.z * scalar;

    return Result;
}

internal v3
operator*=(v3 &A, f32 scalar)
{
    A.x = A.x * scalar;
    A.y = A.y * scalar;
    A.z = A.z * scalar;

    return A;
}

internal f32
DotProd(v2 A, v2 B)
{
    f32 Result = (A.x*B.x) + (A.y*B.y);
    return Result;
}

internal f32
Square(f32 value)
{
    f32 Result = value * value;
    return Result;
}

#define SERENE_MATH_H
#endif