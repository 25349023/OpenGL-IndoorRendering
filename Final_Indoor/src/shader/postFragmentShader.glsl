#version 430 core

layout (location = 0) uniform sampler2D tex;

layout (location = 0) out vec4 fragColor;

in VS_OUT
{
    vec2 texcoord;
} fs_in;

vec3 check_normalize(vec3 v) {
    if (v == vec3(0.0)) {
        return v;
    }
    return normalize(v);
}

void main(void) {
    fragColor = texture(tex, fs_in.texcoord);
}
