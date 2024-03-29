// Termm--Fall 2020

#include "Material.hpp"
#include "Primitive.hpp"
#include "Ray.hpp"
#include "polyroots.hpp"
#include <glm/gtx/extented_min_max.hpp> // For max(3 elements)

#include <glm/ext.hpp>

#include <iostream>

//---------------------------------------------------------------------------------------
void HitRecord::setFaceAndNormal(const Ray& r, const glm::vec3& outwardNormalOfSurface)
{
    m_isFrontFace = !(glm::dot(r.m_direction, outwardNormalOfSurface) > 0.0f); // if ray and outward normal face the same way, then ray is inside the object
    m_normal = m_isFrontFace ? outwardNormalOfSurface : -outwardNormalOfSurface;
}

//---------------------------------------------------------------------------------------
Primitive::~Primitive() {}

//---------------------------------------------------------------------------------------
Sphere::~Sphere() {}

//---------------------------------------------------------------------------------------
bool Sphere::hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const
{
    // Assumes sphere center is at (0,0,0) with radius of 1
    // Solution is a quadratic equation
    float A = glm::dot(r.m_direction, r.m_direction);
    float B = 2 * glm::dot(r.m_direction, r.m_origin);
    float C = glm::dot(r.m_origin, r.m_origin) - 1.0f;

    double roots[2];
    size_t numRoots = quadraticRoots((double) A, (double) B, (double) C, roots);

    if (numRoots == 0)
    {
        return false;
    }
    if (numRoots == 1 || (numRoots == 2 && roots[0] == roots[1]))
    {
        float t = float(roots[0]);
        if (!(t > t_0 && t < t_1))
        {
            return false;
        }
        rec.m_t = t;
        rec.m_hitpoint = r.at(t);
        glm::vec3 outwardNormalOfSurface = glm::normalize(rec.m_hitpoint);
        rec.setFaceAndNormal(r, outwardNormalOfSurface);
        // Material is set by geometry node in A4.cpp
        return true;
    }
    float t = glm::min(float(roots[0]), float(roots[1]));
    if (!(t > t_0 && t < t_1)) // smaller t is not in our range
    {
        // try larger t
        t = glm::max(float(roots[0]), float(roots[1]));
        if (!(t > t_0 && t < t_1))
        {
            return false; // Intersection is outside our range
        }
    }
    rec.m_t = t;
    rec.m_hitpoint = r.at(t);
    glm::vec3 outwardNormalOfSurface = glm::normalize(rec.m_hitpoint);
    rec.setFaceAndNormal(r, outwardNormalOfSurface);

    // Material is set by the geometry node in A4.cpp

    return true;
}

//---------------------------------------------------------------------------------------
Cube::~Cube() {}

//---------------------------------------------------------------------------------------
bool Cube::hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const
{
    glm::vec3 tHitPos = -r.m_origin / r.m_direction;  // This gives us [tx, ty, tz] where each ti is the t value for the ray's intersection with the plane defined by i = ti
    glm::vec3 tHitEnd = (glm::vec3(1.0f, 1.0f, 1.0f) - r.m_origin) / r.m_direction;

    if (glm::any(glm::isnan(tHitPos)) || glm::any(glm::isnan(tHitEnd))) return false;

    // if tHitEnd.x < tHitPos.x, then our ray is approaching the box from the right hand side
    // It hits the right boundary first before the left
    glm::vec3 tEntry = glm::min(tHitPos, tHitEnd); // Computes element-wise min, gets our entry t's on all axes
    glm::vec3 tExit = glm::max(tHitPos, tHitEnd); // Computes element-wise max, gets our exit t's on all axes.

    if (tEntry.x > tExit.y || tEntry.y > tExit.x) return false;

    float tLastEntry = tEntry.x;
    float tFirstExit = tExit.x;

    if (tEntry.y > tLastEntry)
    {
        tLastEntry = tEntry.y;
    }
    if (tExit.y < tFirstExit)
    {
        tFirstExit = tExit.y;
    }

    if (tLastEntry > tExit.z || tEntry.z > tFirstExit) return false;

    if (tEntry.z > tLastEntry)
    {
        tLastEntry = tEntry.z;
    }
    if (tExit.z < tFirstExit)
    {
        tFirstExit = tExit.z;
    }

    if (isinff(tLastEntry) || isinff(tFirstExit)) return false;

    // tFirstExit < tLastEntry means that we exit out of one (or more) of our "sweetspots" before the last "sweetspot" is entered
    // tLastEntry > t_1 or tLastEntry < t_0 mean we're not in our [t_0, t_1] bounds
    if (tFirstExit < tLastEntry || tLastEntry > t_1 || tLastEntry < t_0)
    {
        return false;
    }

    rec.m_t = tLastEntry; // each successive entry intersection clamps our bounds, so the last entry intersection is the one with the box.
    rec.m_hitpoint = r.at(rec.m_t);
    
    glm::vec3 outwardNormalOfSurface = glm::vec3(0.0f);

    if (tLastEntry == tEntry.x)
    {
        outwardNormalOfSurface.x = (r.m_direction.x > 0.0f) ? -1.0f : 1.0f; // If the ray's direction is positive in the axis that it hit the box on, that means it hit m_pos's side first before m_end's
    } else if (tLastEntry == tEntry.y)
    {
        outwardNormalOfSurface.y = (r.m_direction.y > 0.0f) ? -1.0f : 1.0f;
    } else // tLastEntry == tEntry.z
    {
        outwardNormalOfSurface.z = (r.m_direction.z > 0.0f) ? -1.0f : 1.0f;
    }

    rec.setFaceAndNormal(r, outwardNormalOfSurface);

    return true;
}

//---------------------------------------------------------------------------------------
NonhierSphere::~NonhierSphere() {}

