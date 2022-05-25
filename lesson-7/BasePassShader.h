//
// Created by tsin on 22-5-25.
//

#ifndef SOFT_RENDER_BASEPASSSHADER_H
#define SOFT_RENDER_BASEPASSSHADER_H


#include "IOurGL.h"

class BasePassShader : public IOurGL{
public:
    BasePassShader(Model *model, TGAImage *rt, int rt_scale_w, int rt_scale_h, vec3f eye, vec3f center, vec3f light_dir);

    vec3f vertex(int i) override;

    void fragment(mat<3,3> gl_vertexes, vec3f barycenter, TGAColor &color) override;

    void set_mvp(mat4 shadow_mvp){
        this->shadow_mvp = shadow_mvp * (vp * pj * mv).invert();
    }

    void set_shadow_map(TGAImage &image){
        shadow_map = image;
    }

    TGAImage shadow_map;

    mat4 shadow_mvp;
};


#endif //SOFT_RENDER_BASEPASSSHADER_H
