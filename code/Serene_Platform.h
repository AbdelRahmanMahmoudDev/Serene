// This file will contain all structures passed from the OS to the game
#if !defined(SERENE_PLATFORM_H)

#include <stdio.h>

struct GameSoundOutput
{
	u64 SoundBufferSize;
	u16 NumberOfChannels;
	u32 SamplesPerSecond;
	u32 BytesPerSample;
	u16 BitsPerSample;
	u8* SoundData;
	b32 IsBufferFilled;
};

struct GameButtonState
{
	i32 HalfTransitionCount;
	b32 EndedPress;
};

struct GameController
{
	b32 IsConnected;
	b32 IsAnalog;
    
	//Left stick
	f32 LeftStickAverageX;
	f32 LeftStickAverageY;
    
	//Right stick
	f32 RightStickAverageX;
	f32 RightStickAverageY;
    
	//Left Trigger
	f32 LeftTriggerAverage;
    
	//Right Trigger
	f32 RightTriggerAverage;
    
	union
	{
		GameButtonState Buttons[14];
		struct 
		{
            GameButtonState DPadUp;
            GameButtonState DPadDown;
            GameButtonState DPadLeft;
            GameButtonState DPadRight;
            
            GameButtonState A;
            GameButtonState B;
            GameButtonState X;
            GameButtonState Y;
            
            GameButtonState Start;
            GameButtonState Back;
            
            GameButtonState LeftShoulder;
            GameButtonState RightShoulder;
            
            GameButtonState LeftThumb;
            GameButtonState RightThumb;
		};
	};
};

struct GameInput
{
	//timestep
	f32 TargetSecondsPerFrame;
    
	//debug mouse stuff
	GameButtonState MouseButtons[5];
	i32 MouseX, MouseY, MouseZ;
	//1 keyboard + 4 game pads
	GameController Controllers[5];
};

GameController* GetController(GameInput* Input, u32 ControllerIndex)
{
	Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
	GameController* Result = &Input->Controllers[ControllerIndex]; 
	return Result;
}

struct DebugPlatformReadFileResult
{
	u32 ContentSize;
	void* Content;
};

struct ThreadContext
{
	i32 Placeholder;
};

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DebugPlatformReadFileResult name(ThreadContext* Thread, char* FileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(ThreadContext* Thread, char* FileName, void* WriteableMemory, u32 MemorySize)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(ThreadContext* Thread, void* FreeableMemory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

struct GameRendererDimensions
{
    u32 ScreenWidth;
    u32 ScreenHeight;

	// 2 dimensional vector
};

struct GameAssetPath
{
	char *AssetPath;
};


// Permanent Storage is data that persists between frames (Game state, renderer buffer)
// Transient Storage is data that doesn't necessarily live in memory between frames (disk i/o)
// IMPORTANT(Abdo): All storage must be cleared to zero at startup
// Debug file functions are stored here
struct GameMemory
{
	b32 IsInitialized;
	u64 PermanentStorageSize;
	void* PermanentStorage;
    
	u64 TransientStorageSize;
	void* TransientStorage;
    

	// These functions are set the OS
	// Different OSs will have different implementations for file i/o
	debug_platform_read_entire_file* DebugPlatformReadEntireFile;
	debug_platform_write_entire_file* DebugPlatformWriteEntireFile;
	debug_platform_free_file_memory* DebugPlatformFreeFileMemory;
};


#define GAME_GENERATE_AUDIO(name) void name(ThreadContext* Thread, GameSoundOutput* SoundOutput)
typedef GAME_GENERATE_AUDIO(game_generate_audio);

#define GAME_UPDATE(name) void name(ThreadContext* Thread, GameMemory* Memory, GameRendererDimensions *renderer_dimensions, GameInput* Input, GameSoundOutput* SoundOutput, GameAssetPath *asset_path)
typedef GAME_UPDATE(game_update);

// Note(abdo): These functions were taken from HandmadeHero for educational purposes only
// they're currently being used in Serene_Win32.cpp 
// Important(abdo): Use c string functions instead of these

// quick and dirt concatenate strings
internal void
CatStrings(i64 SourceACount, char *SourceA,
		   i64 SourceBCount, char *SourceB,
		   i64 DestinationCount, char *Destination)
{
	for(i32 index = 0; index < SourceACount; ++index)
	{
		*Destination++ = *SourceA++;
	}

	for(i32 index = 0; index < SourceBCount; ++index)
	{
		*Destination++ = *SourceB++;
	}

	//Add null termination
	*Destination++ = 0;
}


internal i32
StringLength(char *String)
{
	i32 result = 0;
	while(*String++)
	{
		result++;
	}

	return result;
}


#define SERENE_PLATFORM_H
#endif