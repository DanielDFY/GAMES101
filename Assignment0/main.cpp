#include<cmath>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<iostream>

int main(){

    // Basic Example of cpp
    std::cout << "********************* Basic Example of cpp *********************" << std::endl;
    float a = 1.0f, b = 2.0f;
    std::cout << a << std::endl;
    std::cout << a/b << std::endl;
    std::cout << std::sqrt(b) << std::endl;
    std::cout << std::acos(-1) << std::endl;
    std::cout << std::sin(30.0/180.0*acos(-1)) << std::endl;

    // Example of vector
    std::cout << "********************* Example of vector *********************" << std::endl;
    // vector definition
    Eigen::Vector3f v(1.0f,2.0f,3.0f);
    Eigen::Vector3f w(1.0f,0.0f,0.0f);
    // vector output
    std::cout << "Example of output \n";
    std::cout << v << std::endl;
    std::cout << std::endl;
    // vector add
    std::cout << "Example of add \n";
    std::cout << v + w << std::endl;
    std::cout << std::endl;
    // vector scalar multiply
    std::cout << "Example of scalar multiply \n";
    std::cout << v * 3.0f << std::endl;
    std::cout << std::endl;
    std::cout << 2.0f * v << std::endl;
    std::cout << std::endl;

    // Example of matrix
    std::cout << "********************* Example of matrix *********************" << std::endl;
    // matrix definition
    Eigen::Matrix3f i,j;
    i << 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f;
    j << 2.0f, 3.0f, 1.0f, 4.0f, 6.0f, 5.0f, 9.0f, 7.0f, 8.0f;
    // matrix output
    std::cout << "Example of output \n";
    std::cout << i << std::endl;
    std::cout << std::endl;
    // matrix add i + j
    std::cout << i + j << std::endl;
    std::cout << std::endl;
    // matrix scalar multiply i * 2.0
    std::cout << i * 2.0f << std::endl;
    std::cout << std::endl;
    // matrix multiply i * j
    std::cout << i * j << std::endl;
    std::cout << std::endl;
    // matrix multiply vector i * v
    std::cout << i * v <<std::endl;
    std::cout << std::endl;

    std::cout << "********************* transformation *********************" << std::endl;
    // Given point(2, 1), rotate it around the origin
    // for 45 degrees, then translate it with vector(1, 2).
    Eigen::Vector3f p(2.0f, 1.0f, 1.0f);
    Eigen::Matrix3f rotation, translation;
    constexpr float theta = 45.0f / 180.0f * M_PI;

    rotation << 
        std::cos(theta),    -std::sin(theta),   0,
        std::sin(theta),    std::cos(theta),    0,
        0,                  0,                  1;
    translation << 
        1,                  0,                  1,
        0,                  1,                  2,
        0,                  0,                  1;
    
    std::cout << translation * rotation * p << std::endl;

    return 0;
}