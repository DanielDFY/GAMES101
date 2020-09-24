cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

add_executable(RayTracing main.cpp stb_image_write.h Object.hpp Sphere.hpp Sphere.cpp Utility.hpp Triangle.hpp Triangle.cpp Scene.hpp Light.hpp Renderer.cpp)
target_compile_options(RayTracing PUBLIC -Wall -Wextra -pedantic -Wshadow -Wreturn-type -fsanitize=undefined)
target_compile_features(RayTracing PUBLIC cxx_std_17)
target_link_libraries(RayTracing PUBLIC -fsanitize=undefined)
