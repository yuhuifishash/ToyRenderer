#include "shaders/Dielectric.hpp"
#include "samplers/SamplerInstance.hpp"

#include "Onb.hpp"

namespace SimplePathTracer
{
    Vec3 Dielectric::fresnelSchlick(const Vec3& n, const Vec3& Wr) const {
        //n为单位法向量， Wr为单位反射向量
        float cosTheta = glm::dot(n, Wr);
        Vec3 reflect_inv = Vec3(1.0, 1.0, 1.0) - reflect;
        return reflect + reflect_inv * pow(1.0f - cosTheta, 5.0f);
    }

    Dielectric::Dielectric(Material& material, vector<Texture>& textures)
        : Shader(material, textures)
    {
        auto reflect_ptr = material.getProperty<Property::Wrapper::RGBType>("absorbed");
        if (reflect_ptr) reflect = (*reflect_ptr).value;
        else reflect = { 1, 1, 1 };

        auto ior_ptr = material.getProperty<Property::Wrapper::FloatType>("ior");
        if (ior_ptr) ior = (*ior_ptr).value;
        else ior = 1;
    }
    Scattered Dielectric::shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const {
        //镜面反射
        float pdf = 1.f;
        //计算反射光线 R = I - 2(I dot N)N
        Vec3 Wr = ray.direction - 2 * (glm::dot(ray.direction, normal)) * normal;
        Wr = glm::normalize(Wr);
        Vec3 attenuation = fresnelSchlick(normal, Wr) / abs(glm::dot(ray.direction, normal));
        
        //镜面折射
        float r_pdf = 1.f;

        return {
            Ray{hitPoint, Wr},
            attenuation,
            Vec3{0},
            pdf
        };
    }
}