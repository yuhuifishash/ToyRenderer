#pragma once
#ifndef __SIMPLE_PATH_TRACER_HPP__
#define __SIMPLE_PATH_TRACER_HPP__

#include "scene/Scene.hpp"
#include "Ray.hpp"
#include "Camera.hpp"
#include "intersections/HitRecord.hpp"
#include "shaders/ShaderCreator.hpp"
#include "PhotonKdTree.hpp"

#include <tuple>
namespace PhotonMap
{
    using namespace NRenderer;
    using namespace std;

    class Photon
    {
    public:
        Vec3 Pos;
        Vec3 Direction;
        Vec3 Power;
    };

    class PhotonMap
    {
    public:
        PhotonMap(int photonSampleNum);
        ~PhotonMap();
        int PhotonNum;
        int PhotonSampleNum;
        int maxPhotonNum;
        Photon* PhotonM;
        //光子图的范围
        Vec3 box_min;
        Vec3 box_max;

        PhotonKdTree* KdTree;
        void BuildKdTree();

        void StorePhoton(Photon p);
        tuple<std::vector<Photon*>, float> GetNearestNPhotons(const Vec3& pos, int N, float R2);
        tuple<std::vector<Photon*>, float> GetNearestNPhotonsDebug(const Vec3& pos, int N, float R2);

        const int EstimatesN = 500;
        const int CausticsEstimatesN = 50;

        const float EstimmatesR2 = 100;
        RGB DensityEstimates(const Vec3& pos, const Vec3& BRDF, bool is_Caustics);

        void PrintPhotonMap();
        void KdTreeTest();//调用前需要保证PhotonNum为0
    };

    class PhotonMapRender
    {
    public:
    private:
        SharedScene spScene;
        Scene& scene;
        //PhotonMap photon_map;

        unsigned int width;
        unsigned int height;
        unsigned int depth;
        unsigned int samples;

        using SCam = Camera;
        SCam camera;

        vector<SharedShader> shaderPrograms;
        PhotonMap* GlobalpnMap;
        PhotonMap* CausticspnMap;

        const int PhotonSampleNum = 200000;
        const int CausticPhotonSampleNum = 50000;

    public:
        PhotonMapRender(SharedScene spScene)
            : spScene(spScene)
            , scene(*spScene)
            , camera(spScene->camera)
        {
            width = scene.renderOption.width;
            height = scene.renderOption.height;
            depth = scene.renderOption.depth;
            samples = scene.renderOption.samplesPerPixel;
            GlobalpnMap = nullptr;
            CausticspnMap = nullptr;
        }
        ~PhotonMapRender() 
        {
            if (GlobalpnMap != nullptr) {
                delete  GlobalpnMap;
            }
            if (CausticspnMap != nullptr) {
                delete CausticspnMap;
            }
        };

        using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
        RenderResult render();

        tuple<Ray, Vec3> generatePhoton();// <ray, power>
        void createGlobalPhotonMap();
        void createCausticsPhotonMap();
        void PathTracingWithPhotonMap(RGBA* pixels);
        void PathTracingWithPhotonMapTask(RGBA* pixels, int width, int height, int off, int step);//用于并行
        void TraceGlobalPhoton(const Ray& r, int currDepth, Vec3 power);
        void TraceCausticsPhoton(const Ray& r, int currDepth, Vec3 power);

        RGB TraceWithPm(const Ray& r, int currDepth);
        tuple<bool, RGB> SampleDirectLight(const Vec3& hitPoint, const Vec3& normal, const Vec3& BRDF);
        tuple<bool, RGB> SampleIndirectDiffUseLight(const Vec3& hitPoint, const Vec3& normal, const Vec3& BRDF);

        RGB getAmbientRGB(const Ray& r);
        HitRecord closestHitObject(const Ray& r);
        tuple<float, Vec3> closestHitLight(const Ray& r);
    };
}

#endif