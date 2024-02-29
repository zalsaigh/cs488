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
std::unordered_map<unsigned int, SceneNode*> SceneNode::selectionMap;


//---------------------------------------------------------------------------------------
SceneNode::SceneNode(const std::string& name)
  : m_name(name),
	m_nodeType(NodeType::SceneNode),
	trans(mat4()),
	invtrans(mat4()),
	localTrans(mat4()),
	invLocalTrans(mat4()),
	localRotations(mat4()),
	localScales(mat4()),
	localTranslations(mat4()),
	localViewRotations(mat4()),
	isSelected(false),
	m_nodeId(nodeInstanceCount++)
{
	SceneNode::selectionMap[m_nodeId] = this;
}

//---------------------------------------------------------------------------------------
// Deep copy
SceneNode::SceneNode(const SceneNode & other)
	: m_nodeType(other.m_nodeType),
	  m_name(other.m_name),
	  trans(other.trans),
	  invtrans(other.invtrans),
	  localTrans(other.localTrans),
	  invLocalTrans(other.invLocalTrans),
	  localRotations(other.localRotations),
	  localScales(other.localScales),
	  localTranslations(other.localTranslations),
	  localViewRotations(other.localViewRotations)
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
void SceneNode::set_local_transform() {
	set_local_transform(localViewRotations * localTranslations * localRotations * localScales);
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
	localRotations = glm::rotate(degreesToRadians(angle), rot_axis) * localRotations;
	set_local_transform();
}

//---------------------------------------------------------------------------------------
void SceneNode::rotate(const glm::vec3& axisOfRotation, float angleInRadians) {
	localRotations = glm::rotate(angleInRadians, axisOfRotation) * localRotations;
	set_local_transform();
}

//---------------------------------------------------------------------------------------
void SceneNode::rotateAboutView(const glm::vec3& axisOfRotation, float angleInRadians) {
	localViewRotations = glm::rotate(angleInRadians, axisOfRotation) * localViewRotations;
	set_local_transform();
}

//---------------------------------------------------------------------------------------
void SceneNode::scale(const glm::vec3 & amount) {
	localScales = glm::scale(amount) * localScales;
	set_local_transform();
}

//---------------------------------------------------------------------------------------
void SceneNode::translate(const glm::vec3& amount) {
	localTranslations = glm::translate(amount) * localTranslations;
	set_local_transform();
}


//---------------------------------------------------------------------------------------
int SceneNode::totalSceneNodes() {
	return nodeInstanceCount;
}

//---------------------------------------------------------------------------------------
void SceneNode::toggleSelection() {
	isSelected = !isSelected;
}

//---------------------------------------------------------------------------------------
const glm::vec3 SceneNode::getSelectionRGBColor() const{
	return glm::vec3((m_nodeId & 0x000000FF) >> 0, 
					 (m_nodeId & 0x0000FF00) >> 8,
					 (m_nodeId & 0x00FF0000) >> 16);
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
