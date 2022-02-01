#include <cmath>

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

// TODO(Abdo): collapse this into another function in the asset system
internal void
ConstructAssetDirectory(char *desired_path, char *asset_path, char *resource_path)
{
	strcpy(desired_path, asset_path);
	strcat(desired_path, resource_path);
}

// world stuff

#define PIXELS_PER_METER 80

internal f32 
MetersToPixels(f32 value_in_meters)
{
	f32 Result = value_in_meters * PIXELS_PER_METER;
	return(Result);
}

internal hmm_vec3 
MetersToPixels(hmm_vec3 vec3_in_meters)
{
	hmm_vec3 Result = {};
	Result.X = vec3_in_meters.X * PIXELS_PER_METER;
	Result.Y = vec3_in_meters.Y * PIXELS_PER_METER;
	Result.Z = vec3_in_meters.Z * PIXELS_PER_METER;
	return(Result);
}

internal f32
PixelsToMeters(f32 value_in_pixels)
{
	f32 Result = value_in_pixels / PIXELS_PER_METER;
	return(Result);
}

internal hmm_vec3
PixelsToMeters(hmm_vec3 vec3_in_pixels)
{
	hmm_vec3 Result = {};
	Result.X = vec3_in_pixels.X / PIXELS_PER_METER;
	Result.Y = vec3_in_pixels.Y / PIXELS_PER_METER;
	Result.Z = vec3_in_pixels.Z / PIXELS_PER_METER;
	return(Result);
}

#define TILE_DIMM PIXELS_PER_METER
#define TILE_COUNT_X 16
#define TILE_COUNT_Y 9

global u32 TileMap01[TILE_COUNT_Y][TILE_COUNT_X]
{
	{0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0}
};

global u32 TileMap02[TILE_COUNT_Y][TILE_COUNT_X]
{
	{0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0}
};

internal b32
IsPointEmpty(hmm_vec3 point, u32 *tiles)
{
	b32 Result = false;
    u32 player_tile_map_position_x = (u32)(point.X / TILE_DIMM);
    u32 player_tile_map_position_y = (u32)(point.Y / TILE_DIMM);
    
    if((player_tile_map_position_x >= 0 && player_tile_map_position_x <= TILE_COUNT_X) &&
       (player_tile_map_position_y >= 0 && player_tile_map_position_y <= TILE_COUNT_Y))
    {
        if(tiles[player_tile_map_position_y * TILE_COUNT_X + player_tile_map_position_x] == 1)
        {
            Result = true;
        }
    }
	return(Result);
}

internal b32
IsPointEmpty(World *world, hmm_vec3 point)
{
    
}

