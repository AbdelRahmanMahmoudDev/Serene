#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_TextureCoordinate;
layout (location = 3) in float a_TextureID;

out vec4 v_Color;
out vec2 v_TextureCoordinate;
out float v_TextureID;

// uniform mat4 u_ModelViewProjection;
uniform mat4 u_ProjectionView;
uniform mat4 u_Transform;

void main()
{
    v_Color = a_Color;
    v_TextureCoordinate = a_TextureCoordinate;
    v_TextureID = a_TextureID;
    gl_Position = u_ProjectionView * u_Transform * vec4(a_Position, 1.0f);
}