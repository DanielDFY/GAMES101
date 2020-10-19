cmake_minimum_required (VERSION 3.10)

project ("RayTracing")

set(CMAKE_CXX_STANDARD 17)

add_executable(RayTracing main.cpp Object.hpp Sphere.hpp Sphere.cpp Utility.hpp Utility.cpp Triangle.hpp Triangle.cpp
        Scene.hpp Scene.cpp Light.hpp Area_light.hpp Area_light.cpp BVH.hpp BVH.cpp Bounding_box.hpp Bounding_box.cpp
        Ray.hpp Ray.cpp Material.hpp Material.cpp Intersection.hpp Renderer.hpp Renderer.cpp Vector.hpp
        stb_image_write.h OBJ_Loader.hpp)

