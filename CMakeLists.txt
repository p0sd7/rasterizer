cmake_minimum_required(VERSION 3.16)


project(rasterizer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(ENABLE_WERROR "Enable thorough warnings check (-Wall -Wextra -Wpedantic -Werror )" OFF)

if (ENABLE_SANITIZERS)
    if (MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
    endif()
endif()

# Dependencies
add_subdirectory(third_party/glm)
if (MSVC)
    target_compile_options(glm PRIVATE /bigobj)
endif()

add_library(stb STATIC)
target_include_directories(stb PUBLIC third_party/stb)
target_sources(stb PRIVATE third_party/stb/stb_image_impl.cpp)

add_library(tiny_obj_loader STATIC)
target_include_directories(tiny_obj_loader PUBLIC third_party/tiny_obj_loader)
target_sources(tiny_obj_loader PRIVATE third_party/tiny_obj_loader/tiny_obj_loader_impl.cpp)


# Structure
add_library(lib STATIC)
target_include_directories(lib PUBLIC src/lib)
target_sources(lib PRIVATE src/lib/lib.cpp)
target_link_libraries(lib PUBLIC stb tiny_obj_loader glm::glm)

# Executables
add_executable(points src/points.cpp)
target_link_libraries(points PRIVATE lib)

add_executable(lines src/lines.cpp)
target_link_libraries(lines PRIVATE lib)

add_executable(model src/model.cpp)
target_link_libraries(model PRIVATE lib)


if (ENABLE_WERROR)
    if (MSVC)
        target_compile_options(lib PRIVATE /W4 /WX)
        target_compile_options(points PRIVATE /W4 /WX)
        target_compile_options(lines PRIVATE /W4 /WX)
        target_compile_options(model PRIVATE /W4 /WX)
    else()
        target_compile_options(lib PRIVATE -Wall -Wextra -Wpedantic -Werror)
        target_compile_options(points PRIVATE -Wall -Wextra -Wpedantic -Werror)
        target_compile_options(lines PRIVATE -Wall -Wextra -Wpedantic -Werror)
        target_compile_options(model PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

