// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

class Ray;
class Material;
class Primitive;

class HitRecord
{
public:
  glm::vec3 m_hitpoint;
  glm::vec3 m_normal;
  float m_t;
  Material* m_mat;
  const Primitive* m_prim;
  bool m_isFrontFace;

  void setFaceAndNormal(const Ray& r, const glm::vec3& outwardNormalOfSurface);
};

class Primitive {
public:
  virtual ~Primitive();

  virtual bool hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const = 0;
  virtual void setPosition(glm::vec3 pos) = 0;
  virtual glm::vec3 getPosition() const = 0;
  virtual double getDimensionalSize() const = 0; // Radius for sphere, size for box, etc.
  virtual glm::vec3 getEndPosition() const = 0;
  virtual glm::vec2 getTextureCoords(const HitRecord &rec) const = 0;
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();

  virtual bool hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const override;
  virtual void setPosition(glm::vec3 pos) override {} // Does nothing
  virtual glm::vec3 getPosition() const override { return {0, 0, 0}; }
  virtual double getDimensionalSize() const override { return 1.0; } // Unused
  virtual glm::vec3 getEndPosition() const override { return {0, 0, 0}; } // Unused
  virtual glm::vec2 getTextureCoords(const HitRecord &rec) const override {return {0, 0}; } // Unused
};

class Cube : public Primitive {
public:
  virtual ~Cube();

  virtual bool hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const override;
  virtual void setPosition(glm::vec3 pos) override {} // Does nothing
  virtual glm::vec3 getPosition() const override { return {0, 0, 0}; }
  virtual double getDimensionalSize() const override { return 1.0; } // Unused
  virtual glm::vec3 getEndPosition() const override { return {0, 0, 0}; } // Unused
  virtual glm::vec2 getTextureCoords(const HitRecord &rec) const override {return {0, 0}; } // Unused
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();

  virtual bool hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const override;
  virtual void setPosition(glm::vec3 pos) override { m_pos = pos; }
  virtual glm::vec3 getPosition() const override { return m_pos; }
  virtual double getDimensionalSize() const override { return m_radius; }
  virtual glm::vec3 getEndPosition() const override { return {0, 0, 0}; } // Unused
  virtual glm::vec2 getTextureCoords(const HitRecord &rec) const override;

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
    // To center the cube, translate m_pos by (-size/2 - m_pos) on all axes
    m_end = glm::vec3(pos.x + size, pos.y + size, pos.z + size);
  }

  NonhierBox(const glm::vec3& pos, const glm::vec3 &end) : m_pos(pos), m_size(0.0), m_end(end) {}  // Don't care about mesh size because it's not used
  
  virtual ~NonhierBox();

  virtual bool hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const override;
  virtual void setPosition(glm::vec3 pos) override
  {
    m_end = m_end - (pos - m_pos);
    m_pos = pos;
  }
  virtual glm::vec3 getPosition() const override { return m_pos; }
  virtual double getDimensionalSize() const override { return m_size; } // Unused
  virtual glm::vec3 getEndPosition() const override { return m_end; }
  virtual glm::vec2 getTextureCoords(const HitRecord &rec) const override;

private:
  glm::vec3 m_pos;
  double m_size;
  glm::vec3 m_end;
};
