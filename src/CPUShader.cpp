// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "CPUShader.h"

class Vec3f
{
public:
    float x, y, z;

    Vec3f() {}
    Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3f fromRGB(uint32_t color)
    {
        return Vec3f((color & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f);
    }

    Vec3f operator+(const Vec3f &other) const
    {
        return Vec3f(x + other.x, y + other.y, z + other.z);
    }

    Vec3f operator-(const Vec3f &other) const
    {
        return Vec3f(x - other.x, y - other.y, z - other.z);
    }

    Vec3f operator*(const Vec3f &other) const
    {
        return Vec3f(x * other.x, y * other.y, z * other.z);
    }

    Vec3f operator*(float scalar) const
    {
        return Vec3f(x * scalar, y * scalar, z * scalar);
    }

    Vec3f operator/(float scalar) const
    {
        return Vec3f(x / scalar, y / scalar, z / scalar);
    }

    Vec3f operator-() const
    {
        return Vec3f(-x, -y, -z);
    }

    Vec3f &operator+=(const Vec3f &other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3f &operator-=(const Vec3f &other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vec3f &operator*=(const Vec3f &other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    Vec3f &operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vec3f &operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    float dot(const Vec3f &other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3f cross(const Vec3f &other) const
    {
        return Vec3f(y * other.z - z * other.y,
                     z * other.x - x * other.z,
                     x * other.y - y * other.x);
    }

    // float length() const
    // {
    //     return sqrtf(x * x + y * y + z * z);
    // }

    // Vec3f normalized() const
    // {
    //     float len = length();
    //     return Vec3f(x / len, y / len, z / len);
    // }

    uint32_t toRGB()
    {
        uint8_t r = x * 255;
        uint8_t g = y * 255;
        uint8_t b = z * 255;
        return r | (g << 8) | (b << 16);
    }
};

struct Vec2
{
    float x, y;

    Vec2(float x = 0, float y = 0) : x(x), y(y) {}

    Vec2 operator*(const Vec2 &other) const
    {
        return {x * other.x, y * other.y};
    }

    Vec2 operator*(float scalar) const
    {
        return {x * scalar, y * scalar};
    }

    Vec2 operator+(const Vec2 &other) const
    {
        return {x + other.x, y + other.y};
    }

    Vec2 operator-(const Vec2 &other) const
    {
        return {x - other.x, y - other.y};
    }
};

namespace RetroSim::CPUShader
{
    float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    uint32_t SamplePoint(float x, float y, const ShaderParams &params)
    {
        if (x < 0 || y < 0 || x >= params.inputWidth || y >= params.inputHeight)
            return 0; // TODO: return border color (or transparent color)

        int inputX = x;
        int inputY = y;
        return params.inputTexture[inputY * params.inputWidth + inputX];
    }

    uint32_t SampleBilinear(float x, float y, const ShaderParams &params)
    {
        if (x < 0 || y < 0 || x >= params.inputWidth || y >= params.inputHeight)
            return 0; // TODO: return border color (or transparent color)

        int inputX = x;
        int inputY = y;
        float dx = x - inputX;
        float dy = y - inputY;

        uint32_t c00 = params.inputTexture[inputY * params.inputWidth + inputX];
        uint32_t c10 = params.inputTexture[inputY * params.inputWidth + inputX + 1];
        uint32_t c01 = params.inputTexture[(inputY + 1) * params.inputWidth + inputX];
        uint32_t c11 = params.inputTexture[(inputY + 1) * params.inputWidth + inputX + 1];

        uint32_t c0 = Lerp(c00, c10, dx);
        uint32_t c1 = Lerp(c01, c11, dx);
        return Lerp(c0, c1, dy);
    }

    uint32_t Sample(float x, float y, const ShaderParams &params)
    {
        switch (params.filterMode)
        {
        case Point:
            return SamplePoint(x, y, params);
        case Bilinear:
            return SampleBilinear(x, y, params);
        }
    }

    uint32_t Shade(int x, int y, const ShaderParams &params)
    {
        int inputX = x * params.inputWidth / params.outputWidth;
        int inputY = y * params.inputHeight / params.outputHeight;
        return Sample(inputX, inputY, params);
    }

    Vec2 Warp(const Vec2 &pos)
    {
        Vec2 newPos = pos * 2.0f - Vec2(1.0f, 1.0f);

        Vec2 warp = {1.1f, 1.1f};

        newPos = newPos * Vec2(1.0f + (newPos.y * newPos.y) * warp.x, 1.0f + (newPos.x * newPos.x) * warp.y);
        return newPos * 0.5f + Vec2(0.5f, 0.5f);
    }

    void Warp(int x, int y, int screenWidth, int screenHeight, int &outx, int &outy)
    {
        float normalizedX = static_cast<float>(x) / static_cast<float>(screenWidth);
        float normalizedY = static_cast<float>(y) / static_cast<float>(screenHeight);

        normalizedX = normalizedX * 2.0f - 1.0f;
        normalizedY = normalizedY * 2.0f - 1.0f;

        float warpX = 0.05f;
        float warpY = 0.05f;

        normalizedX *= 1.0f + (normalizedY * normalizedY) * warpX;
        normalizedY *= 1.0f + (normalizedX * normalizedX) * warpY;

        normalizedX = normalizedX * 0.5f + 0.5f;
        normalizedY = normalizedY * 0.5f + 0.5f;

        outx = static_cast<int>(normalizedX * screenWidth);
        outy = static_cast<int>(normalizedY * screenHeight);
    }

    void RunShader(const ShaderParams &params)
    {
        for (int y = 0; y < params.outputHeight; y++)
            for (int x = 0; x < params.outputWidth; x++)
            {
                // blur
                // int r = 0, g = 0, b = 0;
                // for(int dy = -1; dy <= 1; dy++)
                //     for(int dx = -1; dx <= 1; dx++)
                //     {
                //         uint32_t c = Shade(x + dx, y + dy, params);
                //         r += c & 0xFF;
                //         g += (c >> 8) & 0xFF;
                //         b += (c >> 16) & 0xFF;
                //     }
                // r /= 9;
                // g /= 9;
                // b /= 9;
                // params.outputTexture[y * params.outputWidth + x] = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);

                int tx, ty;
                Warp(x, y, params.outputWidth, params.outputHeight, tx, ty);

                // guard x and y
                if (tx < 0)
                    tx = 0;
                if (ty < 0)
                    ty = 0;
                if (tx >= params.outputWidth)
                    tx = params.outputWidth - 1;
                if (ty >= params.outputHeight)
                    ty = params.outputHeight - 1;

                uint32_t color;
                if (0 == y % 2)
                {
                    // Vec3f colorL = Vec3f().fromRGB(Shade(tx - 1, ty, params));
                    // Vec3f colorR = Vec3f().fromRGB(Shade(tx + 1, ty, params));
                    // Vec3f colorU = Vec3f().fromRGB(Shade(tx, ty - 1, params));
                    // Vec3f colorD = Vec3f().fromRGB(Shade(tx, ty + 1, params));

                    // Vec3f avgColor = (colorL + colorR + colorU + colorD) / 4.0f;

                    // avgColor *= 0.5f;

                    // uint32_t color = Shade(x, y, params);
                    Vec3f currentColor = Vec3f().fromRGB(Shade(tx, ty, params)) * 0.9f;
                    color = currentColor.toRGB();
                    // float t = 0.5f;
                    // Vec3f blendedColor = currentColor * (1.0f - t) + avgColor * t;
                    // color = blendedColor.toRGB();
                }
                else
                {
                    color = Shade(tx, ty, params);
                }

                int maskr = (x % 3) == y % 2;
                int maskg = ((x + 1) % 3) == y % 2;
                int maskb = ((x + 2) % 3) == y % 2;
                
                // int maskr = (((x + y) % 6) == 0);
                // int maskg = (((x + y) % 6) == 1);
                // int maskb = (((x + y) % 6) == 2);

                uint8_t r = color & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t b = (color >> 16) & 0xFF;

                float rf = r / 255.0f;
                float gf = g / 255.0f;
                float bf = b / 255.0f;

                if (maskr)
                    rf *= 1.1;
                else
                    rf *= 0.9;

                if (maskg)
                    gf *= 1.1;
                else
                    gf *= 0.9;

                if (maskb)
                    bf *= 1.1;
                else
                    bf *= 0.9;

                if (rf > 1.0f)
                    rf = 1.0f;
                if (gf > 1.0f)
                    gf = 1.0f;
                if (bf > 1.0f)
                    bf = 1.0f;

                color = (uint8_t)(rf * 255) | ((uint8_t)(gf * 255) << 8) | ((uint8_t)(bf * 255) << 16);

                params.outputTexture[y * params.outputWidth + x] = color;
            }
    }
}