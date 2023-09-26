//
// Created by tsin on 22-5-25.
//

#include <limits>
#include "IOurGL.h"
vec3f barycenter(mat<3,3> ps, vec2f p){// we don't use z of three ps to calculate barycenter.
    //                B
    //              /
    //             /
    //            /  .P
    //           A------ C
    // A:[0], B:[1], C:[2]

    //------------------------ look the doc , about this setting of X and Y.------------------------
    vec3f X {(ps[2].x - ps[0].x), //AB.x
             (ps[1].x - ps[0].x), //AC.x
             (ps[0].x - p.x)};    //PA.x

    vec3f Y {(ps[2].y - ps[0].y), //AB.y
             (ps[1].y - ps[0].y), //AC.y
             (ps[0].y - p.y)};    //PA.y

    auto cross_ = cross(X, Y);
    double u = (cross_.x / cross_.z);
    double v = (cross_.y / cross_.z);
    //return vec3f{1-u-v, u, v};
    return vec3f {1-u-v,v,u};
}

IOurGL::IOurGL(Model *model, TGAImage *rt, int rt_scale_w, int rt_scale_h, vec3f eye, vec3f center, vec3f light_dir)
        :model(model), rt(rt), rt_w(rt->get_width()), rt_h(rt->get_height()),
         rt_scale_w(rt_scale_w),
         rt_scale_h(rt_scale_h), eye(eye), center(center), light_dir(light_dir){

    // 如果传进来的用于保存渲染结果的rt图, 比我实际渲染的viewport视图大
    // 我就采用rt图居中的一部分写入渲染结果, 也就是边框空白
    this->dx = (rt_w - rt_scale_w) / 2;
    this->dy = (rt_h - rt_scale_h) / 2;

    up = vec3f {0, 1, 0};

    this->modelview();
    this->viewport();
    this->projection();


    zbuffer = std::vector<double>(rt_w * rt_h, -std::numeric_limits<double>::max());
}
void IOurGL::triangle(mat<3,3> gl_vertexes) {
    vec2i minbox{rt_w - 1, rt_h - 1};
    vec2i maxbox{0, 0};

    for (int i = 0; i < 3; ++i) {
        //--min
        minbox.x = std::min(minbox.x, (int)gl_vertexes[i].x);
        minbox.y = std::min(minbox.y, (int)gl_vertexes[i].y);
        //--max
        maxbox.x = std::max(maxbox.x, (int)gl_vertexes[i].x);
        maxbox.y = std::max(maxbox.y, (int)gl_vertexes[i].y);
    }

    vec3f p{0, 0, 0};
    for (p.x = minbox.x; p.x <= maxbox.x; ++p.x) {
        for (p.y = minbox.y; p.y <= maxbox.y; ++p.y) {

            vec3f bc_screen = barycenter(gl_vertexes, vec2f{p.x, p.y});

            // 这一点要特别注意，你使用 p点 作为一个类似迭代器的东西在box的每个像素上遍历
            // 要记得 每次都要清空之前的 像素的状态， 比如z值
            p.z = 0;

            //直接使用像素点的重心坐标， 计算出z的插值
            //三角形顶点的z，还是(-1, 1)范围， 所以记得z-buffer 要用float

            for (int i = 0; i < 3; ++i) {
                p.z += (bc_screen[i] * gl_vertexes[i][2]);
            }

            int pixel_zbuffer_index = p.x + p.y * rt_w;
            if (pixel_zbuffer_index == -714){
                int hs = 10;
            }

            auto &existed_pixel_z = zbuffer.at(pixel_zbuffer_index);

            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0 || p.z < existed_pixel_z) {
                continue;
            } existed_pixel_z = p.z;

            TGAColor color;
            this->fragment(gl_vertexes, bc_screen, color);

            rt->set(p.x, p.y, color);
        }
    }
}

TGAColor IOurGL::sample2D(vec2f uv, double intensity) {
    auto u = model->diffuse(uv) ;
    auto c = u * intensity;
    return c;
}

void IOurGL::run() {


    for (int i = 0; i < model->nfaces(); ++i) {

        mat<3, 3> gl_vertexes;

        for (int j = 0; j < 3; ++j) {
            uniform_vertexes[j] = model->vert(i, j);
            uniform_uvs[j] = model->uv(i, j);
            gl_vertexes[j] = this->vertex(j);
        }

        this->triangle(gl_vertexes);

    }
}


void IOurGL::modelview(){
    //---------camera view (look at)------------
    // todo eye - center 还是center - eye 要和你假定的坐标系一致
    // 就是你假定屏幕重心的摄像机看向z轴负方向, 你这个也要遵守同样的标准
    vec3f z = (eye - center).normalize();
    //note the order of cross v1 and v2.
    vec3f x = cross(up, z).normalize();
    vec3f y = cross( z, x).normalize();

    mat4 view = mat4::identity();


    for (int i = 0; i < 3; ++i) {

        view[0][i] = x[i];
        view[1][i] = y[i];
        view[2][i] = z[i];

        // todo
        view[i][3] = -center[i];

    }
    mv = view;
}
void IOurGL::viewport(){
    mat4 m = mat4::identity();

    m[0][0] = rt_scale_w / 2.; // change x from (-1, 1) to (0, width), 由这一行代码和 m[0][3]哪行共同实现
    m[1][1] = rt_scale_h / 2.;
    m[2][2] = 0.5f; // change z to 0-depth for shadow mapping, 由这一行代码和 m[2][3]哪行共同实现

    m[0][3] = dx + rt_scale_w / 2.;
    m[1][3] = dy + rt_scale_h / 2.;
    m[2][3] = 0.5f;


    vp = m;
}
void IOurGL::projection()
{
    mat4 matrix = mat4::identity();

    //projection
    matrix[3][2] = -1.f / (eye - center).norm();

    pj = matrix;
}

