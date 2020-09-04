#include "Rasterizer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

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

        const Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
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

            const Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
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

        const Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
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

            const Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
            set_pixel(point, line_color);
        }
    }
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f) {
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

void RST::Rasterizer::draw(RST::Pos_buf_id pos_buffer, RST::Ind_buf_id ind_buffer, RST::Primitive type) {
    if (type != RST::Primitive::Triangle) {
        throw std::runtime_error("Drawing primitives other than triangle is not implemented yet!");
    }

    auto& pos_vec = pos_buf[pos_buffer.pos_id];
    auto& ind_vec = ind_buf[ind_buffer.ind_id];

    const auto f1 = (100.0f - 0.1f) / 2.0f;
    const auto f2 = (100.0f + 0.1f) / 2.0f;

    const Eigen::Matrix4f mvp = projection * view * model;
    for (const auto& ind : ind_vec)
    {
        Triangle t;

        Eigen::Vector4f v[] = {
                mvp * to_vec4(pos_vec[ind[0]], 1.0f),
                mvp * to_vec4(pos_vec[ind[1]], 1.0f),
                mvp * to_vec4(pos_vec[ind[2]], 1.0f)
        };

        for (auto& vertex : v) {
            // Homogeneous division
            vertex /= vertex.w();

            //Viewport transformation
            vertex.x() = 0.5f * width * (vertex.x() + 1.0f);
            vertex.y() = 0.5f * height * (vertex.y() + 1.0f);
            vertex.z() = vertex.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i) {
            t.set_vertex(i, v[i].head<3>());
            t.set_vertex(i, v[i].head<3>());
            t.set_vertex(i, v[i].head<3>());
        }

        t.set_color(0.0f,    255.0f,     0.0f,       0.0f);
        t.set_color(1.0f,    0.0f,       255.0f,     0.0f);
        t.set_color(2.0f,    0.0f,       0.0f,       255.0f);

        rasterize_wireframe(t);
    }
}

void RST::Rasterizer::rasterize_wireframe(const Triangle& t) {
    draw_line(t.c(), t.a());
    draw_line(t.c(), t.b());
    draw_line(t.b(), t.a());
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
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buf & RST::Buffers::Depth) == RST::Buffers::Depth) {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

RST::Rasterizer::Rasterizer(int w, int h) : width(w), height(h) {
    const int size = w * h;
    frame_buf.resize(size);
    depth_buf.resize(size);
}

int RST::Rasterizer::get_index(int x, int y) {
    return (height - y) * width + x;
}

void RST::Rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color) {
    if (point.x() < 0 || point.x() >= width ||
        point.y() < 0 || point.y() >= height) return;

    const auto ind = static_cast<int>((height - point.y()) * width + point.x());
    frame_buf[ind] = color;
}