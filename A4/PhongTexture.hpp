// Termm--Winter 2024

#pragma once

#include <glm/glm.hpp>

#include "Image.hpp"
#include "Material.hpp"


class HitRecord;
class Ray;
class PhongTexture : public Material {
public:
  PhongTexture(Image &im, const glm::vec3& ks, double shininess);
  virtual ~PhongTexture();

  Image m_image;
  glm::vec3 m_ks;

  double m_shininess;

  bool Scatter(const Ray &incomingRay, const HitRecord& rec, glm::vec3& outAttenuation, Ray& outScatteredRay) const override;
  const glm::vec3 getColorAt(float u, float v) const;

private:
};
