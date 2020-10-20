#include "BVH.hpp"

#include <algorithm>
#include <cassert>

#include "Bounding_box.hpp"

BVH_tree::BVH_tree() : _root_ptr(nullptr), _split_method(Split_method::NAIVE) { }

BVH_tree::BVH_tree(std::vector<std::shared_ptr<Object>>& obj_ptrs, Split_method split_method)
    : _split_method(split_method) {
    // Record building time
    time_t start, stop;
    time(&start);
    _root_ptr = recursive_build(obj_ptrs, 0, obj_ptrs.size());
    time(&stop);

    // Print results
    const auto diff = static_cast<float>(difftime(stop, start));
    const auto hrs = static_cast<int>(diff / 3600.0f);
    const auto mins = static_cast<int>(diff / 60.0f) - (hrs * 60);
    const auto secs = static_cast<int>(diff) - (hrs * 3600) - (mins * 60);

    printf( "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n", hrs, mins, secs);
}

BVH_tree::BVH_tree(std::vector<std::shared_ptr<Object>>&& obj_ptrs, Split_method split_method)
    : BVH_tree(obj_ptrs, split_method) { }

std::optional<Intersection> BVH_tree::intersect(const Ray& ray) const {
    return intersect(_root_ptr, ray);
}

std::optional<Sample> BVH_tree::sample() const {
    const auto threshold = std::sqrt(get_random_float()) * _root_ptr->area;
    auto s = sample(_root_ptr, threshold);
    if (s) s->pdf /= _root_ptr->area;
    return s;
}

std::unique_ptr<BVH_node> BVH_tree::recursive_build(std::vector<std::shared_ptr<Object>>& obj_ptrs, size_t start, size_t end) {
    if (start >= end)
        return nullptr;

    // Build current node
    const auto obj_span = end - start;
    if (obj_span == 1) {
        auto node_ptr = std::make_unique<BVH_node>();

        // Create leaf node and return early
        node_ptr->bound = obj_ptrs[start]->bound();
        node_ptr->area = obj_ptrs[start]->area();
        node_ptr->obj_ptr = obj_ptrs[start];

        return node_ptr;
    } else if (obj_span == 2) {
        auto node_ptr = std::make_unique<BVH_node>();

        const auto left_idx = start;
        const auto right_idx = start + 1;

        // Assign first object to left
        node_ptr->left_ptr = std::make_unique<BVH_node>();
        node_ptr->left_ptr->bound = obj_ptrs[left_idx]->bound();
        node_ptr->left_ptr->area = obj_ptrs[left_idx]->area();
        node_ptr->left_ptr->obj_ptr = obj_ptrs[left_idx];

        // Assign second object to right
        node_ptr->right_ptr = std::make_unique<BVH_node>();
        node_ptr->right_ptr->bound = obj_ptrs[right_idx]->bound();
        node_ptr->right_ptr->area = obj_ptrs[right_idx]->area();
        node_ptr->right_ptr->obj_ptr = obj_ptrs[right_idx];

        // Construct return node
        node_ptr->bound = union_box(node_ptr->left_ptr->bound, node_ptr->right_ptr->bound);
        node_ptr->area = node_ptr->left_ptr->area + node_ptr->right_ptr->area;

        return node_ptr;
    } else {
        // Split the objects based on chosen method
        switch(_split_method) {
            case Split_method::NAIVE: {
                return naive_partition(obj_ptrs, start, end, obj_span);
            } case Split_method::SAH: {
                return sah_partition(obj_ptrs, start, end);
            }
        }
    }
}

