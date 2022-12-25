#version 430 core

layout (location = 0) uniform sampler2D tex[7];

layout (location = 0) out vec4 fragColor;

uniform int activeTex;
uniform vec3 directionalLight = vec3(-2.845, 2.028, -1.293);

uniform vec3 lightAmbient = vec3(0.1);
uniform vec3 lightDiffuse = vec3(0.7);
uniform vec3 lightSpecular = vec3(0.7);

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
    vec3 V = check_normalize(texture(tex[1], fs_in.texcoord).xyz);
    vec3 N = normalize(texture(tex[2], fs_in.texcoord).xyz);
    vec3 L = normalize(directionalLight);
    vec3 H = normalize(L + V);

    vec3 ka = texture(tex[3], fs_in.texcoord).xyz;
    vec3 kd = texture(tex[4], fs_in.texcoord).xyz;
    vec3 ks = texture(tex[5], fs_in.texcoord).xyz;
    float ns = texture(tex[6], fs_in.texcoord).x;

    vec3 ambient = lightAmbient * ka;
    vec3 diffuse = lightDiffuse * max(dot(N, L), 0.0) * kd;
    vec3 specular = lightSpecular * pow(max(dot(N, H), 0.0), ns) * ks;

    return vec4(ambient + diffuse + specular, 1.0);
}

void main(void) {
    if (activeTex == 0) {
        fragColor = blinn_phong_shading();
    }
    else if (activeTex == 1 || activeTex == 2) {
        vec3 texel = texture(tex[activeTex], fs_in.texcoord).xyz; 
        fragColor = vec4(check_normalize(texel) * 0.5 + 0.5, 1.0);
    } 
    else {
        fragColor = texture(tex[activeTex], fs_in.texcoord);
    }
}
