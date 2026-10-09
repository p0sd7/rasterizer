#include <lib.hpp>


int main()
{
    Image img(64, 64, glm::vec3(0.0f));

    img.at(10, 20) = glm::vec3(1.0f, 2.0f, 3.0) / 255.0f;
    img.at(11, 20) = glm::vec3(10.0, 11.0f, 12.0) / 255.0f;
    img.at(12, 20) = glm::vec3(100.0, 101.0, 102.0f) / 255.0f;

    img.at(10, 21) = glm::vec3(253.0f, 0.0, 0.0) / 255.f;
    img.at(11, 21) = glm::vec3(0.0, 254.0f, 0.0) / 255.f;
    img.at(12, 21) = glm::vec3(0.0, 0.0, 255.0f) / 255.f;

    writeImg(img, "points.png");

    return 0;
}

