#include "Texture.hpp"

Texture::Texture(const std::string& name) {
    image_data = cv::imread(name);
    cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
}

Eigen::Vector3f Texture::get_color(float u, float v, Sample_method method) {
    const auto u_idx = u * image_data.cols;
    const auto v_idx = (1 - v) * image_data.rows;

    if (method == Sample_method::Default) {
        const auto color = image_data.at<cv::Vec3b>(v_idx, u_idx);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    } else if (method == Sample_method::Bilinear) {
        cv::Mat patch;
        cv::getRectSubPix(image_data, cv::Size(1, 1), cv::Point2f(u_idx, v_idx), patch);
        const auto color = patch.at<cv::Vec3b>(0, 0);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    } else {
        throw std::runtime_error("unknown sampling method");
    }
}