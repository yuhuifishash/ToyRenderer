#include "shaders/Dielectric.hpp"

namespace PhotonMap
{
    Vec3 Dielectric::fresnelSchlick(const Vec3& n, const Vec3& Wr) const {
        //nΪ��λ�������� WrΪ��λ��������
        float cosTheta = abs(glm::dot(n, Wr));
        Vec3 reflect_inv = Vec3(1.0, 1.0, 1.0) - reflect;
        return reflect + reflect_inv * pow(1.0f - cosTheta, 5.0f);
    }

    Dielectric::Dielectric(Material& material, vector<Texture>& textures)
        : Shader(material, textures)
    {
        auto reflect_ptr = material.getProperty<Property::Wrapper::RGBType>("reflect");
        if (reflect_ptr) reflect = (*reflect_ptr).value;
        else reflect = { 1, 1, 1 };

        auto ior_ptr = material.getProperty<Property::Wrapper::FloatType>("ior");
        if (ior_ptr) ior = (*ior_ptr).value;
        else ior = 1;
    }
    Scattered Dielectric::shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const {
        //���淴��
        float pdf = 1.f;
        //���㷴����� R = I - 2(I dot N)N
        Vec3 Wr = ray.direction - 2 * (glm::dot(ray.direction, normal)) * normal;
        Wr = glm::normalize(Wr);
        Vec3 attenuation = fresnelSchlick(normal, Wr) / abs(glm::dot(ray.direction, normal));
        
        //��������
        float r_pdf = 1.f;
        Vec3 Wi = -ray.direction;
        //�Ƿ�������ⲿ���ڲ�������ֻ����ӿ������ڲ����������ڲ���������
        bool is_out2in = glm::dot(normal, Wi) > 0;
        float ni_nt = is_out2in ? 1.f / ior : ior / 1.f;
        float cosThetaI = glm::dot(normal, Wi);
        float sin2ThetaI = max(0.f, 1.f - cosThetaI * cosThetaI);
        float sin2ThetaT = ni_nt * ni_nt * sin2ThetaI;
        if (sin2ThetaT >= 1.0f - 0.00001f) {//ȫ����
            return {
                true,
                Ray{hitPoint, Wr},
                attenuation,
                Vec3{0},
                pdf,
            };
        }
        //����������ߺ�BTDF
        float cosThetaT = sqrt(1 - sin2ThetaT);
        Vec3 Wt = ni_nt * (-Wi) + (ni_nt * cosThetaI - cosThetaT) * normal;
        Wt = glm::normalize(Wt);
        Vec3 r_attenuation = (1.f/(ni_nt * ni_nt)) * (Vec3{ 1.0,1.0,1.0 } - fresnelSchlick(normal, Wi)) 
                                / abs(cosThetaI);
        return {
            true,
            Ray{hitPoint, Wr},
            attenuation,
            Vec3{0},
            pdf,
            true,
            Ray{hitPoint,Wt},
            r_attenuation,
            r_pdf,
        };
    }
}