#if !defined(SERENE_TILES_H)

struct TileChunk
{
	u32* Data;
};

struct TileMap
{
	f32 TileWidthInMeters;
	f32 TileWidthInPixels;
	f32 MetersToPixels;

	i32 ChunkShift;
	i32 ChunkMask;
	i32 ChunkDimm;
	i32 ChunkCountX;
	i32 ChunkCountY;

	v2 Origin;

	f32 TileWidth;
	f32 TileHeight;
    
	// Number of tiles in tile map
	i32 WidthCount;
	i32 HeightCount;
    
    TileChunk *TileChunks;
};

#define SERENE_TILES_H
#endif