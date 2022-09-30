
const vec3 lightPosition = vec3(0.0, 10.0, 10.0);
const vec3 lightColor    = vec3(1.0,  1.0,  1.0) * 900.;

const float maxD   = 1e-6; // 1 µm
const float maxOpd = 10. * maxD; // 10 µm
const float scaleD = maxD / maxOpd;
//const float d = scaleD * maxD;

const float n1 = 1.0; // refraction index air
const float n2 = 1.5; // refraction index oil

