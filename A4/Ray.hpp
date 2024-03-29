#pragma once

#include <glm/glm.hpp>

#include <iosfwd>

class Ray
{
    public:
    glm::vec3 m_origin;
    glm::vec3 m_direction;

    Ray() {}
    Ray(const glm::vec3 &origin, const glm::vec3 &direction);

    glm::vec3 at(float t) const;
};

glm::vec3 generateReflectionVector(const glm::vec3 &incomingRayDirection, const glm::vec3 &normal);
std::ostream& operator<<(std::ostream& out, const Ray& ray);