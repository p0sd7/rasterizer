#include <lib.hpp>


int main()
{
    Image img(1920, 1080, glm::vec3(0.0f));

    for (int i = 0; i < img.width(); ++i) {
        for (int j = 0; j < img.height(); ++j) {
            glm::vec3 col{ (static_cast<float>(i) / static_cast<float>(img.width()) - 1.0f), (static_cast<float>(j) / static_cast<float>(img.height() - 1.0f)), 0.0 };
            img.at(i, j) = col;
        }
    }
    writeImg(img, "points.png");

    return 0;
}

