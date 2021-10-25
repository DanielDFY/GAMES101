# GAMES101

This repo is based on the assignments of *Computer Graphics* course [GAMES101](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html) with *code refactoring* and *additional features*.



## Environment

* Language Standard: C++ 17
* Build: CMake
* Additional Libraries:
  * Eigen (linear algebra)
  * OpenCV, stb_image (image processing)
  * OBJ Loader (model loading)



## Gallery

Implement both rasterization and ray tracing.

### Rasterization

* Real-time displaying a textured cow with Blinn-Phong lighting model and bilinear interpolation texture sampling.

  ![Assignment3](Assignment3/image/output_texture_bilinear.png)

  

* Bézier Curve generation with anti-aliasing.

  ![Assignment4](Assignment4/image/bezier_curve_with_anti_aliasing.png)



### Ray Tracing

* Whitted-Style ray tracing (Blinn-Phong lighting model with shadow effects and different materials).

  ![Assignment5](Assignment5/image/output.png)

  

* Bounding Volume Hierarchy (implement naive BVH and BVH with SAH partition)

  ![Assignment6](Assignment6/image/output.png)

  

* Path Tracing (BSDF with microfacet model, multiple importance sampling, anti-aliasing)

  ![Assignment7](Assignment7/image/frosted_glass_64spp.png)



* Rope Simulation (Semi-implicit Euler & Explicit Verlet)

  ![Assignment8](Assignment8/image/semi_implicit_euler_&_explicit_verlet_1024.jpg)
