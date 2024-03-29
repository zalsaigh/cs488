// Term--Winter 2024

#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include <thread>
#include <vector>

#include "A4.hpp"
#include "Ray.hpp"
#include "PhongMaterial.hpp"


const float EPSILON = 0.01f;
const int NUM_SAMPLES = 50;

//----------------------------------------------------------------------------------------
/*
 * Applies transforms recursively
 */
void applyTransforms(SceneNode* node, glm::mat4 stackedTransforms)
{
	node->set_transform(stackedTransforms * node->get_transform());
	for (auto* child : node->children)
	{
		applyTransforms(child, node->get_transform());
	}
}

//---------------------------------------------------------------------------------------
void setColour(Image &image, int x, int y, const glm::vec3 &colour)
{
	image(x, y, 0) = colour[0];
	image(x, y, 1) = colour[1];
	image(x, y, 2) = colour[2];
}

//---------------------------------------------------------------------------------------
bool sceneHit(SceneNode *root, const Ray &ray, float tMin , float tMax, HitRecord &rec)
{
	HitRecord currRec;

	bool hitAnything = false;
	float closestT = tMax;

	for (auto *child : root->children)
	{
		// glm::mat4 invTrans = child->get_inverse();
		// glm::mat4 trans = child->get_transform();
		if (child->m_nodeType == NodeType::SceneNode)
		{
			if (sceneHit(child, ray, tMin, closestT, currRec))
			{
				// if (closestT > glm::length(currRec.m_hitpoint - ray.m_origin))
				// {
				// 	closestT = glm::length(currRec.m_hitpoint - ray.m_origin);
				// 	rec = currRec;
				// 	hitAnything = true;
				// }
				closestT = currRec.m_t;
				rec = currRec;
				hitAnything = true;
				
			}

		} else if (child->m_nodeType == NodeType::GeometryNode) // Leaf node, no children
		{
			GeometryNode *gNode = static_cast<GeometryNode *>(child);
			// Ray transformedRay = Ray(glm::vec3(invTrans * glm::vec4(ray.m_origin, 1.0f)), glm::vec3(invTrans * glm::vec4(ray.m_direction, 0.0f)));
			// transformedRay.m_direction = glm::normalize(transformedRay.m_direction);
			if (gNode->m_primitive->hit(ray, tMin, closestT, currRec))
			{
				
				// currRec.m_hitpoint = glm::vec3(trans * glm::vec4(currRec.m_hitpoint, 1.0f));
				// currRec.m_normal = glm::mat3(glm::transpose(invTrans)) * currRec.m_normal;
				// currRec.m_normal = glm::normalize(currRec.m_normal);
				// if (closestT > glm::length(currRec.m_hitpoint - ray.m_origin))
				// {
				// 	closestT = glm::length(currRec.m_hitpoint - ray.m_origin);
				// 	rec = currRec;
				// 	hitAnything = true;
				// }
				currRec.m_mat = gNode->m_material;
				closestT = currRec.m_t;
				rec = currRec;	
				hitAnything = true;
			}
		}
	}
	return hitAnything;
}

//---------------------------------------------------------------------------------------
glm::vec3 rayColour(SceneNode *root, const Ray &ray, const glm::vec3 & ambient, const std::list<Light *> & lights, const glm::vec3 &eye, int maxDepth=5) 
{
	if (maxDepth <= 0) return {0.0f, 0.0f, 0.0f};
	HitRecord rec;
	if (sceneHit(root, ray, EPSILON, INFINITY, rec))
	{
		glm::vec3 colour(0.0f);
		if (rec.m_mat->m_materialType == MaterialType::PhongMaterial)
		{
			// Formula taken from wikipedia's Phong reflection model and Blinn-Phong Halfway vector replacement
			PhongMaterial *phongMat = static_cast<PhongMaterial *>(rec.m_mat);
			glm::vec3 V = glm::normalize(eye - rec.m_hitpoint);
			glm::vec3 N = glm::normalize(rec.m_normal); // Already normalized
			colour = ambient * phongMat->m_kd;
			glm::vec3 diffuse(0.0f);
			glm::vec3 specular(0.0f);
			for (auto *light : lights)
			{
				Ray shadowRay = Ray(rec.m_hitpoint, light->position - rec.m_hitpoint);
				float lightDistance = glm::length(light->position - rec.m_hitpoint);
				// shadowRay.m_direction /= lightDistance;
				HitRecord _; // We don't care about this hit record, we just care about return value
				if (sceneHit(root, shadowRay, EPSILON, lightDistance, _))
				{
					continue; // This light will not contribute to the total
				}
				glm::vec3 L = shadowRay.m_direction / lightDistance;
				glm::vec3 H = glm::normalize(L + V); // Halfway vector for Blinn-Phong
				glm::vec3 attenuatedColour = light->getAttenuatedColour(lightDistance);
				float dotLN = glm::max(0.0f, glm::dot(L, N)); // From wikipedia - "each term should only be included if the term's dot product is positive"
				diffuse += (attenuatedColour * dotLN);
				if (dotLN > 0.0f)
				{
					specular += (attenuatedColour * glm::pow(glm::dot(H, N), phongMat->m_shininess));// From wikipedia - "Additionally, the specular term should only be included if the dot product of the diffuse term is positive"
				}	
			}
			colour += (phongMat->m_kd * diffuse) + (phongMat->m_ks * specular);

			Ray scatteredRay;
			glm::vec3 attenuation;

			if (phongMat->Scatter(ray, rec, attenuation, scatteredRay))
			{
				colour += glm::vec3{0.1f} * rayColour(root, scatteredRay, glm::vec3(0.0f), lights, eye, maxDepth - 1);
			}

		}
		return colour;
	}
	float t = 0.5f * (glm::normalize(ray.m_direction).y + 1.0f);
	return (1.0f - t)*glm::vec3(1.0f) + t*glm::vec3(0.5f, 0.7f, 1.0f);
}

