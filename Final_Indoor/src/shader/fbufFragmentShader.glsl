#version 430 core

layout (location = 0) uniform sampler2D tex[8];

layout (location = 0) out vec4 fragColor;

uniform int activeTex;
uniform vec3 cameraEye;
uniform vec3 directionalLight;

uniform sampler2D shadowTex;
uniform mat4 shadowMat;

uniform samplerCube shadowCubeTex;
uniform float far;

uniform sampler2D beforeBloomTex;
uniform vec3 pointLight;
uniform vec3 pointLightAttenuation;
uniform vec3 pointLightColor;

uniform bool enableFeature[6];

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

vec3 blinn_phong_shading(vec3 worldVertex, vec3 worldNormal, vec3 ambient, vec3 diffuse, vec3 specular) {
    float shadow = 1.0;
    if (enableFeature[1]) {
        vec4 f_shadowCoord = shadowMat * vec4(worldVertex, 1.0);
        
        float lightSpaceDepth = texture(shadowTex, f_shadowCoord.xy).x;
        float bias = min(0.03, max(0.03 * (1.0 - dot(worldNormal, directionalLight)), 0.005));  
        shadow = f_shadowCoord.z - bias <= lightSpaceDepth ? 1.0 : 0.0;
    }

    return ambient + shadow * (diffuse + specular);
}

vec3 point_light(vec3 worldVertex, vec3 worldNormal, vec3 ambient, vec3 diffuse, vec3 specular) {
    float dis = length(pointLight - worldVertex);
    float attenuation = 1.0 / (pointLightAttenuation.x + pointLightAttenuation.y * dis +
    pointLightAttenuation.z * (dis * dis));
    
    float shadow = 1.0;
    if (enableFeature[5]) {
        vec3 lightToVertex = worldVertex - pointLight;
        float distFromShadowMap = texture(shadowCubeTex, lightToVertex).x * far;
        
        float bias = min(0.03, max(0.03 * (1.0 - dot(worldNormal, pointLight)), 0.005));
        shadow = length(lightToVertex) - bias <= distFromShadowMap ? 1.0 : 0.0;
    }

    return attenuation * pointLightColor * (ambient + shadow * (diffuse + specular));
}

vec3 bloom() {
    vec3 bloomColor = texture(tex[7], fs_in.texcoord).rgb;
    vec3 emissionColor = texture(beforeBloomTex, fs_in.texcoord).rgb;
    
    // HDR
    const float gamma = 2.2;
    vec3 hdrColor = 2 * bloomColor + 0.4 * emissionColor;
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    return pow(mapped, vec3(1.0 / gamma));
}

void main(void) {
    if (activeTex == 0) {
        bool initializeLightParams = false;
        vec3 worldVertex, worldNormal;
        vec3 V, N, L, H;
        vec3 ka, kd, ks;
        float ns;
        vec3 ambient, diffuse, specular;
        
        vec3 color = vec3(0.0);
        if (!enableFeature[0] && !enableFeature[3]) {
            color = texture(tex[4], fs_in.texcoord).rgb;
        }

        if (enableFeature[0]) {
            worldVertex = texture(tex[1], fs_in.texcoord).xyz;
            worldNormal = texture(tex[2], fs_in.texcoord).xyz;

            V = check_normalize(cameraEye - worldVertex);
            N = normalize(worldNormal);
            ka = texture(tex[3], fs_in.texcoord).xyz;
            kd = texture(tex[4], fs_in.texcoord).xyz;
            ks = texture(tex[5], fs_in.texcoord).xyz;
            ns = texture(tex[6], fs_in.texcoord).x;

            L = normalize(directionalLight - worldVertex);
            H = normalize(L + V);

            ambient = Ia * ka;
            diffuse = Id * max(dot(N, L), 0.0) * kd;
            specular = Is * pow(max(dot(N, H), 0.0), ns) * ks;
            initializeLightParams = true;
            
            color += blinn_phong_shading(worldVertex, worldNormal, ambient, diffuse, specular);
        }
        if (enableFeature[3]) {
            if (!initializeLightParams) {
                worldVertex = texture(tex[1], fs_in.texcoord).xyz;
                worldNormal = texture(tex[2], fs_in.texcoord).xyz;

                V = check_normalize(cameraEye - worldVertex);
                N = normalize(worldNormal);
                ka = texture(tex[3], fs_in.texcoord).xyz;
                kd = texture(tex[4], fs_in.texcoord).xyz;
                ks = texture(tex[5], fs_in.texcoord).xyz;
                ns = texture(tex[6], fs_in.texcoord).x;
                initializeLightParams = true;
            }

            L = normalize(pointLight - worldVertex);
            H = normalize(L + V);

            ambient = Ia * ka;
            diffuse = Id * max(dot(N, L), 0.0) * kd;
            specular = Is * pow(max(dot(N, H), 0.0), ns) * ks;
            
            color += point_light(worldVertex, worldNormal, ambient, diffuse, specular);
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