std::unique_ptr<BVH_node> BVH_tree::naive_partition(std::vector<std::shared_ptr<Object>>& obj_ptrs, size_t start, size_t end, size_t obj_span) {
    auto node_ptr = std::make_unique<BVH_node>();

    // Compute the union bounding box of the centroids
    // of the bounding boxes of given range of objects.
    Bounding_box centroid_bound;
    for (auto i = start; i < end; ++i)
        centroid_bound = union_box(centroid_bound, obj_ptrs[i]->bound().centroid());

    // Divide objects along the axis with max extent
    const auto dim = Bounding_box::axis_to_dim(centroid_bound.max_extent());

    const auto iter_start = obj_ptrs.begin() + start;
    const auto iter_end = obj_ptrs.begin() + end;
    std::sort(iter_start, iter_end, [dim](const std::shared_ptr<Object>& lhs, const std::shared_ptr<Object>& rhs) {
        const auto lhs_box_centroid = lhs->bound().centroid();
        const auto rhs_box_centroid = rhs->bound().centroid();

        return lhs_box_centroid[dim] < rhs_box_centroid[dim];
    });

    const auto mid = start + obj_span / 2;

    // Recursively build nodes
    node_ptr->left_ptr = recursive_build(obj_ptrs, start, mid);
    node_ptr->right_ptr = recursive_build(obj_ptrs, mid, end);
    node_ptr->area = 0.0f;

    if (node_ptr->left_ptr != nullptr) {
        node_ptr->bound = union_box(node_ptr->bound, node_ptr->left_ptr->bound);
        node_ptr->area += node_ptr->left_ptr->area;
    }
    if (node_ptr->right_ptr != nullptr) {
        node_ptr->bound = union_box(node_ptr->bound, node_ptr->right_ptr->bound);
        node_ptr->area += node_ptr->right_ptr->area;
    }

    return node_ptr;
}

std::unique_ptr<BVH_node> BVH_tree::sah_partition(std::vector<std::shared_ptr<Object>>& obj_ptrs, size_t start, size_t end) {
    constexpr int bucket_num = 16;

    auto node_ptr = std::make_unique<BVH_node>();

    // Compute bounding box of all objects in BVH node
    Bounding_box bound;
    for (auto i = start; i < end; ++i)
        bound = union_box(bound, obj_ptrs[i]->bound());
    node_ptr->bound = bound;

    // Record variables
    auto min_cost = FLOAT_INFINITY;
    int split = -1;
    Bounding_box::Axis axis = Bounding_box::Axis::AXIS_X;

    // For each of the three axis x/y/z
    const auto axis_list = {Bounding_box::Axis::AXIS_X, Bounding_box::Axis::AXIS_Y, Bounding_box::Axis::AXIS_Z};
    for (const auto current_axis : axis_list) {
        const auto dim = Bounding_box::axis_to_dim(current_axis);

        std::vector<Bounding_box> box_buckets(bucket_num);
        std::vector<size_t> object_counters(bucket_num);

        for (auto i = start; i < end; ++i) {
            const auto& obj_ptr = obj_ptrs[i];
            const auto box = obj_ptr->bound();
            const auto idx = static_cast<int>(bound.offset_ratio(box.centroid())[dim] * bucket_num);
            box_buckets[idx] = union_box(box_buckets[idx], box);
            ++object_counters[idx];
        }

        // DP preparation
        // union box and count accumulation from left to right
        std::vector<Bounding_box> left_union_boxes(bucket_num);
        std::vector<size_t> left_accumulation(bucket_num, 0);
        for (int current_split = 1; current_split < bucket_num; ++current_split) {
            const auto left_last_idx = current_split - 1;
            const auto prev_split = current_split - 1;
            left_union_boxes[current_split] = union_box(box_buckets[left_last_idx], left_union_boxes[prev_split]);
            left_accumulation[current_split] = object_counters[left_last_idx] + left_accumulation[prev_split];
        }

        std::vector<Bounding_box> right_union_boxes(bucket_num);
        std::vector<size_t> right_accumulation(bucket_num, 0);

        // union box and count accumulation from right to left
        right_union_boxes.back() = box_buckets.back();
        right_accumulation.back() = object_counters.back();
        for (int current_split = bucket_num - 2; current_split >= 1; --current_split) {
            const auto right_first_idx = current_split;
            const auto prev_split = current_split + 1;
            right_union_boxes[current_split] = union_box(box_buckets[right_first_idx], right_union_boxes[prev_split]);
            right_accumulation[current_split] = object_counters[right_first_idx] + right_accumulation[prev_split];
        }

        // Find out the split with minimum cost
        for (int current_split = 1; current_split < bucket_num; ++current_split) {
            const auto current_cost = left_union_boxes[current_split].surface_area() * left_accumulation[current_split]
                                      + right_union_boxes[current_split].surface_area() * right_accumulation[current_split];

            if (current_cost < min_cost) {
                min_cost = current_cost;
                split = current_split;
                axis = current_axis;
            }
        }

        return node_ptr;
    }

    assert(split != -1);

    const auto dim = Bounding_box::axis_to_dim(axis);

    const auto iter_start = obj_ptrs.begin() + start;
    const auto iter_end = obj_ptrs.begin() + end;
    std::sort(iter_start, iter_end, [dim](const auto& lhs, const auto& rhs) {
        return lhs->bound().centroid()[dim] < rhs->bound().centroid()[dim];
    });

    // Find the mid with the split we just calculated
    const auto threshold = bound.p_min[dim] + bound.diagonal()[dim] * (static_cast<float>(split) / static_cast<float>(bucket_num));
    const auto comparator = [dim](const float threshold, const std::shared_ptr<Object>& obj_ptr) {
        return obj_ptr->bound().centroid()[dim] > threshold;
    };
    const auto mid = static_cast<size_t>(std::upper_bound(iter_start, std::prev(iter_end), threshold, comparator) - obj_ptrs.begin());

    // Recursively build nodes
    node_ptr->left_ptr = recursive_build(obj_ptrs, start, mid);
    node_ptr->right_ptr = recursive_build(obj_ptrs, mid, end);

    node_ptr->area = 0.0f;
    if (node_ptr->left_ptr != nullptr) node_ptr->area += node_ptr->left_ptr->area;
    if (node_ptr->right_ptr != nullptr) node_ptr->area += node_ptr->right_ptr->area;

    return node_ptr;
}

