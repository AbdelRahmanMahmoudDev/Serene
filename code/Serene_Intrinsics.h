#if !defined(SERENE_INTRINSICS_H)
#include <cmath>

//TODO(abdo): stop relying on CRT and do intrinsics!!!

internal i32
Roundf32Toi32(f32 value)
{
	i32 Result = (i32)roundf(value);
	return Result;
}

internal u32
Roundf32Tou32(f32 value)
{
	u32 Result = (u32)(value + 0.5f);
	return Result;
}

internal i32
Floorf32Toi32(f32 value)
{
	i32 Result = (i32)floorf(value);
	return Result;
}

internal i32
Truncatef32Toi32(f32 value)
{
	i32 Result = (i32)(value);
	return Result;
}

internal i32
CeilFloat(f32 value)
{
	i32 Result = (i32)ceilf(value);
	return Result;
}

struct BitScanResult 
{
	u32 Index;
	b32 IsFound;
};

// This finds the first least significant set bit
// for 32 bit unsigned integers
internal BitScanResult
FindLeastSignificantSetBit32(u32 value)
{
	BitScanResult Result = {};
	#if COMPILER_MSVC
	Result.IsFound = _BitScanForward((unsigned long *)&Result.Index, value);
	#else
	for(u32 Test = 0;
    Test < 32;
    ++Test)
    {
        if(value & (1 << Test))
        {
            Result.Index = Test;
            Result.IsFound = true;
            break;
        }
    }
	#endif

	return Result;
}

#define SERENE_INTRINSICS_H
#endif