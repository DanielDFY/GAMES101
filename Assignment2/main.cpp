#include <iostream>

#include <opencv2/opencv.hpp>

#include "Triangle.hpp"
#include "Rasterizer.hpp"

// Enable 4x SSAA
#define ANTI_ALIASING

constexpr float MY_PI = 3.1415926f;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    // Construct the view matrix by moving eye to the origin
    view << 
        1, 0, 0, -eye_pos[0],
        0, 1, 0, -eye_pos[1],
        0, 0, 1, -eye_pos[2],
        0, 0, 0, 1;

    return view;
}

Eigen::Matrix4f get_model_matrix() {
    return Eigen::Matrix4f::Identity();
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float z_near, float z_far) {
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // Create the projection matrix with the given parameters.

    // Preparation
    const auto eye_fov_rad = eye_fov / 180.0f * MY_PI;
    const auto t = z_near * tan(eye_fov_rad /2.0f); // top plane
    const auto r = t * aspect_ratio;    // right plane
    const auto l = -r;  // left plane
    const auto b = -t;  // bottom plane
    const auto n = -z_near; // near plane
    const auto f = -z_far;  // far plane

    // Frustum -> Cuboid
    Eigen::Matrix4f persp_to_ortho;
    persp_to_ortho << 
        n,    0,    0,      0,
        0,    n,    0,      0,
        0,    0,    n+f,    -n*f,
        0,    0,    1,      0;

    // Orthographic Projection
    Eigen::Matrix4f ortho_proj;
    ortho_proj << 
        2.0f/(r-l), 0,          0,          -(r+l)/(r-l),
        0,          2.0f/(t-b), 0,          -(t+b)/(t-b),
        0,          0,          2.0f/(n-f), -(n+f)/(n-f),
        0,          0,          0,          1;

    // Final perspective projection
    return ortho_proj * persp_to_ortho * projection;
}

int main(int argc, const char** argv) {
    bool real_time = true;
    std::string filename = "output.png";

    if (argc == 2) {
        real_time = false;
        filename = std::string(argv[1]);
    }

    #ifdef ANTI_ALIASING
    RST::Rasterizer r(700, 700, true);
    #else
    RST::Rasterizer r(700, 700);
    #endif

    Eigen::Vector3f eye_pos = {0,0,5};

    std::vector<Eigen::Vector3f> pos {
        {2.0f, 0.0f, -2.0f},
        {0.0f, 2.0f, -2.0f},
        {-2.0f, 0.0f, -2.0f},
        {3.5f, -1.0f, -5.0f},
        {2.5f, 1.5f, -5.0f},
        {-1.0f, 0.5f, -5.0f}
    };

    std::vector<Eigen::Vector3i> ind {
        {0, 1, 2},
        {3, 4, 5}
    };

    std::vector<Eigen::Vector3f> cols {
        {217.0f, 238.0f, 185.0f},
        {217.0f, 238.0f, 185.0f},
        {217.0f, 238.0f, 185.0f},
        {185.0f, 217.0f, 238.0f},
        {185.0f, 217.0f, 238.0f},
        {185.0f, 217.0f, 238.0f}
    };

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);
    auto col_id = r.load_colors(cols);

    int key = 0;
    int frame_count = 0;

    // Generate image
    if (!real_time)
    {
        r.clear(RST::Buffers::Color | RST::Buffers::Depth);

        r.set_model(get_model_matrix());
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0f, 1.0f, 0.1f, 50.0f));

        r.draw(pos_id, ind_id, col_id, RST::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    // Real-time display
    while(key != 27)
    {
        r.clear(RST::Buffers::Color | RST::Buffers::Depth);

        r.set_model(get_model_matrix());
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0f, 1.0f, 0.1f, 50.0f));

        r.draw(pos_id, ind_id, col_id, RST::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
    }

    return 0;
}