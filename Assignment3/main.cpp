#include <iostream>

#include <opencv2/opencv.hpp>
#include "OBJ_Loader.h"

#include "Rasterizer.hpp"
#include "Triangle.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Utility.hpp"

// Enable bilinear interpolation on texture sampling
// #define BILINEAR

struct light {
    Eigen::Vector3f position;
    Eigen::Vector3f intensity;
};

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    view << 1,0,0,-eye_pos[0],
            0,1,0,-eye_pos[1],
            0,0,1,-eye_pos[2],
            0,0,0,1;

    return view;
}

// Rotate around axis-Y
Eigen::Matrix4f get_model_matrix(float angle) {
    Eigen::Matrix4f rotation;
    angle = angle * MY_PI / 180.f;
    rotation << cos(angle), 0.0f,   sin(angle), 0.0f,
                0.0f,       1.0f,   0.0f,       0.0f,
                -sin(angle),0.0f,   cos(angle), 0.0f,
                0.0f,       0.0f,   0.0f,       1.0f;

    Eigen::Matrix4f scale;
    scale << 2.5f, 0.0f, 0.0f, 0.0f,
             0.0f, 2.5f, 0.0f, 0.0f,
             0.0f, 0.0f, 2.5f, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f;

    Eigen::Matrix4f translate;
    translate << 1.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f;

    return translate * rotation * scale;
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


Eigen::Vector3f vertex_shader(const Vertex_shader_payload& payload) {
    return payload.position;
}

Eigen::Vector3f normal_fragment_shader(const Fragment_shader_payload& payload, Sample_method method) {
    const Eigen::Vector3f return_color = (payload.normal.normalized() + Eigen::Vector3f(1.0f, 1.0f, 1.0f)) / 2.f;
    return {return_color.x() * 255.0f, return_color.y() * 255.0f, return_color.z() * 255.0f};
}


Eigen::Vector3f texture_fragment_shader(const Fragment_shader_payload& payload, Sample_method method) {
    Eigen::Vector3f return_color = {0.0f, 0.0f, 0.0f};   // default color

    if (payload.tex_ptr != nullptr) {
        // Get the texture value at the texture coordinates of the current fragment
        return_color = payload.tex_ptr->get_color(payload.tex_coords.x(), payload.tex_coords.y(), method);
    }

    Eigen::Vector3f texture_color;
    texture_color << return_color.x(), return_color.y(), return_color.z();

    const Eigen::Vector3f ka = Eigen::Vector3f(0.005f, 0.005f, 0.005f);
    const Eigen::Vector3f kd = texture_color / 255.0f;
    const Eigen::Vector3f ks = Eigen::Vector3f(0.7937f, 0.7937f, 0.7937f);

    const auto l1 = light{{20.0f, 20.0f, 20.0f}, {500.0f, 500.0f, 500.0f}};
    const auto l2 = light{{-20.0f, 20.0f, 0.0f}, {500.0f, 500.0f, 500.0f}};

    const std::vector<light> lights = {l1, l2};
    const Eigen::Vector3f amb_light_intensity{10.0f, 10.0f, 10.0f};
    const Eigen::Vector3f eye_pos{0.0f, 0.0f, 10.0f};

    const Eigen::Vector3f color = texture_color;
    const Eigen::Vector3f point = payload.view_pos;
    const Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};

    // Blinn-Phong shading
    for (const auto& light : lights) {
        // For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the object.
        const auto light_vec = light.position - point;
        const auto light_attenuation = 1.0f / (light_vec.dot(light_vec));
        const auto light_direction = light_vec.normalized();
        const auto view_direction = (eye_pos - point).normalized();

        // ambient
        result_color += ka.cwiseProduct(amb_light_intensity);

        // diffuse
        result_color += kd.cwiseProduct(light.intensity) * light_attenuation
                        * std::max(0.0f, light_direction.dot(normal));
        
        // specular
        const auto h_vec = (light_direction + view_direction).normalized();
        result_color += ks.cwiseProduct(light.intensity) * light_attenuation
                        * std::pow(std::max(0.0f, h_vec.dot(normal)), 150);
    }

    return result_color * 255.f;
}

