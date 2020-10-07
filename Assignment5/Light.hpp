#pragma once

class Light {
public:
    Light(const Eigen::Vector3f& p, const Eigen::Vector3f& i)
        : position(p)
        , intensity(i)
    {}
    
    virtual ~Light() = default;

    Eigen::Vector3f position;
    Eigen::Vector3f intensity;
};
