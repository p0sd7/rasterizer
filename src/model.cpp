#include <lib.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <filesystem>
#include <memory>
#include <stb_image.h>

struct Texture {
    int width = 0, height = 0;
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> pixels{nullptr, stbi_image_free};

    glm::vec3 sample(glm::vec2 uv) const {
        int x = std::clamp(int(uv.x * width), 0, width - 1);
        int y = std::clamp(int((1.f - uv.y) * height), 0, height - 1);
        auto rgb = pixels.get() + 3 * (y * width + x);
        return glm::vec3(rgb[0], rgb[1], rgb[2]) / 255.f;
    }
};

class Camera
{
public:
    glm::vec3 position, target;
    glm::mat4 view() const {
        return glm::lookAt(position, target, glm::vec3(0, 1, 0));
    }
    glm::mat4 projection(float aspect, float radius) const {
        return glm::perspective(glm::radians(45.f), aspect, radius * .01f, radius * 20.f);
    }
};

struct Vertex {
    glm::vec2 screen;
    float depth, inverseW;
    glm::vec3 world, normal;
    glm::vec2 uv;
};

Vertex vertexShader(glm::vec3 world, glm::vec3 normal, glm::vec2 uv, const glm::mat4& vp, glm::vec2 size)
{
    glm::vec4 clip = vp * glm::vec4(world, 1);
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    return {(glm::vec2(ndc) * .5f + .5f) * size, ndc.z, 1.f / clip.w, world, normal, uv};
}

glm::vec3 phong(glm::vec3 world, glm::vec3 normal, glm::vec3 light, glm::vec3 eye, glm::vec3 albedo)
{
    glm::vec3 n = glm::normalize(normal);
    glm::vec3 l = glm::normalize(light - world);
    glm::vec3 v = glm::normalize(eye - world);
    float diffuse = std::max(glm::dot(n, l), 0.f);
    constexpr float shininess = 16.f;
    constexpr float specularStrength = .08f;
    float specular = diffuse > 0 ? std::pow(std::max(glm::dot(glm::reflect(-l,n), v), 0.f), shininess) : 0.f;
    return albedo * (.12f + .78f * diffuse) + glm::vec3(specularStrength * specular);
}

float edge(glm::vec2 a, glm::vec2 b, glm::vec2 p)
{
    return (b.x-a.x)*(p.y-a.y) - (b.y-a.y)*(p.x-a.x);
}

void rasterize(Vertex a, Vertex b, Vertex c, glm::vec3 light, glm::vec3 eye,
               Image<glm::vec3>& image, Image<float>& depth, const Texture* texture)
{
    float area = edge(a.screen,b.screen,c.screen);
    if (std::abs(area) < 1e-7f) return;
    glm::vec2 low = glm::min(a.screen, glm::min(b.screen,c.screen));
    glm::vec2 high = glm::max(a.screen, glm::max(b.screen,c.screen));
    for (int y = std::max(0,int(std::floor(low.y))); y <= std::min(image.height()-1,int(std::ceil(high.y))); ++y) {
        for (int x = std::max(0,int(std::floor(low.x))); x <= std::min(image.width()-1,int(std::ceil(high.x))); ++x) {
            glm::vec2 pixel(x+.5f,y+.5f);
            glm::vec3 w(edge(b.screen,c.screen,pixel), edge(c.screen,a.screen,pixel), edge(a.screen,b.screen,pixel));
            w /= area;
            if (w.x < 0 || w.y < 0 || w.z < 0) continue;
            float z = w.x*a.depth + w.y*b.depth + w.z*c.depth;
            if (z >= depth.at(x,y)) continue;
            depth.at(x,y) = z;
            w *= glm::vec3(a.inverseW,b.inverseW,c.inverseW);
            w /= w.x + w.y + w.z;
            glm::vec3 albedo = texture ? texture->sample(w.x*a.uv + w.y*b.uv + w.z*c.uv) : glm::vec3(.35f,.65f,.9f);
            image.at(x,y) = phong(w.x*a.world + w.y*b.world + w.z*c.world,
                                  w.x*a.normal + w.y*b.normal + w.z*c.normal, light,eye,albedo);
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: model <path.obj>\n";
        return 1;
    }

    std::filesystem::path texturePath(argv[1]);
    texturePath.replace_filename(texturePath.stem().string() + "_diffuse.tga");
    Texture texture;
    if (std::filesystem::exists(texturePath)) {
        int channels;
        texture.pixels.reset(stbi_load(texturePath.string().c_str(), &texture.width, &texture.height, &channels, 3));
        if (!texture.pixels) {
            std::cerr << "Cannot load texture: " << texturePath << '\n';
            return 1;
        }
    }
    auto triangles = loadObj(argv[1]);
    if (triangles.empty()) return 1;

    glm::vec3 low = triangles.front().v[0], high = low;
    for (const auto& triangle : triangles)
        for (auto p : triangle.v) {
            low = glm::min(low,p);
            high = glm::max(high,p);
        }
    glm::vec3 center = (low+high)*.5f;
    float radius = std::max(glm::length(high-low)*.5f, .001f);
    Camera camera{center + glm::vec3(.0f,.0f,2.0f)*radius, center};
    glm::vec3 light = center + glm::vec3(0,0,4.f)*radius;
    Image image(1024,1024,glm::vec3(.035f));
    Image depth(image.width(),image.height(),std::numeric_limits<float>::infinity());
    glm::vec2 size(image.width(),image.height());

    auto vp = camera.projection(size.x/size.y,radius) * camera.view();

    for (const auto& triangle : triangles) {
        const auto& p = triangle.v;
        auto face = glm::cross(p[1]-p[0],p[2]-p[0]);
        if (glm::dot(face,face) == 0) continue;
        std::array<Vertex,3> vertices;
        for (int i=0;i<3;++i) {
            auto n = triangle.normal[i];
            vertices[i] = vertexShader(p[i], glm::dot(n,n)>0 ? n : glm::normalize(face), triangle.uv[i], vp,size);
        }
        rasterize(vertices[0],vertices[1],vertices[2],light,camera.position,image,depth,
                  triangle.hasUV && texture.pixels ? &texture : nullptr);
    }

    writeImg(image,"model.png");
}
