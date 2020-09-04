#include <iostream>
#include <cmath>

#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>

#include "Triangle.hpp"
#include "Rasterizer.hpp"

// Enable rotation around arbitrary axis that passes the origin
// #define ARBITRARY_AXIS

#ifdef ARBITRARY_AXIS
// Now rotate around given axis
Eigen::Vector3f axis(0.0f, 1.0f, 0.0f);
#endif

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

Eigen::Matrix4f get_model_matrix(float rotation_angle) {
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // Create the model matrix by rotating given triangle around the Z axis.
    const auto rotation_rad = (rotation_angle / 180.0f) * MY_PI;

    model << 
        std::cos(rotation_rad), -std::sin(rotation_rad),    0,          0,
        std::sin(rotation_rad), std::cos(rotation_rad),     0,          0,
        0,                      0,                          1,          0,
        0,                      0,                          0,          1;


    return model;
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
    /*
    Eigen::Matrix4f ortho_proj_scale;
    ortho_proj_scale << 
        2.0f/(r-l), 0,          0,          0,
        0,          2.0f/(t-b), 0,          0,
        0,          0,          2.0f/(n-f), 0,
        0,          0,          0,          1;
    Eigen::Matrix4f ortho_proj_translate;
    ortho_proj_translate << 
        1,          0,          0,          -(r+l)/2.0f,
        0,          1,          0,          -(t+b)/2.0f,
        0,          0,          1,          -(n+f)/2.0f,
        0,          0,          0,          1;
    Eigen::Matrix4f ortho_proj = orthoProj_scale * ortho_proj_translate;
    */
    // Construct the final matrix directly to improve performance
    Eigen::Matrix4f ortho_proj;
    ortho_proj << 
        2.0f/(r-l), 0,          0,          -(r+l)/(r-l),
        0,          2.0f/(t-b), 0,          -(t+b)/(t-b),
        0,          0,          2.0f/(n-f), -(n+f)/(n-f),
        0,          0,          0,          1;

    // Final perspective projection
    return ortho_proj * persp_to_ortho * projection;
}

#ifdef ARBITRARY_AXIS
Eigen::Matrix4f get_rotation(Eigen::Vector3f axis, float rotation_angle) {
    Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();

    // Create the matrix for rotating the triangle around any given axis that passes the origin.
    const auto rotation_rad = (rotation_angle / 180.0f) * MY_PI;

    // Rodrigues’ rotation formula
    Eigen::Matrix3f cross_product_matrix;
    cross_product_matrix << 
        0,          -axis.z(),  axis.y(),
        axis.z(),   0,          -axis.x(),
        -axis.y(),  axis.x(),   0;
    
    rotation.block<3, 3>(0, 0) =
        std::cos(rotation_rad) * Eigen::Matrix3f::Identity() +
        (1.0f - std::cos(rotation_rad)) * axis * axis.transpose() +
        std::sin(rotation_rad) * cross_product_matrix;
    
    return rotation;
}
#endif

int main(int argc, const char** argv) {
    float angle = 0.0f;
    bool real_time = true;
    std::string filename = "output.png";

    if (argc >= 3) {
        real_time = false;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    RST::Rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (!real_time) {
        // false for image generation
        r.clear(RST::Buffers::Color | RST::Buffers::Depth);

        #ifdef ARBITRARY_AXIS
        r.set_model(get_rotation(axis, angle));
        #else
        r.set_model(get_model_matrix(angle));
        #endif
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, RST::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    // true for real-time display
    while (key != 27) { // press `ESC` to close the window
        r.clear(RST::Buffers::Color | RST::Buffers::Depth);

        #ifdef ARBITRARY_AXIS
        r.set_model(get_rotation(axis, angle));
        #else
        r.set_model(get_model_matrix(angle));
        #endif
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0f, 1.0f, 0.1f, 50.0f));

        r.draw(pos_id, ind_id, RST::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        // keyboard controller, A/D for rotating clockwise/counter-clockwise
        if (key == 'a') {
            angle += 10;
        } else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
