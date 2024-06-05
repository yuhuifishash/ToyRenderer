#include "server/Server.hpp"

#include "SimplePathTracer.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace SimplePathTracer
{
    RGB SimplePathTracerRenderer::getAmbientRGB(const Ray& r) {
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
            return texture.rgba[y*texture.width + x];
        }
        return { 0.f,0.f,0.f };
    }

    RGB SimplePathTracerRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }

    void SimplePathTracerRenderer::renderTask(RGBA* pixels, int width, int height, int off, int step) {
        for(int i=off; i<height; i+=step) {
            for (int j=0; j<width; j++) {
                Vec3 color{0, 0, 0};
                for (int k=0; k < samples; k++) {
                    auto r = defaultSamplerInstance<UniformInSquare>().sample2d();
                    float rx = r.x;
                    float ry = r.y;
                    float x = (float(j)+rx)/float(width);
                    float y = (float(i)+ry)/float(height);
                    auto ray = camera.shoot(x, y);
                    color += trace(ray, 0);
                }
                color /= samples;
                color = gamma(color);
                pixels[(height-i-1)*width+j] = {color, 1};
            }
        }
    }

    auto SimplePathTracerRenderer::render() -> RenderResult {
        // shaders
        shaderPrograms.clear();
        ShaderCreator shaderCreator{};
        for (auto& m : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(m, scene.textures));
        }

        RGBA* pixels = new RGBA[1ll*width*height]{};

        // 局部坐标转换成世界坐标
        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        const auto taskNums = 8;
        thread t[taskNums];
        for (int i=0; i < taskNums; i++) {
            t[i] = thread(&SimplePathTracerRenderer::renderTask,
                this, pixels, width, height, i, taskNums);
        }
        for(int i=0; i < taskNums; i++) {
            t[i].join();
        }
        getServer().logger.log("Done...");
        return {pixels, width, height};
    }

    void SimplePathTracerRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        delete[] p;
    }

    HitRecord SimplePathTracerRenderer::closestHitObject(const Ray& r) {
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
    
    tuple<float, Vec3> SimplePathTracerRenderer::closestHitLight(const Ray& r) {
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

    RGB SimplePathTracerRenderer::trace(const Ray& r, int currDepth) {
        if (currDepth == depth) return Vec3{ 0 };
        auto hitObject = closestHitObject(r);
        auto [ t, emitted ] = closestHitLight(r);
        // hit object
        if (hitObject && hitObject->t < t) {
            // cout << r.origin << " " << hitObject->hitPoint << "\n";
            auto mtlHandle = hitObject->material;
            auto scattered = shaderPrograms[mtlHandle.index()]->shade(r, hitObject->hitPoint, hitObject->normal);
            auto scatteredRay = scattered.ray;
            auto attenuation = scattered.attenuation;
            auto emitted = scattered.emitted;
            auto next = trace(scatteredRay, currDepth+1);
            float n_dot_in = glm::dot(hitObject->normal, scatteredRay.direction);
            float pdf = scattered.pdf;

            //对光源重要性采样
            bool is_sample_light = false;//是否对光源进行重要性采样

            //Vec3 next2{ 0.f,0.f,0.f };//对光源重要性采样结果
            //float pdf2 = 1;
            //float n_dot_in2 = 1;
            //if (!scattered.is_specular) {//漫反射表面，考虑对光源进行重要性采样
            //    //由于计算两次采样平均值，所以不需要判断光线是否被物体遮挡
            //    is_sample_light = true;
            //    //判断是否存在环境光
            //    if (scene.ambient.type != Ambient::Type::CONSTANT) {
            //        is_sample_light = false;
            //    }
            //    if (scene.ambient.constant.x > 1e-4 || scene.ambient.constant.y > 1e-4 || scene.ambient.constant.z > 1e-4) {
            //        is_sample_light = false;
            //    }
            //    if (scene.areaLightBuffer.size() < 1) {
            //        is_sample_light = false;
            //    }
            //    if (currDepth >= depth - 1) {
            //        is_sample_light = false;
            //    }
            //    if (is_sample_light) {//对光源进行重要性采样
            //        //首先选择其中一个面光源(这里我们直接平均随机选择一个)
            //        int len = scene.areaLightBuffer.size();
            //        auto rand = defaultSamplerInstance<UniformSampler>().sample1d();
            //        int id = round(rand * (len - 1));
            //        auto& light = scene.areaLightBuffer[id];

            //        //在平面上随机选择一个点
            //        Vec3 target = light.position;
            //        target = target + light.u * defaultSamplerInstance<UniformSampler>().sample1d();
            //        target = target + light.v * defaultSamplerInstance<UniformSampler>().sample1d();

            //        //计算光线方向
            //        Vec3 to_light = target - hitObject->hitPoint;
            //        to_light = glm::normalize(to_light);
            //        //需要保证反射光线与法线夹角为正
            //        if (glm::dot(to_light, hitObject->normal) < 0) {
            //            is_sample_light = false;
            //        }
            //        else {//发射一条光线
            //            auto hitLight = Intersection::xAreaLight(
            //                Ray{ hitObject->hitPoint,to_light }, light, 0.000001, FLOAT_INF);

            //            if (hitLight) {//必定击中光源或物体，但是防止浮点误差
            //                // cout << "hit Light\n";
            //                next2 = trace(Ray{ hitObject->hitPoint,to_light }, currDepth + 1);
            //                //计算pdf  ||x-x'||2
            //                pdf2 = glm::distance(hitLight->hitPoint,hitObject->hitPoint);
            //                pdf2 = pdf2 * pdf2;
            //                auto light_normal = glm::cross(light.u,light.v);
            //                pdf2 /= glm::length(light_normal);
            //                pdf2 /= len;
            //                light_normal = glm::normalize(light_normal);
            //                pdf2 /= abs(glm::dot(light_normal,to_light));
            //                n_dot_in2 = glm::dot(hitObject->normal,to_light);
            //            }
            //            else {
            //                is_sample_light = false;
            //            }
            //        } 
            //    }
            //}
            
            //考虑折射 
            RGB refraction_result{ 0.0f,0.0f,0.0f };
            if (scattered.has_refraction && currDepth <= 2) {
                auto r_pdf = scattered.r_pdf;
                auto r_attenuation = scattered.r_attenuation;
                auto refraction_ray = scattered.r_ray;
                // cout << "refraction\n";
                auto refraction_next = trace(refraction_ray, currDepth + 1);
                float r_n_dot_in = glm::dot(hitObject->normal, refraction_ray.direction);
                // BTDF
                refraction_result += refraction_next * abs(r_n_dot_in) * r_attenuation / r_pdf;
            }

            //if (is_sample_light) {//多重采样，求平均值
            //    return emitted + ((attenuation * next * abs(n_dot_in) / pdf
            //        + attenuation * next2 * abs(n_dot_in2) / pdf2) / 2.0f) + refraction_result;
            //}

            /**
             * emitted      - Le(p, w_0)
             * next         - Li(p, w_i)
             * n_dot_in     - cos<n, w_i>
             * atteunation  - BRDF
             * pdf          - p(w)
             **/
            return emitted + attenuation * next * abs(n_dot_in) / pdf + refraction_result;
        }
        // 
        else if (t != FLOAT_INF) {
            return emitted;
        }
        else {
            return getAmbientRGB(r);
        }
    }
}