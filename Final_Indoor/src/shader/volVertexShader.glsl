#version 430 core

layout (location = 0) in vec2 iv2win_pos;
layout (location = 1) in vec2 iv2tex_coord;

out vec2 texCoords;

void main() {
    gl_Position = vec4(iv2win_pos, -1.0, 1.0);
    texCoords = iv2tex_coord;
}