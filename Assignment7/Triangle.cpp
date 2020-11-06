#include "Triangle.hpp"

Triangle::Triangle(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2,
                std::shared_ptr<Material> material_ptr)
 : _v0(v0), _v1(v1), _v2(v2), _mat_ptr(std::move(material_ptr)) {
    _e1 = _v1 - _v0;
    _e2 = _v2 - _v0;

    // default
    _t0 = _t1 = _t2 = {0.0f, 0.0f};

    const auto n = _e1.cross(_e2);
    _normal = n.normalized();
    _area = 0.5f * n.magnitude() ;
}

std::optional<Intersection> Triangle::intersect(const Ray& ray) {
    // Check surface direction
    if (ray.dir.dot(_normal) > 0.0f)
        return std::nullopt;

    // Moller-Trumbore algorithm
    const auto S = ray.ori - _v0;
    const auto S1 = ray.dir.cross(_e2);
    const auto S2 = S.cross(_e1);
    const auto denominator = S1.dot(_e1);
    if (fabs(denominator) < EPSILON)
        return std::nullopt;

    const auto inv_denominator = 1.0f / denominator;
    const auto t = S2.dot(_e2) * inv_denominator;
    const auto b1 = S1.dot(S) * inv_denominator;
    const auto b2 = S2.dot(ray.dir) * inv_denominator;

    const bool check_intersect = (t > 0.0f) && (b1 > 0.0f) && (b2 > 0.0f) && (1.0f - b1 - b2 > 0.0f);
    if (!check_intersect)
        return std::nullopt;

    Intersection intersection;

    intersection.time = t;
    intersection.pos = ray.at_time(t);
    intersection.normal = _normal;
    intersection.obj_ptr = shared_from_this();
    intersection.mat_ptr = _mat_ptr;

    return intersection;
}

Bounding_box Triangle::bound() const {
    return union_box({_v0, _v1}, _v2);
}

std::optional<Sample> Triangle::sample() {
    const auto x = std::sqrt(get_random_float());
    const auto y = get_random_float();

    const auto c0 = 1.0f - x;
    const auto c1 = x * (1.0f - y);
    const auto c2 = x * y;

    Intersection intersection;
    intersection.pos = _v0 * c0 + _v1 * c1 + _v2 * c2;
    intersection.normal = _normal;
    intersection.uv = _t0 * c0 + _t1 * c1 + _t2 * c2;
    intersection.obj_ptr = shared_from_this();
    intersection.mat_ptr = _mat_ptr;

    const auto pdf = 1.0f / _area;

    return {{intersection, pdf}};
}

std::vector<std::shared_ptr<Triangle>> load_triangles_from_model_file(const std::string& file_name, const std::shared_ptr<Material>& mat_ptr) {
    objl::Loader loader;
    loader.LoadFile(file_name);

    assert(loader.LoadedMeshes.size() == 1);
    const objl::Mesh mesh = loader.LoadedMeshes[0];

    std::vector<std::shared_ptr<Triangle>> triangle_ptrs;

    const auto vertex_num = mesh.Vertices.size();
    for (size_t i = 0; i < vertex_num; i += 3) {
        std::array<Vector3f, 3> triangle_vertices;
        for (size_t j = 0; j < 3; ++j) {
            const auto idx = i + j;
            triangle_vertices[j] = {
                mesh.Vertices[idx].Position.X,
                mesh.Vertices[idx].Position.Y,
                mesh.Vertices[idx].Position.Z
            };
        }

        triangle_ptrs.push_back(std::make_shared<Triangle>(triangle_vertices[0],
                                triangle_vertices[1],
                                triangle_vertices[2],
                                mat_ptr));
    }

    return triangle_ptrs;
}

Triangle_mesh::Triangle_mesh(const std::vector<std::shared_ptr<Triangle>>& triangle_ptr_list,
                            BVH_tree::Split_method split_method)
     : _triangle_ptrs(triangle_ptr_list),
       _bvh_tree(transform_to_object_vector<Triangle>(triangle_ptr_list), split_method),
       _emitting(false) {
    for (const auto& tri_ptr : _triangle_ptrs) {
        if (tri_ptr->emitting()) {
            _emitting = true;
            break;
        }
    }
}