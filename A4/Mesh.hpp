// Termm--Fall 2020

#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"

// Use this #define to selectively compile your code to render the
// bounding boxes around your mesh objects. Uncomment this option
// to turn it on.
//#define RENDER_BOUNDING_VOLUMES

struct Triangle
{
	size_t v1;
	size_t v2;
	size_t v3;

	Triangle( size_t pv1, size_t pv2, size_t pv3 )
		: v1( pv1 )
		, v2( pv2 )
		, v3( pv3 )
	{}

	bool hit(const Ray &r, glm::vec3 vert1, glm::vec3 vert2, glm::vec3 vert3, float t_0, float t_1, HitRecord& rec) const;
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  	Mesh( const std::string& fname );
  	~Mesh();

  	virtual bool hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const override;
  	virtual void setPosition(glm::vec3 pos) override {} // Does nothing
  	virtual glm::vec3 getPosition() const override { return {0, 0, 0}; } // Unused
  	virtual double getDimensionalSize() const override { return 1.0; } // Unused
  	virtual glm::vec3 getEndPosition() const override { return {0, 0, 0}; } // Unused
	virtual glm::vec2 getTextureCoords(const HitRecord &rec) const override { return {0,0}; } // Unused
  
private:
	std::vector<glm::vec3> m_vertices;
	std::vector<Triangle> m_faces;
	NonhierBox *m_boundingBox;

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