Eigen::Vector3f phong_fragment_shader(const Fragment_shader_payload& payload, Sample_method method) {
    const Eigen::Vector3f ka = Eigen::Vector3f(0.005f, 0.005f, 0.005f);
    const Eigen::Vector3f kd = payload.color;
    const Eigen::Vector3f ks = Eigen::Vector3f(0.7937f, 0.7937f, 0.7937f);

    const auto l1 = light{{20.0f, 20.0f, 20.0f}, {500.0f, 500.0f, 500.0f}};
    const auto l2 = light{{-20.0f, 20.0f, 0.0f}, {500.0f, 500.0f, 500.0f}};

    const std::vector<light> lights = {l1, l2};
    const Eigen::Vector3f amb_light_intensity{10.0f, 10.0f, 10.0f};
    const Eigen::Vector3f eye_pos{0.0f, 0.0f, 10.0f};

    const Eigen::Vector3f color = payload.color;
    const Eigen::Vector3f point = payload.view_pos;
    const Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0.0f, 0.0f, 0.0f};
    // Blinn-Phong shading
    for (const auto& light : lights) {
        // For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the object.
        const auto light_vec = light.position - point;
        const auto light_attenuation = 1.0f / (light_vec.dot(light_vec));
        const auto light_direction = light_vec.normalized();
        const auto view_direction = (eye_pos - point).normalized();

        // ambient
        result_color += ka.cwiseProduct(amb_light_intensity);

        // diffuse
        result_color += kd.cwiseProduct(light.intensity) * light_attenuation
                        * std::max(0.0f, light_direction.dot(normal));
        
        // specular
        const auto h_vec = (light_direction + view_direction).normalized();
        result_color += ks.cwiseProduct(light.intensity) * light_attenuation
                        * std::pow(std::max(0.0f, h_vec.dot(normal)), 150);
    }

    return result_color * 255.f;
}

Eigen::Vector3f bump_fragment_shader(const Fragment_shader_payload& payload, Sample_method method) {
    const Eigen::Vector3f ka = Eigen::Vector3f(0.005f, 0.005f, 0.005f);
    const Eigen::Vector3f kd = payload.color;
    const Eigen::Vector3f ks = Eigen::Vector3f(0.7937f, 0.7937f, 0.7937f);

    const auto l1 = light{{20.0f, 20.0f, 20.0f}, {500.0f, 500.0f, 500.0f}};
    const auto l2 = light{{-20.0f, 20.0f, 0.0f}, {500.0f, 500.0f, 500.0f}};

    const std::vector<light> lights = {l1, l2};
    const Eigen::Vector3f amb_light_intensity{10.0f, 10.0f, 10.0f};
    const Eigen::Vector3f eye_pos{0.0f, 0.0f, 10.0f};

    const Eigen::Vector3f color = payload.color;
    const Eigen::Vector3f point = payload.view_pos;
    const Eigen::Vector3f normal = payload.normal;

    constexpr float kh = 0.2f, kn = 0.1f;

    // Implement bump mapping

    // Transform the normal from texture coordinates to local coordinates
    const Eigen::Vector3f t = {
        normal.x()*normal.y()/sqrt(normal.x()*normal.x()+normal.z()*normal.z()),
        sqrt(normal.x()*normal.x()+normal.z()*normal.z()),
        normal.z()*normal.y()/sqrt(normal.x()*normal.x()+normal.z()*normal.z())
    };
    const Eigen::Vector3f b = normal.cross(t);
    Eigen::Matrix3f TBN;
    TBN << t, b, normal;

    // Perturb the normal
    const float u = payload.tex_coords.x(), v = payload.tex_coords.y();
    const float w = payload.tex_ptr->get_width();
    const float h = payload.tex_ptr->get_height();
    const float dU = kh * kn * (payload.tex_ptr->get_color(u+1.0f/w,v,method).norm()-payload.tex_ptr->get_color(u,v,method).norm());
    const float dV = kh * kn * (payload.tex_ptr->get_color(u+1,v+1.0f/h,method).norm()-payload.tex_ptr->get_color(u,v,method).norm());
    const Eigen::Vector3f ln = {-dU, -dV, 1.0f};

    const Eigen::Vector3f result_color = (TBN * ln).normalized() * 255.f;

    return result_color;
}

