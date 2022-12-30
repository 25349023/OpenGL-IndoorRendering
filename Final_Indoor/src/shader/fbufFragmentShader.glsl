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

uniform sampler2D LTC1; // for inverse M
uniform sampler2D LTC2; // GGX norm, fresnel, 0(unused), sphere
uniform vec3 areaLightPoints[4];
uniform vec3 areaLightColor;

uniform bool enableFeature[7];

uniform vec3 Ia = vec3(0.1);
uniform vec3 Id = vec3(0.7);
uniform vec3 Is = vec3(0.2);

const float LUT_SIZE = 64.0; // ltc_texture size
const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
const float LUT_BIAS = 0.5 / LUT_SIZE;

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

vec4 bloom() {
    vec3 bloomColor = texture(tex[7], fs_in.texcoord).rgb;
    vec3 emissionColor = texture(beforeBloomTex, fs_in.texcoord).rgb;

    // HDR
    const float gamma = 2.2;
    vec3 hdrColor = 2 * bloomColor + 0.4 * emissionColor;
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    return vec4(pow(mapped, vec3(1.0 / gamma)), length(emissionColor));
}

// Vector form without project to the plane (dot with the normal)
// Use for proxy sphere clipping
vec3 integrate_edge_vec(vec3 v1, vec3 v2)
{
    // Using built-in acos() function will result flaws
    // Using fitting result for calculating acos()
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5 * inversesqrt(max(1.0 - x * x, 1e-7)) - v;

    return cross(v1, v2) * theta_sintheta;
}

float integrate_edge(vec3 v1, vec3 v2)
{
    return integrate_edge_vec(v1, v2).z;
}

// P is fragPos in world space (LTC distribution)
vec3 ltc_evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4])
{
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // polygon (allocate 4 vertices for clipping)
    vec3 L[4];
    // transform polygon from LTC back to origin Do (cosine weighted)
    L[0] = Minv * (points[0] - P);
    L[1] = Minv * (points[1] - P);
    L[2] = Minv * (points[2] - P);
    L[3] = Minv * (points[3] - P);

    // use tabulated horizon-clipped sphere
    // check if the shading point is behind the light
    vec3 dir = points[0] - P; // LTC space
    vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
    bool behind = (dot(dir, lightNormal) < 0.0);

    // cos weighted space
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    // integrate
    vec3 vsum = vec3(0.0);
    vsum += integrate_edge_vec(L[0], L[1]);
    vsum += integrate_edge_vec(L[1], L[2]);
    vsum += integrate_edge_vec(L[2], L[3]);
    vsum += integrate_edge_vec(L[3], L[0]);

    // form factor of the polygon in direction vsum
    float len = length(vsum);

    float z = vsum.z / len;
    if (behind) {
        z = -z;
    }

    vec2 uv = vec2(z * 0.5f + 0.5f, len); // range [0, 1]
    uv = uv * LUT_SCALE + LUT_BIAS;

    // Fetch the form factor for horizon clipping
    float scale = texture(LTC2, uv).w;

    float sum = len * scale;
    if (!behind) {
        sum = 0.0;
    }

    // Outgoing radiance (solid angle) for the entire polygon
    vec3 Lo_i = vec3(sum, sum, sum);
    return Lo_i;
}

// PBR-maps for roughness (and metallic) are usually stored in non-linear
// color space (sRGB), so we use these functions to convert into linear RGB.
vec3 pow_vec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float gamma = 2.2;
vec3 to_linear(vec3 v) { return pow_vec3(v, gamma); }
vec3 to_srgb(vec3 v) { return pow_vec3(v, 1.0 / gamma); }

