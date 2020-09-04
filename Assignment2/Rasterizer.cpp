#include "Rasterizer.hpp"

#include <algorithm>
#include <vector>
#include <cmath>

#include <opencv2/opencv.hpp>

RST::Pos_buf_id RST::Rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions) {
    const auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

RST::Ind_buf_id RST::Rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices) {
    const auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

RST::Col_buf_id RST::Rasterizer::load_colors(const std::vector<Eigen::Vector3f> &colors) {
    const auto id = get_next_id();
    col_buf.emplace(id, colors);

    return {id};
}

void RST::Rasterizer::draw(Pos_buf_id pos_buffer, Ind_buf_id ind_buffer, Col_buf_id col_buffer, Primitive type) {
    const auto& pos_vec = pos_buf[pos_buffer.pos_id];
    const auto& ind_vec = ind_buf[ind_buffer.ind_id];
    const auto& col_vec = col_buf[col_buffer.col_id];

    constexpr auto f1 = (50.0f - 0.1f) / 2.0f;
    constexpr auto f2 = (50.0f + 0.1f) / 2.0f;

    const Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind_vec) {
        const auto pos0 = pos_vec[i[0]];
        const auto pos1 = pos_vec[i[1]];
        const auto pos2 = pos_vec[i[2]];

        Eigen::Vector4f v[] = {
            mvp * Vector4f(pos0.x(), pos0.y(), pos0.z(), 1.0f),
            mvp * Vector4f(pos1.x(), pos1.y(), pos1.z(), 1.0f),
            mvp * Vector4f(pos2.x(), pos2.y(), pos2.z(), 1.0f)
        };

        for (auto& vertex : v) {
            // Homogeneous division
            vertex /= vertex.w();

            // Viewport transformation
            vertex.x() = 0.5f * width * (vertex.x() + 1.0f);
            vertex.y() = 0.5f * height * (vertex.y() + 1.0f);
            vertex.z() = vertex.z() * f1 + f2;
        }

        Triangle t;
        for (int i = 0; i < 3; ++i) {
            t.set_vertex(i, v[i].head<3>());
            t.set_vertex(i, v[i].head<3>());
            t.set_vertex(i, v[i].head<3>());
        }

        const auto col_x = col_vec[i[0]];
        const auto col_y = col_vec[i[1]];
        const auto col_z = col_vec[i[2]];

        t.set_color(0, col_x[0], col_x[1], col_x[2]);
        t.set_color(1, col_y[0], col_y[1], col_y[2]);
        t.set_color(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

// Screen space rasterization
void RST::Rasterizer::rasterize_triangle(const Triangle& t) {
    const auto vertices = t.to_vector4();
    
    // Find out the bounding box of current triangle. Then iterate through
    // the pixels and find if the current pixel is inside the triangle
    const auto min_max_x = std::minmax_element(vertices.cbegin(), vertices.cend(),
                                    [](const auto& lhs, const auto& rhs) {
                                        return lhs.x() < rhs.x();
                                    });
    const auto min_max_y = std::minmax_element(vertices.cbegin(), vertices.cend(),
                                    [](const auto& lhs, const auto& rhs) {
                                        return lhs.y() < rhs.y();
                                    });

    const int min_x = floor(min_max_x.first->x());
    const int min_y = floor(min_max_y.first->y());
    const int max_x = ceil(min_max_x.second->x());
    const int max_y = ceil(min_max_y.second->y());

    // Set the current pixel to the color of the triangle if it should be painted.
    for (int x = min_x; x <= max_x; ++x) {
        for (int y = min_y; y <= max_y; ++y) {
            if (enable_anti_aliasing) {
                // 4x SSAA
                int valid_super_pixel_counter = 0;
                for (int x_super_i : {0, 1}) {
                    for (int y_super_i : {0, 1}) {
                        const float sample_point_x = static_cast<float>(x) + 0.25f + static_cast<float>(x_super_i) * 0.5f;
                        const float sample_point_y = static_cast<float>(y) + 0.25f + static_cast<float>(y_super_i) * 0.5f;

                        // Get the interpolated z value.
                        auto[alpha, beta, gamma] = t.compute_barycentric(sample_point_x, sample_point_y);
                        float w_reciprocal = 1.0/(alpha / vertices[0].w() + beta / vertices[1].w() + gamma / vertices[2].w());
                        float z_interpolated = alpha * vertices[0].z() / vertices[0].w() + beta * vertices[1].z() / vertices[1].w() + gamma * vertices[2].z() / vertices[2].w();
                        z_interpolated *= w_reciprocal;

                        int super_pixel_index = (y * 2 + y_super_i) * width * 2 + (x * 2 + x_super_i);
                        if (t.inside_triangle(sample_point_x, sample_point_y) && (-z_interpolated < depth_buf[super_pixel_index])) {
                            ++valid_super_pixel_counter;
                            depth_buf[super_pixel_index] = -z_interpolated;
                        }
                    }
                }
                if (valid_super_pixel_counter > 0) {
                    float intensity = static_cast<float>(valid_super_pixel_counter) / 4.0f;
                    blend_pixel({static_cast<float>(x), static_cast<float>(y), 1.0f}, t.get_color() * intensity);
                }
            } else {
                // No anti-aliasing
                float sample_point_x = static_cast<float>(x) + 0.5f;
                float sample_point_y = static_cast<float>(y) + 0.5f;

                auto[alpha, beta, gamma] = t.compute_barycentric(sample_point_x, sample_point_y);
                float w_reciprocal = 1.0/(alpha / vertices[0].w() + beta / vertices[1].w() + gamma / vertices[2].w());
                float z_interpolated = alpha * vertices[0].z() / vertices[0].w() + beta * vertices[1].z() / vertices[1].w() + gamma * vertices[2].z() / vertices[2].w();
                z_interpolated *= w_reciprocal;

                int pixel_index = y * width + x;
                if (t.inside_triangle(sample_point_x, sample_point_y) && (-z_interpolated < depth_buf[pixel_index])) {
                    set_pixel({static_cast<float>(x), static_cast<float>(y), 1.0f}, t.get_color());
                    depth_buf[pixel_index] = -z_interpolated;
                }
            }
        }
    }
}

void RST::Rasterizer::set_model(const Eigen::Matrix4f& m) {
    model = m;
}

void RST::Rasterizer::set_view(const Eigen::Matrix4f& v) {
    view = v;
}

void RST::Rasterizer::set_projection(const Eigen::Matrix4f& p) {
    projection = p;
}

void RST::Rasterizer::clear(RST::Buffers buff) {
    if ((buff & RST::Buffers::Color) == RST::Buffers::Color) {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }

    if ((buff & RST::Buffers::Depth) == RST::Buffers::Depth) {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

RST::Rasterizer::Rasterizer(int w, int h, bool anti_aliasing) : width(w), height(h), enable_anti_aliasing(anti_aliasing) {
    const auto frame_buf_size = w * h;
    // 4x SSAA need 4 times of space
    const auto depth_buf_size = enable_anti_aliasing ? 4 * frame_buf_size : frame_buf_size;
    
    frame_buf.resize(frame_buf_size);
    depth_buf.resize(depth_buf_size);
}

int RST::Rasterizer::get_index(int x, int y) {
    // Reverse y coordinate before index calculation
    return (height - 1 - y) * width + x;
}


void RST::Rasterizer::blend_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color) {
    // Reverse y coordinate before index calculation
    const auto ind = (height - 1 - point.y()) * width + point.x();
    frame_buf[ind] += color;
}

void RST::Rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color) {
    // Reverse y coordinate before index calculation
    const auto ind = (height - 1 - point.y()) * width + point.x();
    frame_buf[ind] = color;
}