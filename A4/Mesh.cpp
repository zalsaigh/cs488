// Termm--Fall 2020

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"
#include "Ray.hpp"

//---------------------------------------------------------------------------------------
bool Triangle::hit(const Ray &r, glm::vec3 vert1, glm::vec3 vert2, glm::vec3 vert3, float t_0, float t_1, HitRecord& rec) const
{

	// glm::vec3 R = r.m_origin - vert1;
	// glm::vec3 subV2V1 = vert2 - vert1;
	// glm::vec3 subV3V1 = vert3 - vert1;
	// glm::vec3 negDir = -r.m_direction;

	// glm::mat3 M(subV2V1, subV3V1, negDir); // Column major, subV2V1 is the first column
	// glm::mat3 M1(R, subV3V1, negDir);
	// glm::mat3 M2(subV2V1, R, negDir);
	// glm::mat3 M3(subV2V1, subV3V1, R);		

	glm::vec3 subV1Orig = vert1 - r.m_origin;
	glm::vec3 subV1V2 = vert1 - vert2;
	glm::vec3 subV1V3 = vert1 - vert3;

	glm::mat3 M(subV1V2, subV1V3, r.m_direction);
	glm::mat3 M1(subV1Orig, subV1V3, r.m_direction);
	glm::mat3 M2(subV1V2, subV1Orig, r.m_direction);
	glm::mat3 M3(subV1V2, subV1V3, subV1Orig);

	float D = glm::determinant(M); 
	float D1 = glm::determinant(M1);
	float D2 = glm::determinant(M2);
	float D3 = glm::determinant(M3);

	if (D == 0.0f) {
		return false;
	}

	float beta = D1 / D;
	float gamma = D2 / D;
	if (!(beta >= 0.0f && gamma >= 0.0f && (beta + gamma <= 1.0f)))
	{
		return false;
	} 
	float t = D3 / D;
	if (t < t_0 || t > t_1) return false;

	rec.m_t = t;
	rec.m_hitpoint = r.at(t);
	glm::vec3 outwardNormalOfSurface = glm::normalize(glm::cross(subV1V2, subV1V3));
	rec.setFaceAndNormal(r, outwardNormalOfSurface);
	return true;

}
//---------------------------------------------------------------------------------------
Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;
	glm::vec3 boxMin;
	glm::vec3 boxMax;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			glm::vec3 newVert = glm::vec3(vx, vy, vz);
			m_vertices.push_back( newVert );
			boxMin = glm::min(boxMin, newVert);
			boxMax = glm::max(boxMax, newVert);
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}

	m_boundingBox = new NonhierBox(boxMin, boxMax);
}

//---------------------------------------------------------------------------------------
Mesh::~Mesh()
{
	delete m_boundingBox;
}

//---------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

//---------------------------------------------------------------------------------------
bool Mesh::hit(const Ray &r, float t_0, float t_1, HitRecord& rec) const
{
	if (!m_boundingBox->hit(r, t_0, t_1, rec)) return false;
	float closestT = t_1;
	bool hitAnything = false;
	HitRecord currRec;
	for (auto &face : m_faces)
	{
		if (face.hit(r, m_vertices[face.v1], m_vertices[face.v2], m_vertices[face.v3], t_0, closestT, currRec))
		{
			closestT = currRec.m_t;
			hitAnything = true;
			rec = currRec;
			rec.m_prim = this;
		}
	}
    return hitAnything;
}
