#if !defined(SERENE_GAME_H)

#include "Serene_Core.h"
#include "Serene_Math.h"
#include "Serene_Tiles.h"

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

struct WorldPosition
{
	i32 TileX;
	i32 TileY;

	v2 TileRelativePos;
};


struct GameState
{
	WorldPosition PlayerPos;
	v2 PlayerVelocity;
	World *world;
	MemoryArena WorldArena;

	BMPAsset BackDrop;
	BMPAsset Player;
	u32 shader_program;
};

#define SERENE_GAME_H
#endif