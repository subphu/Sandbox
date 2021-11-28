#version 450

// Textures ==================================================
layout(set = 0, binding = 0) uniform sampler2D renderResult;

layout(location = 0) in  vec2 fragUV;
layout(location = 0) out vec4 fragColor;


void main() {
    vec4  color = vec4(fragUV.x, fragUV.y, 0.0, 1.0);
    float gamma = 1. / 2.2;
    color = texture(renderResult, fragUV);
    fragColor   = pow(color, vec4(gamma));
}
