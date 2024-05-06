#include <iostream>
#include <cmath>
#include <filesystem>
#include <format>

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
    Julia, MultiJulia,
};

float64 Julia(float64 x, float64 y, float64 cx, float64 cy, float64 radius, int32 iterationDepth)
{
    int32 iteration = 0;

    while (x * x + y * y < radius)
    {
        float64 tempX = x * x - y * y;
        y = 2 * x * y + cy;
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

void Log(string message, bool error = false)
{
    if (error)
        cout << "Error: ";

    cout << message << endl;
}

float64 Interpolate(float64 start, float64 end,float64 pos, string method = "linear") {
    if (method == "linear") return start + ((end - start) * pos);
    else return 0;
}

int32 main()
{
    // Get input from the config
    if (!filesystem::exists("config.yml"))
    {
        Log("'config.yml' does not exist", true);
        return -1;
    }

    Config = YAML::LoadFile("config.yml");

#pragma region Parameters

    // === Fractal Parameters === //
    string fractalTypeString = GetConfigValue("FractalType", (string)"Julia");
    FractalType fractalType;
    if (fractalTypeString == "Julia")
        fractalType = FractalType::Julia;
    else if (fractalTypeString == "MultiJulia")
        fractalType = FractalType::MultiJulia;
    else
    {
        Log(format("Fatal Error: FractalType '{}' is invalid", fractalTypeString), true);
        return -2;
    }

    float64 real = GetConfigValue("Real", 0.0);
    float64 imaginary = GetConfigValue("Imaginary", 0.0);

    float64 multiJuliaExponent = GetConfigValue("MultiJuliaExponent", 2.0);

    // === Image Parameters === //
    int32 width = GetConfigValue("Width", 1024);
    int32 height = GetConfigValue("Height", 1024);

    float64 falloffStrength = GetConfigValue("FalloffStrength", 15.0);
    float64 falloffR = GetConfigValue("FalloffR", 1.0);
    float64 falloffG = GetConfigValue("FalloffG", 1.0);
    float64 falloffB = GetConfigValue("FalloffB", 1.0);

    float64 backgroundR = GetConfigValue("BackgroundR", 0.0);
    float64 backgroundG = GetConfigValue("BackgroundG", 0.0);
    float64 backgroundB = GetConfigValue("BackgroundB", 0.0);
    float64 backgroundA = GetConfigValue("BackgroundA", 1.0);

    filesystem::path outputPath = filesystem::path(GetConfigValue("OutputPath", filesystem::current_path().string()));

    // === Transformation Parameters === //
    bool adjustForAspectRatio = GetConfigValue("AdjustForAspectRatio", true);
    float64 offsetX = GetConfigValue("OffsetX", 0.0);
    float64 offsetY = GetConfigValue("OffsetY", 0.0);
    float64 scaleX = GetConfigValue("ScaleX", 1.0);
    float64 scaleY = GetConfigValue("ScaleY", 1.0);

    // === Calculation Parameters === //
    float64 nonEscapingValue = GetConfigValue("NonEscapingValue", 0.0);
    int32 maxIterations = GetConfigValue("MaxIterations", 1000);
    float64 radius = GetConfigValue("EscapeRadius", 4.0);

    // === Animation Parameters === //
    bool animate = GetConfigValue("Animate", false);
    int32 frameCount = GetConfigValue("FrameCount", 30);

    string interpolationType = GetConfigValue("InterpolationType", (string)"linear");

    bool animateCoordinates = GetConfigValue("AnimateCoordinates", false);
    float64 realStart = GetConfigValue("RealStart", 1.0);
    float64 realEnd = GetConfigValue("RealEnd", 1.0);
    float64 imaginaryStart = GetConfigValue("ImaginaryStart", 1.0);
    float64 imaginaryEnd = GetConfigValue("ImaginaryEnd", 1.0);

#pragma endregion

    if (animate == false) frameCount = 1;
    string timeString = to_string(std::time(nullptr));  // time string for the output folder name if animation is used
    if (animate) filesystem::create_directory(outputPath.append(format("julia_{}", timeString)));
    for (int frame=0;frame<frameCount;frame++)
    {
        if (animate && animateCoordinates)
        {
            real = Interpolate(realStart, realEnd, (float64)frame/(frameCount-1), interpolationType);
            imaginary = Interpolate(imaginaryStart, imaginaryEnd, (float64)frame/(frameCount-1), interpolationType);
        }
        cout << format("Real: {:.5f}, Imaginary: {:.5f}\n", real, imaginary);
        // Compute the julia fractal for each pixel in frame
        Log(format("Computing frame {} of {} ({}.png)...", frame+1, frameCount, frame));
        vector<uint8> image(width * height * 4);
        for (int32 i = 0; i < width; i++)
        {
            for (int32 j = 0; j < height; j++)
            {
                // Calculate pixel coordinates (normally -2 to 2 with a square output)
                float64 x = ((float64)i / (float64)width) * 4 + -2;
                float64 y = ((float64)j / (float64)height) * 4 + -2;

                x -= offsetX;
                y -= offsetY;
                x /= scaleX;
                y /= scaleY;

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
                float64 pixelValue = result / (result + falloffStrength);
                int32 pixelLocation = 4 * width * j + 4 * i;
                image[pixelLocation] = (uint8)(lerp(backgroundR, falloffR, pixelValue) * 255);
                image[pixelLocation + 1] = (uint8)(lerp(backgroundG, falloffG, pixelValue) * 255);
                image[pixelLocation + 2] = (uint8)(lerp(backgroundB, falloffB, pixelValue) * 255);
                image[pixelLocation + 3] = (uint8)(lerp(backgroundA, 1, pixelValue) * 255);
            }

            // Print percentage complete
            // TODO: fix percentage
            cout << "\r             \r" << ((double)(int)(((double)i / (double)width) * 10000)) / 100 << "%" << flush;
        }

        cout << "\r             \r";

        // Encode and save
        filesystem::path path = outputPath;
        if (animate == false) path.append(format("julia_{}.png", to_string(time(nullptr)))).string();
        else path.append(format("{}.png", frame));
        vector<uint8> output;
        lodepng::encode(output, image, width, height);

        if (lodepng::save_file(output, path.string()) == 0)
            Log(format("Saved to file '{}'\n", path.string()));
        else
        {
            Log(format("Failed to save to file '{}'", path.string()), true);
            return -3;
        }
    }
    return 0;
}
