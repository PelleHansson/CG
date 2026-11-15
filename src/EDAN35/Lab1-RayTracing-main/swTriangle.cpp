#include "swTriangle.h"

namespace sw {

bool Triangle::intersect(const Ray &r, Intersection &isect) const {
    Vec3 P = r.orig;
    Vec3 d = r.dir;
    Vec3 V0 = vertices[0];
    Vec3 V1 = vertices[1];
    Vec3 V2 = vertices[2];
    Vec3 e1 = V1 - V0;
    Vec3 e2 = V2 - V0;

    Vec3 N = e1 % e2;

    float m = -1.0f * N * V0;

    float t = (N * P + m)/(-1.0f * N * d);
    
    if (t <= r.minT || t >= r.maxT) return false;

    Vec3 Q = P + t*d;

    Vec3 c = Q - V0;

    float v = (e1 % c).norm()/N.norm();
    float w = (c % e2).norm()/N.norm();

    bool cond1 = (e1 % c) * N >= 0;
    bool cond2 = (c % e2) * N >= 0;
    bool cond3 = v + w < 1;

    if(!cond1 || !cond2 || !cond3) return false;

    isect.hitT = t;
    isect.normal = N;
    isect.normal.normalize();
    isect.frontFacing = (-d * isect.normal) > 0;
    if (!isect.frontFacing) isect.normal = -isect.normal;
    isect.position = Q;
    isect.material = material;
    isect.ray = r;

    return true;
}

} // namespace sw
