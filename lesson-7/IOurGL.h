//
// Created by tsin on 22-5-25.
//

#ifndef SOFT_RENDER_IOURGL_H
#define SOFT_RENDER_IOURGL_H


#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

class IOurGL {

public:
    IOurGL(Model *model, TGAImage *rt, int rt_scale_w, int rt_scale_h, vec3f eye, vec3f center, vec3f light_dir);

    void run();

    virtual vec3f vertex(int i) = 0;
    void triangle(mat<3,3>vertx);

    virtual void fragment(mat<3,3> gl_vertexes, vec3f barycenter, TGAColor &color) = 0;

    TGAColor sample2D(vec2f uv, double intensity);

private:
    void modelview();
    void projection();
    void viewport();

protected:
    mat4 mv, pj, vp;

    vec3f uniform_vertexes[3];
    vec2f uniform_uvs[3];

    std::vector<double> zbuffer;

    vec3f light_dir;

private:
    vec3f eye;
    vec3f center;
    vec3f up;


    Model *model;
    TGAImage *rt;
    int dx, dy;
    int rt_w, rt_h;
    int rt_scale_w, rt_scale_h;
};


#endif //SOFT_RENDER_IOURGL_H