Eigen::Vector3f displacement_fragment_shader(const Fragment_shader_payload& payload, Sample_method method) {
    const Eigen::Vector3f ka = Eigen::Vector3f(0.005f, 0.005f, 0.005f);
    const Eigen::Vector3f kd = payload.color;
    const Eigen::Vector3f ks = Eigen::Vector3f(0.7937f, 0.7937f, 0.7937f);

    const auto l1 = light{{20.0f, 20.0f, 20.0f}, {500.0f, 500.0f, 500.0f}};
    const auto l2 = light{{-20.0f, 20.0f, 0.0f}, {500.0f, 500.0f, 500.0f}};

    const std::vector<light> lights = {l1, l2};
    const Eigen::Vector3f amb_light_intensity{10.0f, 10.0f, 10.0f};
    const Eigen::Vector3f eye_pos{0.0f, 0.0f, 10.0f};

    const Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;   // displaced later
    const Eigen::Vector3f normal = payload.normal;

    constexpr float kh = 0.2f, kn = 0.1f;
    
    // Implement displacement mapping
    
    // Transform the normal from texture coordinates to local coordinates
    const Eigen::Vector3f t = {
        normal.x()*normal.y()/sqrt(normal.x()*normal.x()+normal.z()*normal.z()),
        sqrt(normal.x()*normal.x()+normal.z()*normal.z()),
        normal.z()*normal.y()/sqrt(normal.x()*normal.x()+normal.z()*normal.z())
    };
    const Eigen::Vector3f b = normal.cross(t);
    Eigen::Matrix3f TBN;
    TBN << t, b, normal;

    // Perturb the normal
    const float u = payload.tex_coords.x(), v = payload.tex_coords.y();
    const float w = payload.tex_ptr->get_width();
    const float h = payload.tex_ptr->get_height();
    const float dU = kh * kn * (payload.tex_ptr->get_color(u+1.0f/w,v,method).norm()-payload.tex_ptr->get_color(u,v,method).norm());
    const float dV = kh * kn * (payload.tex_ptr->get_color(u+1,v+1.0f/h,method).norm()-payload.tex_ptr->get_color(u,v,method).norm());
    const Eigen::Vector3f ln = {-dU, -dV, 1.0f};

    // displacement
    point += kn * (TBN * ln).normalized() * payload.tex_ptr->get_color(u, v, method).norm();
    
    Eigen::Vector3f result_color = {0.0f, 0.0f, 0.0f};
    // Blinn-Phong shading
    for (const auto& light : lights) {
        // For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the object.
        const auto light_vec = light.position - point;
        const auto light_attenuation = 1.0f / (light_vec.dot(light_vec));
        const auto light_direction = light_vec.normalized();
        const auto view_direction = (eye_pos - point).normalized();

        // ambient
        result_color += ka.cwiseProduct(amb_light_intensity);

        // diffuse
        result_color += kd.cwiseProduct(light.intensity) * light_attenuation
                        * std::max(0.0f, light_direction.dot(normal));
        
        // specular
        const auto h_vec = (light_direction + view_direction).normalized();
        result_color += ks.cwiseProduct(light.intensity) * light_attenuation
                        * std::pow(std::max(0.0f, h_vec.dot(normal)), 150);
    }

    return result_color * 255.f;
}

