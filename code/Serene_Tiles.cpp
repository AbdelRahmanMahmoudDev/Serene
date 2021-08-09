#include "Serene_Tiles.h"

// This retrieves a tile chunk from an array
internal TileChunk *
GetTileChunk(TileMap *tile_map, i32 TileChunkX, i32 TileChunkY)
{
	TileChunk *TileChunk = 0;

	if((TileChunkX >= 0) && (TileChunkX < tile_map->ChunkDimm) &&
	   (TileChunkY >= 0) && (TileChunkY < tile_map->ChunkDimm))
	{
		  TileChunk = &tile_map->TileChunks[TileChunkY * tile_map->ChunkDimm + TileChunkX]; 
	}

	return TileChunk;
}

// We have a 32 bit uint divided into 24.8 bits
// 24 bits store the chunk position
// 8 bits store the position inside a tile position
internal ChunkPosition
GetChunkPosition(TileMap *tile_map, i32 tile_x, i32 tile_y)
{
	ChunkPosition Result = {};

	Result.ChunkX = tile_x >> tile_map->ChunkShift;
	Result.ChunkY = tile_y >> tile_map->ChunkShift;
	Result.TileRelativePos.x = (f32)(tile_x & tile_map->ChunkMask);
	Result.TileRelativePos.y = (f32)(tile_y & tile_map->ChunkMask);

	return Result;
}

// Get the position of the chunk based on the absolute tile values
// Get the chunk data based on that position
// extract the index of that chunk
internal i32
GetTile(TileMap *tile_map, i32 tile_x, i32 tile_y)
{
	i32 Result = -1;
	ChunkPosition chunk_position = GetChunkPosition(tile_map, tile_x, tile_y);
	TileChunk *chunk = GetTileChunk(tile_map, chunk_position.ChunkX, chunk_position.ChunkY);

	if(chunk)
	{
		Result = chunk->Data[tile_y*tile_map->ChunkDimm + tile_x];
	}

	return Result;
}

internal void
SetTile(TileMap *tile_map, i32 tile_x, i32 tile_y, i32 value)
{
	ChunkPosition chunk_position = GetChunkPosition(tile_map, tile_x, tile_y);
	TileChunk *chunk = GetTileChunk(tile_map, chunk_position.ChunkX, chunk_position.ChunkY);

	if(chunk)
	{
		chunk->Data[tile_y * tile_map->ChunkDimm + tile_x] = value;
	}
}

// Retrieve a tile value
// Result is true if the value is unoccupied
internal b32
IsWorldPointEmpty(TileMap *tile_map, WorldPosition position)
{
	b32 Result = false;

	i32 tile_value = GetTile(tile_map, position.TileX, position.TileY);

	Result = (tile_value == 0);

	return Result;
}

internal void
RecanonicalizeCoord(TileMap *tile_map, i32 tile_count, i32 *tile_pos, f32 *tile_relative_pos)
{
	i32 offset = Roundf32Toi32(*tile_relative_pos / tile_map->TileWidthInMeters);
	*tile_pos += offset;
	*tile_relative_pos -= offset * tile_map->TileWidthInMeters;

	Assert(*tile_relative_pos >= -0.5f * tile_map->TileWidthInMeters);

	Assert(*tile_relative_pos <= 0.5f * tile_map->TileWidthInMeters);
}

internal WorldPosition
RecanonicalizePosition(TileMap *tile_map, WorldPosition position)
{
	WorldPosition Result = position;

	RecanonicalizeCoord(tile_map, tile_map->WidthCount, &Result.TileX, &Result.TileRelativePos.x);
	RecanonicalizeCoord(tile_map, tile_map->HeightCount, &Result.TileY, &Result.TileRelativePos.y);

	return Result;
}

internal void
InitializeArena(MemoryArena *arena, MemoryIndex size, u8 *base)
{
	arena->Base = base;
	arena->Size = size;
	arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
void *
PushSize_(MemoryArena *arena, MemoryIndex size)
{
    Assert((arena->Used + size) <= arena->Size); // Don't overflow your arena
    void *Result = arena->Base + arena->Used; // Retrieve a pointer to the latest memory index before adding the memory
   	arena->Used += size; // add the new size to our index
    
    return(Result);
}