//---------------------------------------------------------------------------------------
bool NonhierSphere::hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const
{
    // Solution is a quadratic equation
    float A = glm::dot(r.m_direction, r.m_direction);
    float B = 2 * glm::dot(r.m_direction, r.m_origin - m_pos);
    float C = glm::dot(r.m_origin - m_pos, r.m_origin - m_pos) - glm::pow(m_radius, 2);

    double roots[2];
    size_t numRoots = quadraticRoots((double) A, (double) B, (double) C, roots);

    if (numRoots == 0)
    {
        return false;
    }
    if (numRoots == 1 || (numRoots == 2 && roots[0] == roots[1]))
    {
        float t = float(roots[0]);
        if (!(t > t_0 && t < t_1))
        {
            return false;
        }
        rec.m_t = t;
        rec.m_hitpoint = r.at(t);
        glm::vec3 outwardNormalOfSurface = glm::normalize(rec.m_hitpoint - m_pos);
        rec.setFaceAndNormal(r, outwardNormalOfSurface);
        // Material is set by geometry node in A4.cpp
        return true;
    }
    float t = glm::min(float(roots[0]), float(roots[1]));
    if (!(t > t_0 && t < t_1)) // smaller t is not in our range
    {
        // try larger t
        t = glm::max(float(roots[0]), float(roots[1]));
        if (!(t > t_0 && t < t_1))
        {
            return false; // Intersection is outside our range
        }
    }
    rec.m_t = t;
    rec.m_hitpoint = r.at(t);
    glm::vec3 outwardNormalOfSurface = glm::normalize(rec.m_hitpoint - m_pos);
    rec.setFaceAndNormal(r, outwardNormalOfSurface);

    // Material is set by the geometry node in A4.cpp

    return true;
}

//---------------------------------------------------------------------------------------
NonhierBox::~NonhierBox() {}

//---------------------------------------------------------------------------------------
bool NonhierBox::hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const
{
    // Code assumes AABB box (axis aligned)
    // Help taken from Scratch-A-Pixel AABB intersection guide: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html

    // Idea is that we want the bounding planes for the box. Once our ray intersects with one of the bounding planes, it's now in the "correct range" for that axis for our box.
    // For example, if the ray passes the x = min(tHitPos.x, tHitEnd.x) plane, we are now in the "correct x range" for our box until the ray leaves through max(tHitPos.x, tHitEnd.x)
    // Thus, a successful intersection occurs when our ray goes through min(tHitPos.x, tHitEnd.x), min(tHitPos.y, tHitEnd.y), and min(tHitPos.z, tHitEnd.z) successively in whatever order
    // AS LONG AS it doesn't EXIT out the max() of the above components before it's done entering all the min()s
    
    // Ray is origin + t*direction, while a boundary plane is defined by a constant
    // Thus, t = (constant - origin) / direction
    glm::vec3 tHitPos = (m_pos - r.m_origin) / r.m_direction;  // This gives us [tx, ty, tz] where each ti is the t value for the ray's intersection with the plane defined by i = ti
    glm::vec3 tHitEnd = (m_end - r.m_origin) / r.m_direction;

    if (glm::any(glm::isnan(tHitPos)) || glm::any(glm::isnan(tHitEnd))) return false;

    // if tHitEnd.x < tHitPos.x, then our ray is approaching the box from the right hand side
    // It hits the right boundary first before the left
    glm::vec3 tEntry = glm::min(tHitPos, tHitEnd); // Computes element-wise min, gets our entry t's on all axes
    glm::vec3 tExit = glm::max(tHitPos, tHitEnd); // Computes element-wise max, gets our exit t's on all axes.

    if (tEntry.x > tExit.y || tEntry.y > tExit.x) return false;

    float tLastEntry = tEntry.x;
    float tFirstExit = tExit.x;

    if (tEntry.y > tLastEntry)
    {
        tLastEntry = tEntry.y;
    }
    if (tExit.y < tFirstExit)
    {
        tFirstExit = tExit.y;
    }

    if (tLastEntry > tExit.z || tEntry.z > tFirstExit) return false;

    if (tEntry.z > tLastEntry)
    {
        tLastEntry = tEntry.z;
    }
    if (tExit.z < tFirstExit)
    {
        tFirstExit = tExit.z;
    }

    if (isinff(tLastEntry) || isinff(tFirstExit)) return false;

    // tFirstExit < tLastEntry means that we exit out of one (or more) of our "sweetspots" before the last "sweetspot" is entered
    // tLastEntry > t_1 or tLastEntry < t_0 mean we're not in our [t_0, t_1] bounds
    if (tFirstExit < tLastEntry || tLastEntry > t_1 || tLastEntry < t_0)
    {
        return false;
    }

    rec.m_t = tLastEntry; // each successive entry intersection clamps our bounds, so the last entry intersection is the one with the box.
    rec.m_hitpoint = r.at(rec.m_t);
    
    glm::vec3 outwardNormalOfSurface = glm::vec3(0.0f);

    if (tLastEntry == tEntry.x)
    {
        outwardNormalOfSurface.x = (r.m_direction.x > 0.0f) ? -1.0f : 1.0f; // If the ray's direction is positive in the axis that it hit the box on, that means it hit m_pos's side first before m_end's
    } else if (tLastEntry == tEntry.y)
    {
        outwardNormalOfSurface.y = (r.m_direction.y > 0.0f) ? -1.0f : 1.0f;
    } else // tLastEntry == tEntry.z
    {
        outwardNormalOfSurface.z = (r.m_direction.z > 0.0f) ? -1.0f : 1.0f;
    }

    rec.setFaceAndNormal(r, outwardNormalOfSurface);

    return true;
}

