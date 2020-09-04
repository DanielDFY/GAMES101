#pragma once

#include <algorithm>

#include <eigen3/Eigen/Eigen>

#include "Triangle.hpp"

using namespace Eigen;

namespace RST {
    enum class Buffers {
        Color = 1,
        Depth = 2
    };

    inline Buffers operator|(Buffers a, Buffers b) {
        return Buffers(static_cast<int>(a) | static_cast<int>(b));
    }

    inline Buffers operator&(Buffers a, Buffers b) {
        return Buffers(static_cast<int>(a) & static_cast<int>(b));
    }

    enum class Primitive {
        Line,
        Triangle
    };

    /*
    * For the curious : The draw function takes data indices in each buffer as its arguments.
    * These structs make sure that if you mix up with their orders, the compiler won't
    * compile it. Aka : Type safety
    */
    struct Pos_buf_id {
        int pos_id = 0;
    };

    struct Ind_buf_id {
        int ind_id = 0;
    };

    class Rasterizer {
    public:
        Rasterizer(int w, int h);
        Pos_buf_id load_positions(const std::vector<Eigen::Vector3f>& positions);
        Ind_buf_id load_indices(const std::vector<Eigen::Vector3i>& indices);

        void set_model(const Eigen::Matrix4f& m);
        void set_view(const Eigen::Matrix4f& v);
        void set_projection(const Eigen::Matrix4f& p);

        void set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color);

        void clear(Buffers buf);

        void draw(Pos_buf_id pos_buffer, Ind_buf_id ind_buffer, Primitive type);

        std::vector<Eigen::Vector3f>& frame_buffer() { return frame_buf; }

    private:
        void draw_line(Eigen::Vector3f begin, Eigen::Vector3f end);
        void rasterize_wireframe(const Triangle& t);

    private:
        Eigen::Matrix4f model;
        Eigen::Matrix4f view;
        Eigen::Matrix4f projection;

        std::map<int, std::vector<Eigen::Vector3f>> pos_buf;
        std::map<int, std::vector<Eigen::Vector3i>> ind_buf;

        std::vector<Eigen::Vector3f> frame_buf;
        std::vector<float> depth_buf;
        int get_index(int x, int y);

        int width, height;

        int next_id = 0;
        int get_next_id() { return next_id++; }
    };
} // namespace RST