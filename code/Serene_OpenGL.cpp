#include "Serene_OpenGL.h"

#include "Serene_Platform.h"

// This normalizes a position based on a screen width and height
// to the [-1, 1] mapping OpenGL wants
// ignore z axis for now
internal v3 
OpenGLNormalizePosition(v3 value, u32 screen_width, u32 screen_height)
{
    v3 Result = {};

    i32 signed_screen_width = (i32)screen_width;
    i32 signed_screen_height = (i32)screen_height;

    if(value.x < 0.0f)
    {
        value.x = 0.0f;
    }

    if(value.x > (f32)screen_width)
    {
        value.x = (f32)screen_width;
    }

    if(value.y < 0.0f)
    {
        value.y = 0.0f;
    }

    if(value.y > (f32)screen_height)
    {
        value.y = (f32)screen_height;
    }

    if(value.x == 0.0f)
    {
        Result.x = -1.0f;
    }
    else
    {
        if(value.x < (f32)(screen_width / 2))
        {
            Result.x = value.x / (f32)(-1 * signed_screen_width);
        }
        else if(value.x == (f32)(screen_width / 2))
        {
            Result.x = 0.0f;
        }
        else
        {
            Result.x = value.x / (f32)screen_width;
        }
    }

    if(value.y == 0.0f)
    {
        Result.y = -1.0f;
    }
    else
    {
        if(value.y < (f32)(screen_height / 2))
        {
            Result.y = value.y / (f32)(-1 * signed_screen_height);
        }
        else if(value.y == (f32)(screen_height / 2))
        {
            Result.y = 0.0f;
        }
        else
        {
            Result.y = value.y / (f32)screen_height;
        }
    }
    return(Result);
}

internal u32 
OpenGLLoadShaders(debug_platform_read_entire_file *pRead_File, char *vertex_path, char *fragment_path, ThreadContext *thread)
{
    // Read our shaders into the appropriate buffers
    u32 Result = 0;

    #if 1
    DebugPlatformReadFileResult vertex_code = pRead_File(thread, vertex_path);
    DebugPlatformReadFileResult fragment_code = pRead_File(thread, fragment_path);
    #else
    char *vertexShaderSource = "#version 460 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    char *fragmentShaderSource = "#version 460 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n\0";

    char *vertex_source = vertexShaderSource;
    char *fragment_source = fragmentShaderSource;
    #endif
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
SetFloat(u32 shader_handle, char *uniform_name, f32 value)
{
    i32 uniform_location = glGetUniformLocation(shader_handle, uniform_name);
    glUniform1f(uniform_location, value);
}

internal void
SetInt(u32 shader_handle, char *uniform_name, i32 value)
{
    i32 uniform_location = glGetUniformLocation(shader_handle, uniform_name);
    glUniform1i(uniform_location, value);
}