#pragma once
#ifndef __CONDUCTOR_HPP__
#define __CONDUCTOR_HPP__

#include "Shader.hpp"

namespace PhotonMap
{
    class Conductor : public Shader
    {
    private:
        Vec3 reflect;
        Vec3 fresnelSchlick(const Vec3& n, const Vec3& Wr) const;
    public:
        Conductor(Material& material, vector<Texture>& textures);
        Scattered shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const;
    };
}

#endif