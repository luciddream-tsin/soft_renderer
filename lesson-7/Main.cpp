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
    vec3f light_dir = vec3f (0,-3,-5).normalize();// NOTE: 这个只是方向不代表光照强度


    // 把点光源的位置当做相机位置, 传入shader, 即从光源看向场景, 不着色, 只是记录光栅化后每个片段到光源的深度
    /* 注意最后一句话, "记录光栅化后的每个fragment到光源的深度", 应当知道光栅化的每个片元是对假想的连续虚拟世界的离散
     * 每个片段实际对应一小块无限密集连续的虚拟世界空间, 由于透视的缘故, 这个空间呈现锥形.
     *
     * 最简单情况下，我们得到的一个fragment的像素也好深度也好, 只是这一小块连续空间中的一条光线, 多次采样指的就是在这个小块内多次采样连续空间, 共同组成一个pixel.
     * 光线追踪的采样次数也是如此，只不过它是从像素发出多个光线到虚拟世界.
     *
     * shadow map 的第一个问题, 阴影锯齿问题(走样严重), 正如前面所说, 虚拟世界中的一个点到光源的距离, 遮挡住了一小块锥形空间在该点之后的所有点(比该点到光源更深的点)
     * 这些点由于透视投影的特性, 如果在离物体很远的地方(比如我面前有一个灯,身后很远的地方有一面很大的墙, 我在墙上的影子就会很大), 就会产生严重的锯齿,边缘块状化.
     *
     * shadow map 的第二个问题, 如果光照方向几乎平行于地面(垂直地面没有该问题), 地面上会凭空产生很多断续的阴影纹理, 这叫自遮挡现象.
     * 我们知道只有当一个像素深度大于depthBuffer里对应的值是, 才判定它是阴影, 但是在一块平坦的地面上为何会出现阴影呢?
     * 又如前所说, depthBuffer里的值是一个连续小块delta里的一个点到光源的深度, 他代表在这个小块内的所有点的深度, 我们把被代表的这些点的深度假想的组成一个平面，
     * 叫做做深度小块, 当光源垂直地面时没问题, 这个深度小块贴在地面上, 但是随着光源方向不断于地面平行, 这个深度小块不断于地面垂直,
     * 当现在又一个视线看向被这个深度小块立起来的部分遮住(相对于光源)的点, 这个点就会被判断成阴影区域, 这并不完全是由于计算机的精度问题, 这还是由于光栅化离散导致的
     *
     * */
    DepthBufferShader depth_shader(&model, &depth_buffer, (3.0 / 4) * w, (3.0 / 4) * h, light, center, light_dir);
    depth_shader.run();



    vec3f eye{0, 0., 2};
    //---------------------------------------
    BasePassShader base_shader(&model, &render_target, (3.0 / 4) * w, (3.0 / 4) * h, eye, center, light_dir);

    base_shader.set_mvp(depth_shader.get_mvp());
    base_shader.set_shadow_map(depth_buffer);

    base_shader.run();




    //---------------------------------------
    // depth_buffer.write_tga_file("depth.tga");
    render_target.write_tga_file("out.tga");
    return 0;
}
