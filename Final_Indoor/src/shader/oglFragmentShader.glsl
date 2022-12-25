#version 430 core

in vec3 f_viewVertex;
in vec3 f_viewNormal;
in vec3 f_viewDirLight;
in vec3 f_worldVertex;
in vec3 f_worldNormal;
in vec3 f_uv;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 worldSpaceVertex;
layout (location = 2) out vec4 worldSpaceNormal;
layout (location = 3) out vec4 ambientColorMap;
layout (location = 4) out vec4 diffuseColorMap;
layout (location = 5) out vec4 specularColorMap;

uniform int pixelProcessId;
uniform sampler2D albedoTex;

uniform bool effectsToggle[1];

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float ns;

uniform vec3 lightAmbient = vec3(0.1);
uniform vec3 lightDiffuse = vec3(0.7);
uniform vec3 lightSpecular = vec3(0.2);

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

vec4 diffuse_color() {
    if (pixelProcessId == 1) {
        return texture(albedoTex, f_uv.xy);
    }
    else if (pixelProcessId == 2) {
        return vec4(kd, 1.0);
    }
}

vec4 blinn_phong_shading() {
    vec3 N = normalize(f_viewNormal);
    vec3 L = normalize(f_viewDirLight);
    vec3 V = normalize(-f_viewVertex);
    vec3 H = normalize(L + V);

    vec4 albedo;
    albedo = diffuse_color();

    vec3 ambient = lightAmbient * ka;
    vec3 diffuse = lightDiffuse * max(dot(N, L), 0.0) * albedo.xyz;
    vec3 specular = lightSpecular * pow(max(dot(N, H), 0.0), ns) * ks;

    return vec4(ambient + diffuse + specular, 1.0);
}

void main() {
    fragColor = blinn_phong_shading();
    worldSpaceVertex = vec4(normalize(f_worldVertex) * 0.5 + 0.5, 1.0);
    worldSpaceNormal = vec4(normalize(f_worldNormal) * 0.5 + 0.5, 1.0);
    ambientColorMap = vec4(ka, 1.0);
    diffuseColorMap = diffuse_color();
    specularColorMap = vec4(ks, 1.0);
}