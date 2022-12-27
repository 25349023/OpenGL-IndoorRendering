#version 430 core

layout (location = 0) uniform sampler2D tex[8];

layout (location = 0) out vec4 fragColor;

uniform int activeTex;
uniform vec3 cameraEye;
uniform vec3 directionalLight;

uniform sampler2D shadowTex;
uniform mat4 shadowMat;

uniform vec3 pointLight;
uniform vec3 pointLightAttenuation;

uniform bool enableFeature[5];

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

vec3 blinn_phong_shading() {
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
        vec4 f_shadowCoord = shadowMat * vec4(worldVertex, 1.0);
        
        float lightSpaceDepth = texture(shadowTex, f_shadowCoord.xy).x;
        float bias = min(0.03, max(0.03 * (1.0 - dot(worldNormal, directionalLight)), 0.005));  
        shadow = f_shadowCoord.z - bias <= lightSpaceDepth ? 1.0 : 0.0;
    }

    return ambient + shadow * (diffuse + specular);
}

vec3 point_light() {
    vec3 worldVertex = texture(tex[1], fs_in.texcoord).xyz;
    vec3 worldNormal = texture(tex[2], fs_in.texcoord).xyz;

    vec3 V = check_normalize(cameraEye - worldVertex);
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(pointLight - worldVertex);
    vec3 H = normalize(L + V);

    vec3 ka = texture(tex[3], fs_in.texcoord).xyz;
    vec3 kd = texture(tex[4], fs_in.texcoord).xyz;
    vec3 ks = texture(tex[5], fs_in.texcoord).xyz;
    float ns = texture(tex[6], fs_in.texcoord).x;

    vec3 ambient = Ia * ka;
    vec3 diffuse = Id * max(dot(N, L), 0.0) * kd;
    vec3 specular = Is * pow(max(dot(N, H), 0.0), ns) * ks;

    float dis = length(pointLight - worldVertex);
    float attenuation = 1.0 / (pointLightAttenuation.x + pointLightAttenuation.y * dis +
    pointLightAttenuation.z * (dis * dis));

    return attenuation * (ambient + diffuse + specular);
}

vec3 bloom() {
    vec3 bloomColor = texture(tex[7], fs_in.texcoord).rgb;
    return bloomColor;
}

void main(void) {
    if (activeTex == 0) {
        vec3 color = vec3(1.0);
        color = texture(tex[4], fs_in.texcoord).rgb;
        if (enableFeature[0] || enableFeature[3]) {
            color = vec3(0.0);
        }

        if (enableFeature[0]) {
            color += blinn_phong_shading();
        }
        if (enableFeature[3]) {
            color += point_light();
        }
        if (enableFeature[4]) {
            color += bloom();
        }

        fragColor = vec4(color, 1.0);
    }
    else if (activeTex == 1 || activeTex == 2) {
        vec3 texel = texture(tex[activeTex], fs_in.texcoord).xyz;
        fragColor = vec4(check_normalize(texel) * 0.5 + 0.5, 1.0);
    }
    else {
        fragColor = texture(tex[activeTex], fs_in.texcoord);
    }
}
