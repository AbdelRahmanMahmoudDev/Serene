#include <cmath>

#include "3rd_Party/glad/glad/glad.h"
#include "3rd_Party/glad/glad/glad.c"

#include "Serene_Game.h"
#include "Serene_Platform.h"
#include "Serene_Intrinsics.h"
#include "Serene_Math.h"
#include "Serene_String.h"
#include "Serene_Memory.cpp"

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

internal void
ConstructAssetDirectory(char *desired_path, char *asset_path, char *resource_path)
{
	strcpy(desired_path, asset_path);
	strcat(desired_path, resource_path);
}

// world stuff

#define PIXELS_PER_METER 100

global u32 tile_dim = PIXELS_PER_METER;


internal f32 
MetersToPixels(f32 value_in_meters)
{
	f32 Result = value_in_meters * PIXELS_PER_METER;
	return(Result);
}

internal v3 
MetersToPixels(v3 vec3_in_meters)
{
	v3 Result = {};
	Result.x = vec3_in_meters.x * PIXELS_PER_METER;
	Result.y = vec3_in_meters.y * PIXELS_PER_METER;
	Result.z = vec3_in_meters.z * PIXELS_PER_METER;
	return(Result);
}

internal f32
PixelsToMeters(f32 value_in_pixels)
{
	f32 Result = value_in_pixels / PIXELS_PER_METER;
	return(Result);
}

internal v3
PixelsToMeters(v3 vec3_in_pixels)
{
	v3 Result = {};
	Result.x = vec3_in_pixels.x / PIXELS_PER_METER;
	Result.y = vec3_in_pixels.y / PIXELS_PER_METER;
	Result.z = vec3_in_pixels.z / PIXELS_PER_METER;
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

// uniform mat4 uClipMatrix;
// uniform mat4 uModelMatrix;
// uniform mat4 uViewMatrix;

global char *player_shader_text = R"(
	##VERTEX
	#version 330 core
	layout (location = 0) in vec3 aPos;
	uniform mat4 uClipView;
	uniform mat4 uTransform;

	void main()
	{
		gl_Position = uClipView * uTransform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	}

	##FRAGMENT
	#version 330 core
    out vec4 FragColor;
    void main()
    {
		FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
)";

global char *static_shader_text = R"(
	##VERTEX
	#version 330 core
	layout (location = 0) in vec3 aPos;
	uniform mat4 uClipView;

	void main()
	{
		gl_Position = uClipView * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	}

	##FRAGMENT
	#version 330 core
    out vec4 FragColor;
    void main()
    {
		FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
)";

struct OpenGLShader 
{
	char Vertex[MAX_STRING_SIZE];
	char Fragment[MAX_STRING_SIZE];
};

global OpenGLShader player_shader;
global OpenGLShader static_shader;
internal void
OpenGLParseShader(char *shader_source, OpenGLShader *ogl_shader)
{
	u32 vertex_size = 0;
	char *shader_explorer = shader_source;
	char *main_token = "main";
	b32 is_vertex_found = false;
	b32 is_fragment_found = false;
	MemoryIndex vertex_offset;
	MemoryIndex fragment_offset;
	u32 hash_counter = 0;
	u32 shader_count = 0;
	
	while(*shader_explorer)
	{
		if(*shader_explorer == '#')
		{
			++hash_counter;
		}
		if(hash_counter == 3)
		{
			hash_counter = 0;
			++shader_count;

			switch(shader_count)
			{
				case 1:
				{
					char *vertex_base = shader_explorer;
					char *vertex_shader_explorer = vertex_base;
					while(*vertex_shader_explorer)
					{
						if(*vertex_shader_explorer == *main_token)
						{
							char *token_start = vertex_shader_explorer;
							char *checkable_start = main_token;
							u32 match_count = 0;
							u32 char_index;
							for(char_index = 0; char_index < StringLength(main_token) - 1; ++char_index)
							{
								if(*++token_start == *++checkable_start)
								{
									++match_count;
								}
							}
							if(match_count == StringLength(main_token) - 1)
							{
								// found vertex main
								char *vertex_shader_main_explorer = vertex_shader_explorer;
								while(*vertex_shader_main_explorer)
								{
									if(*vertex_shader_main_explorer == '}')
									{
										//vertex_shader_end = vertex_shader_main_explorer;
										// calculate the offset
										//*++vertex_shader_main_explorer = '\0';
										vertex_offset = vertex_shader_main_explorer - vertex_base;
										StringCopy(ogl_shader->Vertex, vertex_base, vertex_offset + 2);
										is_vertex_found = true;
										break;
									}
									++vertex_shader_main_explorer;
								}
							}
						}
						if(is_vertex_found)
						{
							break;
						}
						++vertex_shader_explorer;
					}
				} break;

				case 2:
				{
					char *fragment_base = shader_explorer;
					char *fragment_shader_explorer = fragment_base;
					while(*fragment_shader_explorer)
					{
						if(*fragment_shader_explorer == *main_token)
						{
							char *token_start = fragment_shader_explorer;
							char *checkable_start = main_token;
							u32 match_count = 0;
							u32 char_index;
							for(char_index = 0; char_index < StringLength(main_token) - 1; ++char_index)
							{
								if(*++token_start == *++checkable_start)
								{
									++match_count;
								}
							}
							if(match_count == StringLength(main_token) - 1)
							{
								// found vertex main
								char *fragment_shader_main_explorer = fragment_shader_explorer;
								while(*fragment_shader_main_explorer)
								{
									if(*fragment_shader_main_explorer == '}')
									{
										//vertex_shader_end = vertex_shader_main_explorer;
										// calculate the offset
										//*++fragment_shader_main_explorer = '\0';
										fragment_offset = fragment_shader_main_explorer - fragment_base;
										StringCopy(ogl_shader->Fragment, fragment_base, fragment_offset + 2);
										is_fragment_found = true;
										break;
									}
									++fragment_shader_main_explorer;
								}
							}
						}
						if(is_fragment_found)
						{
							break;
						}
						++fragment_shader_explorer;
					}					
				} break;
			} 
		} 
		++shader_explorer;
	}
}

internal u32
OpenGLCompileShader(OpenGLShader *shader)
{
	u32 shader_id = 0;

#if SERENE_DEBUG
	i32 success;
	char infoLog[512];
#endif

	// grab pointers to every shader
	char *vertex_ptr = shader->Vertex;
	char *fragment_ptr = shader->Fragment;

	// Opengl compiles vertex
	i32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_ptr, NULL);
    glCompileShader(vertex_shader);

	// debug only step
	// check shader for errors
#if SERENE_DEBUG
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
		OutputDebugStringA("SHADER::COMPILE::VERTEX::ERROR\n");
		OutputDebugStringA(infoLog);
    }
