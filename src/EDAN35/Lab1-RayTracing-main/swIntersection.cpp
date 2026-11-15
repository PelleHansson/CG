#include "swIntersection.h"

namespace sw {

Ray Intersection::getShadowRay(const Vec3 &lightPos) {
    Vec3 L = lightPos - position;
    float tMax = sqrt(L * L);
    return Ray(position, L.normalize(), 0.01f, tMax);
    // Skuggorna försvinner för väldigt stora epsilon
}

Ray Intersection::getReflectedRay(void) {
    Vec3 N = normal;
    const Vec3 D = ray.dir;

    Vec3 R = D - 2 * (N * D) * N;

    return Ray(position, R.normalize(), 0.01f, FLT_MAX);
}

Ray Intersection::getRefractedRay(void) {
    const Vec3 &D = ray.dir;
    Vec3 N = normal;
    float eta = 1.0f / material.refractiveIndex;
    if (!frontFacing) eta = 1.0f / eta; // Inside material.

    float r = -D * N;
    float c = 1 - std::pow(eta, 2) * (1 - std::pow(r, 2));
    Vec3 t = eta * D + (eta * r - std::sqrt(c)) * N;
    Vec3 R = c < 0.0f ? D - 2 * (N * D) * N : t;

    return Ray(position, R, 0.01f, FLT_MAX);
}

} // namespace sw
