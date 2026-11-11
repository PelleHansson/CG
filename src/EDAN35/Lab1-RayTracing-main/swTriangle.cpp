#include "swTriangle.h"

namespace sw {

bool Triangle::intersect(const Ray &r, Intersection &isect) const {
    // TODO: Implement ray-triangle intersection
    Vec3 e1 = vertices[1] - vertices[0];
    Vec3 e2 = vertices[2] - vertices[0];
    Vec3 n = e1 % e2;

    float denom = -(n * r.dir);
    if (std::fabs(denom) < 1e-8f) return false;

    float t = (n * (r.origin() - vertices[0])) / denom;
    if (t < r.minT || t > r.maxT) return false;


    Vec3 q = r.origin() + t * r.dir;
    Vec3 rs = q - vertices[0];
    Vec3 c1 = e1 % rs;
    Vec3 c2 = rs % e2;
    if (c1 * n < 0.0f) return false;
    if (c2 * n < 0.0f) return false;

    float nLen = std::sqrt(n * n);
    float v = std::sqrt(c1 * c1) / nLen;
    float w = std::sqrt(c2 * c2) / nLen;
    if (v + w > 1.0f) return false;

   Vec3 normal = n;
    normal.normalize();

    isect.hitT = t;
    isect.frontFacing = (-r.dir * normal) > 0.0f;
    if (!isect.frontFacing) normal = -normal;
    isect.normal = normal;
    isect.position = q;
    isect.material = material;
    isect.ray = r;
    return true;
}

} // namespace sw
