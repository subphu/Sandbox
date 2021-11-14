
#define PI 3.14159265358979323846

float refractionAngle(float n1, float theta1, float n2) {
    float sin2 = n1 * sin(theta1) / n2;
    float theta2 = asin(sin2);
    return theta2;
}

vec2 getPhase(float lambda, float opd) {
    float m = (opd/lambda) * 2.0 * PI;
    return vec2(cos(m), sin(m));
}

float getOPD(float d, float theta, float n) {
    return n * 2.0 * d * cos(theta);
}

float calcInterference(float lambda, float opd) {
    float ur = 0.5;     // Upper Reflection
    float lr = 1.0;     // Lower Reflection
    float si = 1.0;     // Start Intensity
    float ri = si - ur; // Refracted Intensity
    vec2  reflected = si * ur * vec2(1.0, 0.0);
    vec2  refracted = ri * lr * getPhase(lambda, opd);
    vec2  interference = reflected + refracted;
    float sensitivity  = dot(interference, interference);
    return sensitivity;
}

float interferences(float wavelength, float delta, float opd) {
    const int bands = 50;
    float tot = 0.0;
    for (int i=-bands ; i<=bands ; i++) {
        float idx = float(i)/float(bands);
        float lambda = wavelength + delta * idx;
        float interference = calcInterference(lambda, opd);
        float sensitivity  = cos(idx * PI)+1.0;
        tot += sensitivity * interference / float(bands*2+1);
    }
    return tot;
}

#include "../functions/spectrum.glsl"


vec3 deltaInterferences(float opd) {
    vec3 outColor = vec3(0.0);
    float tot = 0.0;
    float waveRange = 750. - 380.;
    float sensitivity = 4.0 / 10.;
    for (float i=380. ; i<=750. ; i+=37) {
        float lambda = i * 1e-9;
        float interference = calcInterference(lambda, opd);
        float waveScale = (i - 380.) / waveRange;
        outColor += sensitivity * interference * getColor(waveScale) * vec3(7./7.,7./5.8,7./4.6);
//        tot += sensitivity * interference;
    }
    return outColor;//vec3(tot);
}

vec3 fullInterferences(float opd) {
    vec3 outColor = vec3(0.0);
    float tot = 0.0;
    float waveRange = 750. - 380.;
    float sensitivity = 4.0 / waveRange;
    for (float i=380. ; i<=750. ; i++) {
        float lambda = i * 1e-9;
        float interference = calcInterference(lambda, opd);
        float waveScale = (i - 380.) / waveRange;
        outColor += sensitivity * interference * getColor(waveScale) * vec3(7./7.,7./5.8,7./4.6);
//        tot += sensitivity * interference;
    }
    return outColor;//vec3(tot);
}
