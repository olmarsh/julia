#include <iostream>
#include <cmath>
#include <filesystem>

#include <lodepng.h>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace YAML;

Node Config;

double Julia(long double x, long double y, double cx, double cy, double radius, int iterDepth)
{
    int iteration = 0;

    while (x * x + y * y < radius)
    {
        double temp_x = x * x - y * y;
        y = 2 * x * y  + cy;
        x = temp_x + cx;
        iteration++;

        // If the point never escaped
        if (iteration >= iterDepth)
            return -1;
    }

    // Smoothing formula
    double z = x * x + y * y;
    double ret = iteration + 1 - log(log(z))/log(2);

    if (ret < 0)
        return 0;
    else
        return ret;
}

template <typename T>
T GetConfigValue(const string& key, T defaultValue)
{
    return Config[key] ? Config[key].as<T>() : defaultValue;
}

int main() {
    // Get input from the config
    if (!filesystem::exists("config.yml"))
    {
        cout << "Fatal Error: 'config.yml' does not exit";
        return -1;
    }

    Config = YAML::LoadFile("config.yml");

    double real = GetConfigValue("Real", -1);
    double imaginary = GetConfigValue("Imaginary", -1);

    int width = GetConfigValue("Width", 1024);
    int height = GetConfigValue("Height", 1024);

    // Falloff values - change the colour of the fractal. Lower is brighter
    double rFalloff = GetConfigValue("RFalloff", 15);
    double gFalloff = GetConfigValue("GFalloff", 100);
    double bFalloff = GetConfigValue("BFalloff", 500);

    // Value to assign non-escaping points - 0 is darkest, 1000 is brightest
    int nonEscapingValue = GetConfigValue("NonEscapingValue", 0);

    // Escape radius
    double radius = GetConfigValue("Radius", 4);

    // Output path
    filesystem::path outputPath = filesystem::path(GetConfigValue("OutputPath", filesystem::current_path().string()));

    // Compute the julia fractal for each pixel in image
    cout << "Computing...\n";
    vector<unsigned char> image(width * height * 4);
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // Compute for current pixel
            double result = Julia(((double)i / (double)width) * 4 - 2, ((double)j / (double)height) * 4 - 2, real, imaginary, radius, 1000);

            // If non-escaping, set result to defined value
            if (result == -1)
                result = nonEscapingValue;

            // Write to image vector RGBA format
            image[4 * width * j + 4 * i + 0] = (unsigned char)((result / (result + rFalloff)) * 255);
            image[4 * width * j + 4 * i + 1] = (unsigned char)((result / (result + gFalloff)) * 255);
            image[4 * width * j + 4 * i + 2] = (unsigned char)((result / (result + bFalloff)) * 255);
            image[4 * width * j + 4 * i + 3] = 255;
        }
    }

    // Encode and save
    string path = outputPath.append("Julia_" + to_string(time(0)) + ".png").string();
    vector<unsigned char> output;
    lodepng::encode(output, image, width, height);
    lodepng::save_file(output, outputPath.string());
    cout << "Saved to file '" << outputPath.string() << "'\n";
}
