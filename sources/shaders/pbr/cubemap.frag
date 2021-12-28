#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../functions/constants.glsl"

layout(set = 5, binding = 0) uniform samplerCube cubemap;

layout(location = 0) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;


void main() {
    outColor = texture(cubemap, fragPosition);
}
