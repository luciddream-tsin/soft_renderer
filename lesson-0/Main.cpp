//
// Created by tsin on 22-5-24.
//

#include "tgaimage.h"
 const TGAColor red = TGAColor(255, 0, 0, 255);

int main(){
    TGAImage image(100, 100, TGAImage::RGBA);
    //  |
    //  |
    //  |
    //  |
    //  |
    //  |
    //(0, 0)________________________
    image.set(10, 20, red);
    image.write_tga_file("out.tga");
    return 0;
}