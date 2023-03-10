#version 430 core

out vec4 blurredColor;

in vec2 texCoords;

uniform sampler2D tex;

uniform bool horizontal;
uniform float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

void main()
{
    vec2 tex_offset = 1.0 / textureSize(tex, 0); // gets size of single texel
    vec3 result = texture(tex, texCoords).rgb * weight[0]; // current fragment's contribution
    if (horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(tex, texCoords + vec2(tex_offset.x * i * 2, 0.0)).rgb * weight[i];
            result += texture(tex, texCoords - vec2(tex_offset.x * i * 2, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(tex, texCoords + vec2(0.0, tex_offset.y * i * 2)).rgb * weight[i];
            result += texture(tex, texCoords - vec2(0.0, tex_offset.y * i * 2)).rgb * weight[i];
        }
    }
    blurredColor = vec4(result, 1.0);
}