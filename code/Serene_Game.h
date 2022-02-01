#if !defined(SERENE_GAME_H)

#include "Serene_Core.h"
#include "3rd_Party/Handmade-Math/HandmadeMath.h"
// #include "Serene_Tiles.h"
#include "Serene_OpenGL.h"

struct MemoryArena
{
	u8 *Base;
	MemoryIndex Size;
	MemoryIndex Used;
};

// Each entity would have an AABB?
// Note (Abdo): Think about this more
struct AxisAlignedBoundingBox
{
	hmm_v3 CenterPoint;
	f32 WidthHalfLength;
	f32 HeightHalfLength;
};

struct Entity
{
	// These 2 give the overall span of the entity, used in the renderer
	hmm_v3 bottom_left_corner;
	hmm_v3 dimensions;
    
	// These define the axis aligned bounding box
	hmm_v3 CenterPoint;
	f32 WidthHalfLength;
	f32 HeightHalfLength;
	hmm_v3 HalfExtents;
    
	b32 IsExistant;
};

struct TileMap
{
	u32 *Tile_Data;
};

struct World
{
	TileMap *TileMaps;
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
    // Position in a tileset
	i32 TileX;
	i32 TileY;
    
    // Position inside a tile
    // This allows great precision and therefore
    // smooth movement
	hmm_v2 TileRelativePos;
};

struct GameState
{
	// Player stuff
	hmm_v3 PlayerPosition;
    
	// Ignore these for now
	hmm_v2 PlayerVelocity;
	World *world;
	TileMap *TileMap;
    
	//Entities
	Entity EntityList[10];
    
	MemoryArena WorldArena;
	MemoryArena RendererArena;
    
	u32 shader_program;
    
	PNGAsset Grass;
	PNGAsset Mud;
    
	OpenGL_Batch_State opengl_batch;
	// TODO(Abdo): Move these to some kind o OpenGL Texture struct
	u32 texture_Grass;
	u32 texture_Mud;
};

#define SERENE_GAME_H
#endif