#include "Serene_OpenGL.h"

#include "3rd_Party/Handmade-Math/HandmadeMath.h"

/*
TODO(Abdo):
    -Cache uniforms
    -Is there a way to give the batch shader a custom number of samplers based on driver query?
*/

// Load shader files, parse them and output a shader program
internal u32 
OpenGLLoadShaders(debug_platform_read_entire_file *pRead_File, char *vertex_path, char *fragment_path, ThreadContext *thread)
{
    // Read our shaders into the appropriate buffers
    u32 Result = 0;

    DebugPlatformReadFileResult vertex_code = pRead_File(thread, vertex_path);
    DebugPlatformReadFileResult fragment_code = pRead_File(thread, fragment_path);

    // Create an empty vertex shader handle
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Send the vertex shader source code to GL
    // Note that std::string's .c_str is NULL character terminated.
    char *vertex_text = (char *)vertex_code.Content;
    glShaderSource(vertexShader, 1, &vertex_text, 0);

    // Compile the vertex shader
    glCompileShader(vertexShader);

    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
    	GLint maxLength = 2048;
    	//glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

    	// The maxLength includes the NULL character
        char infoLog[2048];
    	glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
    
    	// We don't need the shader anymore.
    	glDeleteShader(vertexShader);

    	// Use the infoLog as you see fit.
        sprintf_s(infoLog, sizeof(infoLog), "[VERTEX SHADER ERROR] %s\n", &infoLog);
    	OutputDebugStringA(infoLog);
    	// In this simple program, we'll just leave
    	return 0;
    }

    // Create an empty fragment shader handle
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Send the fragment shader source code to GL
    // Note that std::string's .c_str is NULL character terminated.
    char *fragment_text = (char *)fragment_code.Content;
    glShaderSource(fragmentShader, 1, &fragment_text, 0);

    // Compile the fragment shader
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
    	GLint maxLength = 2048;

    	// The maxLength includes the NULL character
        char infoLog[2048];
    	glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
    
    	// We don't need the shader anymore.
    	glDeleteShader(fragmentShader);
    	// Either of them. Don't leak shaders.
    	glDeleteShader(vertexShader);

    	// Use the infoLog as you see fit.
        sprintf_s(infoLog, sizeof(infoLog), "[Fragment SHADER ERROR] %s\n", &infoLog);
    	OutputDebugStringA(infoLog);
    	// In this simple program, we'll just leave
    	return 0;
    }

    // Vertex and fragment shaders are successfully compiled.
    // Now time to link them together into a program.
    // Get a program object.
    GLuint program = glCreateProgram();

    // Attach our shaders to our program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // Link our program
    glLinkProgram(program);
    Result = program;
    // Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE)
    {
    	GLint maxLength = 2048;

    	// The maxLength includes the NULL character
        char infoLog[2048];
    	glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
    
    	// We don't need the program anymore.
    	glDeleteProgram(program);
    	// Don't leak shaders either.
    	glDeleteShader(vertexShader);
    	glDeleteShader(fragmentShader);

    	// Use the infoLog as you see fit.
    	sprintf_s(infoLog, sizeof(infoLog), "[SHADER LINKING ERROR] %s\n", &infoLog);
    	OutputDebugStringA(infoLog);
        // In this simple program, we'll just leave
    	return 0;
    }

    // Always detach shaders after a successful link.
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    return Result;
}

internal u32
OpenGLCreateTexture(PNGAsset *image)
{
    u32 texture_id = 0;
    if(image->Data)
    {
        u32 internal_format = image->Channel_Count == 4 ? GL_RGBA8 : GL_RGB8;
		u32 data_format = image->Channel_Count == 4 ? GL_RGBA : GL_RGB;
        glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
        glTextureStorage2D(texture_id, 1, internal_format, image->Width, image->Height);
        
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    

		glTextureSubImage2D(texture_id, 0, 0, 0, image->Width, image->Height, data_format, GL_UNSIGNED_BYTE, image->Data);
    }
    else
    {
        // image loaded incorrectly
        // TODO(Abdo): Logging
        texture_id = 0;
    }

    return texture_id;
}

////////////////////////////////////////////////////////////////////////////////////////////
//                                    Uniform uploads
// @param-> u32 shader_handle: OpenGL shader object 
// @param-> char *uniform_name: string containing name of uniform
////////////////////////////////////////////////////////////////////////////////////////////
internal void
OpenGLSetFloat(u32 shader_handle, char *uniform_name, f32 value)
{
    i32 uniform_location = glGetUniformLocation(shader_handle, uniform_name);
    glUniform1f(uniform_location, value);
}

internal void
OpenGLSetVec2(u32 shader_handle, char *uniform_name, hmm_v2 value) 
{
    i32 uniform_location = glGetUniformLocation(shader_handle, uniform_name);
    glUniform2f(uniform_location, value.X, value.Y);
}

