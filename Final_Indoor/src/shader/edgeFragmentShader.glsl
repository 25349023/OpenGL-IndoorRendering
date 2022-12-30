#version 430 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D tex;

uniform bool horizontal;
uniform float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

void main()
{
    float sobel_x[9] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
    float sobel_y[9] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
    vec2 tex_offset = 1.0 / textureSize(tex, 0);
    vec3 hori_edge = vec3(0), vert_edge = vec3(0);
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            hori_edge += texture(tex, texCoords + tex_offset * vec2(i, j)).rgb * sobel_x[(1-i)*3+(1-j)];
            vert_edge += texture(tex, texCoords + tex_offset * vec2(i, j)).rgb * sobel_y[(1-i)*3+(1-j)];
        }
    }
    hori_edge = abs(hori_edge);
    vert_edge = abs(vert_edge);
    if (hori_edge.x + hori_edge.y + hori_edge.z >= 1.0 || vert_edge.x + vert_edge.y + vert_edge.z >= 1.0) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        fragColor = texture(tex, texCoords);
    }
}