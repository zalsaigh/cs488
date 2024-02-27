#include "Transformation.hpp"

//----------------------------------------------------------------------------------------
Translation::Translation(SceneNode* node, glm::vec3 t)
    : node(node),
      t(t)
{
    
}

void Translation::execute()
{
    node->translate(t);
}

void Translation::undo()
{
    node->translate(-t);
}

//----------------------------------------------------------------------------------------
Rotation::Rotation(SceneNode* node, glm::vec3 axisOfRotation, float angleOfRotation)
    : node(node),
      axisOfRotation(axisOfRotation),
      angleOfRotation(angleOfRotation)
{
    
}

void Rotation::execute()
{
    node->rotate(axisOfRotation, angleOfRotation);
}

void Rotation::undo()
{
    node->rotate(axisOfRotation, -angleOfRotation);
}

//----------------------------------------------------------------------------------------