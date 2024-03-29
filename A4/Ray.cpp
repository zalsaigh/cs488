#include "Ray.hpp"
#include <glm/ext.hpp>

#include <iostream>

//---------------------------------------------------------------------------------------
Ray::Ray(const glm::vec3 &origin, const glm::vec3 &direction) : m_origin(origin), m_direction(direction) {}

//---------------------------------------------------------------------------------------
glm::vec3 Ray::at(float t) const
{
    return m_origin + t * m_direction;
}

//---------------------------------------------------------------------------------------
glm::vec3 generateReflectionVector(const glm::vec3& incomingRayDirection, const glm::vec3 &normal)
{
    return incomingRayDirection - 2 * glm::dot(incomingRayDirection, normal) * normal;    
}

//---------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const Ray& ray)
{
  out << "Ray = " << glm::to_string(ray.m_origin) << " + t(" << glm::to_string(ray.m_direction) << ")";
  return out;
}
