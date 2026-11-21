#include "lib.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>

#include <tiny_obj_loader.h>
#include <glm/ext.hpp>
#include <glm/ext/vector_float3.hpp>
#include <stb_image_write.h>


void drawLine(glm::ivec2 s, glm::ivec2 e, glm::vec3 col, Image<glm::vec3>& img)
{
    int dx = std::abs(e.x - s.x);
    int dy = -std::abs(e.y - s.y);
    int sx = s.x < e.x ? 1 : -1;
    int sy = s.y < e.y ? 1 : -1;
    int err = dx + dy;

    int x = s.x;
    int y = s.y;
    while (true)
    {
        if (x >= 0 && x < img.width() && y >= 0 && y < img.height())
        {
            img.at(x, y) = col;
        }
        if (x == e.x && y == e.y)
            break;

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y += sy;
        }
    }
}

void drawWireframeTriangle(glm::ivec2 a, glm::ivec2 b, glm::ivec2 c, glm::vec3 col, Image<glm::vec3>& img)
{
    drawLine(a, b, col, img);
    drawLine(b, c, col, img);
    drawLine(c, a, col, img);
}

void drawLine(glm::vec2 s, glm::vec2 e, glm::vec3 col, Image<glm::vec3>& img)
{
    glm::vec2 max(glm::ivec2(img.width() - 1, img.height() - 1));
    drawLine(
        glm::ivec2(s * max),
        glm::ivec2(e * max),
        col,
        img);
}

void drawWireframeTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec3 col, Image<glm::vec3>& img)
{
    glm::vec2 max(glm::ivec2(img.width() - 1, img.height() - 1));
    drawWireframeTriangle(glm::ivec2(a * max), glm::ivec2(b * max), glm::ivec2(c * max), col, img);
}

void writeImg(const Image<glm::vec3>& img, const char* path, bool flipY)
{
    std::vector<std::byte> out;
    out.reserve(3 * img.width() * img.height());
    for (int y = 0; y < img.height(); y++)
    {
        for (int x = 0; x < img.width(); x++)
        {
            glm::vec3 col = img.at(x, y);
            out.push_back(static_cast<std::byte>(std::lround(std::clamp(col.x, 0.0f, 1.0f) * 255.0f)));
            out.push_back(static_cast<std::byte>(std::lround(std::clamp(col.y, 0.0f, 1.0f) * 255.0f)));
            out.push_back(static_cast<std::byte>(std::lround(std::clamp(col.z, 0.0f, 1.0f) * 255.0f)));
        }
    }
    stbi_flip_vertically_on_write(flipY ? 1 : 0);
    int res = stbi_write_png(path, img.width(), img.height(), 3, out.data(), 3 * img.width());
    if (res)
    {
        std::cout << "Saved image to: " << path << std::endl;
    }
    else
    {
        std::cerr << "Failed to save image to: " << path << std::endl;
    }
}

std::vector<Triangle> loadObj(const char* path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    std::vector<Triangle> verts;
    bool res = tinyobj::LoadObj(&attrib, &shapes,
         &materials, &err,
         path, NULL, true);
    if (!res)
    {
        std::cerr << "Failed loading mesh: " << err << std::endl;
        return verts;
    }

    for (auto& shape: shapes)
    {
        auto& mesh = shape.mesh;
        size_t idx = 0;
        for (size_t i = 0; i < mesh.num_face_vertices.size(); i++)
        {
            assert(mesh.num_face_vertices[i] == 3);
            verts.emplace_back();
            for (size_t j = 0; j < mesh.num_face_vertices[i]; j++)
            {
                const auto index = mesh.indices[idx++];
                size_t vid = index.vertex_index;
                int t = index.texcoord_index;
                if (t >= 0)
                    verts.back().uv[j] = glm::vec2(attrib.texcoords[2*t], attrib.texcoords[2*t+1]);
                else {
                    verts.back().hasUV = false;
                    verts.back().uv[j] = glm::vec2(0);
                }
                int n = index.normal_index;
                verts.back().normal[j] = n < 0 ? glm::vec3(0) : glm::vec3(
                    attrib.normals[3*n], attrib.normals[3*n+1], attrib.normals[3*n+2]);
                verts.back().v[j] = glm::vec3(
                    attrib.vertices[vid * 3 + 0],
                    attrib.vertices[vid * 3 + 1],
                    attrib.vertices[vid * 3 + 2]
                );
            }
        }
    }

    return verts;
}

