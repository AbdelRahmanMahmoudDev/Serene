#include "Serene_OpenGL.h"

#include "3rd_Party/Handmade-Math/HandmadeMath.h"

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

internal void
OpenGLUploadTexture(PNGAsset *image, u32 &texture_id)
{
    if(image->Data)
    {
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    

		u32 gl_format = image->Channel_Count == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->Width, image->Height, 0, gl_format, GL_UNSIGNED_BYTE, image->Data);            
    }
    else
    {
        // image loaded incorrectly
        // TODO(Abdo): Logging
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
//                                    Uniform uploads
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
OpenGLSetIntArray(u32 shader_handle, char *uniform_name, i32 count , i32 *value)
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

internal OpenGL_Quad_Vertex *
OpenGLCreateQuad(OpenGL_Quad_Vertex *data_pointer, hmm_v3 bottom_left_corner, f32 size, hmm_v4 color, f32 texture_id)
{
    data_pointer->Position = {bottom_left_corner.X, bottom_left_corner.Y, 0.0f};
    data_pointer->Color = color;
    data_pointer->TextureCoordinate = {0.0f, 0.0f};
    data_pointer->TextureID = texture_id;
    data_pointer++;

    data_pointer->Position = {bottom_left_corner.X + size, bottom_left_corner.Y, 0.0f};
    data_pointer->Color = color;
    data_pointer->TextureCoordinate = {1.0f, 0.0f};
    data_pointer->TextureID = texture_id;
    data_pointer++;

    data_pointer->Position = {bottom_left_corner.X + size, bottom_left_corner.Y + size, 0.0f};
    data_pointer->Color = color;
    data_pointer->TextureCoordinate = {1.0f, 1.0f};
    data_pointer->TextureID = texture_id;
    data_pointer++;

    data_pointer->Position = {bottom_left_corner.X, bottom_left_corner.Y + size, 0.0f};
    data_pointer->Color = color;
    data_pointer->TextureCoordinate = {0.0f, 1.0f};
    data_pointer->TextureID = texture_id;
    data_pointer++;

    return data_pointer;
}