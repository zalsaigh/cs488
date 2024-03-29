// Termm--Fall 2020

#include "PhongMaterial.hpp"
#include "Primitive.hpp"
#include "Ray.hpp"
#include <glm/gtc/random.hpp>

//---------------------------------------------------------------------------------------
PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess )
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
{
	m_materialType = MaterialType::PhongMaterial;
}

//---------------------------------------------------------------------------------------
PhongMaterial::~PhongMaterial()
{}

//---------------------------------------------------------------------------------------
bool PhongMaterial::Scatter(const Ray &incomingRay, const HitRecord& rec, glm::vec3& outAttenuation, Ray& outScatteredRay) const
{
	outScatteredRay.m_origin = rec.m_hitpoint;
	float lenIncRayDir = glm::length(incomingRay.m_direction);
	outScatteredRay.m_direction = generateReflectionVector(glm::normalize(incomingRay.m_direction), rec.m_normal); //+  0.1f * glm::sphericalRand(1.0f);
	// outScatteredRay.m_direction *= lenIncRayDir;
	// TODO: Fuzz factor hardcoded to 0.2f and sphericalRand(1.0f) should be fine? outAttenuation also hardcoded
	outAttenuation = {0.7f, 0.6f, 0.5f};

	return (glm::dot(outScatteredRay.m_direction, rec.m_normal) > 0.0f); // I could just return true if there's no fuzz
}