#endif

	// Opengl compiles fragment
	i32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_ptr, NULL);
    glCompileShader(fragment_shader);

	// debug only step
	// check shader for errors
#if SERENE_DEBUG
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
		OutputDebugStringA("SHADER::COMPILE::FRAGMENT::ERROR\n");
		OutputDebugStringA(infoLog);
    }
#endif

	// link shaders
	shader_id = glCreateProgram();
	glAttachShader(shader_id, vertex_shader);
    glAttachShader(shader_id, fragment_shader);
    glLinkProgram(shader_id);

	// debug only step
	// check for link errors
#if SERENE_DEBUG
	glGetProgramiv(shader_id, GL_LINK_STATUS, &success);
	if (!success) 
	{
		glGetProgramInfoLog(shader_id, 512, NULL, infoLog);
		OutputDebugStringA("SHADER::LINK::ERROR\n");
		OutputDebugStringA(infoLog);
	}
#endif	

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return(shader_id);
}
struct QuadVertex
{
	v3 Position;
	mat4 Transform;
};

global u32 current_index_count = 0;
global QuadVertex *vertex_buffer_base;
global QuadVertex *vertex_buffer_current;

global mat4 view_to_clip = InitMatrix();
global mat4 model_to_view = InitMatrix();
global mat4 translation = InitMatrix();
global mat4 scale = InitMatrix();
global i32 uniform_location;

global QuadVertex player = {};
global f32 player_dim = 50.0f;
global v2 player_pixel_pos = {};
global v2i player_tile_pos = {};

global u32 VBO, VAO, EBO;
global u32 shaderProgram;
global u32 player_shader_id;

global MemoryArena RendererArena;

internal void
OpenGLAddQuad(v2 quad_dim, QuadVertex **vertex_buffer_ptr)
{
    // TODO(ABDO): Handle case of full buffer
    // top right
    QuadVertex *delta_ptr = *vertex_buffer_ptr;
    delta_ptr->Position = {quad_dim.x, quad_dim.y, 0.0f};
    ++delta_ptr;
    // bottom right
    delta_ptr->Position = {quad_dim.x, 0.0f, 0.0f};
    ++delta_ptr;
    // bottom left
    delta_ptr->Position = {0.0f, 0.0f, 0.0f};
    ++delta_ptr;
    //top left
    delta_ptr->Position = {0.0f, quad_dim.y, 0.0f};
    ++delta_ptr;
    
    MemoryIndex data_offset = delta_ptr - *vertex_buffer_ptr;
    *vertex_buffer_ptr += data_offset;
    
    // increment indices to be drawn
    current_index_count += 6;
}

