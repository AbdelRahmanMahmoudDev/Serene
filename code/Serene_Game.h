#if !defined(SERENE_GAME_H)

#include "Serene_Core.h"
#include "3rd_Party/Handmade-Math/HandmadeMath.h"
#include "Serene_Tiles.h"
#include "Serene_OpenGL.h"

struct MemoryArena
{
	u8 *Base;
	MemoryIndex Size;
	MemoryIndex Used;
};

struct World
{
	TileMap *Tiles;
};

struct BMPAsset
{
	u32 *Pixels;
	i32 Width;
	i32 Height;
};

struct PNGAsset
{
	u8 *Data;
	i32 Width;
	i32 Height;
	i32 Channel_Count;
};

struct WorldPosition
{
	i32 TileX;
	i32 TileY;

	hmm_v2 TileRelativePos;
};


struct GameState
{
	WorldPosition PlayerPos;
	hmm_v2 PlayerVelocity;
	World *world;
	MemoryArena WorldArena;

	u32 shader_program;

	PNGAsset Grass;
	PNGAsset Mud;

	// TODO(Abdo): Move these to some kind o OpenGL Texture struct
	u32 texture_0;
	u32 texture_1;
};

#define SERENE_GAME_H
#endif