extern "C" GAME_UPDATE(GameUpdate)
{
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
        
        
		char mud_path[MAX_PATH];
		ConstructAssetDirectory(mud_path, asset_path->AssetPath, "/assets/Textures/Mud.png");
		State->Mud = {};
		State->Mud.Data = stbi_load(mud_path, &State->Mud.Width, &State->Mud.Height, &State->Mud.Channel_Count, 0);
		State->texture_Mud = OpenGLCreateTexture(&State->Mud);
        
		//stb uses malloc and delete
		// TODO(Abdo): make stb use our asset allocater;
		char grass_path[MAX_PATH];
		ConstructAssetDirectory(grass_path, asset_path->AssetPath, "/assets/textures/Grass.png");
		State->Grass = {};
		State->Grass.Data = stbi_load(grass_path, &State->Grass.Width, &State->Grass.Height, &State->Grass.Channel_Count, 0);
		State->texture_Grass = OpenGLCreateTexture(&State->Grass);
        
		// Figure out size of 1 level
		// This won't work at multiple resolutions / fullscreen
		// Create meter units first
		u32 level_size_in_bytes = renderer_dimensions->ScreenHeight * renderer_dimensions->ScreenHeight * sizeof(f32);
        
		// Setting up memory arenas
		InitializeArena(&State->WorldArena,
                        Memory->PermanentStorageSize - sizeof(GameState) - MEGABYTES(500),
						(u8 *)Memory->PermanentStorage + sizeof(GameState));
        
		InitializeArena(&State->RendererArena,
		                MEGABYTES(500),
						(u8 *)Memory->PermanentStorage + sizeof(GameState) + level_size_in_bytes);
		
		u32 max_quad_count = 1000;
		OpenGLInitRenderer(renderer_dimensions, &State->opengl_batch, &State->RendererArena, max_quad_count);
        
		// State
		// Player stuff
		// TODO: (Abdo) Give a spawn position in meters
		State->PlayerPosition = {1.0f, 1.0f, 0.0f};
        
        
        
		// Clear Entity list
		for(u32 entity_index = 0; entity_index < 10; ++entity_index)
		{
			State->EntityList[entity_index] = {};
			State->EntityList[entity_index].IsExistant = false;
		}
        
		// Add entities
		// TODO: (Abdo) Entity grouping!!!!
		// TODO: (Abdo) Change entity bottom_left_corner to centre, this requires changes in rendering code
		// TODO: (Abdo) Finish transitioning to meters 
		Entity flat_entity = {};
		flat_entity.bottom_left_corner = {2.0f , 2.0f, 0.0f};
		flat_entity.dimensions = {5.0f, 5.0f, 0.0f};
		flat_entity.IsExistant = true;
		State->EntityList[0] = flat_entity;
        
		Entity textured_entity = {};
		textured_entity.bottom_left_corner = {4.0f, 4.0f, 0.0f};
		textured_entity.dimensions = {1.0f, 1.0f, 0.0f};
		textured_entity.IsExistant = true;
		State->EntityList[1] = textured_entity;
        
		Entity player_entity = {};
		player_entity.bottom_left_corner = State->PlayerPosition;
		player_entity.dimensions = {1.0f, 1.5f, 0.0f};
		player_entity.IsExistant = true;
		State->EntityList[2] = player_entity;
        
		Memory->IsInitialized = true;
	}
    
    World world = {};
    TileMap tile_maps[2];
    tile_maps[0].Tile_Data = *TileMap01;
    tile_maps[1].Tile_Data = *TileMap02;
    world.TileMaps = tile_maps;
    
    // Camera controls
	local_persist f32 go_up = 1.0f;
	local_persist f32 go_left = 1.0f;
    
	// Input
	for(u32 ControllerIndex = 0; ControllerIndex < 1; ++ControllerIndex)
	{
		GameController* Controller0 = &Input->Controllers[0];
		if(Controller0->IsAnalog)
		{
			// Analog processing
		}
		else
		{
#if CAMERA_PATH
			// Digital processing
			if(Controller0->DPadUp.EndedPress)
			{
				go_up-=25.0f;
				State->opengl_batch.view = HMM_Translate({go_left, go_up, 0.0f});
				State->opengl_batch.camera_transform = State->opengl_batch.projection * State->opengl_batch.view;
				OpenGLSetMat4(State->opengl_batch.shader_program, "u_ProjectionView", State->opengl_batch.camera_transform);
			}
            
			if(Controller0->DPadDown.EndedPress)
			{
				go_up+=25.0f;
				State->opengl_batch.view = HMM_Translate({go_left, go_up, 0.0f});
				State->opengl_batch.camera_transform = State->opengl_batch.projection * State->opengl_batch.view;
				OpenGLSetMat4(State->opengl_batch.shader_program, "u_ProjectionView", State->opengl_batch.camera_transform);
			}
            
			if(Controller0->DPadLeft.EndedPress)
			{
				go_left+=25.0f;
				State->opengl_batch.view = HMM_Translate({go_left, go_up, 0.0f});
				State->opengl_batch.camera_transform = State->opengl_batch.projection * State->opengl_batch.view;
				OpenGLSetMat4(State->opengl_batch.shader_program, "u_ProjectionView", State->opengl_batch.camera_transform);
			}
            
			if(Controller0->DPadRight.EndedPress)
			{
				go_left-=25.0f;
				State->opengl_batch.view = HMM_Translate({go_left, go_up, 0.0f});
				State->opengl_batch.camera_transform = State->opengl_batch.projection * State->opengl_batch.view;
				OpenGLSetMat4(State->opengl_batch.shader_program, "u_ProjectionView", State->opengl_batch.camera_transform);
			}
#else
			// Digital processing
			// Player movement code
			f32 velocity = 7.5f;
			hmm_v3 new_pos = MetersToPixels(State->PlayerPosition);
			if(Controller0->DPadUp.EndedPress)
			{
				new_pos.Y += velocity;
			}
            
			if(Controller0->DPadDown.EndedPress)
			{
				new_pos.Y -= velocity;
			}
            
			if(Controller0->DPadLeft.EndedPress)
			{
				new_pos.X -= velocity;
			}
            
			if(Controller0->DPadRight.EndedPress)
			{
				new_pos.X += velocity;
			}		
            
			// TODO: (Abdo) Collision detection
			// TODO: (Abdo) Check collisions in meters, not pixels
			u32 *tiles_ptr = *TileMap01;
            
            
            
			hmm_vec3 player_position_in_pixels = MetersToPixels(State->PlayerPosition);
			hmm_v3 player_bottom_left = player_position_in_pixels;
			hmm_v3 player_bottom_right = {player_position_in_pixels.X + TILE_DIMM, player_position_in_pixels.Y, 0.0f};
			if(IsPointEmpty(player_bottom_left, world.TileMaps[0].Tile_Data) &&
			   IsPointEmpty(player_bottom_right, world.TileMaps[0].Tile_Data))
            {
                // move accepted
                State->PlayerPosition =  PixelsToMeters(new_pos);
            }	
            else 
            {
                State->PlayerPosition = PixelsToMeters({80.0f, 80.0f, 0.0f});
            }	
#endif
		}
	}
    
	// Audio
	if(!SoundOutput->IsBufferFilled)
	{
		ThreadContext thread = {};
		GameGenerateAudio(&thread, SoundOutput);
		SoundOutput->IsBufferFilled = true;
	}
    
	// Collision loop
	// We need data about where stuff exists within a level
	for(u32 entity_index = 0; entity_index < 10; ++entity_index)
	{
		Entity entity = State->EntityList[entity_index];
		if(entity.IsExistant)
		{
			// Check stuff
		}
	}
    
	// Render 
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
	// Drawing a tile map
	// TODO: (Abdo) do pixel conversions on renderer side
	f32 scale = PIXELS_PER_METER;
	for(u32 tile_index_y = 0;
		tile_index_y < 9;
		++tile_index_y)
    {
        for(u32 tile_index_x = 0;
            tile_index_x < 16;
            ++tile_index_x)
        {
            if(world.TileMaps[0].Tile_Data[TILE_COUNT_X * tile_index_y + tile_index_x] == 1)
            {
                OpenGLPushTexturedQuad(&State->opengl_batch, {(f32)(tile_index_x * scale), (f32)(tile_index_y * scale), 0.0f}, {scale, scale}, State->texture_Mud);
            }
            else
            {
                OpenGLPushTexturedQuad(&State->opengl_batch, {(f32)(tile_index_x * scale), (f32)(tile_index_y * scale), 0.0f}, {scale, scale}, State->texture_Grass);
            }
        }
    }
    
	// Can passing the opengl_batch_state ever time be avoided?
    
	// Draw player
#if	0
	OpenGLPushTexturedQuad(&State->opengl_batch, State->EntityList[2].bottom_left_corner + State->PlayerPosition,
                           State->EntityList[2].dimensions, State->texture_Mud);
#else
	OpenGLPushFlatQuad(&State->opengl_batch, MetersToPixels(State->PlayerPosition),
                       {scale, scale}, {0.5f, 0.0f, 0.5f, 1.0f});
#endif	
    
	OpenGLFlush(&State->opengl_batch);
    
}