// Term--Winter 2024

#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

#include "A4.hpp"
#include "Ray.hpp"
#include "PhongMaterial.hpp"
#include "PhongTexture.hpp"


const float EPSILON = 0.01f;
const int NUM_SAMPLES = 6;

const int FRAMES_PER_SECOND = 24;  // Frames per second
const int DURATION = 11;  // Duration of animation in seconds
const int TOTAL_FRAME_COUNT = FRAMES_PER_SECOND * DURATION;

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
		// TODO IN THE FUTURE: abstract into a base color() function overriden for each material instead of this if/else if
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

		} else if (rec.m_mat->m_materialType == MaterialType::PhongTexture)
		{
			PhongTexture *phongText = static_cast<PhongTexture *>(rec.m_mat);
			glm::vec3 V = glm::normalize(eye - rec.m_hitpoint);
			glm::vec3 N = glm::normalize(rec.m_normal); // Already normalized
			glm::vec2 uv = rec.m_prim->getTextureCoords(rec);
			glm::vec3 kd = phongText->getColorAt(uv[0], uv[1]);
			
			colour = ambient * kd;
			glm::vec3 diffuse(0.0f);
			glm::vec3 specular(0.0f);

			for (auto *light : lights)
			{
				Ray shadowRay = Ray(rec.m_hitpoint, light->position - rec.m_hitpoint);
				float lightDistance = glm::length(light->position - rec.m_hitpoint);
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
					specular += (attenuatedColour * glm::pow(glm::dot(H, N), phongText->m_shininess));// From wikipedia - "Additionally, the specular term should only be included if the dot product of the diffuse term is positive"
				}	
			}
			colour += (kd * diffuse) + (phongText->m_ks * specular);

			Ray scatteredRay;
			glm::vec3 attenuation;

			if (phongText->Scatter(ray, rec, attenuation, scatteredRay))
			{
				colour += (phongText->m_ks / 10.0f) * rayColour(root, scatteredRay, glm::vec3(0.0f), lights, eye, maxDepth - 1);
			}
			// std::cout << "We get here, color = " << glm::to_string(colour) << "\n";
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
	const float defocusDiskAngle,
	const float defocusDiskRadius,
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
				float horizontalRandom = glm::linearRand(-0.5f, 0.5f) * horizontalSpacing;
				float verticalRandom = glm::linearRand(-0.5f, 0.5f) * verticalSpacing;
				glm::vec3 randSample{horizontalRandom, verticalRandom, 0.0f};
				glm::vec3 randomDiskPoint = (defocusDiskAngle <= 0) ? glm::vec3(0.0f) : glm::vec3(glm::diskRand(defocusDiskRadius), 0.0f);
				Ray r = Ray(eye + randomDiskPoint, pWorld3D + randSample - (eye + randomDiskPoint));
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
	float ny = float(image.height());
	float nx = float(image.width());
	float aspectRatio = nx/ny;
	float defocusAngle = glm::radians(1.5f); // Angle of the cone with base being the defocus disk and tip being `view` (lookAt vector)
	float focusDist = glm::length(view - eye);;  // Just set the focus plane to the viewport
	float defocusRadius = focusDist * tan(defocusAngle / 2.0f); // the tan (opp / adj) gives (radius of disk / focusDist), so mult by focusDist to get radius
	float fov = glm::radians(float(fovy));
	float d = 1.0f; // d sets the viewing plane for the scene/world behind it.
	// float h = 2.0f * d * glm::tan(fov / 2);
	float h = 2.0f * focusDist * glm::tan(fov / 2);
	float w = h * aspectRatio;

	// glm::mat4 T1 = glm::translate(glm::vec3(-nx/2, -ny/2, d));
	glm::mat4 T1 = glm::translate(glm::vec3(-nx/2, -ny/2, focusDist));
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

	// Subdivision by height
	int chunkSizeY = image.height() / nthreads;

	float horizontalSpacing = w / nx;
	float verticalSpacing = h / ny;

	for (int i = 0; i < nthreads; i++)
	{
		int startX = 0;
		int endX = image.width();

		int startY = i * chunkSizeY;
		int endY = (i + 1) * chunkSizeY;

		threads.emplace_back(std::thread(
			processChunk,
			startX,
			endX,
			startY,
			endY,
			horizontalSpacing,
			verticalSpacing,
			root,
			std::ref(image),
			std::cref(eye),
			defocusAngle,
			defocusRadius,
			std::cref(ambient),
			std::cref(lights),
			std::cref(screenToWorld))
		);
	}

	for (auto &thread : threads)
	{
		thread.join();
		std::cout << "Thread finished\n";
	}

  	std::cout << "Calling A4_Render(\n" <<
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

