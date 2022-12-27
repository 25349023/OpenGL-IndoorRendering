#version 430 core

layout (location = 0) in vec3 v_vertex;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_uv;
layout (location = 3) in vec3 v_tangent;
layout (location = 4) in vec3 v_bitangent;

out vec3 f_worldVertex;
out vec3 f_worldNormal;
out vec3 f_uv;
out vec4 f_shadowCoord;
out mat3 f_TBN;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform mat4 modelMat;
uniform mat4 modelRotateMat;
uniform mat4 shadowMat;

vec3 check_normalize(vec3 v) {
    if (v == vec3(0.0)) {
        return v;
    }
    return normalize(v);
}

void commonProcess() {
    mat4 modelViewMat = viewMat * modelMat;
    vec4 viewVertex = modelViewMat * vec4(v_vertex, 1.0);

    vec4 worldVer = (modelMat * vec4(v_vertex, 1.0));
    f_worldVertex = vec3(worldVer / worldVer.w);
    f_worldNormal = vec3(modelRotateMat * vec4(v_normal, 0.0));
    f_uv = v_uv;
    f_shadowCoord = shadowMat * vec4(v_vertex, 1.0);

    vec3 T = normalize(mat3(modelRotateMat) * v_tangent);
    vec3 B = normalize(mat3(modelRotateMat) * v_bitangent);
    vec3 N = normalize(f_worldNormal);
    f_TBN = mat3(T, B, N);
    
    gl_Position = projMat * viewVertex;
}

void main() {
    commonProcess();
}