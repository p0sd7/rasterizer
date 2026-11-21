#pragma once

#include <cassert>
#include <cstddef>
#include <vector>
#include <memory>
#include <stb_image.h>
#include <array>

#include <glm/glm.hpp>


template<typename PixelType>
class Image
{
public:
    Image(int wid, int hei, PixelType clearColor)
        : data_(wid*hei, clearColor)
        , width_(wid)
        , height_(hei)
    { }

    int width() const { return width_; }
    int height() const { return height_; }

    PixelType& at(int x, int y)
    {
        assert(x >= 0 && x < width_);
        assert(y >= 0 && y < height_);
        size_t pixel_index = y*width() + x;
        return data_[pixel_index];
    }

    const PixelType& at(int x, int y) const
    {
        assert(x >= 0 && x < width_);
        assert(y >= 0 && y < height_);
        size_t pixel_index = y*width() + x;
        return data_[pixel_index];
    }
private:
    std::vector<PixelType> data_;
    int width_;
    int height_;
};

struct Triangle
{
    std::array<glm::vec3, 3> v;
    std::array<glm::vec3, 3> normal;
    std::array<glm::vec2, 3> uv;
    bool hasUV = true;
};


struct Texture {
    int width = 0, height = 0;
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> pixels{nullptr, stbi_image_free};

    glm::vec3 sample(glm::vec2 uv) const;
};

struct Vertex {
    glm::vec2 screen;
    float depth, inverseW;
    glm::vec3 world, normal;
    glm::vec2 uv;
};

Vertex vertexShader(glm::vec3 world, glm::vec3 normal, glm::vec2 uv, const glm::mat4& vp, glm::vec2 size);
glm::vec3 fragmentShader(glm::vec3 world, glm::vec3 normal, glm::vec2 uv,
                         glm::vec3 light, glm::vec3 eye, const Texture* texture);

void drawLine(glm::ivec2 s, glm::ivec2 e, glm::vec3 col, Image<glm::vec3>& img);

void drawLine(glm::vec2 s, glm::vec2 e, glm::vec3 col, Image<glm::vec3>& img);


void writeImg(const Image<glm::vec3>& img, const char* path, bool flipY=true);

std::vector<Triangle> loadObj(const char* path);

