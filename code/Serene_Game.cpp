#include "Serene_Game.h"
#include "Serene_Platform.h"
#include "Serene_Intrinsics.h"
#include "Serene_Memory.cpp"

#include "3rd_Party/Handmade-Math/HandmadeMath.h"
#include "3rd_Party/glad/glad/glad.h"
#include "3rd_Party/glad/glad/glad.c"
#include "Serene_OpenGL.h"
#include "Serene_OpenGL.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include "3rd_Party/stb_image/stb_image.h"

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



#include "Serene_Tiles.cpp"

// TODO(Abdo): collapse this into another function in the asset system
internal void
ConstructAssetDirectory(char *desired_path, char *asset_path, char *resource_path)
{
	strcpy(desired_path, asset_path);
	strcat(desired_path, resource_path);
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
		// NOTE(Abdo): Right now, assets are being loaded onto the stack or using a separate heap allocation using CRT
		// In the future, this should be allocated on a memory arena that we control
		// TODO(Abdo): modify stb_image.h to allocate stuff on the memory arena
		// TODO(Abdo): Asset system!!!!
		char vertex_shader_path[MAX_PATH];
		ConstructAssetDirectory(vertex_shader_path, asset_path->AssetPath, "/Shaders/BasicFill.vert");

		char fragment_shader_path[MAX_PATH];
		ConstructAssetDirectory(fragment_shader_path, asset_path->AssetPath, "/Shaders/BasicFill.frag");

        State->opengl_batch.shader_program = OpenGLLoadShaders(Memory->DebugPlatformReadEntireFile, vertex_shader_path, fragment_shader_path, &thread);

		stbi_set_flip_vertically_on_load(1);
		
		//stb uses malloc and delete
		// TODO(Abdo): make stb use our asset allocater;
		char grass_path[MAX_PATH];
		ConstructAssetDirectory(grass_path, asset_path->AssetPath, "/assets/textures/Grass.png");
		State->Grass = {};
		State->Grass.Data = stbi_load(grass_path, &State->Grass.Width, &State->Grass.Height, &State->Grass.Channel_Count, 0);
		State->texture_0 = OpenGLCreateTexture(&State->Grass);

		char mud_path[MAX_PATH];
		ConstructAssetDirectory(mud_path, asset_path->AssetPath, "/assets/Textures/Mud.png");
		State->Mud = {};
		State->Mud.Data = stbi_load(mud_path, &State->Mud.Width, &State->Mud.Height, &State->Mud.Channel_Count, 0);
		State->texture_1 = OpenGLCreateTexture(&State->Mud);

		// Setting up memory arenas
		InitializeArena(&State->WorldArena,
					 	MEGABYTES(10),
						(u8 *)Memory->PermanentStorage + sizeof(GameState));

		InitializeArena(&State->RendererArena,
		                MEGABYTES(500),
						(u8 *)Memory->PermanentStorage + sizeof(GameState) + MEGABYTES(10));						

		State->world = PushStruct(&State->WorldArena, World);
		World *world = State->world;
		world->Tiles = PushStruct(&State->WorldArena, TileMap);
		TileMap *tile_map = world->Tiles;
		
		u32 max_quad_count = 1000;
		OpenGLInitRenderer(renderer_dimensions, &State->opengl_batch, &State->RendererArena, max_quad_count);

		// NOTE(Abdo): This tile map struct is outdated!!
		// TODO(Abdo): Reconstruct this to fit new world structure
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

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	OpenGLPushFlatQuad(&State->opengl_batch, {0.0f , 0.0f, 0.0f}, 100.0f, {1.0f, 0.0f, 0.0f, 1.0f});
	OpenGLPushFlatQuad(&State->opengl_batch, {125.0f , 0.0f, 0.0f}, 100.0f, {0.0f, 1.0f, 0.0f, 1.0f});
	OpenGLPushFlatQuad(&State->opengl_batch, {-125.0f , 0.0f, 0.0f}, 100.0f, {0.0f, 0.0f, 1.0f, 1.0f});
	OpenGLPushFlatQuad(&State->opengl_batch, {250.0f , 0.0f, 0.0f}, 100.0f, {1.0f, 1.0f, 0.0f, 1.0f});
	OpenGLPushFlatQuad(&State->opengl_batch, {-250.0f , 0.0f, 0.0f}, 100.0f, {1.0f, 0.0f, 1.0f, 1.0f});
	OpenGLPushFlatQuad(&State->opengl_batch, {0.0f , -150.0f, 0.0f}, 100.0f, {0.0f, 1.0f, 1.0f, 1.0f});

	OpenGLPushTexturedQuad(&State->opengl_batch, {125.0f, -200.0f, 0.0f}, 100.0f, State->texture_0);
	OpenGLPushTexturedQuad(&State->opengl_batch, {-125.0f, -200.0f, 0.0f}, 100.0f, State->texture_1);
	OpenGLFlush(&State->opengl_batch);
	
#if 0
	// Draws a grid of quads
	// TODO(Abdo): Pull this out into a function that can easily sample from a texture atlas and draw basic levels
	for (f32 y = -360.0f;
		y < 360.0f;
		y += 100.0f)
	{
		for(f32 x = -640.0f;
		   x < 640.0f;
		   x += 100.0f)
		{
			buffer = OpenGLCreateQuad(buffer, {x, y, 0.0f}, 100.0f, {1.0f, 0.0f, 0.0f, 1.0f}, 0.0f);
			index_count += 6;
		}
	}
#endif
}