//---------------------------------------------------------------------------------------
SceneNode *getNode(SceneNode *root, const char *nodeName)
{
	for (auto *child : root->children)
	{
		if (child->m_name == nodeName) return child;
	}
	return nullptr;
}

//---------------------------------------------------------------------------------------
glm::vec3 findStraightLineIntersection(GeometryNode *movingNode, GeometryNode *groundNode)
{
	// Just assume sphere and box for now

	glm::vec3 bottomOfBall = movingNode->m_primitive->getPosition() - glm::vec3(0.0, movingNode->m_primitive->getDimensionalSize(), 0.0);
	return glm::vec3(bottomOfBall.x, groundNode->m_primitive->getEndPosition().y, bottomOfBall.z);
}

//---------------------------------------------------------------------------------------
std::array<glm::vec3, TOTAL_FRAME_COUNT> getBallAnimationPositions(GeometryNode *movingNode, glm::vec3 intersectionPoint)
{
	glm::vec3 bottomOfBall = movingNode->m_primitive->getPosition() - glm::vec3(0.0, movingNode->m_primitive->getDimensionalSize(), 0.0);
	double difference = bottomOfBall.y - intersectionPoint.y;

	std::array<glm::vec3, TOTAL_FRAME_COUNT> ballPositions;

	// We want sin(pi/2) on frames 12, 36, 60, 84
	// We want sin(0) on frames 0, 24, 48, 72, 96, ... , 240

	double frequency = 1 / double(FRAMES_PER_SECOND);

	for (int i = 0; i < TOTAL_FRAME_COUNT; i++)
	{
		double sinusoidalOutput = (1.0 + glm::sin(2.0 * M_PI * frequency * float(i) - M_PI_2)) / 2.0;
		glm::vec3 pos = movingNode->m_primitive->getPosition() - glm::vec3(0.0, difference * sinusoidalOutput, 0.0);
		ballPositions[i] = pos;
	}
	return ballPositions;
}

//---------------------------------------------------------------------------------------
void CreateFrames(
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
		const std::list<Light *> & lights,

		const char* fileStub
)
{
	applyTransforms(root, glm::mat4(1.0f));
	SceneNode *ballNode = getNode(root, "s1");
	SceneNode *groundNode = getNode(root, "s3");
	if (!ballNode || !groundNode)
	{
		std::cerr << "Could not find either node that will be animated or ground node!\n";
		return;
	}

	if ((ballNode->m_nodeType != NodeType::GeometryNode) || (groundNode->m_nodeType != NodeType::GeometryNode))
	{
		std::cerr << "Either the node that will be animated or the ground node are not Geometry nodes.\n";
		return;
	}

	GeometryNode *ballGNode = static_cast<GeometryNode *>(ballNode);
	GeometryNode *groundGNode = static_cast<GeometryNode *>(groundNode);

	if (!ballGNode || !groundGNode)
	{
		std::cerr << "Either the node that willl be animated or the ground node are nullptr.\n";
	}

	glm::vec3 intersectionPoint = findStraightLineIntersection(ballGNode, groundGNode);

	// Sin-based function that returns a list of positions for the ball for each frame
	std::array<glm::vec3, TOTAL_FRAME_COUNT> ballAnimationPositions = getBallAnimationPositions(ballGNode, intersectionPoint); 

	glm::vec3 ballYAxis = up; // up is 0, 1, 0, so the camera rotates around the origin y-axis
	for (int i = 0; i < TOTAL_FRAME_COUNT; i++)
	{
		ballGNode->m_primitive->setPosition(ballAnimationPositions[i]);
		const glm::vec3 currEye = glm::vec3(glm::rotate(2.0f * float(M_PI) * (float(i) / float(TOTAL_FRAME_COUNT - 1)), ballYAxis) * glm::vec4(eye, 1.0f)); // i can only go from 0 to TOTAL_FRAME_COUNT - 1
		A4_Render(root, image, currEye, view, up, fovy, ambient, lights); 
		// A4_Render(root, image, eye, view, up, fovy, ambient, lights);
		std::stringstream currFileSuffix;
		currFileSuffix << std::setw(3) << std::setfill('0') << i;
		std::string filename = fileStub + currFileSuffix.str() + ".png";
		image.savePng("./output/" + filename);
		std::cout << "Completed frame " << i + 1 << "\n";
	}
}