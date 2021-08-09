#if !defined(SERENE_MATH_H)

// 2 component vector 
// STUDY(abdo): See if this union can be of use later on
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

internal f32
Square(f32 value)
{
    f32 Result = value * value;
    return Result;
}

internal f32
DotProd(v2 A, v2 B)
{
    f32 Result = (A.x*B.x) + (A.y*B.y);
    return Result;
}

#define SERENE_MATH_H
#endif