int main(int argc, const char** argv) {
    std::vector<std::shared_ptr<Triangle>> TriangleList;

    const Eigen::Vector3f eye_pos = {0.0f, 0.0f, 10.0f};
    float angle = 140.0f;
    bool real_time = true;

    std::string filename = "output.png";
    std::string obj_path = "../model/spot/";

    // Load .obj File
    objl::Loader Loader;
    if (!Loader.LoadFile("../model/spot/spot_triangulated_good.obj")) {
        std::cout << "unknown model file" << std::endl;
        return 0;
    }

    for(const auto& mesh : Loader.LoadedMeshes) {
        for(int i=0; i < mesh.Vertices.size(); i += 3) {
            const auto tri_ptr = std::make_shared<Triangle>();
            for(int j = 0; j < 3; ++j) {
                tri_ptr->set_vertex(j, Eigen::Vector4f(mesh.Vertices[i+j].Position.X, mesh.Vertices[i+j].Position.Y, mesh.Vertices[i+j].Position.Z,1.0f));
                tri_ptr->set_normal(j, Eigen::Vector3f(mesh.Vertices[i+j].Normal.X, mesh.Vertices[i+j].Normal.Y, mesh.Vertices[i+j].Normal.Z));
                tri_ptr->set_tex_coord(j, Eigen::Vector2f(mesh.Vertices[i+j].TextureCoordinate.X, mesh.Vertices[i+j].TextureCoordinate.Y));
            }
            TriangleList.push_back(tri_ptr);
        }
    }

    RST::Rasterizer r(700, 700);

    #ifdef BILINEAR
    constexpr auto method = Sample_method::Bilinear;
    #else
    constexpr auto method = Sample_method::Default;
    #endif

    const std::string height_texture_path = "hmap.jpg";
    const auto height_texture_ptr = std::make_shared<Texture>(obj_path + height_texture_path);
    r.set_texture(height_texture_ptr, method);

    std::function<Eigen::Vector3f(Fragment_shader_payload, Sample_method)> active_shader = normal_fragment_shader;

    if (argc >= 2) {
        real_time = false;
        filename = std::string(argv[1]);

        if (argc == 3 && std::string(argv[2]) == "texture") {
            std::cout << "Rasterizing using the texture shader\n";
            active_shader = texture_fragment_shader;

            const std::string texture_path = "spot_texture.png";
            const auto texture_ptr = std::make_shared<Texture>(obj_path + texture_path);
            r.set_texture(texture_ptr, method);
        } else if (argc == 3 && std::string(argv[2]) == "normal") {
            std::cout << "Rasterizing using the normal shader\n";
            active_shader = normal_fragment_shader;
        } else if (argc == 3 && std::string(argv[2]) == "phong") {
            std::cout << "Rasterizing using the phong shader\n";
            active_shader = phong_fragment_shader;
        } else if (argc == 3 && std::string(argv[2]) == "bump") {
            std::cout << "Rasterizing using the bump shader\n";
            active_shader = bump_fragment_shader;
        } else if (argc == 3 && std::string(argv[2]) == "displacement") {
            std::cout << "Rasterizing using the displacement shader\n";
            active_shader = displacement_fragment_shader;
        }
    }

    r.set_vertex_shader(vertex_shader);
    r.set_fragment_shader(active_shader);

    int key = 0;
    int frame_count = 0;

    if (!real_time) {
        r.clear(RST::Buffers::Color | RST::Buffers::Depth);
        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0f, 1.0f, 0.1f, 50.0f));

        r.draw(TriangleList);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    while(key != 27) {
        r.clear(RST::Buffers::Color | RST::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0f, 1.0f, 0.1f, 50.0f));

        r.draw(TriangleList);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        if (key == 'a' ) {
            angle -= 0.1f;
        } else if (key == 'd') {
            angle += 0.1f;
        }

    }
    return 0;
}
