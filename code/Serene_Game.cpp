#include "Serene_Game.h"
#include "Serene_Platform.h"
#include "Serene_Intrinsics.h"

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

        State->shader_program = OpenGLLoadShaders(Memory->DebugPlatformReadEntireFile, vertex_shader_path, fragment_shader_path, &thread);

		stbi_set_flip_vertically_on_load(1);

		char grass_path[MAX_PATH];
		ConstructAssetDirectory(grass_path, asset_path->AssetPath, "/assets/textures/Grass.png");
		State->Grass = {};
		State->Grass.Data = stbi_load(grass_path, &State->Grass.Width, &State->Grass.Height, &State->Grass.Channel_Count, 0);
		OpenGLUploadTexture(&State->Grass, State->texture_0);
		stbi_image_free(State->Grass.Data);		

		char mud_path[MAX_PATH];
		ConstructAssetDirectory(mud_path, asset_path->AssetPath, "/assets/Textures/Mud.png");
		State->Mud = {};
		State->Mud.Data = stbi_load(mud_path, &State->Mud.Width, &State->Mud.Height, &State->Mud.Channel_Count, 0);
		OpenGLUploadTexture(&State->Mud, State->texture_1);
		stbi_image_free(State->Mud.Data);

		// Setting up memory arenas
		InitializeArena(&State->WorldArena,
					 	Memory->PermanentStorageSize - sizeof(GameState),
						(u8*)Memory->PermanentStorage + sizeof(GameState));

		State->world = PushStruct(&State->WorldArena, World);
		World *world = State->world;
		world->Tiles = PushStruct(&State->WorldArena, TileMap);
		TileMap *tile_map = world->Tiles;
		
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


		// DON'T DELETE ME!!!
#if 0
		// TODO(Abdo): Move this to Opengl grid drawing routine
		// TODO(Abdo): Create a full transform in this instead of just a translation
		// 2 / number of columns
		// 2 / number of rows
		u32 index = 0;
		for(f32 row_index = -1.0f;
		    row_index < 1.0f;
			row_index+=0.25)
			{
				for(f32 column_index = -1.0f;
				    column_index < 1.0f;
					column_index+=0.25)
					{
						hmm_v2 current_translation;
						current_translation.X = column_index + 0.5f;
						current_translation.Y = row_index + 0.5f;

						State->Translations[index++] = current_translation;
					}
			}
#endif

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

	// TODO(Abdo): Move all this to initialization

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Mote(Abdo): This library has a right handed [-1,1] coordinate system
	// but the matrices are row major
	// So transforms are calculated SRT instead of TRS
	// final matrices are calculated MVP instead of PVM,.. etc
	hmm_mat4 projection = HMM_Orthographic(((f32)renderer_dimensions->ScreenWidth / 2.0f) * -1.0f, (f32)renderer_dimensions->ScreenWidth / 2.0f,
	                                       ((f32)renderer_dimensions->ScreenHeight / 2.0f) * -1.0f, (f32)renderer_dimensions->ScreenHeight / 2.0f,
										   -1.0f, 1.0f);

	hmm_mat4 view = HMM_Translate({0.0f, 0.0f, -1.0f});

	hmm_mat4 model = HMM_Translate({0.0f, 0.0f, 0.0f}) * HMM_Rotate(0.0f, {0.0f, 0.0f, 1.0f}) * HMM_Scale({1.0f, 1.0f, 1.0f});

	u32 vertex_array_object = 0;
	glGenVertexArrays(1, &vertex_array_object);
	glBindVertexArray(vertex_array_object);

#define MAX_QUAD_COUNT 1000
#define MAX_VERTEX_COUNT MAX_QUAD_COUNT * 4
#define MAX_INDEX_COUNT MAX_QUAD_COUNT * 6

	u32 vertex_buffer_object = 0;
	glGenBuffers(1, &vertex_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_COUNT * sizeof(OpenGL_Quad_Vertex), 0, GL_DYNAMIC_DRAW);

	OpenGL_Quad_Vertex Vertices[MAX_VERTEX_COUNT];
	OpenGL_Quad_Vertex *buffer = Vertices;

	u32 index_data[MAX_INDEX_COUNT];
	u32 offset = 0;
	for(int index = 0;
	    index < MAX_INDEX_COUNT;
		index += 6)
		{
			index_data[0 + index] = 0 + offset; 
			index_data[1 + index] = 1 + offset; 
			index_data[2 + index] = 2 + offset; 
			index_data[3 + index] = 2 + offset; 
			index_data[4 + index] = 3 + offset; 
			index_data[5 + index] = 0 + offset; 

			offset += 4;
		}

	u32 index_buffer_object = 0;
	glGenBuffers(1, &index_buffer_object);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_data), index_data, GL_DYNAMIC_DRAW);	


	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
	                      sizeof(OpenGL_Quad_Vertex),
						  (void *)offsetof(OpenGL_Quad_Vertex, Position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
	                      sizeof(OpenGL_Quad_Vertex),
						  (void *)offsetof(OpenGL_Quad_Vertex, Color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
	                      sizeof(OpenGL_Quad_Vertex),
						  (void *)offsetof(OpenGL_Quad_Vertex, TextureCoordinate));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE,
	                      sizeof(OpenGL_Quad_Vertex),
						  (void *)offsetof(OpenGL_Quad_Vertex, TextureID));						  					  
 		
	glBindTextureUnit(0, State->texture_0);
	glBindTextureUnit(1, State->texture_1);
	
	u32 index_count = 0;
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
	buffer = OpenGLCreateQuad(buffer, {100.0f, 0.0f, 0.0f}, 100.0f, {1.0f, 0.0f, 0.0f, 1.0f}, 0.0f);
	index_count += 6;
	buffer = OpenGLCreateQuad(buffer, {-100.0f, 0.0f, 0.0f}, 100.0f, {1.0f, 0.0f, 0.0f, 1.0f}, 1.0f);
	index_count += 6;
	buffer = OpenGLCreateQuad(buffer, {0.0f, 100.0f, 0.0f}, 100.0f, {1.0f, 0.0f, 0.0f, 1.0f}, 1.0f);
	index_count += 6;
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);

	glBindVertexArray(vertex_array_object);
	glUseProgram(State->shader_program);

	i32 texture_slots[2] = {0, 1};

	OpenGLSetMat4(State->shader_program, "u_Projection", projection);
	OpenGLSetMat4(State->shader_program, "u_View", view);
	OpenGLSetMat4(State->shader_program, "u_Model", model);
	OpenGLSetIntArray(State->shader_program, "u_TextureSlots", 2, texture_slots);

	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
}