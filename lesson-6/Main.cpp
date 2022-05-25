//
// Created by tsin on 22-5-25.
//
#include <limits>
#include "model.h"
#include "Shader.h"


int main()
{
    const int w = 800, h = 800;
    //remember to turn texture on! (and normal spec, if you need)
    Model model{"../obj/african_head.obj", true};
    TGAImage image(w, h, TGAImage::RGB);


    //---------------------------------------
    vec3f eye(2, 2, 10), center(0.2, 0, 0);
    vec3f light_dir = vec3f (0,0,-2).normalize();
    Shader shader(&model, &image,(3.0/4)*w, (3.0/4)*h, eye, center, light_dir);
    shader.run();

    //---------------------------------------

    image.write_tga_file("out.tga");
    return 0;
}
