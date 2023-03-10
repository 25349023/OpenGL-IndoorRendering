#version 430 core

layout (location = 0) in vec3 position;

uniform mat4 mvpMat;

void main() {
    gl_Position = mvpMat * vec4(position, 1.0);
}