#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Misc {
    mat4 model;
    vec3 viewPosition;
    uint isLight;
    float reflectance;
};

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPosition;

void main() {
    vec4 worldPos = model * vec4(inPosition, 1.0);
    fragPosition  = vec3(worldPos);
    fragTexCoord  = inTexCoord;
    fragNormal    = mat3(transpose(inverse(model))) * inNormal;

    gl_Position =  proj * view * worldPos;
}