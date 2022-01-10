#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../functions/constants.glsl"

// Buffers ==================================================

layout(push_constant) uniform Misc {
    mat4 model;
    vec3 viewPosition;
    uint isLight;
    float reflectance;
};

layout(set = 1, binding = 0) uniform Lights {
    vec4 color;
    vec4 position[4];
    uint total;
    float radiance;
} lights;

// Textures ==================================================
layout(set = 2, binding = 0) uniform sampler2D albedoMap;
layout(set = 2, binding = 1) uniform sampler2D aoMap;
layout(set = 2, binding = 2) uniform sampler2D metallicMap;
layout(set = 2, binding = 3) uniform sampler2D normalMap;
layout(set = 2, binding = 4) uniform sampler2D roughnessMap;

layout(set = 3, binding = 0) uniform sampler2D heightMap;
layout(set = 4, binding = 0) uniform sampler2D interferenceImage;
layout(set = 5, binding = 0) uniform samplerCube cubemap;
layout(set = 5, binding = 1) uniform samplerCube envMap;
layout(set = 5, binding = 2) uniform samplerCube reflMap;
layout(set = 5, binding = 3) uniform sampler2D brdfMap;

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
//    vec4  heightmap = vec4(.2);
    
    vec3  N = getNormal();
    float d = heightmap.x * scaleD;
    float theta1 = getTheta1(N);
    float theta2 = refractionAngle(n1, theta1, n2);
    float opd    = getOPD(d, theta2, n2);
    
    vec2 interferenceUV = vec2(opd, reflectance);
    vec4 iridescence = texture(interferenceImage, interferenceUV);
    
    
    // PBR
    vec3  albedo    = pow(texture(albedoMap, fragTexCoord).rgb, vec3(2.2));
    float metallic  = texture(metallicMap, fragTexCoord).r;
    float roughness = texture(roughnessMap, fragTexCoord).r;
    float ao        = texture(aoMap, fragTexCoord).r;

    vec3 V = normalize(viewPosition - fragPosition);
    vec3 R = reflect(-V, N);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);
    
    for(uint i = 0; i < lights.total; ++i) {
        vec3 lightPosition = lights.position[i].xyz;
        
        vec3 L = normalize(lightPosition - fragPosition);
        vec3 H = normalize(V + L);
        float dist = length(lightPosition - fragPosition);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance = lights.color.xyz * lights.radiance * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3  nominator   = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    vec3 irradiance = texture(envMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(reflMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdfMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F + brdf.y);
//    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    specular = specular * iridescence.xyz;
    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    outColor = vec4(color, 1.0);
    
    if (isLight == 1) outColor = lights.color;
}
