#include "shaders/Conductor.hpp"

namespace PhotonMap
{
    Vec3 Conductor::fresnelSchlick(const Vec3& n, const Vec3& Wr) const {
        //nΪ��λ�������� WrΪ��λ��������
        float cosTheta = glm::dot(n, Wr);
        Vec3 reflect_inv = Vec3(1.0, 1.0, 1.0) - reflect;
        return reflect + reflect_inv * pow(1.0f - cosTheta, 5.0f);
    }

    Conductor::Conductor(Material& material, vector<Texture>& textures)
        : Shader(material, textures)
    {
        auto reflect_ptr = material.getProperty<Property::Wrapper::RGBType>("reflect");
        if (reflect_ptr) reflect = (*reflect_ptr).value;
        else reflect = { 1, 1, 1 };
    }
    Scattered Conductor::shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const {
        //���淴��, ����pdf = 1, ֻ�����������
        float pdf = 1.f;
        //���㷴����� R = I - 2(I dot N)N
        Vec3 Wr = ray.direction - 2 * (glm::dot(ray.direction, normal)) * normal;
        Wr = glm::normalize(Wr);
        Vec3 attenuation = fresnelSchlick(normal, Wr) / abs(glm::dot(ray.direction,normal));
        return {
            true,
            Ray{hitPoint, Wr},
            attenuation,
            Vec3{0},
            pdf
        };
    }
}