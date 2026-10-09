#pragma once

#include <cassert>
#include <cstddef>
#include <vector>
#include <stdexcept>
#include <array>

#include <glm/glm.hpp>


template<typename... T>
[[noreturn]] inline void implementMe(const T&... args)
{
    (((void)args),...);
    throw std::runtime_error("Implement me!");
}

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
};


void drawLine(glm::ivec2 s, glm::ivec2 e, glm::vec3 col, Image<glm::vec3>& img);

void drawWireframeTriangle(glm::ivec2 a, glm::ivec2 b, glm::ivec2 c, glm::vec3 col, Image<glm::vec3>& img);

void drawLine(glm::vec2 s, glm::vec2 e, glm::vec3 col, Image<glm::vec3>& img);

void drawWireframeTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec3 col, Image<glm::vec3>& img);


void writeImg(const Image<glm::vec3>& img, const char* path, bool flipY=true);

std::vector<Triangle> loadObj(const char* path);

