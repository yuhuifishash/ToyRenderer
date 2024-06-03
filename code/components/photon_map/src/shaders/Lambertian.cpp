#include "shaders/Lambertian.hpp"
#include "samplers/SamplerInstance.hpp"
#include "Onb.hpp"

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
        Vec3 origin = hitPoint;
        //对cosThetaI进行重要性采样
        float U1 = defaultSamplerInstance<UniformSampler>().sample1d();
        float U2 = defaultSamplerInstance<UniformSampler>().sample1d();
        float x = cos(2 * PI * U2) * sqrt(U1);
        float y = sin(2 * PI * U2) * sqrt(U1);
        float z = sqrt(1 - U1);
        float pdf = z / PI;
        Vec3 random(x, y, z);

        /*Vec3 random = defaultSamplerInstance<HemiSphere>().sample3d();
        float pdf = 1 / (2 * PI);*/

        Onb onb{ normal };
        Vec3 direction = glm::normalize(onb.local(random));

        auto attenuation = albedo / PI;

        return {
            false,
            Ray{origin, direction},
            attenuation,
            Vec3{0},
            pdf
        };
    }
}