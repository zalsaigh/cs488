// Termm-Fall 2020

#include "SceneNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <sstream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;


// Static class variable
unsigned int SceneNode::nodeInstanceCount = 0;


//---------------------------------------------------------------------------------------
SceneNode::SceneNode(const std::string& name)
  : m_name(name),
	m_nodeType(NodeType::SceneNode),
	trans(mat4()),
	invtrans(mat4()),
	localTrans(mat4()),
	invLocalTrans(mat4()),
	localRotationsAndScales(mat4()),
	localTranslations(mat4()),
	localViewRotations(mat4()),
	isSelected(false),
	m_nodeId(nodeInstanceCount++)
{

}

//---------------------------------------------------------------------------------------
// Deep copy
SceneNode::SceneNode(const SceneNode & other)
	: m_nodeType(other.m_nodeType),
	  m_name(other.m_name),
	  trans(other.trans),
	  invtrans(other.invtrans),
	  localTrans(other.localTrans)
{
	for(SceneNode * child : other.children) {
		this->children.push_front(new SceneNode(*child));
	}
}

//---------------------------------------------------------------------------------------
SceneNode::~SceneNode() {
	for(SceneNode * child : children) {
		delete child;
	}
}

//---------------------------------------------------------------------------------------
void SceneNode::set_transform(const glm::mat4& m) {
	trans = m;
	invtrans = glm::inverse(trans);
}

//---------------------------------------------------------------------------------------
void SceneNode::set_local_transform(const glm::mat4& m) {
	localTrans = m;
	invLocalTrans = glm::inverse(localTrans);
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_transform() const {
	return trans;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_inverse() const {
	return invtrans;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_local_transform() const {
	return localTrans;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_inverse_local_transform() const {
	return invLocalTrans;
}

//---------------------------------------------------------------------------------------
void SceneNode::add_child(SceneNode* child) {
	children.push_back(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::remove_child(SceneNode* child) {
	children.remove(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::rotate(char axis, float angle) {
	vec3 rot_axis;

	switch (axis) {
		case 'x':
			rot_axis = vec3(1,0,0);
			break;
		case 'y':
			rot_axis = vec3(0,1,0);
	        break;
		case 'z':
			rot_axis = vec3(0,0,1);
	        break;
		default:
			break;
	}
	localRotationsAndScales = glm::rotate(degreesToRadians(angle), rot_axis) * localRotationsAndScales;
	// localTrans = localRotationsAndScales * localTrans;
	set_local_transform(localViewRotations * localTranslations * localRotationsAndScales);
}

//---------------------------------------------------------------------------------------
void SceneNode::rotate(const glm::vec3& axisOfRotation, float angleInRadians) {
	localRotationsAndScales = glm::rotate(angleInRadians, axisOfRotation) * localRotationsAndScales;
	// localTrans = localTrans * glm::rotate(angleInRadians, axisOfRotation);
	set_local_transform(localViewRotations * localTranslations * localRotationsAndScales);
}

//---------------------------------------------------------------------------------------
void SceneNode::rotateAboutView(const glm::vec3& axisOfRotation, float angleInRadians) {
	// localTrans = glm::rotate(angleInRadians, axisOfRotation) * localTrans;
	localViewRotations = glm::rotate(angleInRadians, axisOfRotation) * localViewRotations;
	set_local_transform(localViewRotations * localTranslations * localRotationsAndScales);
}

//---------------------------------------------------------------------------------------
void SceneNode::scale(const glm::vec3 & amount) {
	// localTrans = localTrans * glm::scale(amount);
	localRotationsAndScales = localRotationsAndScales * ::scale(amount);
	set_local_transform(localViewRotations * localTranslations * localRotationsAndScales);
}

//---------------------------------------------------------------------------------------
void SceneNode::translate(const glm::vec3& amount) {
	// localTrans =  glm::translate(amount) * localTrans;
	localTranslations = glm::translate(amount) * localTranslations;
	set_local_transform(localViewRotations * localTranslations * localRotationsAndScales);
}


//---------------------------------------------------------------------------------------
int SceneNode::totalSceneNodes() const {
	return nodeInstanceCount;
}

//---------------------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const SceneNode & node) {

	//os << "SceneNode:[NodeType: ___, name: ____, id: ____, isSelected: ____, transform: ____"
	switch (node.m_nodeType) {
		case NodeType::SceneNode:
			os << "SceneNode";
			break;
		case NodeType::GeometryNode:
			os << "GeometryNode";
			break;
		case NodeType::JointNode:
			os << "JointNode";
			break;
	}
	os << ":[";

	os << "name:" << node.m_name << ", ";
	os << "id:" << node.m_nodeId;
	os << "]";

	return os;
}
