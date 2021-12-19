#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../functions/constants.glsl"

// Buffers ==================================================

layout(push_constant) uniform Misc {
    mat4 model;
    vec3 viewPosition;
    uint sampleSize;
    uint isLight;
};

layout(set = 1, binding = 0) buffer Interference {
    vec4 color[];
} interference;

layout(set = 1, binding = 1) uniform Lights {
    vec4 color;
    vec4 position[4];
    float radiance;
    uint total;
} lights;

// Textures ==================================================
layout(set = 2, binding = 0) uniform sampler2D albedoMap;
layout(set = 2, binding = 1) uniform sampler2D aoMap;
layout(set = 2, binding = 2) uniform sampler2D metallicMap;
layout(set = 2, binding = 3) uniform sampler2D normalMap;
layout(set = 2, binding = 4) uniform sampler2D roughnessMap;

layout(set = 3, binding = 0) uniform sampler2D heightMap;

// Inputs ==================================================
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPosition;

// Outputs ==================================================
layout(location = 0) out vec4 outColor;

// Functions ==================================================
#include "../functions/interference.glsl"
#include "../functions/render_function.glsl"
#include "../functions/pbr.glsl"

void main() {
    vec4  heightmap = texture(heightMap, fragTexCoord);
    
    vec3  N = getNormal();
    float d = heightmap.x * scaleD;
    float theta1 = getTheta1(N);
    float theta2 = refractionAngle(n1, theta1, n2);
    float opd    = getOPD(d, theta2, n2);

//    uint idx = getIndex1D(opd);
//    outColor = interference.color[idx];
    
    vec4 pbrColor = vec4(pbr(), 1.0);
    outColor = pbrColor;
    
    int idx = int(opd * sampleSize);
    vec4 iridescence = interference.color[idx];
    outColor = iridescence;
//
//    float metallic  = texture(metallicMap, fragTexCoord).r;
//    if (metallic > 0.5) {
//        outColor = pbrColor * interference.color[idx] * 2.4;
//    }
    if (isLight == 1) outColor = lights.color;
}
