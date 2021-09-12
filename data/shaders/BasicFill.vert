#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Color;

out vec3 v_Color;

// uniform mat4 u_ModelViewProjection;
uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    v_Color = a_Color;
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position.x, a_Position.y, 0.0f, 1.0f);
}