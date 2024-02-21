#include <bits/stdc++.h>
#include <lodepng.h>
using namespace std;

// computes julia fractal for specific coordinates
double julia(long double x, long double y, double cx, double cy, double radius, int iter_depth) {
    int iteration = 0;
    while (x * x + y * y < radius) {
        double temp_x = x * x - y * y;
        y = 2 * x * y  + cy;
        x = temp_x + cx;
        iteration++;
        if (iteration >= iter_depth) return iteration;
    }
    // smoothing formula
    double z = x * x + y * y;
    double ret = iteration + 1 - log(log(z))/log(2);
    if (ret < 0) return 0; else return ret;
}

int main() {
    double radius = 4;  // escape radius
    int width = 1024;  // screen dimensions
    int height = 1024;

    double re, im;
    double r_falloff = 15; double g_falloff = 100; double b_falloff = 500;
    cout << "real, imaginary: "; cin >> re >> im;
    char edit_format;
    cout << "edit output format? y/n: "; cin >> edit_format;
    if (edit_format == 'y' or edit_format == 'Y') {
        cout << "width, height: ";
        cin >> width >> height;
        cout << "r,g,b falloff (lower is brighter): ";
        cin >> r_falloff >> g_falloff >> b_falloff;
    }

    vector<unsigned char> image(width*height*4);

    //compute the julia fractal for each pixel in image
    for (int i=0;i<width;i+=1) {
    for (int j=0;j<height;j+=1) {
        // computes julia fractal for the current pixel
        double julia_result = julia(((double)i/(double)width)*4-2,((double)j/(double)height)*4-2,re,im,radius,1000);
        // writes to image vector RGBA format
        image[4 * width * j + 4 * i + 0] = (unsigned char)((julia_result/(julia_result+r_falloff))*255);
        image[4 * width * j + 4 * i + 1] = (unsigned char)((julia_result/(julia_result+g_falloff))*255);
        image[4 * width * j + 4 * i + 2] = (unsigned char)((julia_result/(julia_result+b_falloff))*255);
        image[4 * width * j + 4 * i + 3] = 255;
    }
    }
    vector<unsigned char> output;
    
    // encode and save
    lodepng::encode(output, image, width, height);
    lodepng::save_file(output, "output.png");
    cout << "Done";
    return 0;
}