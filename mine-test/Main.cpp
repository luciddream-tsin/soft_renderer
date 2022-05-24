#include <iostream>
#include "tgaimage.h"
#include "model.h"

typedef struct vec2i_{
    int x;
    int y;
} vec2i_;

typedef struct vec3f_{
    float x;
    float y;
    float z;
} ;

typedef struct vec3i_{
    int x;
    int y;
    int z;
};
extern mat<4,4> Viewport; // "OpenGL" state matrices
extern mat<4,4> Projection;
extern mat<4,4> ModelView;

void line(vec2i_ a, vec2i_ b, TGAImage &image, TGAColor color){

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

void triangle_1(vec2i_ t0, vec2i_ t1, vec2i_ t2, TGAImage &image, TGAColor color){
    line(t0, t1, image, color);
    line(t1, t2, image, color);
    line(t2, t0, image, color);
}
vec3i_ cross_(vec3i_ &X, vec3i_ &Y){
    vec3i_ cross{
            X.y*Y.z - X.z*Y.y,
            X.z*Y.x - X.x*Y.z,
            X.x*Y.y - X.y*Y.x
    };

    return cross;
}

vec3f_ check(vec2i_ *ps, vec2i_ p){

    vec3i_ X {ps[1].x - ps[0].x, ps[2].x - ps[0].x, ps[0].x - p.x};
    vec3i_ Y {ps[1].y - ps[0].y, ps[2].y - ps[0].y, ps[0].y - p.y};

    vec3i_ cross = cross_(X, Y);
    float u = (float)(cross.x * 1.0 / cross.z);
    float v = (float)(cross.y * 1.0 / cross.z);
    return vec3f_{u, v, 1 - u - v};

}

vec3f check_(vec3f *ps, vec3f p){
    const vec3f X {ps[1].x - ps[0].x, ps[2].x - ps[0].x, ps[0].x - p.x};
    const vec3f Y {ps[1].y - ps[0].y, ps[2].y - ps[0].y, ps[0].y - p.y};

    vec3f cross_ = cross(X, Y);
    float u = (float)(cross_.x * 1.0 / cross_.z);
    float v = (float)(cross_.y * 1.0 / cross_.z);
    return vec3f{1-u-v, u, v};

}

void triangle_2(vec2i_ *t, TGAImage &image, TGAColor color){
    struct vec2i_ minbox{image.get_width() - 1, image.get_height() - 1};
    struct vec2i_ maxbox{0, 0};

    for (int i = 0; i < 3; ++i) {
        //--min
        minbox.x = std::min(minbox.x, t[i].x);
        minbox.y = std::min(minbox.y, t[i].y);
        //--max
        maxbox.x = std::max(maxbox.x, t[i].x);
        maxbox.y = std::max(maxbox.y, t[i].y);
    }

    for (int i = minbox.x; i < maxbox.x; ++i) {
        for (int j = minbox.y; j < maxbox.y; ++j) {
            struct vec2i_ p{i, j};
            vec3f_ coord = check(t, p);
            if (coord.x<0 || coord.y<0 || coord.z<0){continue;}
            image.set(p.x, p.y, color);
        }
    }
}

void triangle_3(float *zbuffer, vec3f *triang_vtxs, vec2 *triang_uvs, TGAImage &diff, TGAImage &image, TGAColor color){

    //triang_vtxs 三角形的三个顶点

    vec2 minbox{(double )image.width() , (double)image.height()};
    vec2 maxbox{-1, -1};

    for (int i = 0; i < 3; ++i) {

        //模型坐标范围是0-1的float, 将每个顶点的xy转换到屏幕空间范围
        triang_vtxs[i].x = (triang_vtxs[i].x + 1) / 2 * image.width();
        triang_vtxs[i].y = (triang_vtxs[i].y + 1) / 2 * image.height();

        triang_uvs[i].x = triang_uvs[i].x * diff.width();
        triang_uvs[i].y = triang_uvs[i].y * diff.height();

        //--min
        minbox.x = std::min(minbox.x, triang_vtxs[i].x);
        minbox.y = std::min(minbox.y, triang_vtxs[i].y);
        //--max
        maxbox.x = std::max(maxbox.x, triang_vtxs[i].x);
        maxbox.y = std::max(maxbox.y, triang_vtxs[i].y);

    }

#pragma omp parallel for

    for (int i = minbox.x; i <= (int)maxbox.x; i++) {
        for (int j = minbox.y; j <= (int)maxbox.y; j++) {
            vec3f p{(float)i, (float)j, 0};

            //还有一个重要问题是，check返回的重心坐标的顺序
            vec3f coord = check_(triang_vtxs, p);
            if (coord.x<0 || coord.y<0 || coord.z<0){continue;}

            //直接使用像素点的重心坐标， 计算出z的插值
            //三角形顶点的z，还是0-1范围， 所以记得zbuffer 要用float
            for (int k = 0; k < 3; ++k) {
                p.z += triang_vtxs[k].z * coord[k];
            }

            // u v
            int u_ = coord * vec3f (triang_uvs[0].x, triang_uvs[1].x, triang_uvs[2].x);
            int v_ = coord * vec3f (triang_uvs[0].y, triang_uvs[1].y, triang_uvs[2].y);


            if (zbuffer[i*image.width()+j] < p.z) {
                zbuffer[i*image.width()+j] = p.z;
                image.set(p.x, p.y, diff.get(u_, v_));
            }
        }
    }

}

void triangle_4(float *zbuffer, vec3f *triang_vtxs, vec2 *triang_uvs, TGAImage &diff, TGAImage &image, TGAColor color){

    //triang_vtxs 三角形的三个顶点

    vec2 minbox{(double )image.width() , (double)image.height()};
    vec2 maxbox{-1, -1};

    for (int i = 0; i < 3; ++i) {
        //将每个顶点的xy转换到屏幕空间范围, 已经在视口矩阵完成
        //triang_vtxs[i].x = (triang_vtxs[i].x + 1) / 2 * image.width();
        //triang_vtxs[i].y = (triang_vtxs[i].y + 1) / 2 * image.height();

        triang_uvs[i].x = triang_uvs[i].x * diff.width();
        triang_uvs[i].y = triang_uvs[i].y * diff.height();

        //--min
        minbox.x = std::min(minbox.x, triang_vtxs[i].x);
        minbox.y = std::min(minbox.y, triang_vtxs[i].y);
        //--max
        maxbox.x = std::max(maxbox.x, triang_vtxs[i].x);
        maxbox.y = std::max(maxbox.y, triang_vtxs[i].y);

    }

#pragma omp parallel for

    for (int i = minbox.x; i <= (int)maxbox.x; i++) {
        for (int j = minbox.y; j <= (int)maxbox.y; j++) {
            vec3f p{(float)i, (float)j, 0};

            //还有一个重要问题是，check返回的重心坐标的顺序
            vec3f coord = check_(triang_vtxs, p);
            if (coord.x<0 || coord.y<0 || coord.z<0){continue;}

            //直接使用像素点的重心坐标， 计算出z的插值
            //三角形顶点的z，还是0-1范围， 所以记得zbuffer 要用float
            for (int k = 0; k < 3; ++k) {
                p.z += triang_vtxs[k].z * coord[k];
            }

            // u v
            int u_ = coord * vec3f (triang_uvs[0].x, triang_uvs[1].x, triang_uvs[2].x);
            int v_ = coord * vec3f (triang_uvs[0].y, triang_uvs[1].y, triang_uvs[2].y);


            if (zbuffer[i*image.width()+j] < p.z) {
                zbuffer[i*image.width()+j] = p.z;
                image.set(p.x, p.y, diff.get(u_, v_));
            }
        }
    }

}




void draw_head_color(TGAImage &testImage){

    Model model {"./obj/african_head/african_head.obj"};
    for (int i = 0; i < model.nfaces(); ++i) {
        struct  vec2i_ tmp [3];
        for (int j = 0; j < 3; ++j) {
            auto v = model.vert(i,j);
            tmp[j].x = (v.x + 1)/2.0 * testImage.width();
            tmp[j].y = (v.y + 1)/2.0 * testImage.height();
        }
        triangle_2(tmp, testImage, TGAColor(rand(), rand(), rand()));
    }

}
void draw_head_light(TGAImage &testImage){
    Model model {"./obj/african_head/african_head.obj"};

    vec3f light_dir{0, 0, -1};

    for (int i = 0; i < model.nfaces(); ++i) {
        struct vec2i_ screen_pos [3];
        vec3f world_pos [3];
        for (int j = 0; j < 3; ++j) {
            auto v = model.vert(i,j);
            screen_pos[j].x = (v.x + 1) / 2.0 * testImage.width();
            screen_pos[j].y = (v.y + 1) / 2.0 * testImage.height();

            world_pos[j] = v;
        }

        const vec3f a = world_pos[2] - world_pos[0];
        const vec3f b = world_pos[1] - world_pos[0];
        vec3f normal = cross(a, b);
        normal.normalize();

        float intensity = normal * light_dir;
        if (intensity>0) {
            triangle_2(screen_pos, testImage, TGAColor(intensity * 255, intensity * 255, intensity * 255));
        }
    }
}


void draw_head_z_buffer(TGAImage &testImage){
    Model model {"../bins/obj/african_head/african_head.obj"};

    vec3f light_dir{0, 0, -1};
    float *zbuffer = new float[testImage.width() * testImage.height()];

    for (int j = 0; j <testImage.width(); ++j) {
        for (int k = 0; k < testImage.height(); ++k) {
            zbuffer[j*testImage.width() + k] = -100000000.0;
        }
    }

    TGAImage diffuse = model.diffuse();
    for (int i = 0; i < model.nfaces(); ++i) {
        vec3f triang_vtxs [3];
        vec2 triang_uvs[3];

        for (int j = 0; j < 3; ++j) {
            triang_vtxs[j] = model.vert(i, j);
            triang_uvs[j] = model.uv(i, j);
        }

        const vec3f a = triang_vtxs[2] - triang_vtxs[0];
        const vec3f b = triang_vtxs[1] - triang_vtxs[0];
        vec3f normal = cross(a, b);

        normal.normalize();

        float intensity = normal * light_dir;

        if (intensity>0) {
            triangle_3(zbuffer, triang_vtxs, triang_uvs, diffuse, testImage, TGAColor(intensity * 255, intensity * 255, intensity * 255));
        }

    }
}

void draw_head_perspective_projection(TGAImage &testImage){

    Model model {"../bins/obj/african_head/african_head.obj"};
    TGAImage diffuse = model.diffuse();

    //-----------------------light
    vec3f light_dir{0, 0, -1};

    //-----------------------z-buffer
    float *zbuffer = new float[testImage.width() * testImage.height()];
    for (int j = 0; j <testImage.width(); ++j) {
        for (int k = 0; k < testImage.height(); ++k) {
            zbuffer[j*testImage.width() + k] = -100000000.0;
        }
    }

    //-----------------------mvp matrix

    vec3f camera{0, 0, 3};
    projection(camera.z);

    int width = testImage.width();
    int height = testImage.height();
    viewport(width/8, height/8, width*3/4, height*3/4);


    //-----------------------iterator faces
    for (int i = 0; i < model.nfaces(); ++i) {
        vec3f triang_vtxs[3];//我们将屏幕坐标用vec3里的xy表示顺便携带了z的信息
        vec2 triang_uvs[3];

        vec4 mvp_x[3];

        for (int j = 0; j < 3; ++j) {



            mvp_x[j] = Viewport * Projection * embed<4>(model.vert(i,j));
            triang_vtxs[j].x = mvp_x[j][0] / mvp_x[j][3];
            triang_vtxs[j].y = mvp_x[j][1] / mvp_x[j][3];
            triang_vtxs[j].z = mvp_x[j][2] / mvp_x[j][3];


            triang_uvs[j] = model.uv(i, j);
        }



        const vec3f a = triang_vtxs[2] - triang_vtxs[0];
        const vec3f b = triang_vtxs[1] - triang_vtxs[0];
        vec3f normal = cross(a, b);

        normal.normalize();
        float intensity = normal * light_dir;


        if (intensity>0) {
            triangle_4(zbuffer, triang_vtxs, triang_uvs, diffuse, testImage, TGAColor(intensity * 255, intensity * 255, intensity * 255));
        }

    }
}



void draw_head_move(TGAImage &testImage){

    Model model {"../bins/obj/african_head/african_head.obj"};
    TGAImage diffuse = model.diffuse();

    //-----------------------light
    vec3f light_dir{0, 0, -1};

    //-----------------------z-buffer
    float *zbuffer = new float[testImage.width() * testImage.height()];
    for (int j = 0; j <testImage.width(); ++j) {
        for (int k = 0; k < testImage.height(); ++k) {
            zbuffer[j*testImage.width() + k] = -100000000.0;
        }
    }

    //-----------------------mvp matrix

    vec3f camera{4, 0, 3};
    vec3f center{0, 1, 0};

    lookat(camera, center, vec3f{0, 1, 0});

    projection((camera - center).norm());

    int width = testImage.width();
    int height = testImage.height();
    viewport(width/8, height/8, width*3/4, height*3/4);


    //-----------------------iterator faces
    for (int i = 0; i < model.nfaces(); ++i) {
        vec3f triang_vtxs[3];//我们将屏幕坐标用vec3里的xy表示顺便携带了z的信息
        vec2 triang_uvs[3];

        vec4 mvp_x[3];

        for (int j = 0; j < 3; ++j) {

            vec4 tmp = embed<4>(model.vert(i, j));
            tmp = ModelView * tmp;

            mvp_x[j] =  Viewport * Projection*  tmp;

            triang_vtxs[j].x = mvp_x[j][0] / mvp_x[j][3];
            triang_vtxs[j].y = mvp_x[j][1] / mvp_x[j][3];
            triang_vtxs[j].z = mvp_x[j][2] / mvp_x[j][3];


            triang_uvs[j] = model.uv(i, j);
        }



        const vec3f a = triang_vtxs[2] - triang_vtxs[0];
        const vec3f b = triang_vtxs[1] - triang_vtxs[0];
        vec3f normal = cross(a, b);

        normal.normalize();
        float intensity = normal * light_dir;


        if (intensity>0) {
            triangle_4(zbuffer, triang_vtxs, triang_uvs, diffuse, testImage, TGAColor(intensity * 255, intensity * 255, intensity * 255));
        }

    }
}

int main() {

    TGAImage testImage(800, 800, TGAImage::RGB);
    for (int i = 0; i < 700; ++i) {
        for (int j = 0; j < 700; ++j) {
            testImage.set(i, j, TGAColor(5, 5, 5));
        }
    }
    draw_head_z_buffer(testImage);

    testImage.write_tga_file("out.tga");

    return 0;
}