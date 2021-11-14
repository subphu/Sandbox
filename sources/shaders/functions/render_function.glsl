
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, fragTexCoord).rgb;
    
    vec3 Q1  = dFdx(fragPosition);
    vec3 Q2  = dFdy(fragPosition);
    vec2 st1 = dFdx(fragTexCoord);
    vec2 st2 = dFdy(fragTexCoord);

    vec3 N   = normalize(fragNormal);
    vec3 T   = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    vec3 normal = normalize(TBN * tangentNormal);
    return normal;
}

vec3 getNormal() {
    return fragNormal;
    return getNormalFromMap();
}

float getTheta1StaticLight(vec3 N) {
    vec3  L = normalize(lightPosition);
    vec3  V = normalize(viewPosition - fragPosition);
    vec3  H = normalize(L+V);
    float NdotH = dot(N,H);
    float cosTheta1 = dot(H,L);
    float theta1 = acos(cosTheta1);
    return theta1;
}

float getTheta1(vec3 N) {
    vec3 lightDir = normalize( viewPosition * 10.0 - fragPosition );
    float theta1 = acos(dot(lightDir, normalize( N )));
    return theta1;
}

uint getIndex1D(float opd) {
    float scale = opd / maxOpd;
    return uint(scale * float(buffSize * buffSize));
}

uint getIndex2D(float scaleRad) {
    uint x   = uint(buffSize * scaleRad);
    uint y   = uint(buffSize * scaleD);
    return uint(y * buffSize + x);
}
