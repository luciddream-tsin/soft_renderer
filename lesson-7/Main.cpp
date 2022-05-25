//
// Created by tsin on 22-5-25.
//
#include <limits>
#include "model.h"
#include "BasePassShader.h"
#include "DepthBufferShader.h"


int main()
{
    const int w = 800, h = 800;
    //remember to turn texture on! (and normal spec, if you need)
    Model model{"../obj/diablo3_pose.obj", true};
    TGAImage render_target(w, h, TGAImage::RGB);
    TGAImage depth_buffer(w, h, TGAImage::GRAYSCALE);


    //-------------------------------------------------------------------------
    vec3f light(0, 3, 5), center(0, 0, 0);
    vec3f light_dir = vec3f (0,-3,-5).normalize();// NOTE: it's just direction , not effect intensity
    DepthBufferShader depth_shader(&model, &depth_buffer, (3.0 / 4) * w, (3.0 / 4) * h, light, center, light_dir);
    depth_shader.run();



    vec3f eye{0, 0., 2};
    //---------------------------------------
    BasePassShader base_shader(&model, &render_target, (3.0 / 4) * w, (3.0 / 4) * h, eye, center, light_dir);

    base_shader.set_mvp(depth_shader.get_mvp());
    base_shader.set_shadow_map(depth_buffer);

    base_shader.run();




    //---------------------------------------
    depth_buffer.write_tga_file("depth.tga");
    render_target.write_tga_file("out.tga");
    return 0;
}
