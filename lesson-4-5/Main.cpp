//
// Created by tsin on 22-5-24.
//

#include <limits>
#include "model.h"
#include "tgaimage.h"
#include "geometry.h"
#include "numeric"

const int w = 800;
const int h = 800;

//define zbuffer globally.
std::vector<double> zbuffer;

//FIXME : when the direction is {-1, -1, -1}, som black face loss in front.
vec3f light_dir = vec3f{0, 0, -1}.normalize();

// TODO 修复位置很远时模型不缩小问题
// TODO 修复不能随意调整center和eye问题
vec3f eye{0, 0, 1.2}; // position, z not too small , inner the model will wrong.
vec3f center{0, 0, 0};// look at

mat4 modelview(vec3f eye, vec3f center, vec3f up){
    //-------------------model------------------

    //---------camera view (look at)------------
    vec3f z = (eye - center).normalize();
    //note the order of cross v1 and v2.
    vec3f x = cross(up, z).normalize();
    vec3f y = cross(z, x).normalize();

    mat4 view = mat4::identity();
    for (int i = 0; i < 3; ++i) {
       view[0][i] = x[i];
       view[1][i] = y[i];
       view[2][i] = z[i];

       view[i][3] = -center[i];
    }
    return view;
}
mat4 viewport(int dx, int dy, int rt_w, int rt_h){
    mat4 m = mat4::identity();
    m[0][0] = rt_w / 2.;
    m[1][1] = rt_h / 2.;
    m[2][2] = 1.0;

    m[0][3] = dx + rt_w / 2.;
    m[1][3] = dy + rt_h / 2.;
    m[2][3] = 0.0;

    return m;
}

mat4 projection()
{
    mat4 matrix = mat4::identity();

    //projection
    //FIXME : when the eye is very far, but the head not very small
    matrix[3][2] = -1.f / (eye - center).norm();
    return matrix;
}


//
vec3f barycenter(vec3f *ps, vec2f p){// we don't use z of three ps to calculate barycenter.
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

void triangle(vec3f *ps, vec2f *uvs, TGAImage &image, double intensity, Model &model){
    vec2i minbox{(int)image.get_width() - 1, (int)image.get_height() - 1};
    vec2i maxbox{0, 0};

    for (int i = 0; i < 3; ++i) {
        //--min
        minbox.x = std::min(minbox.x, (int)ps[i].x);
        minbox.y = std::min(minbox.y, (int)ps[i].y);
        //--max
        maxbox.x = std::max(maxbox.x, (int)ps[i].x);
        maxbox.y = std::max(maxbox.y, (int)ps[i].y);
    }

    vec3f p{0, 0, 0};
    for (p.x = minbox.x; p.x <= maxbox.x; ++p.x) {
        for (p.y = minbox.y; p.y <= maxbox.y; ++p.y) {
            //
            vec3f bc_screen = barycenter(ps, vec2f{p.x, p.y});

            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0){continue;}

            p.z = 0;//--------------------------------------------------this is very important, because it's only for one pixel.
            for (int i = 0; i < 3; ++i) p.z += (bc_screen[i] * ps[i][2]);

            vec2f uv{0, 0};//-----------------------------------this is also only for one pixel.
            for (int i = 0; i < 3; ++i) {
                uv.x += (bc_screen[i] * uvs[i].x);
                uv.y += (bc_screen[i] * uvs[i].y);
            };
            TGAColor color = model.diffuse(uv) * intensity;

            if (zbuffer.at(int(p.x + p.y * w)) < p.z) { // use 'at' check out of range.
                zbuffer[int(p.x + p.y * w)] = p.z;
                image.set(p.x, p.y, color);
            }
        }
    }
}
void lambert_textured_lighting(vec3f light_dir, Model &model, TGAImage &image){

    //model and view matrix
    mat4 mv = modelview(eye, center, vec3f(0, 1, 0));

    //projection matrix
    mat4 pj = projection();

    //view port matrix to screen
    mat4 vp = viewport(w / 8, h / 8, w * 3 / 4, h * 3 / 4);

    for (int i = 0; i < model.nfaces(); ++i) {

        //------------------------------------------------------------


        vec3f screen_coords[3];
        vec3f world_coords[3];
        vec2f uv_coords[3];

        for (int j = 0; j < 3; ++j) {
            vec3f v = model.vert(i, j);


            // 将每个顶点的xy转换到屏幕空间范围, 已经在视口矩阵完成
            // screen_coords[j] = {
            //      (v.x+1)/2*image.get_width(),
            //      (v.y+1)/2*image.get_height(),
            //      v.z
            // };


            auto coord  = embed<4>(  pj *  mv * embed<4>(v));
            //model_view and projection, you should devide w
            screen_coords[j].x = coord[0]/coord[3];
            screen_coords[j].y = coord[1]/coord[3];
            screen_coords[j].z = coord[2]/coord[3];

            // viewport
            screen_coords[j] = embed<3>(vp * embed<4>(screen_coords[j]));

            uv_coords[j] = model.uv(i, j);

            //而模型的法线还得用原始的，三角形面的法线等，
            //我们还是利用模型原始的数值进行计算, 为了减小误差，这也比较合理
            world_coords[j] = v;
        }

        //------------------------------------------------------------
        vec3f n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
        n.normalize();
        double intensity = dot(n, light_dir);

        //------------------------------------------------------------
        if (intensity > 0) // 背面剔除
            triangle(screen_coords, uv_coords, image, intensity, model);
    }
}



int main()
{
    //remember to turn texture on! (and normal spec, if you need)
    Model model{"../obj/african_head.obj", true};
    TGAImage image(w, h, TGAImage::RGB);
    zbuffer = std::vector<double>(w * h, -std::numeric_limits<double>::max());

    lambert_textured_lighting(light_dir, model, image);
    //FIXME : some small black point in result.
    image.write_tga_file("out.tga");
    return 0;
}
