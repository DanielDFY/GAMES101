cmake_minimum_required (VERSION 3.10)

project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

add_executable(RayTracing main.cpp Math.hpp Math.cpp Object.hpp Object.cpp  Mesh.hpp Mesh.cpp
        Scene.hpp Scene.cpp BVH.hpp BVH.cpp BoundingBox.hpp BoundingBox.cpp 
        Ray.hpp Ray.cpp Material.hpp Material.cpp Intersection.hpp Renderer.hpp Renderer.cpp 
        stb_image_write.h OBJ_Loader.h)