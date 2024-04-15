#include <iostream>
#include <cmath>
#include <filesystem>

#include <lodepng.h>
#include <yaml-cpp/yaml.h>

typedef float float32;
typedef double float64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

using namespace std;

YAML::Node Config;

enum class FractalType
{
    Julia,
    MultiJulia,
};

float64 Julia(float64 x, float64 y, float64 cx, float64 cy, float64 radius, int32 iterationDepth)
{
    int32 iteration = 0;

    while (x * x + y * y < radius)
    {
        float64 tempX = x * x - y * y;
        y = 2 * x * y  + cy;
        x = tempX + cx;
        iteration++;

        // If the point never escaped
        if (iteration >= iterationDepth)
            return -1;
    }

    // Smoothing formula
    float64 z = x * x + y * y;
    float64 ret = iteration + 1 - log(log(z)) / log(2);

    return ret < 0 ? 0 : ret;
}

double MultiJulia(long double x, long double y, double n, double radius, int iterationDepth)
{
    int iteration = 0;
    double cx = x;
    double cy = y;

    while (x * x + y * y < radius)
    {
        double tempX = pow((x * x + y * y), n / 2) * cos(n * atan2(y, x)) + cx;
        y = pow((x * x + y * y), n / 2) * sin(n * atan2(y, x)) + cy;
        x = tempX;
        iteration++;

        // If the point never escaped
        if (iteration >= iterationDepth)
            return -1;
    }

    // Smoothing formula
    double z = x * x + y * y;
    double ret = iteration + 1 - log(log(z)) / log(2);

    return ret < 0 ? 0 : ret;
}

template<typename T>
T GetConfigValue(const string& key, T defaultValue)
{
    return Config[key] ? Config[key].as<T>() : defaultValue;
}

int32 main() {
    // Get input from the config
    if (!filesystem::exists("config.yml"))
    {
        cout << "Fatal Error: 'config.yml' does not exit";
        return -1;
    }

    Config = YAML::LoadFile("config.yml");

#pragma region Parameters

    // === Fractal Parameters === //
    float64 real = GetConfigValue("Real", 0.0);
    float64 imaginary = GetConfigValue("Imaginary", 0.0);
    string fractalTypeString = GetConfigValue("FractalType", (string)"Julia");
    float64 multiJuliaExponent = GetConfigValue("MultiJuliaExponent", 2.0);

    FractalType fractalType;
    if (fractalTypeString == "Julia")
        fractalType = FractalType::Julia;
    else if (fractalTypeString == "MultiJulia")
        fractalType = FractalType::MultiJulia;
    else
    {
        cout << format("Fatal Error: FractalType '{}' is invalid", fractalTypeString);
        return -1;
    }

    // === Image Parameters === //
    int32 width = GetConfigValue("Width", 1024);
    int32 height = GetConfigValue("Height", 1024);

    // Falloff values - Strength is the falloff, the others are the colour tint
    float64 falloffStrength = GetConfigValue("RFalloff", 15.0);
    float64 falloffR = GetConfigValue("FalloffR", 0.03);
    float64 falloffG = GetConfigValue("FalloffG", 0.2);
    float64 falloffB = GetConfigValue("FalloffB", 1.0);

    filesystem::path outputPath = filesystem::path(GetConfigValue("OutputPath", filesystem::current_path().string()));

    // === Transformation Parameters === //
    bool adjustForAspectRatio = GetConfigValue("AdjustForAspectRatio", true);
    float64 offsetX = GetConfigValue("OffsetX", 0.0);
    float64 offsetY = GetConfigValue("OffsetY", 0.0);
    float64 scaleX = GetConfigValue("ScaleX", 1.0);
    float64 scaleY = GetConfigValue("ScaleY", 1.0);

    // === Calculation Parameters === //
    // Value to assign non-escaping points - 0 is darkest, 1 is brightest
    float64 nonEscapingValue = GetConfigValue("NonEscapingValue", 0.0);

    // Max iteration depth
    int32 maxIterations = GetConfigValue("MaxIterations", 1000);

    // Escape radius
    float64 radius = GetConfigValue("Radius", 4.0);

#pragma endregion

    // Compute the julia fractal for each pixel in image
    cout << "Computing...\n";

    vector<uint8> image(width * height * 4);
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // Calculate pixel coordinates (normally -2 to 2 with a square output)
            float64 x = ((float64)i / (float64)width)  * 4 + -2;
            float64 y = ((float64)j / (float64)height) * 4 + -2;

            x /= scaleX;
            y /= scaleY;
            x -= offsetX;
            y -= offsetY;

            if (adjustForAspectRatio)
                x *= (float64)width / (float64)height;

            // Compute for current pixel
            float64 result;
            switch (fractalType)
            {
                case FractalType::Julia:
                    result = Julia(x, y, real, imaginary, radius, maxIterations);
                    break;
                case FractalType::MultiJulia:
                    result = MultiJulia(x, y, multiJuliaExponent, radius, maxIterations);
                    break;
            }

            // If non-escaping, set result to defined value
            if (result == -1)
                result = nonEscapingValue * (float64)maxIterations;

            // Write to image vector RGBA format
            float64 pixelValue = result / (result + falloffStrength) * 255;
            int32 pixelLocation = 4 * width * j + 4 * i;
            image[pixelLocation]     = (uint8)(pixelValue * falloffR);
            image[pixelLocation + 1] = (uint8)(pixelValue * falloffG);
            image[pixelLocation + 2] = (uint8)(pixelValue * falloffB);
            image[pixelLocation + 3] = 255;
        }

        // Print percentage complete
        // TODO: fix percentage
        cout << "\r             \r" << ((double)(int)(((double)i/(double)width)*10000))/100 << "%";
    }

    // Encode and save
    string path = outputPath.append("julia_" + to_string(time(nullptr)) + ".png").string();
    vector<uint8> output;
    lodepng::encode(output, image, width, height);

    if (lodepng::save_file(output, outputPath.string()) == 0)
        cout << "\r             \rSaved to file '" << outputPath.string() << "'\n";
    else
        cout << "\r             \rFailed to save to file '" << outputPath.string() << "'\n";
}
