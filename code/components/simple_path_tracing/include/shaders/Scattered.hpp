#pragma once
#ifndef __SCATTERED_HPP__
#define __SCATTERED_HPP__

#include "Ray.hpp"

namespace SimplePathTracer
{
    struct Scattered
    {
        Ray ray = {};
        Vec3 attenuation = {};
        Vec3 emitted = {};
        float pdf = {0.f};

        bool has_refraction = false;
        Ray r_ray = {};
        Vec3 r_attenuation = {};
        float r_pdf = { 0.f };
     
    };
    
}

#endif