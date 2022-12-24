#version 430 core

in vec3 f_viewVertex;
in vec3 f_viewNormal;
in vec3 f_worldVertex;
in vec3 f_worldNormal;
in vec3 f_uv;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 worldSpaceVertex;
layout (location = 2) out vec4 worldSpaceNormal;

uniform int pixelProcessId;
uniform sampler2D albedoTex;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;

uniform vec3 lightAmbient = vec3(0.2);
uniform vec3 lightDiffuse = vec3(0.64);
uniform vec3 lightSpecular = vec3(0.16);

void texture_mapping() {
    vec4 texel = texture(albedoTex, f_uv.xy);
    if (texel.a < 0.5) {
        discard;
    }
    fragColor = texel;
}

void simple_shading() {
    fragColor = vec4(kd, 1.0);
}

void phong_shading() {
    vec3 N = normalize(f_worldNormal);
    vec3 L = normalize(vec3(0.4, 0.5, 0.8));
    vec3 V = normalize(-f_worldVertex);
    vec3 R = normalize(2 * dot(L, N) * N - L);

    vec4 albedo;
    vec3 k_s;
    if (pixelProcessId == 1) {
        albedo = texture(albedoTex, f_uv.xy);
        if (albedo.a < 0.3) {
            discard;
        }
        k_s = vec3(0.0);
    } else if (pixelProcessId == 8) {
        albedo = vec4(0.0, 0.5, 0.7, 1.0);
        k_s = vec3(1.0);
    }

    vec3 ambient = lightAmbient * albedo.xyz;
    vec3 diffuse = lightDiffuse * max(dot(N, L), 0.0) * albedo.xyz;
    vec3 specular = lightSpecular * pow(max(dot(N, R), 0.0), 1.0) * k_s;

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}

void main() {
    if (pixelProcessId == 1) {
        texture_mapping();
    }
    else if (pixelProcessId == 2) {
        simple_shading();
    }
    worldSpaceVertex = vec4(normalize(f_worldVertex) * 0.5 + 0.5, 1);
    worldSpaceNormal = vec4(normalize(f_worldNormal) * 0.5 + 0.5, 1);
}