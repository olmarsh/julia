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

    if (ret < 0) return 0;
    return ret;
}

double MultiJulia(long double x, long double y, double n, double radius, int iterDepth)
{
    int iteration = 0;
    double cx = x;
    double cy = y;

    while (x * x + y * y < radius)
    {
        double temp_x = pow((x * x + y * y), n/2) * cos(n * atan2(y, x)) + cx;
        y = pow((x * x + y * y), n/2) * sin(n * atan2(y, x)) + cy;
        x = temp_x;

        iteration++;
        // If the point never escaped
        if (iteration >= iterDepth)
            return -1;
    }

    // Smoothing formula
    double z = x * x + y * y;
    double ret = iteration + 1 - log(log(z))/log(2);

    if (ret < 0) return 0;
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

    // Fractal type
    int fractalType = GetConfigValue("FractalType", 0);

    // Multijulia exponent
    double exponent = GetConfigValue("Exponent", 2.0);

    // Julia set parameters
    double real = GetConfigValue("Real", -1.0);
    double imaginary = GetConfigValue("Imaginary", -1.0);

    // Size parameters
    int width = GetConfigValue("Width", 1024);
    int height = GetConfigValue("Height", 1024);
    //double scale = GetConfigValue("Scale", 4.0);

    // Falloff values - change the colour of the fractal. Lower is brighter
    double rFalloff = GetConfigValue("RFalloff", 15.0);
    double gFalloff = GetConfigValue("GFalloff", 100.0);
    double bFalloff = GetConfigValue("BFalloff", 500.0);

    // Value to assign non-escaping points - 0 is darkest, 1000 is brightest
    int nonEscapingValue = GetConfigValue("NonEscapingValue", 0);

    // Max iteration depth
    int maxIterations = GetConfigValue("MaxIterations", 1000);

    // Escape radius
    double radius = GetConfigValue("Radius", 4.0);

    // Border values
    double minX = GetConfigValue("MinX", -2.0);
    double maxX = GetConfigValue("MaxX", 2.0);
    double minY = GetConfigValue("MinY", -2.0);
    double maxY = GetConfigValue("MaxY", 2.0);

    // Output path
    filesystem::path outputPath = filesystem::path(GetConfigValue("OutputPath", filesystem::current_path().string()));

    // Compute the julia fractal for each pixel in image
    cout << "Computing...\n";
    //double aspectRatio = (double)width / (double)height;
    vector<unsigned char> image(width * height * 4);
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // Compute for current pixel
            double x = ((double)i / (double)width)  * (maxX-minX) + minX;
            double y = ((double)j / (double)height) * (maxY-minY) + minY;
            double result;

            if (fractalType == 1) result = MultiJulia(x, y, exponent, radius, maxIterations);
            else result = Julia(x, y, real, imaginary, radius, maxIterations);

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
    if (lodepng::save_file(output, outputPath.string()) == 0)
        cout << "Saved to file '" << outputPath.string() << "'\n";
    else
        cout << "Failed to save to file '" << outputPath.string() << "'\n";
}