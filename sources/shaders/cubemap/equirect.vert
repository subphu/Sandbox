#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shader_viewport_layer_array : enable

layout(push_constant) uniform Misc {
    mat4 mvp;
    int layer;
};

layout(location = 0) in  vec3 inPosition;
layout(location = 0) out vec3 fragPosition;

void main() {
    fragPosition = inPosition;
    gl_Position  = mvp * vec4(inPosition, 1.0);
    gl_Layer     = layer;
}
