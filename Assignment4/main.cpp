#include <iostream>
#include <chrono>
#include <queue>

#include <opencv2/opencv.hpp>

// Enable anti-aliasing
// #define ANTI_ALIASING

constexpr float t_step = 0.001f;

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) {
    if (event == cv::EVENT_LBUTTONDOWN && (control_points.size() < 4)) {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) {
    const auto &p_0 = points[0];
    const auto &p_1 = points[1];
    const auto &p_2 = points[2];
    const auto &p_3 = points[3];

    for (float t = 0.0; t <= 1.0f; t += t_step) {
        const auto point = std::pow(1.0f - t, 3) * p_0
                    + 3 * t * std::pow(1.0f - t, 2) * p_1
                    + 3 * std::pow(t, 2) * (1.0f - t) * p_2
                    + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) {
    // Implement de Casteljau's algorithm
    const auto num_control_points = control_points.size();
    if (num_control_points == 1) {
        return control_points[0];
    } else {
        std::vector<cv::Point2f> left_control_points(&(control_points[0]), &(control_points[num_control_points-1]));
        std::vector<cv::Point2f> right_control_points(&(control_points[1]), &(control_points[num_control_points]));

        const auto left_control_point = recursive_bezier(left_control_points, t);
        const auto right_control_point = recursive_bezier(right_control_points, t);

        return left_control_point * (1.0f-t) + right_control_point * t;
    }
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window)  {
    // Iterate through all t = 0 to t = 1 with small steps,
    // and call de Casteljau's recursive Bezier algorithm.
    for (float t = 0.0f; t <= 1.0f; t += t_step) {
        const auto point = recursive_bezier(control_points, t);
        #ifdef ANTI_ALIASING
        // Anti-aliasing

        // Find the closest 4 pixels.
        const auto base_x = floor(point.x);
        const auto base_y = floor(point.y);
        const auto x_diff = point.x - base_x;
        const auto y_diff = point.y - base_y;

        const auto min_x = std::min(std::max(0, (x_diff < 0.5f ? static_cast<int>(base_x) - 1 : static_cast<int>(base_x))), window.cols); 
        const auto max_x = std::min(std::max(0, (x_diff < 0.5f ? static_cast<int>(base_x) : static_cast<int>(base_x) + 1)), window.cols); 
        const auto min_y = std::min(std::max(0, (y_diff < 0.5f ? static_cast<int>(base_y) - 1 : static_cast<int>(base_y))), window.rows); 
        const auto max_y = std::min(std::max(0, (y_diff < 0.5f ? static_cast<int>(base_y) : static_cast<int>(base_y) + 1)), window.rows); 

        // Calculate the squared distance to the center of each pixel and find the shortest.
        std::queue<float> queue_dist_squared;
        constexpr float min_dist_squared = 2.0f;  // squared distance will not be more than 2.
        for (const auto x : {min_x, max_x}) {
            for (const auto y : {min_y, max_y}) {
                const auto pixel_center_x = static_cast<float>(x) + 0.5f;
                const auto pixel_center_y = static_cast<float>(y) + 0.5f;
                const auto dist_squared = std::pow(point.x - pixel_center_x, 2) + std::pow(point.y - pixel_center_y, 2);
                queue_dist_squared.push(dist_squared);
                if (dist_squared < min_dist_squared) min_dist_squared = dist_squared;
            }
        }

        // Calculate the color value of each pixel
        for (const auto x : {min_x, max_x}) {
            for (const auto y : {min_y, max_y}) {
                // based on the relative ratio value of distance
                const auto intensity = min_dist_squared / queue_dist_squared.front();
                queue_dist_squared.pop();

                window.at<cv::Vec3b>(y, x)[1] = std::max(static_cast<uchar>(255 * intensity), window.at<cv::Vec3b>(y, x)[1]);
            }
        }
        #else
        // No Anti-aliasing
        window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
        #endif
    }
}

int main()  {
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) {
        for (const auto &point : control_points) {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) {
            #ifndef ANTI_ALIASING
            naive_bezier(control_points, window);
            #endif
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

    return 0;
}
