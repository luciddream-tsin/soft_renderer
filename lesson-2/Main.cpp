//
// Created by tsin on 22-5-24.
//
#include "model.h"
#include "tgaimage.h"
#include "geometry.h"

const int w = 800;
const int h = 800;
vec3f mine_cross(vec3f &X, vec3f &Y){
    vec3f cross_{
            X.y*Y.z - X.z*Y.y,
            X.z*Y.x - X.x*Y.z,
            X.x*Y.y - X.y*Y.x
    };

    return cross_;
}

//
vec3f barycenter(vec2i *ps, vec2i p){
   //                B
   //              /
   //             /
   //            /  .P
   //           A------ C
   // A:[0], B:[1], C:[2]

   // look the doc , about this setting of X and Y.
    vec3f X {(double)(ps[1].x - ps[0].x), //AB.x
             (double)(ps[2].x - ps[0].x), //AC.x
             (double)(ps[0].x - p.x)};    //PA.x

    vec3f Y {(double)(ps[1].y - ps[0].y), //AB.y
             (double)(ps[2].y - ps[0].y), //AC.y
             (double)(ps[0].y - p.y)};    //PA.y

    //auto cross_ = cross(X, Y);
    auto cross_ = mine_cross(X, Y);
    double u = (cross_.x / cross_.z);
    double v = (cross_.y / cross_.z);
    return vec3f{u, v, 1 - u - v};
}

void triangle(vec2i *ps, TGAImage &image, TGAColor color){
    vec2i minbox{(int)image.get_width() - 1, (int)image.get_height() - 1};
    vec2i maxbox{0, 0};

    for (int i = 0; i < 3; ++i) {
        //--min
        minbox.x = std::min(minbox.x, ps[i].x);
        minbox.y = std::min(minbox.y, ps[i].y);
        //--max
        maxbox.x = std::max(maxbox.x, ps[i].x);
        maxbox.y = std::max(maxbox.y, ps[i].y);
    }

    vec2i p;
    for (p.x = minbox.x; p.x < maxbox.x; ++p.x) {
        for (p.y = minbox.y; p.y < maxbox.y; ++p.y) {
            vec3f bc = barycenter(ps, p);
            if (bc.x < 0 || bc.y < 0 || bc.z < 0){continue;}
            image.set(p.x, p.y, color);
        }
    }
}
void lambert_lighting(vec3f light_dir, Model &model, TGAImage &image){

    for (int i = 0; i < model.nfaces(); ++i) {


        //------------------------------------------------------------
        vec2i screen_coords[3];
        vec3f world_coords[3];


        for (int j = 0; j < 3; ++j) {
            vec3f v = model.vert(i, j);

            //因为到目前为止，我们都是在正交投影，还没有加入透视投影，
            //所以我们就是直接利用模型的xy坐标进行绘制三角形和平行投影，
            //由于模型的尺寸是在-1到1所以我们把它放大到图片的尺寸空间就是图片的宽度和高度
            screen_coords[j] = {(int)((v.x + 1) / 2 * image.get_width()),
                                (int)((v.y+1)/2*image.get_height())};
            //而模型的法线hai得用原始的，三角形面的法线等，
            //我们还是利用模型原始的数值进行计算，for decrease 误差，这也比较合理
            world_coords[j] = v;
        }

        //------------------------------------------------------------
        vec3f n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
        n.normalize();
        double intensity = dot(n, light_dir);

        //------------------------------------------------------------
        if (intensity > 0) // back calling----------------------------
        triangle(screen_coords, image, TGAColor(255, 255, 255)*intensity);

    }


}


int main()
{
    Model model{"../obj/african_head.obj"};
    TGAImage image(w, h, TGAImage::RGB);
    vec3f light_dir{0, 0, -1};
    lambert_lighting(light_dir, model, image);

    //FIXME : some small black point in result.
    image.write_tga_file("out.tga");
    return 0;
}
