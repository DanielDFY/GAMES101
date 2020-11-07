# Assignment 6: Acceleration Structure

Implement **Bounding Volume Hierarchy (BVH)**.

Implement [**Surface Area Heuristic (SAH)**](http://15462.courses.cs.cmu.edu/fall2015/lecture/acceleration/slide_024)



* `intersect` in `Bounding_box.cpp` :

  Test if the given ray intersects current bounding box.
  
* `recursive_build` in `BVH.cpp` :

  Implement two different methods to construct BVH (normal method and SAH method).
  
* `intersect` in `BVH.cpp` :

  Recursively calculate intersection for a given ray and a built BVH tree.



To enable SAH for BVH construction, uncomment the definition of macro `SVH` in `main.cpp`.



## Run

Modify the path in `CMakeLists.txt`, then

```shell
mkdir build
cd build
cmake ..

make
./RayTracing	(save the result image in to file output.png)
```



## Image

A bunny formed by triangle mesh.

![output](image/output.png)



## Evaluation

* **Normal BVH** (less time for construction, more time for searching)

```
 - Generating BVH for Bunny...
BVH Generation complete: 
Time Taken: 0 hrs, 0 mins, 0 secs

 - Generating BVH for Scene...
BVH Generation complete: 
Time Taken: 0 hrs, 0 mins, 0 secs

 - Rendering Scene...
[======================================================================] 100 %
Render complete: 
Time taken: 0 hours
          : 0 minutes
          : 8 seconds
```

* **BVH with SAH** (more  time for construction, less time for searching)

```
 - Generating BVH for Bunny with SAH...
BVH Generation complete: 
Time Taken: 0 hrs, 0 mins, 1 secs

 - Generating BVH for Scene with SAH...
BVH Generation complete: 
Time Taken: 0 hrs, 0 mins, 0 secs

 - Rendering Scene...
[======================================================================] 100 %
Render complete: 
Time taken: 0 hours
          : 0 minutes
          : 7 seconds
```

