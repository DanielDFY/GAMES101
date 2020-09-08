#include "Rasterizer.hpp"

#include <algorithm>
#include <cmath>

#include <opencv2/opencv.hpp>

#include "Utility.hpp"

RST::Pos_buf_id RST::Rasterizer::load_positions(const std::vector<Eigen::Vector3f>& positions) {
    const auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

RST::Ind_buf_id RST::Rasterizer::load_indices(const std::vector<Eigen::Vector3i>& indices) {
    const auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

RST::Col_buf_id RST::Rasterizer::load_colors(const std::vector<Eigen::Vector3f>& cols) {
    const auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

RST::Norm_buf_id RST::Rasterizer::load_normals(const std::vector<Eigen::Vector3f>& normals)
{
    const auto id = get_next_id();
    norm_buf.emplace(id, normals);

    return {id};
}

// Bresenham's line drawing algorithm
void RST::Rasterizer::draw_line(Eigen::Vector3f begin, Eigen::Vector3f end) {
    const auto x1 = begin.x();
    const auto y1 = begin.y();
    const auto x2 = end.x();
    const auto y2 = end.y();

    Eigen::Vector3f line_color = {255.0f, 255.0f, 255.0f};

    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;

    dx = x2 - x1;
    dy = y2 - y1;
    dx1 = fabs(dx);
    dy1 = fabs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;

    if (dy1 <= dx1) {
        if (dx >= 0) {
            x = x1;
            y = y1;
            xe = x2;
        } else {
            x = x2;
            y = y2;
            xe = x1;
        }

        const Eigen::Vector2i point = Eigen::Vector2i(x, y);
        set_pixel(point, line_color);
        for (i = 0; x < xe; ++i) {
            x += 1;
            if (px < 0) {
                px=px+2*dy1;
            } else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    y += 1;
                } else {
                    y -=1 ;
                }
                px += 2 * (dy1 - dx1);
            }

            const Eigen::Vector2i point = Eigen::Vector2i(x, y);
            set_pixel(point, line_color);
        }
    } else {
        if(dy >= 0) {
            x = x1;
            y = y1;
            ye = y2;
        } else {
            x = x2;
            y = y2;
            ye = y1;
        }

        const Eigen::Vector2i point = Eigen::Vector2i(x, y);
        set_pixel(point, line_color);
        for(i = 0; y < ye; ++i) {
            y += 1;
            if(py <= 0) {
                py += 2 * dx1;
            } else {
                if((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    x += 1;
                } else {
                    x -= 1;
                }
                py += 2 * (dx1 - dy1);
            }

            const Eigen::Vector2i point = Eigen::Vector2i(x, y);
            set_pixel(point, line_color);
        }
    }
}

void RST::Rasterizer::draw(const std::vector<std::shared_ptr<Triangle>>& TriangleList) {
    constexpr auto f1 = (50.0f - 0.1f) / 2.0f;
    constexpr auto f2 = (50.0f + 0.1f) / 2.0f;

    const auto width_half = 0.5f * width;
    const auto height_half = 0.5f * height;

    const auto mv = view * model;
    const auto mvp = projection * mv;

    for (const auto& tri : TriangleList) {
        std::array<Eigen::Vector4f, 3> tri_pos_view_space {
                (mv * tri->v[0]),
                (mv * tri->v[1]),
                (mv * tri->v[2])
        };

        // First get position in clip space
        std::array<Eigen::Vector4f, 3> tri_pos_screen_space = {
                mvp * tri->v[0],
                mvp * tri->v[1],
                mvp * tri->v[2]
        };

        // Then transform to screen space
        for (auto& vertex : tri_pos_screen_space) {
            // homogeneous division
            vertex /= vertex.w();

            // viewport transformation
            vertex.x() = width_half * (vertex.x() + 1.0f);
            vertex.y() = height_half * (vertex.y() + 1.0f);
            vertex.z() = vertex.z() * f1 + f2;
        }

        // Transfrom normal vectors
        Eigen::Matrix4f inv_mv = mv.inverse().transpose();
        std::array<Eigen::Vector4f, 3> tri_norm_view_space = {
            inv_mv * Eigen::Vector4f(tri->normal[0].x(), tri->normal[0].y(), tri->normal[0].z(), 0.0f),
            inv_mv * Eigen::Vector4f(tri->normal[1].x(), tri->normal[1].y(), tri->normal[1].z(), 0.0f),
            inv_mv * Eigen::Vector4f(tri->normal[2].x(), tri->normal[2].y(), tri->normal[2].z(), 0.0f)
        };

        Triangle tri_view_space = *tri;
        for (int i = 0; i < 3; ++i) {
            // screen space coordinates
            tri_view_space.set_vertex(i, tri_pos_view_space[i]);
            // view space normal
            tri_view_space.set_normal(i, tri_norm_view_space[i].head<3>());
            // default color
            tri_view_space.set_color(i, 148.0f, 121.0f, 92.0f);
        }
        
        // Also pass screen space vertice position
        rasterize_triangle(tri_view_space, tri_pos_screen_space);
    }
}

// Screen space rasterization
void RST::Rasterizer::rasterize_triangle(const Triangle& tri_view_space, const std::array<Eigen::Vector4f, 3>& tri_pos_screen_space)  {    
    // Find out the bounding box of current triangle.
    const auto min_max_x = std::minmax_element(tri_pos_screen_space.cbegin(), tri_pos_screen_space.cend(),
                                    [](const auto& lhs, const auto& rhs) {
                                        return lhs.x() / lhs.w() < rhs.x() / rhs.w();
                                    });
    const auto min_max_y = std::minmax_element(tri_pos_screen_space.cbegin(), tri_pos_screen_space.cend(),
                                    [](const auto& lhs, const auto& rhs) {
                                        return lhs.y() / lhs.w() < rhs.y() / rhs.w();
                                    });

    const auto min_x = static_cast<int>(floor(min_max_x.first->x()));
    const auto min_y = static_cast<int>(floor(min_max_y.first->y()));
    const auto max_x = static_cast<int>(ceil(min_max_x.second->x()));
    const auto max_y = static_cast<int>(ceil(min_max_y.second->y()));
    
    // iterate through the pixels and find if the current pixel is inside the triangle
    for (int x = min_x; x <= max_x; ++x) {
        for (int y = min_y; y <= max_y; ++y) {
            float sample_point_x = static_cast<float>(x) + 0.5f;
            float sample_point_y = static_cast<float>(y) + 0.5f;

            // compute barycentric coordinates
            auto[alpha, beta, gamma] = compute_barycentric_2d(sample_point_x, sample_point_y, tri_pos_screen_space);
            // interpolated view space depth for the current pixel
            const auto Z = 1.0f / (alpha / tri_pos_screen_space[0].w() + beta / tri_pos_screen_space[1].w() + gamma / tri_pos_screen_space[2].w());
            // depth between z_near and z_far, used for z-buffer
            const auto zp = Z * (alpha * tri_pos_screen_space[0].z() / tri_pos_screen_space[0].w() + beta * tri_pos_screen_space[1].z() / tri_pos_screen_space[1].w() + gamma * tri_pos_screen_space[2].z() / tri_pos_screen_space[2].w());

            const auto pixel_index = y * width + x;
            if (inside_triangle_2d(sample_point_x, sample_point_y, tri_pos_screen_space) && (-zp < depth_buf[pixel_index])) {
                // interpolate attributes
                const auto interpolated_color = interpolate(alpha, beta, gamma, tri_view_space.color[0], tri_view_space.color[1], tri_view_space.color[2]);
                const auto interpolated_normal = interpolate(alpha, beta, gamma, tri_view_space.normal[0], tri_view_space.normal[1], tri_view_space.normal[2]);
                const auto interpolated_tex_coords = interpolate(alpha, beta, gamma, tri_view_space.tex_coords[0], tri_view_space.tex_coords[1], tri_view_space.tex_coords[2]);
                const auto interpolated_shading_coords = interpolate(alpha, beta, gamma, tri_view_space.v[0], tri_view_space.v[1], tri_view_space.v[2]);

                // fragment processing
                Fragment_shader_payload payload( interpolated_color, interpolated_normal.normalized(), interpolated_tex_coords, tex_ptr);
                payload.view_pos = interpolated_shading_coords;

                // set the current pixel to the final color.
                set_pixel({x, y}, fragment_shader(payload, method));
                depth_buf[pixel_index] = -zp;
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

void RST::Rasterizer::clear(RST::Buffers buf) {
    if ((buf & RST::Buffers::Color) == RST::Buffers::Color) {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0.0f, 0.0f, 0.0f});
    }

    if ((buf & RST::Buffers::Depth) == RST::Buffers::Depth) {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

RST::Rasterizer::Rasterizer(int w, int h, std::shared_ptr<Texture> tp)
                         : width(w), height(h), tex_ptr(nullptr) {
    const auto buf_size = w * h;
    frame_buf.resize(buf_size);
    depth_buf.resize(buf_size);
}

int RST::Rasterizer::get_index(int x, int y) {
    // Reverse y coordinate before index calculation
    return (height - y) * width + x;
}

void RST::Rasterizer::set_pixel(const Eigen::Vector2i &point, const Eigen::Vector3f &color) {
    // Reverse y coordinate before index calculation
    const auto ind = (height - point.y()) * width + point.x();
    frame_buf[ind] = color;
}

void RST::Rasterizer::set_vertex_shader(std::function<Eigen::Vector3f(Vertex_shader_payload)> vert_shader) {
    vertex_shader = vert_shader;
}

void RST::Rasterizer::set_fragment_shader(std::function<Eigen::Vector3f(Fragment_shader_payload, Sample_method)> frag_shader) {
    fragment_shader = frag_shader;
}