extern "C" GAME_UPDATE(GameUpdate)
{
	// Memory
	Assert(sizeof(GameState) <= Memory->PermanentStorageSize);
	GameState* State = (GameState*)Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		MemoryIndex quad_count = 10000;
		MemoryIndex vertex_count = quad_count * 4;
		MemoryIndex index_count = quad_count * 6;
		MemoryIndex renderer_arena_size = vertex_count + index_count;
		InitializeArena(&RendererArena, renderer_arena_size , (u8*)Memory->TransientStorage);
		vertex_buffer_base = (QuadVertex *)RendererArena.Base;
		vertex_buffer_current = vertex_buffer_base;
		u32 *index_buffer_base = (u32*)vertex_buffer_base + vertex_count;
		
        OpenGLAddQuad({player_dim, player_dim}, &vertex_buffer_current);
		OpenGLParseShader(player_shader_text, &player_shader);
        
		u32 offset = 0;
		for(u32 index = 0;
	    	index < index_count;
			index += 6)
        {
            index_buffer_base[0 + index] = 0 + offset;
            index_buffer_base[1 + index] = 1 + offset;
            index_buffer_base[2 + index] = 2 + offset;
            index_buffer_base[3 + index] = 2 + offset;
            index_buffer_base[4 + index] = 3 + offset;
            index_buffer_base[5 + index] = 0 + offset;
            
            offset += 4;
        }
        
		// We need to load all driver functions into the game dll
		// IMPORTANT(Abdo): This only applies to the OpenGL Renderer
		// TODO(Abdo): Do this somewhere more sensible!!
		i32 glad_status = gladLoadGL();
		glViewport(0, 0, renderer_dimensions->ScreenWidth, renderer_dimensions->ScreenHeight);
    	glGenVertexArrays(1, &VAO);
    	glGenBuffers(1, &VBO);
    	glGenBuffers(1, &EBO);
    	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    	glBindVertexArray(VAO);
        
    	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * vertex_count, vertex_buffer_base, GL_DYNAMIC_DRAW);
        
		// These 2 lines will be done with each new addition to the vertex buffer
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_current - vertex_buffer_base, vertex_buffer_base);
        
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * index_count, index_buffer_base, GL_DYNAMIC_DRAW);
    	glEnableVertexAttribArray(0);
    	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)OFFSETOF(QuadVertex, Position));
        
		player_shader_id = OpenGLCompileShader(&player_shader);		
		glUseProgram(player_shader_id);

		view_to_clip = Perspective((f32)renderer_dimensions->ScreenWidth, (f32)renderer_dimensions->ScreenHeight, 0.1f, 100.0f);
		model_to_view = Translate({0.0f, 0.0f, -0.1f});
		scale = Scale({1.0f, 1.0f, 1.0f});
        
		mat4 clip_view = model_to_view * view_to_clip;
		uniform_location = glGetUniformLocation(player_shader_id, "uClipView");
    	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (clip_view.m));

		Memory->IsInitialized = true;
	}
    
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
			// Digital processing
			// Player movement code
			local_persist f32 speed = 1.0f;
			if(Controller0->DPadUp.EndedPress)
			{
				player_pixel_pos.y += speed * Input->TargetSecondsPerFrame;
				translation = Translate({player_pixel_pos.x, player_pixel_pos.y, 0.0f});
			}
            
			if(Controller0->DPadDown.EndedPress)
			{
				player_pixel_pos.y -= speed * Input->TargetSecondsPerFrame;
				translation = Translate({player_pixel_pos.x, player_pixel_pos.y, 0.0f});
			}
            
			if(Controller0->DPadLeft.EndedPress)
			{
				player_pixel_pos.x -= speed * Input->TargetSecondsPerFrame;
				translation = Translate({player_pixel_pos.x, player_pixel_pos.y, 0.0f});
			}
            
			if(Controller0->DPadRight.EndedPress)
			{
				player_pixel_pos.x += speed * Input->TargetSecondsPerFrame;
				translation = Translate({player_pixel_pos.x, player_pixel_pos.y, 0.0f});
			}		
            
		}
	}
    
    // Render 
	glViewport(0, 0, renderer_dimensions->ScreenWidth, renderer_dimensions->ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
	// TODO(Abdo): GUI options for wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	mat4 transform = scale * translation;
	uniform_location = glGetUniformLocation(player_shader_id, "uTransform");
    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (transform.m));
    glDrawElements(GL_TRIANGLES, current_index_count, GL_UNSIGNED_INT, 0);
	
#if SERENE_DEBUG	
	player_tile_pos.x = (u32)player_pixel_pos.x / tile_dim;
	player_tile_pos.y = (u32)player_pixel_pos.y / tile_dim;
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "Tile(%d, %d)\n", player_tile_pos.x, player_tile_pos.y);
	//OutputDebugStringA(buffer);
#endif    
	// Audio
	if(!SoundOutput->IsBufferFilled)
	{
		ThreadContext thread = {};
		GameGenerateAudio(&thread, SoundOutput);
		SoundOutput->IsBufferFilled = true;
	}
}