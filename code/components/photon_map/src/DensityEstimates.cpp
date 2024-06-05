#include "server/Server.hpp"
#include "PhotonMap.hpp"
#include "intersections/intersections.hpp"
#include "Onb.hpp"

namespace PhotonMap
{

    //<photons, max_r2>
    tuple<std::vector<Photon*>,float> PhotonMap::GetNearestNPhotons(const Vec3& pos, int N, float R2) {
        NearestPhotonsHandler handler;
        handler.N = N;
        handler.max_r2 = R2;
        handler.X = pos;
        KdTree->GetNearestNPhotons(handler, KdTree->root);
        std::vector<Photon*> res;
        while (!handler.q.empty()) {
            res.push_back(handler.q.top().P);
            handler.q.pop();
        }
        return {res, handler.max_r2};
    }

    //���Ǽ���ֻ����������������й�������������
    RGB PhotonMap::DensityEstimates(const Vec3& pos, const Vec3& BRDF, bool is_Caustics) {
        int N = 0;
        if (!is_Caustics) {
            N = EstimatesN;
        }
        else {
            N = CausticsEstimatesN;
        }
        auto [photons, max_r2] = GetNearestNPhotons(pos,N,EstimmatesR2);
        if (photons.size() == 0 || abs(max_r2) < 1e-4) {
            return { 0.f,0.f,0.f };
        }
        Vec3 res;
        for (const auto& p : photons) {
            res += BRDF * p->Power;
        }
        return res/(PI*max_r2*PhotonSampleNum*4);
    }

    tuple<bool, RGB> PhotonMapRender::SampleDirectLight(const Vec3& hitPoint, const Vec3& normal, const Vec3& BRDF) {
        if (scene.areaLightBuffer.size() < 1) {
            return { false,{ 0.f,0.f,0.f } };
        }

        //�Թ�Դ������Ҫ�Բ���
        //����ѡ������һ�����Դ(��������ֱ��ƽ�����ѡ��һ��)
        int len = scene.areaLightBuffer.size();
        auto rand = defaultSamplerInstance<UniformSampler>().sample1d();
        int id = round(rand * (len - 1));
        auto& light = scene.areaLightBuffer[id];

        //��ƽ�������ѡ��һ����
        Vec3 target = light.position;
        target = target + light.u * defaultSamplerInstance<UniformSampler>().sample1d();
        target = target + light.v * defaultSamplerInstance<UniformSampler>().sample1d();

        //������߷���
        Vec3 to_light = target - hitPoint;
        to_light = glm::normalize(to_light);
        //��Ҫ��֤��������뷨�߼н�Ϊ��
        if (glm::dot(to_light, normal) < 0) {
            return { false,{ 0.f,0.f,0.f } };
        }
        else {//����һ������
            auto hitObject = closestHitObject(Ray{ hitPoint,to_light });
            auto [t, emitted] = closestHitLight(Ray{ hitPoint,to_light });
            if (hitObject && hitObject->t < t) {
                //�������壬˵�����߱��ڵ�
                return { false,{ 0.f,0.f,0.f } };
            }
            else {
                auto hitLight = Intersection::xAreaLight(
                                Ray{ hitPoint,to_light }, light, 0.000001, FLOAT_INF);
                if (hitLight) {//��ֹ���־�������
                    float pdf2 = 1;
                    float n_dot_in2 = 1;
                    //����pdf  ||x-x'||2
                    pdf2 = glm::distance(hitLight->hitPoint, hitPoint);
                    pdf2 = pdf2 * pdf2;
                    auto light_normal = glm::cross(light.u, light.v);
                    pdf2 /= glm::length(light_normal);
                    pdf2 /= len;
                    light_normal = glm::normalize(light_normal);
                    pdf2 /= abs(glm::dot(light_normal, to_light));
                    n_dot_in2 = glm::dot(normal, to_light);

                    return { true,BRDF * light.radiance * abs(n_dot_in2) / pdf2 };
                }
                else {
                    return { false,{ 0.f,0.f,0.f } };
                }
                
            }
        } 
    }

    tuple<bool, RGB> PhotonMapRender::SampleIndirectDiffUseLight(const Vec3& hitPoint, const Vec3& normal, const Vec3& BRDF) {
        Vec3 origin = hitPoint;
        //��cosThetaI������Ҫ�Բ���
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

        Ray NRay{ origin, direction };

        auto hitObject = closestHitObject(NRay);
        auto [t, emitted] = closestHitLight(NRay);
        if (hitObject && hitObject->t < t) {//��������
            auto mtlHandle = hitObject->material;
            auto scattered = shaderPrograms[mtlHandle.index()]->shade(NRay, hitObject->hitPoint, hitObject->normal);
            if (!scattered.is_specular) {//����������������
                //ʹ��ȫ�ֹ���ͼ���й���
                auto next = GlobalpnMap->DensityEstimates(hitObject->hitPoint, scattered.attenuation, false);
                float n_dot_in = glm::dot(hitObject->normal, direction);
                return { true,next * abs(n_dot_in) * BRDF / pdf };
            }
            else {
                return { false,{ 0.f,0.f,0.f } };
            }
        }
        else {//���й�Դ����ʲô��û�л��У�ֱ�ӷ���
            return { false,{ 0.f,0.f,0.f } };
        }
    }
}