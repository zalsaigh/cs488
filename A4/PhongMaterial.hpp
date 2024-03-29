// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

#include "Material.hpp"

class HitRecord;
class Ray;
class PhongMaterial : public Material {
public:
  PhongMaterial(const glm::vec3& kd, const glm::vec3& ks, double shininess);
  virtual ~PhongMaterial();

  glm::vec3 m_kd;
  glm::vec3 m_ks;

  double m_shininess;

  bool Scatter(const Ray &incomingRay, const HitRecord& rec, glm::vec3& outAttenuation, Ray& outScatteredRay) const override;

private:
};
