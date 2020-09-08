#pragma once

#include <algorithm>
#include <memory>

#include <eigen3/Eigen/Eigen>

#include "Triangle.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

namespace RST {
    enum class Buffers {
        Color = 1,
        Depth = 2
    };

    inline Buffers operator|(Buffers a, Buffers b) {
        return Buffers((int)a | (int)b);
    }

    inline Buffers operator&(Buffers a, Buffers b) {
        return Buffers((int)a & (int)b);
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

    struct Col_buf_id {
        int col_id = 0;
    };

    struct Norm_buf_id {
        int norm_id = 0;
    };

    class Rasterizer {
    public:
        Rasterizer(int w, int h, std::shared_ptr<Texture> tp = nullptr);
        Pos_buf_id load_positions(const std::vector<Eigen::Vector3f>& positions);
        Ind_buf_id load_indices(const std::vector<Eigen::Vector3i>& indices);
        Col_buf_id load_colors(const std::vector<Eigen::Vector3f>& colors);
        Norm_buf_id load_normals(const std::vector<Eigen::Vector3f>& normals);

        void set_model(const Eigen::Matrix4f& m);
        void set_view(const Eigen::Matrix4f& v);
        void set_projection(const Eigen::Matrix4f& p);

        void set_texture(std::shared_ptr<Texture> tp, Sample_method m) { tex_ptr = tp; method = m; }

        void set_vertex_shader(std::function<Eigen::Vector3f(Vertex_shader_payload)> vert_shader);
        void set_fragment_shader(std::function<Eigen::Vector3f(Fragment_shader_payload, Sample_method)> frag_shader);

        void set_pixel(const Eigen::Vector2i &point, const Eigen::Vector3f &color);

        void clear(Buffers buf);

        void draw(Pos_buf_id pos_buffer, Ind_buf_id ind_buffer, Col_buf_id col_buffer, Primitive type);
        void draw(const std::vector<std::shared_ptr<Triangle>>& TriangleList);

        std::vector<Eigen::Vector3f>& frame_buffer() { return frame_buf; }

    private:
        void draw_line(Eigen::Vector3f begin, Eigen::Vector3f end);

        void rasterize_triangle(const Triangle& tri_view_space, const std::array<Eigen::Vector4f, 3>& tri_pos_screen_space);

        // VERTEX SHADER -> MVP -> Clipping -> /.W -> VIEWPORT -> DRAWLINE/DRAWTRI -> FRAGSHADER

    private:
        Eigen::Matrix4f model;
        Eigen::Matrix4f view;
        Eigen::Matrix4f projection;

        std::map<int, std::vector<Eigen::Vector3f>> pos_buf;
        std::map<int, std::vector<Eigen::Vector3i>> ind_buf;
        std::map<int, std::vector<Eigen::Vector3f>> col_buf;
        std::map<int, std::vector<Eigen::Vector3f>> norm_buf;

        std::shared_ptr<Texture> tex_ptr;
        Sample_method method;

        std::function<Eigen::Vector3f(Vertex_shader_payload)> vertex_shader;
        std::function<Eigen::Vector3f(Fragment_shader_payload, Sample_method)> fragment_shader;

        std::vector<Eigen::Vector3f> frame_buf;
        std::vector<float> depth_buf;
        int get_index(int x, int y);

        int width, height;

        int next_id = 0;
        int get_next_id() { return next_id++; }
    };
}
