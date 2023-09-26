//
// Created by tsin on 22-5-25.
//

#ifndef SOFT_RENDER_DEPTHBUFFERSHADER_H
#define SOFT_RENDER_DEPTHBUFFERSHADER_H

#include "IOurGL.h"

class DepthBufferShader : public IOurGL{
public:
    DepthBufferShader(Model *model, TGAImage *rt, int rt_scale_w, int rt_scale_h, vec3f eye, vec3f center);

    vec3f vertex(int i) override;

    void fragment(mat<3,3> gl_vertexes, vec3f barycenter, TGAColor &color) override;

    mat4 get_mvp(){
        return mvp;
    }
    mat4 mvp;
};



#endif //SOFT_RENDER_DEPTHBUFFERSHADER_H
