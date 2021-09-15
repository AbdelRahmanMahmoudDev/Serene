#version 460 core

in vec4 v_Color;
in vec2 v_TextureCoordinate;
in float v_TextureID;

layout (location = 0) out vec4 o_Fragment;

uniform sampler2D u_TextureSlots[2];

void main()
{
    int texture_index = int(v_TextureID);
    o_Fragment = texture(u_TextureSlots[texture_index], v_TextureCoordinate);
}