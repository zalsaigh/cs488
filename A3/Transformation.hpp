#include <glm/glm.hpp>

#include "SceneNode.hpp"


class Transformation
{
public:
	virtual ~Transformation();
	virtual void execute() = 0;
	virtual void undo() = 0;
};

class Translation : Transformation
{
public:
    Translation(SceneNode* node, glm::vec3 t);
	void execute() override;
	void undo() override;

	glm::vec3 t;
	SceneNode* node;

};

class Rotation : Transformation
{
public:
    Rotation(SceneNode* node, glm::vec3 axisOfRotation, float angleOfRotation);
	void execute() override;
	void undo() override;

	glm::vec3 axisOfRotation;
	float angleOfRotation;
	SceneNode* node;

};