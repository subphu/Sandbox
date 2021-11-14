
#define PI 3.14159265358979323846

float refractionAngle(float n1, float theta1, float n2) {
    float sin2 = n1 * sin(theta1) / n2;
    float theta2 = asin(sin2);
    return theta2;
}

vec2 snellLaw(float lambda, float opd) {
    float m = (opd/lambda) * 2.0 * PI;
    return vec2(cos(m), sin(m));
}

float getOPD(float d, float theta, float n) {
    return n * 2.0 * d * cos(theta);
}

float power(vec2 l) {
    return dot(l, l);
}