std::optional<Intersection> BVH_tree::intersect(const std::unique_ptr<BVH_node>& node_ptr, const Ray& ray) const {
    // Traverse the BVH to check intersection
    if (node_ptr == nullptr || !node_ptr->bound.intersect(ray))
        return std::nullopt;

    if (node_ptr->left_ptr == nullptr && node_ptr->right_ptr == nullptr && node_ptr->obj_ptr != nullptr) {
        return node_ptr->obj_ptr->intersect(ray);
    } else {
        const auto left_intersection = intersect(node_ptr->left_ptr, ray);
        const auto right_intersection = intersect(node_ptr->right_ptr, ray);

        if (left_intersection || right_intersection) {
            if (left_intersection && right_intersection) {
                return left_intersection->time < right_intersection->time ? left_intersection : right_intersection;
            } else {
                return left_intersection ? left_intersection : right_intersection;
            }
        } else {
            return std::nullopt;
        }
    }
}

std::optional<Sample> BVH_tree::sample(const std::unique_ptr<BVH_node>& node_ptr, float threshold) const {
    if (node_ptr == nullptr)
        return std::nullopt;

    if (node_ptr->left_ptr == nullptr && node_ptr->right_ptr == nullptr && node_ptr->obj_ptr != nullptr) {
        auto s = node_ptr->obj_ptr->sample();
        if (s) s->pdf *= node_ptr->area;
        return s;
    } else {
        if (node_ptr->left_ptr != nullptr && node_ptr->right_ptr != nullptr) {
            return (node_ptr->left_ptr->area > threshold) ? sample(node_ptr->left_ptr, threshold) : sample(node_ptr->right_ptr, threshold - node_ptr->left_ptr->area);
        } else {
            return (node_ptr->left_ptr != nullptr) ? sample(node_ptr->left_ptr, threshold) : sample(node_ptr->right_ptr, threshold - node_ptr->left_ptr->area);
        }
    }
}