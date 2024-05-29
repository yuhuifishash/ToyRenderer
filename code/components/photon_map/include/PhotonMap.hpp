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
        Vec3 Pos;
        Vec3 Direction;
        Vec3 Power;
    };

    class PhotonMap
    {
    public:
        PhotonMap();
        ~PhotonMap();
        int PhotonNum;
        int maxPhotonNum;
        Photon* PhotonM;
        //¹â×ÓÍ¼µÄ·¶Î§
        Vec3 box_min;
        Vec3 box_max;
        void StorePhoton(Photon* p);
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
        }
        ~PhotonMapRender() = default;

        using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
        RenderResult render();

        tuple<Ray, float> generatePhoton(HitRecord hit);// <ray, power_scale>
        void generatePhotonMap();
        void TracePhoton(const Ray& r, int currDepth, Vec3 power);
        HitRecord closestHitObject(const Ray& r);
    };
}

#endif