internal void
OpenGLSetInt(u32 shader_handle, char *uniform_name, i32 value)
{
    i32 uniform_location = glGetUniformLocation(shader_handle, uniform_name);
    glUniform1i(uniform_location, value);
}

internal void
OpenGLSetu32Array(u32 shader_handle, char *uniform_name, i32 count , i32 *value)
{
    i32 uniform_location = glGetUniformLocation(shader_handle, uniform_name);
   //glUniform1i(uniform_location, value);
   glUniform1iv(uniform_location, count, value);
}

internal void
OpenGLSetMat4(u32 shader_handle, char *uniform_name, hmm_m4 matrix)
{
    i32 unifrom_location = glGetUniformLocation(shader_handle, uniform_name);
    glUniformMatrix4fv(unifrom_location, 1, GL_FALSE, (f32 *)matrix.Elements);
}
////////////////////////////////////////////////////////////////////////////////////////////
//                                    Uniform uploads
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
//                                    Batch renderer routines
////////////////////////////////////////////////////////////////////////////////////////////
internal void
OpenGLInitRenderer(GameRendererDimensions *renderer_dimensions, OpenGL_Batch_State *batch_state, MemoryArena *renderer_arena, u32 max_quad_count)
{
    batch_state->max_quad_count = max_quad_count;
    batch_state->max_vertex_count = max_quad_count * 4;
    batch_state->max_index_count = max_quad_count * 6;
    batch_state->current_index_count = 0;

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &batch_state->max_texture_slot_count);

    batch_state->vertex_data_base = PushArray(renderer_arena, batch_state->max_vertex_count, OpenGL_Quad_Vertex);
    batch_state->vertex_data_ptr = batch_state->vertex_data_base;

    u32 *index_data_base = PushArray(renderer_arena, batch_state->max_index_count, u32);
    u32 *index_step_ptr = index_data_base;

	u32 offset = 0;
	for(u32 index = 0;
	    index < batch_state->max_index_count;
		index += 6)
	{
        index_step_ptr[0+index] = 0 + offset;
        index_step_ptr[1+index] = 1 + offset;
        index_step_ptr[2+index] = 2 + offset;
        index_step_ptr[3+index] = 2 + offset;
        index_step_ptr[4+index] = 3 + offset;
        index_step_ptr[5+index] = 0 + offset;

		offset += 4;
	}

    // 1x1 white texture for single shader batch rendering
    glGenTextures(1, &batch_state->white_texture_handle);
    glBindTexture(GL_TEXTURE_2D, batch_state->white_texture_handle);
    
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    

    u32 whiteness = 0xFFFFFFFF;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whiteness); 

    batch_state->texture_slot_base = PushArray(renderer_arena, batch_state->max_texture_slot_count * sizeof(u32), u32);
    u32 *texture_slot_ptr = batch_state->texture_slot_base;

    texture_slot_ptr[0] = batch_state->white_texture_handle;
    for(u32 index = 1;
        index < (u32)batch_state->max_texture_slot_count;
        index++)
    {
        texture_slot_ptr[index] = 0;
    }
    batch_state->texture_slot_index = 1;

    batch_state->texture_sampler_base = PushArray(renderer_arena, batch_state->max_texture_slot_count * sizeof(i32), i32);
    i32 *texture_sampler_ptr = batch_state->texture_sampler_base;
    for(i32 index = 0;
        index < batch_state->max_texture_slot_count;
        index++)
    {
        texture_sampler_ptr[index] = index;
    }


#if 1
    batch_state->projection = HMM_Orthographic(0.0f, (f32)renderer_dimensions->ScreenWidth,
	                                           0.0f, (f32)renderer_dimensions->ScreenHeight,
										       -1.0f, 1.0f);

	batch_state->view = HMM_Translate({0.0f, 0.0f, -1.0f});	

    batch_state->camera_transform = batch_state->projection * batch_state->view;
#else 
    batch_state->projection = HMM_Perspective(45.0f,
                                              (f32)renderer_dimensions->ScreenWidth / (f32)renderer_dimensions->ScreenHeight,
                                              0.1f, 100.0f);

    batch_state->view = HMM_Translate({0.0f, 0.0f, 0.1f});
