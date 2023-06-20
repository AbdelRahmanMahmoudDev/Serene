#include <cmath>

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
DotProd(v3 A, v3 B)
{
    f32 Result = (A.x*B.x) + (A.y*B.y) + (A.z*B.z);
    return Result;
}

internal f32
Square(f32 value)
{
    f32 Result = value * value;
    return Result;
}

/*
NOTE(Abdo) Concerning coordinate spaces and matrices
The engine uses a coordinate system where:
+x is right
+y is up
-z is forward

The engine assumes:
matrices are filled in column major ordering

The justification for these conventions is that most
math textbooks use these conventions and it would be easier
if we could copy things from textbooks without much
confusion
*/

/*
a 0 0 d
0 b 0 e
0 0 c f
0 0 0 1
*/
internal mat4
Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near_plane, f32 far_plane)
{
    mat4 result = {};

    f32 a = 2 / (right - left);
    f32 b = 2 / (top - bottom);
    f32 c =  (2 / (near_plane - far_plane));
    f32 d = (left + right) / (left - right);
    f32 e = (bottom + top) / (bottom - top);
    f32 f = (near_plane + far_plane) / (near_plane - far_plane);

    result.m[0] = a;
    result.m[5] = b;
    result.m[10] = c;
    result.m[12] = d;
    result.m[13] = e;
    result.m[14] = f;
    result.m[15] = 1.0f;
    return(result);

}

/*
a 0  c 0
0 b  d 0
0 0  e f
0 0 -1 0
*/
internal mat4
Perspective(f32 width, f32 height, f32 near_plane, f32 far_plane)
{
    mat4 result = {};

    f32 left = 0.0f;
    f32 right = width;
    f32 bottom = 0.0f;
    f32 top = height;

    f32 a = (2.0f * near_plane) / (right - left);
    f32 b = (2.0f * near_plane) / (top - bottom);
    f32 c = ((right + left) / (right - left));
    f32 d = ((top + bottom) / (top - bottom));
    f32 e = ((near_plane + far_plane) / (near_plane - far_plane));
    f32 f = (2 * near_plane * far_plane) / (near_plane - far_plane);

    result.m[0] = a;
    result.m[5] = b;
    result.m[8] = c;
    result.m[9] = d;
    result.m[10] = e;
    result.m[11] = -1.0f;
    result.m[14] = f;

    return(result);
}

/*
1 0 0 a
0 1 0 b
0 0 1 c
0 0 0 1
*/
internal mat4
Translate(v3 translation_vector)
{
    mat4 result = {};

    f32 a = translation_vector.x;
    f32 b = translation_vector.y;
    f32 c = translation_vector.z;

    result.m[0] = 1;
    result.m[5] = 1;
    result.m[10] = 1;
    result.m[12] = a;
    result.m[13] = b;
    result.m[14] = c;
    result.m[15] = 1;

    return(result);
}

// TODO(Abdo)
// Implement Rotation!!!

internal mat4
Rotate()
{
    mat4 result = {};
    return(result);
}

/*
a 0 0 0
0 b 0 0
0 0 c 0
0 0 0 1
*/
internal mat4
Scale(v3 scale_vector)
{
    mat4 result = {};

    f32 a = scale_vector.x;
    f32 b = scale_vector.y;
    f32 c = scale_vector.z;

    result.m[0] = a;
    result.m[5] = b;
    result.m[10] = c;
    result.m[15] = 1;
    return(result);
}

internal mat4
InitMatrix()
{
    mat4 result = {};
    result.m[0] = 1;
    result.m[5] = 1;
    result.m[10] = 1;
    result.m[15] = 1;
    return(result);
}

internal mat4
ComputeModelMatrix(mat4 *scale, mat4 *rotation, mat4 *translation)
{
    mat4 result;

    u32 column_index; // for each column
    u32 row_index; // for each row
    u32 element_index; // for each element

    for(column_index = 0;
        column_index < 4; 
        ++column_index)    
    {    
        for(row_index = 0;
            row_index < 4;
            ++row_index)    
        {    
            result.m[(4 * column_index) + row_index] = 0.0f;
            for(element_index = 0;
                element_index < 4;
                ++element_index)    
            {    
                result.m[(4 * column_index) + row_index] += scale->m[(4 * column_index) + element_index] * rotation->m[(4 * element_index) + row_index];
            }    
        }    
    } 

    for(column_index = 0;
        column_index < 4; 
        ++column_index)    
    {    
        for(row_index = 0;
            row_index < 4;
            ++row_index)    
        {    
            result.m[(4 * column_index) + row_index] = 0.0f;
            for(element_index = 0;
                element_index < 4;
                ++element_index)    
            {    
                result.m[(4 * column_index) + row_index] += result.m[(4 * column_index) + element_index] * translation->m[(4 * element_index) + row_index];
            }    
        }    
    }    
    return(result);
}

internal mat4
operator*(mat4 A, mat4 B)
{
    mat4 result;

    u32 column_index; // for each column
    u32 row_index; // for each row
    u32 element_index; // for each element

    for(column_index = 0;
        column_index < 4; 
        ++column_index)    
    {    
        for(row_index = 0;
            row_index < 4;
            ++row_index)    
        {    
            result.m[(4 * column_index) + row_index] = 0.0f;
            for(element_index = 0;
                element_index < 4;
                ++element_index)    
            {    
                result.m[(4 * column_index) + row_index] += A.m[(4 * column_index) + element_index] * B.m[(4 * element_index) + row_index];
            }    
        }    
    }    
    return(result);
}