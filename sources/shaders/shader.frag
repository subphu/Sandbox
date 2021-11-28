#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) buffer outputBuffer { vec4 imageData[]; };

layout(set = 1, binding = 2) uniform Misc {
    uint sampleSize;
} misc;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 fragPosition;

layout(location = 0) out vec4 outColor;


void main() {
    uint x = uint(fragTexCoord.x * misc.sampleSize);
    uint y = uint(fragTexCoord.y * misc.sampleSize);
    uint idx = y*misc.sampleSize + x;
    outColor = imageData[idx];
    outColor = texture(texSampler, fragTexCoord);
}