#endif

    glUseProgram(batch_state->shader_program);
    OpenGLSetMat4(batch_state->shader_program, "u_ProjectionView", batch_state->camera_transform);
    OpenGLSetu32Array(batch_state->shader_program, "u_TextureSlots", batch_state->max_texture_slot_count, batch_state->texture_sampler_base);

	glGenVertexArrays(1, &batch_state->vertex_array_object);
	glBindVertexArray(batch_state->vertex_array_object);

    glGenBuffers(1, &batch_state->vertex_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, batch_state->vertex_buffer_object);
	glBufferData(GL_ARRAY_BUFFER, batch_state->max_vertex_count * sizeof(OpenGL_Quad_Vertex), 0, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &batch_state->index_buffer_object);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch_state->index_buffer_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, batch_state->max_index_count * sizeof(u32), index_data_base, GL_DYNAMIC_DRAW);

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


}

// Creates vertex data for a quad (4 vertices)
// advances vertex data pointer
// uploads vertices and issues draw call ONLY if max index count reached
internal void
OpenGLPushFlatQuad(OpenGL_Batch_State *batch_state, hmm_v3 bottom_left_corner, hmm_v3 dimensions, hmm_v4 color)
{
    if(batch_state->current_index_count >= batch_state->max_index_count)
    {
        // Upload batch
        u32 data_to_be_drawn_size = (u32)((u8 *)batch_state->vertex_data_ptr - (u8 *)batch_state->vertex_data_base);

        glBindBuffer(GL_ARRAY_BUFFER, batch_state->vertex_buffer_object);
        glBufferSubData(GL_ARRAY_BUFFER, 0, data_to_be_drawn_size, batch_state->vertex_data_base);

        glBindVertexArray(batch_state->vertex_array_object);
	    glUseProgram(batch_state->shader_program);

        // TODO(Abdo): each quad should have it's own transform
	    batch_state->transform = HMM_Translate({0.0f, 0.0f, 0.0f}) * HMM_Rotate(0.0f, {0.0f, 0.0f, 1.0f}) * HMM_Scale({1.0f, 1.0f, 1.0f});

	    OpenGLSetMat4(batch_state->shader_program, "u_Transform", batch_state->transform);

	    glDrawElements(GL_TRIANGLES, batch_state->current_index_count, GL_UNSIGNED_INT, 0);

        // Reset batch
        batch_state->vertex_data_ptr = batch_state->vertex_data_base;
        batch_state->current_index_count = 0;
    }

    // white 1x1 texture
    f32 texture_id = 0.0f;

    batch_state->vertex_data_ptr->Position = {bottom_left_corner.X, bottom_left_corner.Y, 0.0f};
    batch_state->vertex_data_ptr->Color = color;
    batch_state->vertex_data_ptr->TextureCoordinate = {0.0f, 0.0f};
    batch_state->vertex_data_ptr->TextureID = texture_id;
    batch_state->vertex_data_ptr++;

    batch_state->vertex_data_ptr->Position = {bottom_left_corner.X + dimensions.X, bottom_left_corner.Y, 0.0f};
    batch_state->vertex_data_ptr->Color = color;
    batch_state->vertex_data_ptr->TextureCoordinate = {1.0f, 0.0f};
    batch_state->vertex_data_ptr->TextureID = texture_id;
    batch_state->vertex_data_ptr++;

    batch_state->vertex_data_ptr->Position = {bottom_left_corner.X + dimensions.X, bottom_left_corner.Y + dimensions.Y, 0.0f};
    batch_state->vertex_data_ptr->Color = color;
    batch_state->vertex_data_ptr->TextureCoordinate = {1.0f, 1.0f};
    batch_state->vertex_data_ptr->TextureID = texture_id;
    batch_state->vertex_data_ptr++;

    batch_state->vertex_data_ptr->Position = {bottom_left_corner.X, bottom_left_corner.Y + dimensions.Y, 0.0f};
    batch_state->vertex_data_ptr->Color = color;
    batch_state->vertex_data_ptr->TextureCoordinate = {0.0f, 1.0f};
    batch_state->vertex_data_ptr->TextureID = texture_id;
    batch_state->vertex_data_ptr++;

    batch_state->current_index_count += 6;
}

