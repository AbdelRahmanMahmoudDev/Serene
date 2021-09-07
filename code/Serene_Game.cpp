#include "Serene_Game.h"
#include "Serene_Platform.h"
#include "Serene_Intrinsics.h"

#include "3rd_Party/Handmade-Math/HandmadeMath.h"
#include "3rd_Party/glad/glad/glad.h"
#include "3rd_Party/glad/glad/glad.c"
#include "Serene_OpenGL.h"
#include "Serene_OpenGL.cpp"

// The game layer is currently only responsible for filling the sound buffer
// Sound play and update is managed by the OS
extern "C" GAME_GENERATE_AUDIO(GameGenerateAudio)
{
    i16 amplitude = 32760;
	f32 frequency = 441.0f;
	double t = (2 * Pi * frequency) / (SoundOutput->SamplesPerSecond * SoundOutput->NumberOfChannels); // wave phase
	for(i32 sampleIndex = 0; sampleIndex < SoundOutput->SoundBufferSize; ++sampleIndex)
	{
#if 0
		SoundOutput->SoundData[sampleIndex] = (u8)copysign(1.0, (amplitude * sin(t * sampleIndex)));
		SoundOutput->SoundData[sampleIndex+1] = (u8)copysign(1.0, (amplitude * sin(t * sampleIndex)));
#else
		SoundOutput->SoundData[sampleIndex] = 0;
#endif
	}
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

#include "Serene_Tiles.cpp"

// struct members pulled from https://www.fileformat.info/format/bmp/egff.htm
// web page includes information about the file format
#pragma pack(push, 1)
struct BMPHeader
{
	u16 FileType;        /* File type, always ("BM") */
	u32 FileSize;        /* Size of the file in bytes */
	u16 Reserved1;       /* Always 0 */
	u16 Reserved2;       /* Always 0 */
	u32 BitmapOffset;    /* Starting position of image data in bytes */
	u32 Size;            /* Size of this header in bytes */
	i32 Width;           /* Image width in pixels */
	i32 Height;          /* Image height in pixels */
	u16 Planes;          /* Number of color planes */
	u16 BitsPerPixel;    /* Number of bits per pixel */
	u32 Compression;     /* Compression methods used */
	u32 SizeOfBitmap;    /* Size of bitmap in bytes */
	i32 HorzResolution;  /* Horizontal resolution in pixels per meter */
	i32 VertResolution;  /* Vertical resolution in pixels per meter */
	u32 ColorsUsed;      /* Number of colors in the image */
	u32 ColorsImportant; /* Minimum number of important colors */
	u32 RedMask;         /* Mask identifying bits of red component */
	u32 GreenMask;       /* Mask identifying bits of green component */
	u32 BlueMask;        /* Mask identifying bits of blue component */
};
#pragma pack(pop)


internal BMPAsset
DEBUGLoadBMP(char *path, debug_platform_read_entire_file *pReadEntireFile, ThreadContext *thread)
{
	BMPAsset Result = {};
	DebugPlatformReadFileResult read_result = pReadEntireFile(thread, path);

	if(read_result.ContentSize != 0)
	{
		BMPHeader *bitmap_header = (BMPHeader *)read_result.Content;
		u32 *pixels = (u32 *)((u8 *)read_result.Content + bitmap_header->BitmapOffset);
		Result.Pixels = pixels;
		Result.Width = bitmap_header->Width;
		Result.Height = bitmap_header->Height;

		Assert(bitmap_header->Compression == 3);

		u32 red_mask = bitmap_header->RedMask;
		u32 green_mask = bitmap_header->GreenMask;
		u32 blue_mask = bitmap_header->BlueMask;
		u32 alpha_mask = ~(red_mask | green_mask | blue_mask);

		BitScanResult red_shift = FindLeastSignificantSetBit32(red_mask); 
		BitScanResult green_shift = FindLeastSignificantSetBit32(green_mask); 
		BitScanResult blue_shift = FindLeastSignificantSetBit32(blue_mask); 
		BitScanResult alpha_shift = FindLeastSignificantSetBit32(alpha_mask); 

		Assert(red_shift.IsFound);
		Assert(green_shift.IsFound);
		Assert(blue_shift.IsFound);
		Assert(alpha_shift.IsFound);

		u32 *SourceDest = pixels;
		for(i32 y = 0; y < bitmap_header->Height; ++y)
		{
			for(i32 x = 0; x < bitmap_header->Width; ++x)
			{
                u32 C = *SourceDest;
                *SourceDest++ = ((((C >> alpha_shift.Index) & 0xFF) << 24) |
                                 (((C >> red_shift.Index) & 0xFF) << 16) |
                                 (((C >> green_shift.Index) & 0xFF) << 8) |
                                 (((C >> blue_shift.Index) & 0xFF) << 0));
			}
		}
	}

	return Result;
}


extern "C" GAME_UPDATE(GameUpdate)
{
	// 9 rows, 16 columns
	local_persist u32 test_level_layout[256][256] = 
	{
        {2, 2, 2, 2,  2, 2, 2, 2,  1, 2, 2, 2,  2, 2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  1, 2, 2, 2,  2, 2, 2, 2, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 2, 2, 2,  2, 2, 2, 2,  1, 2, 2, 2,  2, 2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  1, 2, 2, 2,  2, 2, 2, 2, 2},
        {2, 2, 2, 2,  2, 2, 2, 2,  1, 2, 2, 2,  2, 2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  1, 2, 2, 2,  2, 2, 2, 2, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2,  2, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 2},
        {2, 2, 2, 2,  2, 2, 2, 2,  1, 2, 2, 2,  2, 2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  1, 2, 2, 2,  2, 2, 2, 2, 2},
	};


	// Memory
	Assert(sizeof(GameState) <= Memory->PermanentStorageSize);
	GameState* State = (GameState*)Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		// We need to load all driver functions into the game dll
		// IMPORTANT(Abdo): This only applies to the OpenGL Renderer
		// TODO(Abdo): Do this somewhere more sensible!!
		i32 glad_status = gladLoadGL();
		glViewport(0, 0, renderer_dimensions->ScreenWidth, renderer_dimensions->ScreenHeight);
		// Placeholder thread thing
		ThreadContext thread = {};
        
		// Loading game assets
		// Right now, this depends on setting the working directory in Visual Studio
		// TODO(Abdo): Construct an asset path during runtime
#if 0
		State->BackDrop = DEBUGLoadBMP("assets/test_background.bmp", Memory->DebugPlatformReadEntireFile, &thread);
		State->Player = DEBUGLoadBMP("Character1/dude.bmp", Memory->DebugPlatformReadEntireFile, &thread);
#else
		#define MAX_PATH 260
		char *VertPath = "/Shaders/BasicFill.vert";
		char full_vert[MAX_PATH];
		strcpy(full_vert, asset_path->AssetPath);
		strcat(full_vert, VertPath);

		char *FragPath = "/Shaders/BasicFill.frag";
		char full_frag[MAX_PATH];
		strcpy(full_frag, asset_path->AssetPath);
		strcat(full_frag, FragPath);



        State->shader_program = OpenGLLoadShaders(Memory->DebugPlatformReadEntireFile, full_vert, full_frag, &thread);
#endif
		// Setting up memory arenas
		InitializeArena(&State->WorldArena,
					 	Memory->PermanentStorageSize - sizeof(GameState),
						(u8*)Memory->PermanentStorage + sizeof(GameState));

		State->world = PushStruct(&State->WorldArena, World);
		World *world = State->world;
		world->Tiles = PushStruct(&State->WorldArena, TileMap);
		TileMap *tile_map = world->Tiles;
		
		
		tile_map->ChunkShift = 8;
		tile_map->ChunkMask = (1 << tile_map->ChunkShift) - 1;
		tile_map->ChunkDimm = 1 << tile_map->ChunkShift;
		tile_map->ChunkCountX = 32;
		tile_map->ChunkCountY = 32;
		tile_map->TileChunks = PushArray(&State->WorldArena, tile_map->ChunkCountX * tile_map->ChunkCountY, TileChunk);

		tile_map->TileWidthInMeters = 1.4f;
		tile_map->TileWidthInPixels = 60;
		tile_map->MetersToPixels = (f32)tile_map->TileWidthInPixels / (f32)tile_map->TileWidthInMeters;

		// The origin of the world is designed to be at the bottom left
		// This means the x origin remains the same
		// But the y origin must be moved from the top left to the bottom left
		// the bottom left is the top left offset by the bitmap height
		// This is because the OS creates a buffer that starts at the top left 
		tile_map->Origin = {0.0f, 0.0f};

		// State
		State->PlayerPos.TileX = 3;
		State->PlayerPos.TileY = 3;
		State->PlayerPos.TileRelativePos.X = 5.0f;
		State->PlayerPos.TileRelativePos.Y = 5.0f;

		for (i32 Y = 0;
			Y < 256;
			++Y)
			{
				for(i32 X = 0;
				    X < 256;
					++X)
					{
						i32 value = 0;
						if(test_level_layout[Y][X] > 0)
						{
							value = test_level_layout[Y][X] == 1 ? 1 : 2;
						}

						SetTile(State->WorldArena, tile_map, X, Y, value);
					}
			}


		Memory->IsInitialized = true;
	}
    
	World *world = State->world;
	TileMap *tile_map = world->Tiles;

	f32 PlayerR = 0.3f;
	f32 PlayerG = 0.5f;
	f32 PlayerB = 0.2f;
	f32 PlayerHeight = 1.4f;
	f32 PlayerWidth = 0.75f * PlayerHeight;
    
	//Input
	for(u32 ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
	{
		GameController* Controller0 = &Input->Controllers[ControllerIndex];
		if(Controller0->IsAnalog)
		{
			//Analog processing
		}
		else
		{
			//Digital processing
            hmm_v2 PlayerAcceleration = {};
			if(Controller0->DPadUp.EndedPress)
			{
				PlayerAcceleration.Y = 1.0f;
			}
            
			if(Controller0->DPadDown.EndedPress)
			{
				PlayerAcceleration.Y = -1.0f;
			}
            
			if(Controller0->DPadLeft.EndedPress)
			{
				PlayerAcceleration.X = -1.0f;
			}
            
			if(Controller0->DPadRight.EndedPress)
			{
				PlayerAcceleration.X = 1.0f;
			}
			PlayerAcceleration *= 4.0f;
			// Poor man's friction
			PlayerAcceleration += -0.9f * State->PlayerVelocity;
		}
	}
    
	// Audio
	if(!SoundOutput->IsBufferFilled)
	{
		ThreadContext thread = {};
		GameGenerateAudio(&thread, SoundOutput);
		SoundOutput->IsBufferFilled = true;
	}

	// Render 
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	f32 tile_dimms = 60.0f;
	i32 column_count = CeilFloat((f32)renderer_dimensions->ScreenWidth / tile_dimms);
	i32 row_count = CeilFloat((f32)renderer_dimensions->ScreenHeight / tile_dimms);

	hmm_v2 origin = {0.0f, 0.0f};

	Quadrilateral tile = {};
	tile.Position[0] = {-0.5f, -0.5f, 0.0f}; // bottom left
	tile.Position[1] = {0.5f, -0.5f, 0.0f}; // bottom right
	tile.Position[2] = {0.5f, 0.5f, 0.0f}; // top right
	tile.Position[3] = {-0.5f, 0.5f, 0.0f}; // top left

	i32 indices[] = {0, 1, 2, 2, 3, 0};

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	hmm_mat4 model = HMM_Transpose(HMM_Translate({0.0f, 0.0f, 0.0f}) *
					 HMM_Scale({60.0f / (f32)renderer_dimensions->ScreenWidth, 60.0f / (f32)renderer_dimensions->ScreenHeight, 1.0f}));
	hmm_mat4 view = HMM_Transpose(HMM_Translate({0.0f, 0.0f, -3.0f}));
	//hmm_mat4 projection = HMM_Transpose(HMM_Orthographic(0.0f, (f32)renderer_dimensions->ScreenWidth, (f32)renderer_dimensions->ScreenHeight, 0.0f, 0.1f, 100.0f));
	hmm_mat4 projection = HMM_Mat4d(1.0f);
	hmm_mat4 model_view_projection = projection * view * model;


	f32 offset = 3.0f;
	for(i32 row_index = 0;
	    row_index <= row_count;
		++row_index)
		{
			for(i32 column_index = 0;
				column_index <= column_count;
				++column_index)
				{
#if 0					
					tile.Position[0] = OpenGLNormalizePosition({(column_index*tile_dimms)+origin.x,
					 											(row_index*tile_dimms)+origin.y, 0.0f},
					                                           renderer_dimensions->ScreenWidth,
															   renderer_dimensions->ScreenHeight);

					tile.Position[1] = OpenGLNormalizePosition({(column_index*tile_dimms)+(origin.x+tile_dimms),
																(row_index*tile_dimms)+origin.y, 0.0f},
					                                           renderer_dimensions->ScreenWidth,
															   renderer_dimensions->ScreenHeight);

					tile.Position[2] = OpenGLNormalizePosition({(column_index*tile_dimms)+(origin.x+tile_dimms),
																(row_index*tile_dimms)+(origin.y+tile_dimms), 0.0f},
					                                           renderer_dimensions->ScreenWidth,
															   renderer_dimensions->ScreenHeight);

					tile.Position[3] = OpenGLNormalizePosition({(column_index*tile_dimms)+origin.x,
					                                            (row_index*tile_dimms)+(origin.y+tile_dimms), 0.0f},
					                                           renderer_dimensions->ScreenWidth,
															   renderer_dimensions->ScreenHeight);
					tile.Position[0] = {(column_index*tile_dimms)+origin.x,
					 					(row_index*tile_dimms)+origin.y, 0.0f};

					tile.Position[1] = {(column_index*tile_dimms)+(origin.x+tile_dimms),
										(row_index*tile_dimms)+origin.y, 0.0f};

					tile.Position[2] = {(column_index*tile_dimms)+(origin.x+tile_dimms),
										(row_index*tile_dimms)+(origin.y+tile_dimms), 0.0f};

					tile.Position[3] = {(column_index*tile_dimms)+origin.x,
					                    (row_index*tile_dimms)+(origin.y+tile_dimms), 0.0f};
#else
#if 1
				model = HMM_Transpose(HMM_Translate({ row_index * tile_dimms, column_index * tile_dimms, 0.0f }));
#else
				model = HMM_Mat4d(1.0f);  
#endif

					model_view_projection = projection * view * model;
#endif

					if(((column_index % 2) == 0) && ((row_index % 2) == 0))
					{
						tile.Color[0] = {1.0f, 0.0f, 0.0f};															   														   															   
						tile.Color[1] = {1.0f, 0.0f, 0.0f};															   														   															   
						tile.Color[2] = {1.0f, 0.0f, 0.0f};															   														   															   
						tile.Color[3] = {1.0f, 0.0f, 0.0f};
					}
					else
					{
						tile.Color[0] = {0.0f, 0.0f, 1.0f};															   														   															   
						tile.Color[1] = {0.0f, 0.0f, 1.0f};															   														   															   
						tile.Color[2] = {0.0f, 0.0f, 1.0f};															   														   															   
						tile.Color[3] = {0.0f, 0.0f, 1.0f};
					}

					u32 vertex_array_object = 0;
					glGenVertexArrays(1, &vertex_array_object);
					glBindVertexArray(vertex_array_object);

					u32 vertex_buffer_object = 0;
					glGenBuffers(1, &vertex_buffer_object);
					glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
					glBufferData(GL_ARRAY_BUFFER, sizeof(Quadrilateral), &tile, GL_STATIC_DRAW);
					glEnableVertexAttribArray(0);
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
					                      offsetof(Quadrilateral, Position) / sizeof(f32),
										  (void *)(offsetof(Quadrilateral, Position)));
					glEnableVertexAttribArray(1);
					glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
					                      offsetof(Quadrilateral, Color) / sizeof(f32),
										  (void *)(offsetof(Quadrilateral, Color)));										  

					u32 element_buffer_object = 0;
					glGenBuffers(1, &element_buffer_object);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

					glUseProgram(State->shader_program);

					SetMat4(State->shader_program, "u_Projection", projection);
					SetMat4(State->shader_program, "u_View", view);
					SetMat4(State->shader_program, "u_Model", model);

					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				}
		}

}