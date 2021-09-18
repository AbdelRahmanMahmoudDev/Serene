#version 460 core

#define MAX_SAMPLERS 32

in vec4 v_Color;
in vec2 v_TextureCoordinate;
in float v_TextureID;

layout (location = 0) out vec4 o_Fragment;

uniform sampler2D u_TextureSlots[MAX_SAMPLERS];

void main()
{
    int texture_index = int(v_TextureID);

    // white textures get things drawn on top of them
    // 1.0f colors won't affect textures
    o_Fragment = texture(u_TextureSlots[texture_index], v_TextureCoordinate) * v_Color;
}