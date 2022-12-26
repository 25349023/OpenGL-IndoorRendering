#version 430 core

layout (location = 0) uniform sampler2D tex[8];

layout (location = 0) out vec4 fragColor;

uniform int activeTex;
uniform vec3 cameraEye;
uniform vec3 directionalLight;

uniform bool enableFeature[2];

uniform vec3 Ia = vec3(0.1);
uniform vec3 Id = vec3(0.7);
uniform vec3 Is = vec3(0.2);

in VS_OUT
{
    vec2 texcoord;
} fs_in;

vec3 check_normalize(vec3 v) {
    if (v == vec3(0.0)) {
        return v;
    }
    return normalize(v);
}

vec4 blinn_phong_shading() {
    vec3 worldVertex = texture(tex[1], fs_in.texcoord).xyz;
    vec3 worldNormal = texture(tex[2], fs_in.texcoord).xyz;
    
    vec3 V = check_normalize(cameraEye - worldVertex);
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(directionalLight - worldVertex);
    vec3 H = normalize(L + V);

    vec3 ka = texture(tex[3], fs_in.texcoord).xyz;
    vec3 kd = texture(tex[4], fs_in.texcoord).xyz;
    vec3 ks = texture(tex[5], fs_in.texcoord).xyz;
    float ns = texture(tex[6], fs_in.texcoord).x;

    vec3 ambient = Ia * ka;
    vec3 diffuse = Id * max(dot(N, L), 0.0) * kd;
    vec3 specular = Is * pow(max(dot(N, H), 0.0), ns) * ks;

    float shadow = 1.0;
    if (enableFeature[1]) {
        shadow = texture(tex[7], fs_in.texcoord).x;
    }
    
    return vec4(shadow * (ambient + diffuse + specular), 1.0);
}

void main(void) {
    if (activeTex == 0) {
        vec4 color = vec4(1.0);
        color = texture(tex[4], fs_in.texcoord);
        if (enableFeature[0]) {
            color = blinn_phong_shading();
        }
        
        fragColor = color;
    }
    else if (activeTex == 1 || activeTex == 2) {
        vec3 texel = texture(tex[activeTex], fs_in.texcoord).xyz; 
        fragColor = vec4(check_normalize(texel) * 0.5 + 0.5, 1.0);
    } 
    else {
        fragColor = texture(tex[activeTex], fs_in.texcoord);
    }
}
