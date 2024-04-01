// Termm--Winter 2024

#include "PhongTexture.hpp"
#include "Primitive.hpp"
#include "Ray.hpp"
#include <glm/gtc/random.hpp>
#include <iostream>

//---------------------------------------------------------------------------------------
PhongTexture::PhongTexture(
	Image &im, const glm::vec3& ks, double shininess )
	: m_image(im)
	, m_ks(ks)
	, m_shininess(shininess)
{
	m_materialType = MaterialType::PhongTexture;
    // std::cout << m_image.width() << " " << m_image.height();
}

//---------------------------------------------------------------------------------------
PhongTexture::~PhongTexture()
{}

//---------------------------------------------------------------------------------------
bool PhongTexture::Scatter(const Ray &incomingRay, const HitRecord& rec, glm::vec3& outAttenuation, Ray& outScatteredRay) const
{
	outScatteredRay.m_origin = rec.m_hitpoint;
	float lenIncRayDir = glm::length(incomingRay.m_direction);
	outScatteredRay.m_direction = generateReflectionVector(glm::normalize(incomingRay.m_direction), rec.m_normal); //+  0.1f * glm::sphericalRand(1.0f);
	// outScatteredRay.m_direction *= lenIncRayDir;
	// TODO: Fuzz factor hardcoded to 0.2f and sphericalRand(1.0f) should be fine? outAttenuation also hardcoded
	outAttenuation = {0.7f, 0.6f, 0.5f};

	return (glm::dot(outScatteredRay.m_direction, rec.m_normal) > 0.0f); // I could just return true if there's no fuzz
}

//---------------------------------------------------------------------------------------
const glm::vec3 PhongTexture::getColorAt(float u, float v) const
{
    // std::cout << "u = " << u << ", v = " << v << "\n";
    int i = (int) glm::floor(u * float(m_image.width()));
    int j = (int) glm::floor(v * float(m_image.height()));
    if (i == m_image.width()) i--;
    if (j == m_image.height()) j--;
    // std::cout << "i = " << i << ", " << "j = " << j << "\n";
    return glm::vec3(m_image(i, j, 0), m_image(i, j, 1), m_image(i, j, 2));
}
