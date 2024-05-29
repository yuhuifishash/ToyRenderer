#pragma once
#ifndef __HEMI_SPHERE_HPP__
#define __HEMI_SPHERE_HPP__

#include "Sampler3d.hpp"
#include <ctime>

namespace PhotonMap
{
    using namespace std;
    class HemiSphereNormal : public Sampler3d
    {
    private:
        constexpr static float C_PI = 3.14159265358979323846264338327950288f;
        
        default_random_engine e;
        uniform_real_distribution<float> u;
        Vec3 normal;
    public:
        HemiSphereNormal(const Vec3& n)
            : e               ((unsigned int)time(0) + insideSeed())
            , u               (-1, 1)
            , normal          (n)
        {}

        Vec3 sample3d() override {
            Vec3 res;
            do {
                res = Vec3(u(e), u(e), u(e));
            } while (glm::dot(res, res) >= 1.0 - 1e-12 || glm::dot(res, res) <= 1e-12 || dot(res, n) < 0);
            return glm::normalize(res);
        }
    };
}

#endif