#include "lib.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>

#include <tiny_obj_loader.h>
#include <stb_image_write.h>


Vertex vertexShader(glm::vec3 world, glm::vec3 normal, glm::vec2 uv, const glm::mat4& vp, glm::vec2 size)
{
    glm::vec4 clip = vp * glm::vec4(world, 1);
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    return {(glm::vec2(ndc) * .5f + .5f) * size, ndc.z, 1.f / clip.w, world, normal, uv};
}

glm::vec3 Texture::sample(glm::vec2 uv) const {
    int x = std::clamp(int(uv.x * width), 0, width - 1);
    int y = std::clamp(int((1.f - uv.y) * height), 0, height - 1);
    auto rgb = pixels.get() + 3 * (y * width + x);
    return glm::vec3(rgb[0], rgb[1], rgb[2]) / 255.f;
}

glm::vec3 fragmentShader(glm::vec3 world, glm::vec3 normal, glm::vec2 uv,
                         glm::vec3 light, glm::vec3 eye, const Texture* texture)
{
    glm::vec3 albedo = texture ? texture->sample(uv) : glm::vec3(.35f,.65f,.9f);
	glm::vec3 n = glm::normalize(normal); // normal
    glm::vec3 l = glm::normalize(light - world); // from object to light
	glm::vec3 v = glm::normalize(eye - world); // from object to camera
    float diffuse = std::max(glm::dot(n, l), 0.f);
    constexpr float shininess = 16.f;
    constexpr float specularStrength = .08f;
    float specular = diffuse > 0 ? std::pow(std::max(glm::dot(glm::reflect(-l,n), v), 0.f), shininess) : 0.f;
    return albedo * (.12f + .78f * diffuse) + glm::vec3(specularStrength * specular);
}

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

void drawLine(glm::vec2 s, glm::vec2 e, glm::vec3 col, Image<glm::vec3>& img)
{
    glm::vec2 max(glm::ivec2(img.width() - 1, img.height() - 1));
    drawLine(
        glm::ivec2(s * max),
        glm::ivec2(e * max),
        col,
        img);
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

