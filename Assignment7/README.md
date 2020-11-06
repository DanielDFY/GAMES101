# Assignment 7: Path Tracing

Implement **Path Tracing** with **Cook–Torrance Bidirectional Reflectance Distribution Function (BRDF)**.

Implement **multiple importance sampling** for *direct* light illumination.

Implement **anti-aliasing** and **multi-threading acceleration**.



// todo

* Implement **GGX microfacet model** for **Bidirectional Scattering Distribution Function (BSDF)** with **importance sampling**.

* Implement **Bi-directional Path Tracing (BDPT)**



## Run

Modify the path in `CMakeLists.txt`

```
mkdir build
cd build
cmake ..

make
./RayTracing	(save the result image in to file output.png)
```



## Image

| Scene                         | PT 16 spp                         | PT 64 spp                         |
| ----------------------------- | --------------------------------- | --------------------------------- |
| Cornellbox (diffuse material) | ![output](image/output_16spp.png) | ![output](image/output_64spp.png) |