vec3 area_light(vec3 worldVertex, vec3 N, vec3 V, vec3 kd, vec3 ks) {
    ks = to_linear(ks);
    float dotNV = clamp(dot(N, V), 0.0f, 1.0f);

    // use roughness and sqrt(1-cos_theta) to sample M_texture
    float roughness = 1;
    vec2 uv = vec2(roughness, sqrt(1.0f - dotNV));
    uv = uv * LUT_SCALE + LUT_BIAS;

    // get 4 parameters for inverse_M
    vec4 t1 = texture(LTC1, uv);

    // Get 2 parameters for Fresnel calculation
    vec4 t2 = texture(LTC2, uv);

    mat3 Minv = mat3(
        vec3(t1.x, 0, t1.y),
        vec3(0, 1, 0),
        vec3(t1.z, 0, t1.w)
    );

    // Evaluate LTC shading
    vec3 diffuse = 1.2 * kd * ltc_evaluate(N, V, worldVertex, mat3(1), areaLightPoints);
    vec3 specular = ltc_evaluate(N, V, worldVertex, Minv, areaLightPoints);

    // GGX BRDF shadowing and Fresnel
    // t2.x: shadowedF90 (F90 normally it should be 1.0)
    // t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
    specular *= ks * t2.x + (1.0f - ks) * t2.y;

    return to_srgb(areaLightColor * (specular + diffuse));
}

void main(void) {
    if (activeTex == 0) {
        vec3 worldVertex = texture(tex[1], fs_in.texcoord).xyz;
        vec3 worldNormal = texture(tex[2], fs_in.texcoord).xyz;
        vec3 V = check_normalize(cameraEye - worldVertex);
        vec3 N = normalize(worldNormal);
        vec3 L, H;
        vec3 ka = texture(tex[3], fs_in.texcoord).xyz;
        vec3 kd = texture(tex[4], fs_in.texcoord).xyz;
        vec3 ks = texture(tex[5], fs_in.texcoord).xyz;
        float ns = texture(tex[6], fs_in.texcoord).x;

        vec3 ambient, diffuse, specular;
        
        int cel_step_count = 3;
        
        vec3 color = vec3(0.0);
        if (!enableFeature[0] && !enableFeature[3] && !enableFeature[6]) {
            color = texture(tex[4], fs_in.texcoord).rgb;
        }

        if (enableFeature[0]) {
            L = normalize(directionalLight - worldVertex);
            H = normalize(L + V);

            ambient = Ia * ka;
            diffuse = Id * max(dot(N, L), 0.0) * kd;
            specular = Is * pow(max(dot(N, H), 0.0), ns) * ks;

            color += blinn_phong_shading(worldVertex, worldNormal, ambient, diffuse, specular);
        }

        if (enableFeature[3]) {
            L = normalize(pointLight - worldVertex);
            H = normalize(L + V);

            ambient = Ia * ka;
            diffuse = Id * max(dot(N, L), 0.0) * kd;
            specular = Is * pow(max(dot(N, H), 0.0), ns) * ks;

            color += point_light(worldVertex, worldNormal, ambient, diffuse, specular);
        }

        if (enableFeature[4]) {
            vec4 b = bloom();
            if (b.w > 0.0) {
                color = 1.4 * b.rgb;
            } else {
                color += b.rgb;
            }
        }

        if (enableFeature[6]) {
            color += area_light(worldVertex, N, V, kd, ks);
        }

        if (enableFeature[6]) {
            float sobel_x[][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
            float sobel_y[][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
            vec2 tex_offset = 1.0 / textureSize(tex[0], 0);
            vec3 hori_edge = vec3(0), vert_edge = vec3(0);
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    hori_edge += texture(tex[0], fs_in.texcoord + tex_offset * vec2(i, j)).rgb * sobel_x[1-i][1-j];
                    vert_edge += texture(tex[0], fs_in.texcoord + tex_offset * vec2(i, j)).rgb * sobel_y[1-i][1-j];
                }
            }
            if (any(greaterThanEqual(abs(hori_edge), vec3(0.5))) || any(greaterThanEqual(abs(vert_edge), vec3(0.5)))) {
                color = vec3(0.0);
            } 
            else {
                N = normalize(worldNormal);
                L = normalize(directionalLight - worldVertex);

                float nl = dot(N, L);
                float multiplier = 1.0;

                if (nl > 0.5) 
                    multiplier = 1.0;
                else if (nl > 0.0)
                    multiplier = 0.7;
                else 
                    multiplier = 0.2;

                color *= multiplier;
            }
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
