//
// Created by tsin on 22-5-25.
//

#include "BasePassShader.h"

BasePassShader::BasePassShader(Model *model, TGAImage *rt, int rt_scale_w, int rt_scale_h, vec3f eye, vec3f center, vec3f light_dir):
        IOurGL(model, rt, rt_scale_w, rt_scale_h, eye, center, light_dir){

}
vec3f BasePassShader::vertex(int i) {
    vec3f  vert = uniform_vertexes[i];
    auto coord  = embed<4>(  vp * pj *  mv * embed<4>(vert));
    //you should devide w
    vert.x = coord[0]/coord[3];
    vert.y = coord[1]/coord[3];
    vert.z = coord[2]/coord[3];
    return vert;
}

void BasePassShader::fragment(mat<3,3> gl_vertexes, vec3f barycenter, TGAColor &color) {

    //----------------------------
    vec3f n = cross(uniform_vertexes[2] - uniform_vertexes[0],
                    uniform_vertexes[1] - uniform_vertexes[0]).normalize();
    double intensity = dot(n, light_dir);


    //----------------------------
    vec4f sb_p = shadow_mvp * embed<4>(gl_vertexes * barycenter);  // corresponding point in the shadow buffer
    sb_p = sb_p / sb_p[3];

    auto c = shadow_map.get(int(sb_p[0]), int(sb_p[1]))[0];
    double shadow = .3 + .7 * ( c <=
                              sb_p[2]-170);  // magic coeff to avoid z-fighting


    //-----------------------------------
    vec2f uv{0, 0};//this is also only for one pixel.
    for (int i = 0; i < 3; ++i) {
        uv.x += (barycenter[i] * uniform_uvs[i].x);
        uv.y += (barycenter[i] * uniform_uvs[i].y);
    };
    auto cc = sample2D(uv, intensity);
    color = cc * shadow;
}