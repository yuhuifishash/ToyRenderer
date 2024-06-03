#pragma once
#ifndef __SIMPLE_PATH_TRACER_HPP__
#define __SIMPLE_PATH_TRACER_HPP__

#include "scene/Scene.hpp"
#include "Ray.hpp"
#include "Camera.hpp"
#include "intersections/HitRecord.hpp"
#include "shaders/ShaderCreator.hpp"

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
        int maxPhotonNum;
        Photon* PhotonM;
        //光子图的范围
        Vec3 box_min;
        Vec3 box_max;
        void StorePhoton(Photon p);
        std::vector<Photon> GetNearestNPhotons(Vec3 pos, int N, int R);
        RGB DensityEstimates(Vec3 pos);
        void PrintPhotonMap();
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
        }
        ~PhotonMapRender() 
        {
            if (GlobalpnMap != nullptr) {
                delete  GlobalpnMap;
            }
        };

        using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
        RenderResult render();

        tuple<Ray, Vec3> generatePhoton();// <ray, power>
        void createGlobalPhotonMap();
        void PathTracingWithPhotonMap(RGBA* pixels);
        void PathTracingWithPhotonMapTask(RGBA* pixels, int width, int height, int off, int step);//用于并行
        void TraceGlobalPhoton(const Ray& r, int currDepth, Vec3 power);
        RGB TraceWithPm(const Ray& r, int currDepth);
        RGB getAmbientRGB(const Ray& r);
        HitRecord closestHitObject(const Ray& r);
        tuple<float, Vec3> closestHitLight(const Ray& r);
    };
}

#endif