
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */


// cameras/fisheye.cpp*
#include "stdafx.h"
#include "cameras/fisheye.h"
#include "paramset.h"
#include "sampler.h"

// FisheyeCamera Method Definitions
float FisheyeCamera::GenerateRay(const CameraSample &sample,
                                     Ray *ray) const {
    // Compute fisheye camera ray direction
    // Generate raster and camera samples
    Point Pras(sample.imageX, sample.imageY, 0);
    Point Pcamera;
    RasterToCamera(Pras, &Pcamera);

    float u = sample.imageX / film->xResolution;
    float v = sample.imageY / film->yResolution;
    float rmax = 2.0f * lensRadius * sinf(FieldOfView * 0.25f);
    float r = sqrt(u*u + v*v);

    if (r > rmax) {
        Vector dir (0.f, 0.f, 0.f);
        *ray = Ray(Point(0,0,0), dir, 0.f, INFINITY);
    }
    else {
        float phi = acosf((r != 0.0f)? u/r: 0.0f);
        float theta = 2.0f * asinf(r/(2.0f * lensRadius));

        if(v < 0.0f) phi = -phi;

        Vector dir(cosf(theta), -cosf(phi)*sinf(theta),
                   sinf(phi) * sinf(theta));

        *ray = Ray(Point(0,0,0), dir, 0.f, INFINITY);
    }

    // Modify ray for depth of field
    if (lensRadius > 0.) {
    }

    ray->time = sample.time;
    CameraToWorld(*ray, ray);
    return 1.f;
}


FisheyeCamera *CreateFisheyeCamera(const ParamSet &params,
        const AnimatedTransform &cam2world, Film *film) {
    // Extract common camera parameters from _ParamSet_
    float shutteropen = params.FindOneFloat("shutteropen", 0.f);
    float shutterclose = params.FindOneFloat("shutterclose", 1.f);
    if (shutterclose < shutteropen) {
        Warning("Shutter close time [%f] < shutter open [%f].  Swapping them.",
                shutterclose, shutteropen);
        swap(shutterclose, shutteropen);
    }
    float lensradius = params.FindOneFloat("lensradius", 0.f);
    float focaldistance = params.FindOneFloat("focaldistance", 1e30f);
    float frame = params.FindOneFloat("frameaspectratio",
        float(film->xResolution)/float(film->yResolution));
    float screen[4];
    if (frame > 1.f) {
        screen[0] = -frame;
        screen[1] =  frame;
        screen[2] = -1.f;
        screen[3] =  1.f;
    }
    else {
        screen[0] = -1.f;
        screen[1] =  1.f;
        screen[2] = -1.f / frame;
        screen[3] =  1.f / frame;
    }
    int swi;
    const float *sw = params.FindFloat("screenwindow", &swi);
    if (sw && swi == 4)
        memcpy(screen, sw, 4*sizeof(float));
    float fov = params.FindOneFloat("fov", 90.);
    return new FisheyeCamera(cam2world, screen, shutteropen,
        shutterclose, lensradius, focaldistance, fov, film);
}
