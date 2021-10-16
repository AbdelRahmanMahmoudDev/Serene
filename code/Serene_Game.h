#if !defined(SERENE_GAME_H)

#include "Serene_Core.h"
#include "3rd_Party/Handmade-Math/HandmadeMath.h"
#include "Serene_Tiles.h"
#include "Serene_OpenGL.h"

#include "3rd_Party/box2d/include/box2d/b2_world.h"
#include "3rd_Party/box2d/include/box2d/b2_math.h"

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

// this might be temporary as an easy way to persist between frames
#if 0
struct GamePhysics
{
	b2World *physics_world;
	b2BodyDef ground_body_definition;
	b2Body *ground_body;
	b2PolygonShape ground_polygon;
};
#endif 

struct GameState
{
	WorldPosition PlayerPos;
	hmm_v2 PlayerVelocity;
	World *world;

	MemoryArena WorldArena;
	MemoryArena RendererArena;
	MemoryArena PhysicsArena;

	u32 shader_program;

	PNGAsset Grass;
	PNGAsset Mud;

	OpenGL_Batch_State opengl_batch;
	// TODO(Abdo): Move these to some kind o OpenGL Texture struct
	u32 texture_0;
	u32 texture_1;
};

#define SERENE_GAME_H
#endif