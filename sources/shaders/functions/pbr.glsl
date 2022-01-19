

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    // UE4 use square roughness
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float a1 = (roughness + 1.0);
    float k = (a1*a1) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec4 fresnelSchlick(float cosTheta, vec4 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec4 fresnelSchlickRoughness(float cosTheta, vec4 F0, float roughness) {
    return F0 + (max(vec4(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec4 pbr() {
    vec4  albedo    = texture(albedoMap, fragTexCoord);
    float metallic  = texture(metallicMap, fragTexCoord).r;
    float roughness = texture(roughnessMap, fragTexCoord).r;
    float ao        = texture(aoMap, fragTexCoord).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(viewPosition - fragPosition);
    vec3 R = reflect(-V, N);

    vec4 F0 = mix(vec4(0.04,0.04,0.04,1.), albedo, metallic);
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
         vec4 specular = nominator / denominator;
         
         vec4 kS = F;
         vec4 kD = vec4(1.0) - kS;
         kD *= 1.0 - metallic;

         float NdotL = max(dot(N, L), 0.0);

         Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    vec4 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec4 kS = F;
    vec4 kD = (1.0 - kS) * (1.0 - metallic);
    vec4 irradiance = texture(envMap, N);
    vec4 diffuse = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec4 prefilteredColor = textureLod(reflMap, R,  roughness * MAX_REFLECTION_LOD);
    vec2 brdf  = texture(brdfMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec4 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    vec4 ambient = (kD * diffuse + specular) * ao;
    
    vec4 color = ambient + Lo;

    // HDR tonemapping
    color.rgb = color.rgb / (color.rgb + vec3(1.0));

    return color;
}
