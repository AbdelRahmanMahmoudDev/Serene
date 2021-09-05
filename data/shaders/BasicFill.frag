#version 460 core

in vec3 v_Color;

out vec4 o_Fragment;

void main()
{
    o_Fragment = vec4(v_Color.x, v_Color.y, v_Color.z, 1.0f);
}