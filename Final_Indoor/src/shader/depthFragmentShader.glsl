#version 430 core

out vec4 fragDepth;

void main() {
    fragDepth = vec4(vec3(gl_FragCoord.z), 1.0);
}