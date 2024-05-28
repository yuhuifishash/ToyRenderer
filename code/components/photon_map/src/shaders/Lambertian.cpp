#include "shaders/Lambertian.hpp"

namespace PhotonMap
{
    Lambertian::Lambertian(Material& material, vector<Texture>& textures)
        : Shader                (material, textures)
    {
        auto diffuseColor = material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
        if (diffuseColor) albedo = (*diffuseColor).value;
        else albedo = {1, 1, 1};
    }
    Scattered Lambertian::shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const {
        return {
            false,

        };
    }
}