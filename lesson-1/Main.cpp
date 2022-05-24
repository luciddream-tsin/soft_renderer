//
// Created by tsin on 22-5-24.
//

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"


void line(vec2i a, vec2i b, TGAImage &image, TGAColor color){
    int x0 = a.x, y0 = a.y;
    int x1 = b.x, y1 = b.y;
    int xi = -01, yi = -01;
    bool steep = (std::abs(y1-y0) > std::abs(x1-x0)) ? true : false;
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    bool reverse = (x0 > x1) ? true : false;
    if (reverse) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    bool down = (y0 > y1)? true :false;
    float e = 0, t = std::abs((y1-y0)*1.0/(x1-x0));
    for (xi = x0, yi = y0; xi < x1; ++xi) {
        if (steep)
            image.set(yi, xi, color);
        else
            image.set(xi, yi, color);
        e += t;
        if (e > 0.5){
            yi += ((down) ? -1 : 1);
            e -= 1;
        }
    }
}

int main(){
    Model model("../obj/african_head.obj");

    int w = 800;
    int h = 800;
    TGAImage image(w, h, TGAImage::RGBA);

    for (int i = 0; i < model.nfaces(); ++i) {
        for (int j = 0; j < 3; ++j) {
            vec3f v3_0 = model.vert(i, j);
            vec3f v3_1 = model.vert(i, (j + 1) % 3);

            vec2i v2_0 {(int)((v3_0.x+1)/2*w), (int)((v3_0.y+1)/2*h)};
            vec2i v2_1 {(int)((v3_1.x+1)/2*w), (int)((v3_1.y+1)/2*h)};

            line(v2_0, v2_1, image, TGAColor(255,255,255));

        }
    }
    image.write_tga_file("out.tga");
    return 0;
}