// Creates vertex data for a quad (4 vertices)
// advances vertex data pointer
// uploads texture to OpenGL
// uploads vertices and issues draw call ONLY if max index count reached OR max texture slot reached
internal void
OpenGLPushTexturedQuad(OpenGL_Batch_State *batch_state, hmm_v3 bottom_left_corner, hmm_v3 dimensions, u32 texture_id)
{
    if(batch_state->current_index_count >= batch_state->max_index_count ||
       batch_state->texture_slot_index > (u32)(batch_state->max_texture_slot_count - 1))
    {
        // Upload batch
        u32 data_to_be_drawn_size = (u32)((u8 *)batch_state->vertex_data_ptr - (u8 *)batch_state->vertex_data_base);

        glBindBuffer(GL_ARRAY_BUFFER, batch_state->vertex_buffer_object);
        glBufferSubData(GL_ARRAY_BUFFER, 0, data_to_be_drawn_size, batch_state->vertex_data_base);

        glBindVertexArray(batch_state->vertex_array_object);
	    glUseProgram(batch_state->shader_program);

        // TODO(Abdo): each quad should have it's own transform
	    batch_state->transform = HMM_Translate({0.0f, 0.0f, 0.0f}) * HMM_Rotate(0.0f, {0.0f, 0.0f, 1.0f}) * HMM_Scale({1.0f, 1.0f, 1.0f});

	    OpenGLSetMat4(batch_state->shader_program, "u_Transform", batch_state->transform);

	    glDrawElements(GL_TRIANGLES, batch_state->current_index_count, GL_UNSIGNED_INT, 0);

        // Reset batch
        batch_state->vertex_data_ptr = batch_state->vertex_data_base;
        batch_state->current_index_count = 0;
    }

    f32 test_texture_id = 0.0f;
    for(u32 index = 0;
        index < batch_state->texture_slot_index;
        index++)
    {
        if(batch_state->texture_slot_base[index] == texture_id)
        {
            test_texture_id = (f32)batch_state->texture_slot_base[index];
        }
    }

    if(test_texture_id == 0.0f)
    {
        batch_state->texture_slot_base[batch_state->texture_slot_index] = texture_id;
        batch_state->texture_slot_index++;
        test_texture_id = (f32)texture_id;
    }

    hmm_v4 color = {1.0f, 1.0f, 1.0f, 1.0f};

    batch_state->vertex_data_ptr->Position = {bottom_left_corner.X, bottom_left_corner.Y, 0.0f};
    batch_state->vertex_data_ptr->Color = color;
    batch_state->vertex_data_ptr->TextureCoordinate = {0.0f, 0.0f};
    batch_state->vertex_data_ptr->TextureID = (f32)test_texture_id;
    batch_state->vertex_data_ptr++;

    batch_state->vertex_data_ptr->Position = {bottom_left_corner.X + dimensions.X, bottom_left_corner.Y, 0.0f};
    batch_state->vertex_data_ptr->Color = color;
    batch_state->vertex_data_ptr->TextureCoordinate = {1.0f, 0.0f};
    batch_state->vertex_data_ptr->TextureID = (f32)test_texture_id;
    batch_state->vertex_data_ptr++;

    batch_state->vertex_data_ptr->Position = {bottom_left_corner.X + dimensions.X, bottom_left_corner.Y + dimensions.Y, 0.0f};
    batch_state->vertex_data_ptr->Color = color;
    batch_state->vertex_data_ptr->TextureCoordinate = {1.0f, 1.0f};
    batch_state->vertex_data_ptr->TextureID = (f32)test_texture_id;
    batch_state->vertex_data_ptr++;

    batch_state->vertex_data_ptr->Position = {bottom_left_corner.X, bottom_left_corner.Y + dimensions.Y, 0.0f};
    batch_state->vertex_data_ptr->Color = color;
    batch_state->vertex_data_ptr->TextureCoordinate = {0.0f, 1.0f};
    batch_state->vertex_data_ptr->TextureID = (f32)test_texture_id;
    batch_state->vertex_data_ptr++;

    batch_state->current_index_count += 6;
}

// Give user the ability end a batch whenever they like
internal void OpenGLFlush(OpenGL_Batch_State *batch_state)
{
    // Upload batch
    u32 data_to_be_drawn_size = (u32)((u8 *)batch_state->vertex_data_ptr - (u8 *)batch_state->vertex_data_base);

    glBindVertexArray(batch_state->vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, batch_state->vertex_buffer_object);
    glBufferSubData(GL_ARRAY_BUFFER, 0, data_to_be_drawn_size, batch_state->vertex_data_base);



    for(u32 index = 0;
        index < batch_state->texture_slot_index;
        index++)
    {
        glBindTextureUnit(index, batch_state->texture_slot_base[index]);
    }

    // TODO(Abdo): each quad should have it's own transform
	batch_state->transform = HMM_Translate({0.0f, 0.0f, 0.0f}) * HMM_Rotate(0.0f, {0.0f, 0.0f, 1.0f}) * HMM_Scale({1.0f, 1.0f, 1.0f});

	OpenGLSetMat4(batch_state->shader_program, "u_Transform", batch_state->transform);
    OpenGLSetu32Array(batch_state->shader_program, "u_TextureSlots", batch_state->max_texture_slot_count, batch_state->texture_sampler_base);

	glUseProgram(batch_state->shader_program);
	glDrawElements(GL_TRIANGLES, batch_state->current_index_count, GL_UNSIGNED_INT, 0);

    //reset batch
    batch_state->vertex_data_ptr = batch_state->vertex_data_base;
    batch_state->current_index_count = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////
//                                    Batch renderer routines
////////////////////////////////////////////////////////////////////////////////////////////