#version 430 core

in vec3 f_worldVertex;
in vec3 f_worldNormal;
in vec3 f_uv;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 worldSpaceVertex;
layout (location = 2) out vec4 worldSpaceNormal;
layout (location = 3) out vec4 ambientColorMap;
layout (location = 4) out vec4 diffuseColorMap;
layout (location = 5) out vec4 specularColorMap;
layout (location = 6) out vec4 shininessMap;

uniform int pixelProcessId;
uniform sampler2D albedoTex;

uniform bool effectsToggle[1];

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float ns;

uniform vec3 lightAmbient = vec3(0.1);
uniform vec3 lightDiffuse = vec3(0.7);
uniform vec3 lightSpecular = vec3(0.7);

vec3 check_normalize(vec3 v) {
    if (v == vec3(0.0)) {
        return v;
    }
    return normalize(v);
}

vec4 diffuse_color() {
    if (pixelProcessId == 1) {
        vec4 albedo = texture(albedoTex, f_uv.xy);
        if (albedo.a < 0.5) {
            discard;
        }
        
        return albedo;
    }
    else if (pixelProcessId == 2) {
        return vec4(kd, 1.0);
    }
}

void main() {
    fragColor = diffuse_color();    

    worldSpaceVertex = vec4(f_worldVertex, 1.0);
    worldSpaceNormal = vec4(f_worldNormal, 1.0);
    ambientColorMap = vec4(ka, 1.0);
    diffuseColorMap = diffuse_color();
    specularColorMap = vec4(ks, 1.0);
    shininessMap = vec4(ns);
}