// Termm--Fall 2020

#include <iostream>

#include <glm/ext.hpp>

#include "Light.hpp"

Light::Light()
  : colour(0.0, 0.0, 0.0),
    position(0.0, 0.0, 0.0)
{
  falloff[0] = 1.0;
  falloff[1] = 0.0;
  falloff[2] = 0.0;
}
//---------------------------------------------------------------------------------------
glm::vec3 Light::getAttenuatedColour(float distance)
{
  // Assumes that falloff is not all 0
  return colour / (falloff[0] + falloff[1] * distance + falloff[2] * distance * distance); // Light attenuation formula taken from learnWebGL
}

//---------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const Light& l)
{
  out << "L[" << glm::to_string(l.colour) 
  	  << ", " << glm::to_string(l.position) << ", ";
  for (int i = 0; i < 3; i++) {
    if (i > 0) out << ", ";
    out << l.falloff[i];
  }
  out << "]";
  return out;
}
