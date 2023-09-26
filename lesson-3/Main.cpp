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

vec3f mine_cross(vec3f &X, vec3f &Y){
    vec3f cross_{
            X.y*Y.z - X.z*Y.y,
            X.z*Y.x - X.x*Y.z,
            X.x*Y.y - X.y*Y.x
    };

    return cross_;
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

    //auto cross_ = cross(X, Y);
    auto cross_ = mine_cross(X, Y);
    double u = (cross_.x / cross_.z);
    double v = (cross_.y / cross_.z);

    //还有一个重要问题是，返回的重心坐标的顺序
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

            // 这一点要特别注意，你使用 p点 作为一个类似迭代器的东西在box的每个像素上遍历
            // 要记得 每次都要清空之前的 像素的状态， 比如z值
            p.z = 0;

            //直接使用像素点的重心坐标， 计算出z的插值
            //三角形顶点的z，还是(-1, 1)范围， 所以记得zbuffer 要用float
            for (int i = 0; i < 3; ++i) p.z += (bc_screen[i] * ps[i][2]);

            vec2f uv{0, 0};// this is also only for one pixel.
            for (int i = 0; i < 3; ++i) {
                uv.x += (bc_screen[i] * uvs[i].x);
                uv.y += (bc_screen[i] * uvs[i].y);
            };

            TGAColor color = model.diffuse(uv) * intensity;


            if (zbuffer[int(p.x + p.y * w)] < p.z) {
                zbuffer[int(p.x + p.y * w)] = p.z;
                image.set(p.x, p.y, color);
            }
        }
    }
}
void lambert_textured_lighting(vec3f light_dir, Model &model, TGAImage &image){

    for (int i = 0; i < model.nfaces(); ++i) {

        //------------------------------------------------------------
        // 我们将屏幕坐标用vec3中的xy表示, 顺便携带了z的信息,
        // 因为z保留为-1 - 1, 所以要用float形式,
        // 就连同xy一起以float形式保存, 即使转为屏幕坐标后, xy是int类型

        vec3f screen_coords[3];
        vec3f world_coords[3];
        vec2f uv_coords[3];

        for (int j = 0; j < 3; ++j) {
            vec3f v = model.vert(i, j);

            // 因为到目前为止，我们都是在正交投影，还没有加入透视投影，
            // 所以我们就是直接利用模型的xy坐标进行绘制三角形和平行投影，
            // 由于模型的尺寸是在-1到1, 所以我们把它放大到图片的尺寸空间, 就是图片的宽度和高度
            // 将每个顶点的xy转换到屏幕空间范围, 但是z保持不变 (-1, 1)
            screen_coords[j] = {(v.x+1)/2*image.get_width(),
                                (v.y+1)/2*image.get_height(),
                                v.z };

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
    vec3f light_dir{0, 0, -1};
    lambert_textured_lighting(light_dir, model, image);
    //FIXME : some small black point in result.
    image.write_tga_file("out.tga");
    return 0;
}
