#include "server/Server.hpp"

#include "PhotonMap.hpp"
#include "samplers/HemisphereNormal.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace PhotonMap
{
    auto PhotonMapRender::render() -> RenderResult {
        // shaders
        shaderPrograms.clear();
        ShaderCreator shaderCreator{};
        for (auto& m : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(m, scene.textures));
        }

        RGBA* pixels = new RGBA[1ll * width * height]{};

        // �ֲ�����ת������������
        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);




        getServer().logger.log("Done...");
        return { pixels, width, height };
    }



    tuple<Ray, float> PhotonMapRender::generatePhoton(HitRecord hit) {
        //��ʱֻ֧�����Դ
        int len1 = scene.areaLightBuffer.size();
        auto r = defaultSamplerInstance<UniformSampler>().sample1d();
        int id = round(r * (len1 - 1));
        auto& light = scene.areaLightBuffer[len1];
        Vec3 start = light.position;
        start = start + light.u * defaultSamplerInstance<UniformSampler>().sample1d();
        start = start + light.v * defaultSamplerInstance<UniformSampler>().sample1d();
        
        auto direction = defaultSamplerInstance<HemiSphere>().sample3d();
        Vec3 normal = glm::cross(light.u, light.v);
        float powerScale = abs(glm::dot(direction, normal));
        return{
            Ray{start,direction},
            powerScale
        };
    }

    void PhotonMapRender::TracePhoton(const Ray& r, int currDepth, Vec3 power) {
        if (currDepth == depth) {
            return;
        }
        auto hitObject = closestHitObject(r);
        if (hitObject) {//�������ཻ
            // cout << r.origin << " " << hitObject->hitPoint << "\n";
            auto mtlHandle = hitObject->material;
            auto scattered = shaderPrograms[mtlHandle.index()]->shade(r, hitObject->hitPoint, hitObject->normal);
            //���淴��/����
            if (scattered.is_specular) {
                TracePhoton(scattered.ray, currDepth + 1, power);
                if (scattered.has_refraction) {
                    TracePhoton(scattered.r_ray, currDepth + 1, power);
                }
            }
            else {

            }

        }
    }




    HitRecord PhotonMapRender::closestHitObject(const Ray& r) {
        HitRecord closestHit = nullopt;
        float closest = FLOAT_INF;
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& t : scene.triangleBuffer) {
            auto hitRecord = Intersection::xTriangle(r, t, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        return closestHit;
    }


}