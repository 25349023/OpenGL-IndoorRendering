#version 430 core

layout (location = 0) uniform sampler2D tex;

layout (location = 0) out vec4 fragColor;

in VS_OUT
{
    vec2 texcoord;
} fs_in;

void main(void) {
    fragColor = texture(tex, fs_in.texcoord);
//    fragColor = vec4(1, 1, 0, 1);
}
