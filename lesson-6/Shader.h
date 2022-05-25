//
// Created by tsin on 22-5-25.
//

#ifndef SOFT_RENDER_BASEPASSSHADER_H
#define SOFT_RENDER_SHADER_H


#include "IOurGL.h"

class BasePassShader : public IOurGL{
public:
    BasePassShader(Model *model, TGAImage *rt, int rt_scale_w, int rt_scale_h, vec3f eye, vec3f center, vec3f light_dir);

    vec3f vertex(int i) override;

    void fragment(vec3f barycenter, TGAColor &color) override;
};


#endif //SOFT_RENDER_BASEPASSSHADER_H
