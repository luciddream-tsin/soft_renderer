//
// Created by tsin on 22-5-25.
//

#include "Shader.h"

Shader::Shader(Model *model, TGAImage *rt, int rt_scale_w, int rt_scale_h, vec3f eye, vec3f center, vec3f light_dir):
        IOurGL(model, rt, rt_scale_w, rt_scale_h, eye, center, light_dir){

}
vec3f Shader::vertex(int i) {
    vec3f  vert = uniform_vertexes[i];
    auto coord  = embed<4>(  pj *  mv * embed<4>(vert));
    //model_view and projection, you should devide w
    vert.x = coord[0]/coord[3];
    vert.y = coord[1]/coord[3];
    vert.z = coord[2]/coord[3];
    // viewport
    vert = embed<3>(vp * embed<4>(vert));
    return vert;
}

void Shader::fragment(vec3f barycenter, TGAColor &color) {

    vec3f n = cross(uniform_vertexes[2] - uniform_vertexes[0],
                    uniform_vertexes[1] - uniform_vertexes[0]).normalize();

    double intensity = dot(n, light_dir);

    vec2f uv{0, 0};//-----------------------------------this is also only for one pixel.
    for (int i = 0; i < 3; ++i) {
        uv.x += (barycenter[i] * uniform_uvs[i].x);
        uv.y += (barycenter[i] * uniform_uvs[i].y);
    };
    auto cc = sample2D(uv, intensity);
    color = cc;
}