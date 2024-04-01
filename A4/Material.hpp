// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

class HitRecord;
class Ray;

enum class MaterialType {
	PhongMaterial,
  PhongTexture
};

class Material {
public:
  virtual ~Material();
  MaterialType m_materialType;

  virtual bool Scatter(const Ray &incomingRay, const HitRecord& rec, glm::vec3& outAttenuation, Ray& outScatteredRay) const = 0;

protected:
  Material();
};