//---------------------------------------------------------------------------------------
void processChunk(
	int startX,
	int endX,
	int startY,
	int endY,
	float horizontalSpacing,
	float verticalSpacing,

	SceneNode *root,
	Image &image,
	const glm::vec3 &eye,
	const glm::vec3 & ambient,
	const std::list<Light *> & lights,

	const glm::mat4 &screenToWorld
)
{
	for (int i = startX; i < endX; i++)
	{
		for (int j = startY; j < endY; j++)
		{
			glm::vec4 pWorld = screenToWorld * glm::vec4(i, j, 0.0f, 1.0f);
			glm::vec3 pWorld3D(pWorld);
			glm::vec3 colour(0.0f);
			for (int sample = 0; sample < NUM_SAMPLES; sample++)
			{
				// Ray r = Ray(eye, glm::normalize(pWorld3D - eye));
				float horizontalRandom = glm::linearRand(-0.5f, 0.5f) * horizontalSpacing;
				float verticalRandom = glm::linearRand(-0.5f, 0.5f) * verticalSpacing;
				glm::vec3 randSample{horizontalRandom, verticalRandom, 0.0f};
				Ray r = Ray(eye, pWorld3D + randSample - eye);
				colour += rayColour(root, r, ambient, lights, eye);
			}
			colour /= NUM_SAMPLES;
			setColour(image, i, j, colour);
		}
	}
}
//---------------------------------------------------------------------------------------
void A4_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters  
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {
	applyTransforms(root, glm::mat4(1.0f));
	float ny = float(image.height());
	float nx = float(image.width());
	float aspectRatio = nx/ny;
	float fov = glm::radians(float(fovy));
	float d = 1.0f; // d sets the viewing plane for the scene/world behind it.
	float h = 2.0f * d * glm::tan(fov / 2);
	float w = h * aspectRatio;

	glm::mat4 T1 = glm::translate(glm::vec3(-nx/2, -ny/2, d));
	glm::mat4 S2 = glm::scale(glm::vec3(-h / ny, -w / nx, 1.0f)); // negative y component because y goes down in screen space
	glm::vec3 eyeBasisW = glm::normalize(view - eye); 
	glm::vec3 eyeBasisU = glm::normalize(glm::cross(up, eyeBasisW));
	glm::vec3 eyeBasisV = glm::cross(eyeBasisW, eyeBasisU);

	glm::mat4 R3(
		eyeBasisU.x, eyeBasisU.y, eyeBasisU.z, 0.0f,
		eyeBasisV.x, eyeBasisV.y, eyeBasisV.z, 0.0f,
		eyeBasisW.x, eyeBasisW.y, eyeBasisW.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	glm::mat4 T4 = glm::translate(eye);

	glm::mat4 screenToWorld = T4 * R3 * S2 * T1;

	const unsigned int nthreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;

	std::cout << "Num processors: " <<  nthreads << "\n";

	int chunkSizeX = image.width() / (nthreads / 2);
	int chunkSizeY = image.height() / (nthreads / 2);

	float horizontalSpacing = w / nx;
	float verticalSpacing = h / ny;

	for (int i = 0; i < nthreads / 2; i++)
	{
		// if ((image.width() - i) % 10 == 0)
		// {
		// 	std::cout << "Lines left: " <<  image.width() - i << "\n"; 
		// }
		for (int j = 0; j < nthreads / 2; j++)
		{
			// glm::vec4 pWorld = screenToWorld * glm::vec4(i, j, 0.0f, 1.0f);
			// glm::vec3 pWorld3D(pWorld);
			// glm::vec3 colour(0.0f);
			// for (int sample = 0; sample < NUM_SAMPLES; sample++)
			// {
			// 	// Ray r = Ray(eye, glm::normalize(pWorld3D - eye));
			// 	float horizontalRandom = glm::linearRand(-0.5f, 0.5f) * horizontalSpacing;
			// 	float verticalRandom = glm::linearRand(-0.5f, 0.5f) * verticalSpacing;
			// 	glm::vec3 randSample{horizontalRandom, verticalRandom, 0.0f};
			// 	Ray r = Ray(eye, pWorld3D + randSample - eye);
			// 	colour += rayColour(root, r, ambient, lights, eye);
			// }
			// colour /= NUM_SAMPLES;
			// setColour(image, i, j, colour);
			int startX = i * chunkSizeX;
			int endX = (i + 1) * chunkSizeX;

			int startY = j * chunkSizeY;
			int endY = (j + 1) * chunkSizeY;

			threads.emplace_back(std::thread(processChunk, startX, endX, startY, endY, horizontalSpacing, verticalSpacing, root, std::ref(image), std::cref(eye), std::cref(ambient), std::cref(lights), std::cref(screenToWorld)));
		}
	}

	for (auto &thread : threads)
	{
		thread.join();
		std::cout << "Thread finished\n";
	}

  	std::cout << "F20: Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;
}
