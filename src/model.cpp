#include <lib.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

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
};

Vertex vertexShader(glm::vec3 world, glm::vec3 normal, const glm::mat4& vp, glm::vec2 size)
{
    glm::vec4 clip = vp * glm::vec4(world, 1);
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    return {(glm::vec2(ndc) * .5f + .5f) * size, ndc.z, 1.f / clip.w, world, normal};
}

glm::vec3 phong(glm::vec3 world, glm::vec3 normal, glm::vec3 light, glm::vec3 eye)
{
    glm::vec3 n = glm::normalize(normal);
    glm::vec3 l = glm::normalize(light - world);
    glm::vec3 v = glm::normalize(eye - world);
    float diffuse = std::max(glm::dot(n, l), 0.f);
    float specular = diffuse > 0 ? std::pow(std::max(glm::dot(glm::reflect(-l,n), v), 0.f), 32.f) : 0.f;
    return glm::vec3(.35f,.65f,.9f) * (.12f + .78f * diffuse) + glm::vec3(.55f * specular);
}

float edge(glm::vec2 a, glm::vec2 b, glm::vec2 p)
{
    return (b.x-a.x)*(p.y-a.y) - (b.y-a.y)*(p.x-a.x);
}

void rasterize(Vertex a, Vertex b, Vertex c, glm::vec3 light, glm::vec3 eye,
               Image<glm::vec3>& image, Image<float>& depth)
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
            // World-space attributes need perspective-correct weights.
            w *= glm::vec3(a.inverseW,b.inverseW,c.inverseW);
            w /= w.x + w.y + w.z;
            image.at(x,y) = phong(w.x*a.world + w.y*b.world + w.z*c.world,
                                  w.x*a.normal + w.y*b.normal + w.z*c.normal, light,eye);
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: model <path.obj>\n";
        return 1;
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
    Camera camera{center + glm::vec3(.45f,.2f,2.8f)*radius, center};
    glm::vec3 light = center + glm::vec3(-2,3,4)*radius;
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
            vertices[i] = vertexShader(p[i], glm::dot(n,n)>0 ? n : glm::normalize(face), vp,size);
        }
        rasterize(vertices[0],vertices[1],vertices[2],light,camera.position,image,depth);
    }
    writeImg(image,"model.png");
}
