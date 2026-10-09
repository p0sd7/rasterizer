#include <lib.hpp>

#include <cmath>
#include <numbers>


int main()
{
    Image img(1024, 1024, glm::vec3(0.0f));

    int angles = 64;
    for (int a = 0; a < angles; a++)
    {
        float angle = 2.0f * std::numbers::pi * float(a) / float(angles);
        float s = std::sin(angle);
        float c = std::cos(angle);
        glm::vec2 cs = glm::vec2(c, s);

        glm::vec3 col = glm::vec3(0.0f, float(a) / float(angles - 1), 1.0f - float(a) / float(angles - 1));
        drawLine(glm::vec2(0.5f) + cs * 0.1f, glm::vec2(0.5f) + cs * 0.4f, col, img);
    }

    writeImg(img, "lines.png");

    return 0;
}

