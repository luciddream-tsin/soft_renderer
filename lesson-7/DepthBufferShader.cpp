//
// Created by tsin on 22-5-25.
//

#include "DepthBufferShader.h"

DepthBufferShader::DepthBufferShader(Model *model, TGAImage *rt, int rt_scale_w, int rt_scale_h, vec3f eye, vec3f center, vec3f light_dir):
        IOurGL(model, rt, rt_scale_w, rt_scale_h, eye, center, light_dir){

}
vec3f DepthBufferShader::vertex(int i) {
    vec3f  vert = uniform_vertexes[i];
    mvp = vp * pj *  mv;
    auto coord  = embed<4>(  mvp * embed<4>(vert));
    //model_view and projection, you should devide w
    vert.x = coord[0]/coord[3];
    vert.y = coord[1]/coord[3];
    vert.z = coord[2]/coord[3];
    return vert;
}

void DepthBufferShader::fragment(mat<3,3>gl_vertexes, vec3f barycenter, TGAColor &color) {
    double z = 0;
    for (int i = 0; i < 3; ++i) z += (barycenter[i] * gl_vertexes[i][2]);
    color = TGAColor(255, 255, 255) * (z/500.f );
}