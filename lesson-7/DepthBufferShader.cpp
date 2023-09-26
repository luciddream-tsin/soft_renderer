//
// Created by tsin on 22-5-25.
//

#include "DepthBufferShader.h"

DepthBufferShader::DepthBufferShader(Model *model, TGAImage *rt, int rt_scale_w, int rt_scale_h, vec3f eye, vec3f center):
        IOurGL(model, rt, rt_scale_w, rt_scale_h, eye, center, vec3f(0, 0, 0)){
    // 注意 depth buffer shader 中 light_dir 直接作为相机的center传入, 光照方向就是视角方向, 他不需要真的光照方向计算着色,
    // 因此我们把传给IOurGL的真的用于计算光照着色的light_dir设为0
}
vec3f DepthBufferShader::vertex(int i) {

    vec3f vert = uniform_vertexes[i];

    // 在经过mvp矩阵之前, 顶点还是(-1, 1)的
    // 只是在vp也就是viewport矩阵我们把xy转换到窗口的0-width和0-height, z从(-1, 1)转换到(0, 1)
    mvp = vp * pj *  mv;

    auto coord  = embed<4>(  mvp * embed<4>(vert));
    if (coord[1] < 0) {
        int sdf = 3;
    }
    //model_view and projection, you should devide w
    vert.x = coord[0]/coord[3];
    vert.y = coord[1]/coord[3];
    vert.z = coord[2]/coord[3];

    return vert;
}

void DepthBufferShader::fragment(mat<3,3>gl_vertexes, vec3f barycenter, TGAColor &color) {
    double z = 0;
    for (int i = 0; i < 3; ++i) z += (barycenter[i] * gl_vertexes[i][2]);

    color = TGAColor(255, 255, 255) * z; // 深度图转换到(0, 255)
}