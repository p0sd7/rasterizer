#include <lib.hpp>
#include <iostream>


int main(int argc, char** argv)
{
    if (argc <= 1)
    {
        std::cerr << "Path to the model expected as the first argument!\n";
        const char* execPath = argc > 0 && argv[0] && argv[0][0] ? argv[0] : "model";
        std::cerr << "Usage: " << execPath << " <path.obj>" << std::endl;
        return 1;
    }
    const char* path = argv[1];
    std::cout << "Model path: " << path << std::endl;

    Image img(2048, 2048, glm::vec3(0.0f));
    auto tris = loadObj(path);
    if (tris.empty())
        return 1;

    for (auto& tri: tris)
    {
        drawWireframeTriangle(
            glm::vec2(tri.v[0].x, tri.v[0].y) * 0.3f + 0.5f,
            glm::vec2(tri.v[1].x, tri.v[1].y) * 0.3f + 0.5f,
            glm::vec2(tri.v[2].x, tri.v[2].y) * 0.3f + 0.5f,
            glm::vec3(1.0f), img);
    }

    writeImg(img, "model.png");

    return 0;
}

