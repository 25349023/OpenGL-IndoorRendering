#version 430 core

in vec4 fragPos;
out vec4 fragDepth;

uniform vec3 lightPos;
uniform float far;

void main() {
    float lightDistance = length(fragPos.xyz - lightPos);
    lightDistance = lightDistance / far;

    gl_FragDepth = lightDistance;
    fragDepth = vec4(vec3(lightDistance), 1.0);
}