#version 430 core

in vec3 f_worldVertex;
in vec3 f_worldNormal;
in vec3 f_uv;
in mat3 f_TBN;

layout (location = 0) out vec4 worldSpaceVertex;
layout (location = 1) out vec4 worldSpaceNormal;
layout (location = 2) out vec4 ambientColorMap;
layout (location = 3) out vec4 diffuseColorMap;
layout (location = 4) out vec4 specularColorMap;
layout (location = 5) out vec4 shininessMap;
layout (location = 6) out vec4 emissionMap;
layout (location = 7) out vec4 lightScatteringMap;

uniform int pixelProcessId;
uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform vec3 directionalLight;

uniform bool hasNorm;
uniform bool isEmissive;
uniform bool isLight;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float ns;
uniform vec3 em;

uniform sampler2D shadowTex;

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
    worldSpaceVertex = vec4(f_worldVertex, 1.0);

    worldSpaceNormal = vec4(f_worldNormal, 1.0);
    if (hasNorm) {
        vec3 normal = texture(normalTex, f_uv.xy).rgb * 2.0 - 1.0;
        worldSpaceNormal = vec4(normalize(f_TBN * normal), 1.0);
    }
    
    ambientColorMap = vec4(ka, 1.0);
    diffuseColorMap = diffuse_color();
    specularColorMap = vec4(ks, 1.0);
    shininessMap = vec4(ns);
    if (isEmissive) {
        emissionMap = vec4(em + 0.01, 1.0);
    } else {
        emissionMap = vec4(0.0);
    }
    
    const vec4 none = vec4(vec3(0.1), 0.0);
    if (isLight) {
        worldSpaceVertex = none;
        worldSpaceNormal = none;
        ambientColorMap = none;
        diffuseColorMap = none;
        specularColorMap = none;
        shininessMap = none;
        lightScatteringMap = vec4(1.0);
    } else {
        lightScatteringMap = vec4(0.0, 0.0, 0.0, 1.0);
    }
}