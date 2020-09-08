#pragma once

#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>

enum class Sample_method {
    Default = 1,    // no interpolation
    Bilinear = 2    // bilinear interpolation
};

class Texture {
public:
    Texture(const std::string& name);

    int get_width() { return image_data.cols; }

    int get_height() { return image_data.rows; }

    Eigen::Vector3f get_color(float u, float v, Sample_method method);

private:
    cv::Mat image_data;
};