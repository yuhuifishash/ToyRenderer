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

        // 局部坐标转换成世界坐标
        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        //测试代码
        /*PhotonMap tmp(1000);
        tmp.KdTreeTest();*/

        createGlobalPhotonMap();
        createCausticsPhotonMap();

        GlobalpnMap->BuildKdTree();
        CausticspnMap->BuildKdTree();
        //  GlobalpnMap->PrintPhotonMap();
        PathTracingWithPhotonMap(pixels);

        getServer().logger.log("Done...");
        return { pixels, width, height };
    }

    RGB PhotonMapRender::getAmbientRGB(const Ray& r) {
        if (scene.ambient.type == Ambient::Type::CONSTANT) {
            return scene.ambient.constant;
        }
        else if (scene.ambient.type == Ambient::Type::ENVIROMENT_MAP) {
            Texture& texture = scene.textures[scene.ambient.environmentMap.index()];
            auto r_d = r.direction;
            //环境映射
            float m = sqrt(r_d.x * r_d.x + r_d.y * r_d.y + (r_d.z + 1) * (r_d.z + 1));
            Vec3 n = { r_d.x / m ,r_d.y / m ,(r_d.z + 1) / m };
            float u = r_d.x / (2 * m) + 0.5;
            float v = r_d.y / (2 * m) + 0.5;
            int x = u * texture.width;
            int y = v * texture.height;
            if (x <= 0) { x = 0; }
            if (y <= 0) { y = 0; }
            if (x >= texture.width) { x = texture.width - 1; }
            if (y >= texture.height) { y = texture.height - 1; }
            return texture.rgba[y * texture.width + x];
        }
        return { 0.f,0.f,0.f };
    }

    tuple<Ray, Vec3> PhotonMapRender::generatePhoton() {
        //暂时只支持面光源
        int len1 = scene.areaLightBuffer.size();
        auto r = defaultSamplerInstance<UniformSampler>().sample1d();
        int id = round(r * (len1 - 1));
        auto& light = scene.areaLightBuffer[id];
        Vec3 start = light.position;
        start = start + light.u * defaultSamplerInstance<UniformSampler>().sample1d();
        start = start + light.v * defaultSamplerInstance<UniformSampler>().sample1d();
        
        auto direction = defaultSamplerInstance<HemiSphere>().sample3d();
        Vec3 normal = glm::cross(light.u, light.v);
        float A = glm::length(normal);
        normal = glm::normalize(normal);
        float cosTheta = abs(glm::dot(direction, normal));
        return{
            Ray{start,direction},
            cosTheta * light.radiance * A * 4.f * PI,//均匀选择光源上一个点，对该点进行球形均匀采样
        };
    }

    void PhotonMapRender::TraceGlobalPhoton(const Ray& r, int currDepth, Vec3 power) {
        if (currDepth == depth) {
            return;
        }
        auto hitObject = closestHitObject(r);
        if (hitObject) {//与物体相交
            // cout << r.origin << " " << hitObject->hitPoint << "\n";
            auto mtlHandle = hitObject->material;
            auto scattered = shaderPrograms[mtlHandle.index()]->shade(r, hitObject->hitPoint, hitObject->normal);
            //镜面反射/折射
            if (scattered.is_specular) {//反射
                int NBxDf = (int)scattered.is_specular + (int)scattered.has_refraction;
                float cosTheta = abs(glm::dot(scattered.ray.direction, hitObject->normal));
                Vec3 nPower = power * scattered.attenuation * cosTheta / scattered.pdf;

                TraceGlobalPhoton(scattered.ray, currDepth + 1, nPower / (float)NBxDf);

                if (scattered.has_refraction) {//折射
                    float r_cosTheta = abs(glm::dot(scattered.r_ray.direction, hitObject->normal));
                    Vec3 r_nPower = power * scattered.r_attenuation * r_cosTheta / scattered.r_pdf;

                    TraceGlobalPhoton(scattered.r_ray, currDepth + 1, r_nPower / (float)NBxDf);
                }
            }
            else {//漫反射
                Photon pn;
                pn.Pos = hitObject->hitPoint;
                pn.Direction = r.direction;
                pn.Power = power;
                GlobalpnMap->StorePhoton(pn);
                // 1/6的概率被吸收
                float rand = defaultSamplerInstance<UniformSampler>().sample1d();
                if (rand > 1.0f/6.0f) {//光子没有被吸收
                    float cosTheta = abs(glm::dot(scattered.ray.direction, hitObject->normal));
                    Vec3 nPower = power * scattered.attenuation * cosTheta / scattered.pdf;
                    TraceGlobalPhoton(scattered.ray, currDepth + 1, nPower);
                }
            }

        }
    }

    void PhotonMapRender::TraceCausticsPhoton(const Ray& r, int currDepth, Vec3 power) {
        if (currDepth == depth) {
            return;
        }
        auto hitObject = closestHitObject(r);
        if (hitObject) {//与物体相交
            // cout << r.origin << " " << hitObject->hitPoint << "\n";
            auto mtlHandle = hitObject->material;
            auto scattered = shaderPrograms[mtlHandle.index()]->shade(r, hitObject->hitPoint, hitObject->normal);
            //镜面反射/折射
            if (scattered.is_specular) {//反射
                int NBxDf = (int)scattered.is_specular + (int)scattered.has_refraction;
                float cosTheta = abs(glm::dot(scattered.ray.direction, hitObject->normal));
                Vec3 nPower = power * scattered.attenuation * cosTheta / scattered.pdf;

                TraceCausticsPhoton(scattered.ray, currDepth + 1, nPower / (float)NBxDf);

                if (scattered.has_refraction) {//折射
                    float r_cosTheta = abs(glm::dot(scattered.r_ray.direction, hitObject->normal));
                    Vec3 r_nPower = power * scattered.r_attenuation * r_cosTheta / scattered.r_pdf;

                    TraceCausticsPhoton(scattered.r_ray, currDepth + 1, r_nPower / (float)NBxDf);
                }
            }
            else {//漫反射表面 不再继续反射
                //如果经过了至少一次镜面反射/折射，则记录该光子
                if (currDepth > 0) {
                    Photon pn;
                    pn.Pos = hitObject->hitPoint;
                    pn.Direction = r.direction;
                    pn.Power = power;
                    CausticspnMap->StorePhoton(pn);
                }
            }
        }
    }

    PhotonMap::PhotonMap(int photonSampleNum) {
        PhotonSampleNum = photonSampleNum;
        maxPhotonNum = photonSampleNum * 8;
        PhotonNum = 0;
        PhotonM = new Photon[maxPhotonNum];
        box_min = Vec3(1e6, 1e6, 1e6);
        box_max = Vec3(-1e6, -1e6, -1e6);
        KdTree = nullptr;
    }

    PhotonMap::~PhotonMap() {
        if (KdTree != nullptr) {
            delete KdTree;
        }
        delete[]PhotonM;
    }

    void PhotonMap::StorePhoton(Photon p) {
        if (PhotonNum >= maxPhotonNum) {
            return;
        }
        PhotonM[PhotonNum++] = p;
        box_min = Vec3(min(box_min.x,p.Pos.x), min(box_min.y,p.Pos.y), min(box_min.z,p.Pos.z));
        box_max = Vec3(max(box_min.x, p.Pos.x), max(box_min.y, p.Pos.y), max(box_min.z, p.Pos.z));
    }

    void PhotonMapRender::createGlobalPhotonMap() {
        GlobalpnMap = new PhotonMap(PhotonSampleNum);
        for (int i = 0; i < PhotonSampleNum; ++i) {
            auto [ray,power] = generatePhoton();
            TraceGlobalPhoton(ray, 0, power);
        }
    }

    void PhotonMapRender::createCausticsPhotonMap() {
        CausticspnMap = new PhotonMap(CausticPhotonSampleNum);
        for (int i = 0; i < PhotonSampleNum; ++i) {
            auto [ray, power] = generatePhoton();
            TraceCausticsPhoton(ray, 0, power);
        }
    }

    RGB PhotonMapRender::TraceWithPm(const Ray& r, int currDepth) {
        if (currDepth == depth) return Vec3{ 0 };
        auto hitObject = closestHitObject(r);
        auto [t, emitted] = closestHitLight(r);
        // hit object
        if (hitObject && hitObject->t < t) {
            // cout << r.origin << " " << hitObject->hitPoint << "\n";
            auto mtlHandle = hitObject->material;

            auto scattered = shaderPrograms[mtlHandle.index()]->shade(r, hitObject->hitPoint, hitObject->normal);
            if (scattered.is_specular) {//镜面反射/折射
                auto scatteredRay = scattered.ray;
                auto attenuation = scattered.attenuation;
                auto emitted = scattered.emitted;
                auto next = TraceWithPm(scatteredRay, currDepth + 1);
                float n_dot_in = glm::dot(hitObject->normal, scatteredRay.direction);
                float pdf = scattered.pdf;

                //考虑折射 
                RGB refraction_result{ 0.0f,0.0f,0.0f };
                if (scattered.has_refraction && currDepth <= 2) {
                    auto r_pdf = scattered.r_pdf;
                    auto r_attenuation = scattered.r_attenuation;
                    auto refraction_ray = scattered.r_ray;
                    // cout << "refraction\n";
                    auto refraction_next = TraceWithPm(refraction_ray, currDepth + 1);
                    float r_n_dot_in = glm::dot(hitObject->normal, refraction_ray.direction);
                    // BTDF
                    refraction_result += refraction_next * abs(r_n_dot_in) * r_attenuation / r_pdf;
                }
                return emitted + attenuation * next * abs(n_dot_in) / pdf + refraction_result;
            }
            else {//漫反射
                //计算漫反射表面的直接照明(直接对光源采样)
                Vec3 DirectLight(0.f, 0.f, 0.f);
                int direct_cnt = 0;
                for (int i = 0; i < 4; ++i) {
                    auto [tag, L] = SampleDirectLight(hitObject->hitPoint, hitObject->normal, scattered.attenuation);
                    if (tag) {
                        direct_cnt += 1;
                    }
                    DirectLight += L;
                }
                if (direct_cnt != 0) {
                    DirectLight /= direct_cnt;
                }

                //计算焦散
                Vec3 CausticsLight = CausticspnMap->DensityEstimates(hitObject->hitPoint, scattered.attenuation, true);
                //计算间接漫反射， 
                //额外反射一次，估计击中位置附近的全局光子（多次重复该过程，同时该反射不能击中光源）
                Vec3 IndirectDiffUseLight(0.f, 0.f, 0.f);
                int diffuse_cnt = 0;
                for (int i = 0; i < 4; ++i) {
                    auto [tag, L] = SampleIndirectDiffUseLight(hitObject->hitPoint, hitObject->normal, scattered.attenuation);
                    if (tag) {
                        diffuse_cnt += 1;
                    }
                    IndirectDiffUseLight += L;
                }
                if (diffuse_cnt != 0) {
                    IndirectDiffUseLight /= diffuse_cnt;
                }
                return DirectLight + IndirectDiffUseLight + CausticsLight;
            }
        }
        //击中光源
        else if (t != FLOAT_INF) {
            return emitted;
        }
        else {
            return getAmbientRGB(r);
        }
    }

    RGB gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }

    void PhotonMapRender::PathTracingWithPhotonMapTask(RGBA* pixels, int width, int height, int off, int step) {
        for (int i = off; i < height; i += step) {
            for (int j = 0; j < width; j++) {
                Vec3 color(0.f, 0.f, 0.f);
                for (int k = 0; k < samples; k++) {
                    auto r = defaultSamplerInstance<UniformInSquare>().sample2d();
                    float rx = r.x;
                    float ry = r.y;
                    float x = (float(j) + rx) / float(width);
                    float y = (float(i) + ry) / float(height);
                    auto ray = camera.shoot(x, y);
                    color += TraceWithPm(ray, 0);
                }
                color /= samples;
                color = gamma(color);
                pixels[(height - i - 1) * width + j] = { color, 1 };
                
            }
        }
    }

    void PhotonMapRender::PathTracingWithPhotonMap(RGBA* pixels) {
        const auto taskNums = 8;
        thread t[taskNums];
        for (int i = 0; i < taskNums; i++) {
            t[i] = thread(&PhotonMapRender::PathTracingWithPhotonMapTask,
                this, pixels, width, height, i, taskNums);
        }
        for (int i = 0; i < taskNums; i++) {
            t[i].join();
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

    tuple<float, Vec3> PhotonMapRender::closestHitLight(const Ray& r) {
        Vec3 v = {};
        HitRecord closest = getHitRecord(FLOAT_INF, {}, {}, {});
        for (auto& a : scene.areaLightBuffer) {
            auto hitRecord = Intersection::xAreaLight(r, a, 0.000001, closest->t);
            if (hitRecord && closest->t > hitRecord->t) {
                closest = hitRecord;
                v = a.radiance;
            }
        }
        return { closest->t, v };
    }


    

}