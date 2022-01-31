#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../functions/constants.glsl"

// Buffers ==================================================

layout(push_constant) uniform Misc {
    mat4 model;
    vec3 viewPosition;
    uint isLight;
};

layout(set = 1, binding = 0) uniform Lights {
    vec4 color;
    vec4 position[4];
    uint total;
    float radiance;
} lights;

layout(set = 1, binding = 1) uniform Params {
    vec4 albedo;
    float metallic;
    float roughness;
    float ao;
    uint  useTexture;
    uint  useFluid;
    
    uint  interference;
    uint  phaseShift;
    float thicknessScale;
    float refractiveIndex;
    float reflectanceValue;
    float opdOffset;
    uint  opdSample;
} params;

// Textures ==================================================
layout(set = 2, binding = 0) uniform sampler2D albedoMap;
layout(set = 2, binding = 1) uniform sampler2D aoMap;
layout(set = 2, binding = 2) uniform sampler2D metallicMap;
layout(set = 2, binding = 3) uniform sampler2D normalMap;
layout(set = 2, binding = 4) uniform sampler2D roughnessMap;

layout(set = 3, binding = 0) uniform sampler2D heightMap;
layout(set = 4, binding = 0) uniform sampler2D interferenceImage;
layout(set = 4, binding = 1) buffer  markBuffer { float markAlpha[]; };
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
    // PBR
    vec3  N         = fragNormal;
    vec4  albedo    = params.albedo;
    float metallic  = params.metallic;
    float roughness = params.roughness;
    float ao        = params.ao;
    if (params.useTexture > 0) {
        N         = getNormalFromMap();
        albedo    = texture(albedoMap, fragTexCoord);
        metallic  = texture(metallicMap, fragTexCoord).r;
        roughness = texture(roughnessMap, fragTexCoord).r;
        ao        = texture(aoMap, fragTexCoord).r;
    }
    
    vec4 iridescence = vec4(1.);
    if (params.interference > 0) {
        vec4  heightmap = params.useFluid > 0 ? texture(heightMap, fragTexCoord) : vec4(1.);
        float n2 = params.refractiveIndex;
        float d  = heightmap.x * params.thicknessScale;
        float theta1 = getTheta1(N);
        float theta2 = refractionAngle(n1, theta1, n2);
        float opd    = getOPD(d, theta2, n2) / 8.0;
        opd = mod(opd + params.opdOffset, 1.);
        vec2 interferenceUV = vec2(opd, params.reflectanceValue);
        iridescence = texture(interferenceImage, interferenceUV);
        markAlpha[int(opd * params.opdSample)] = 1.0;
    }
    
    vec3 V = normalize(viewPosition - fragPosition);
    vec3 R = reflect(-V, N);

    vec4 F0 = mix(vec4(0.04), albedo, metallic);
    F0.a = albedo.a;
    vec4 Lo = vec4(0.0);
    
    for(uint i = 0; i < lights.total; ++i) {
        vec3 lightPosition = lights.position[i].xyz;
        
        vec3 L = normalize(lightPosition - fragPosition);
        vec3 H = normalize(V + L);
        float dist = length(lightPosition - fragPosition);
        float attenuation = 1.0 / (dist * dist);
        vec4 radiance = lights.color * lights.radiance * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec4  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec4  nominator   = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec4 specular = (nominator / denominator) * iridescence;
        
        vec4 kS = F;
        vec4 kD = vec4(1.0) - kS;
        kD *= 1.0 - metallic;
        kD.a = F.a;

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    vec4 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec4 kS = F;
    vec4 kD = (1.0 - kS) * (1.0 - metallic);
    kD.a = F.a;
    
    vec4 irradiance = texture(envMap, N);
    vec4 diffuse = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec4 prefilteredColor = textureLod(reflMap, R,  roughness * MAX_REFLECTION_LOD);
    vec2 brdf  = texture(brdfMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec4 specular = prefilteredColor * (F + brdf.y);
//    vec4 specular = prefilteredColor * (F * brdf.x + brdf.y);
    specular.a = max(max(specular.r, specular.g), specular.b)/1.5 + 0.05;
    
    specular = specular * iridescence;
    vec4 ambient = (kD * diffuse + specular) * vec4(vec3(ao), 1.);
    
    vec4 color = ambient + Lo;

    // HDR tonemapping
    color.rgb = color.rgb / (color.rgb + vec3(1.0));
    outColor = color;
    
    if (isLight == 1) outColor = lights.color;
}
