// Termm-Fall 2020

#include "JointNode.hpp"

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;
	m_currentAngle = 0.0f;
	m_axis = 'x';
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	if (min < max)
	{
		m_axis = 'y';
	}
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_z(double min, double init, double max) {
	if (min < max)
	{
		m_axis = 'z';
	}
	m_joint_z.min = min;
	m_joint_z.init = init;
	m_joint_z.max = max;
}
//---------------------------------------------------------------------------------------
void JointNode::rotate_joint(float angle) {
	switch (m_axis) {
		case 'x':
			if (angle >= 0)
			{
				angle = glm::min(float(m_joint_x.max)  - m_currentAngle, angle);
			} else {
				angle = glm::max(float(m_joint_x.min)  - m_currentAngle, angle);
			}
			break;
		case 'y':
			if (angle >= 0)
			{
				angle = glm::min(float(m_joint_y.max) - m_currentAngle, angle);
			} else {
				angle = glm::max(float(m_joint_y.min)  - m_currentAngle, angle);
			}
			break;
		case 'z':
			if (angle >= 0)
			{
				angle = glm::min(float(m_joint_z.max) - m_currentAngle, angle);
			} else {
				angle = glm::max(float(m_joint_z.min)  - m_currentAngle, angle);
			}
			break;
		default:
			return; // Do not rotate on some weird axis
	}
	if (0.000001f > angle && angle > -0.000001f) return;
	rotate(m_axis, angle);
	m_currentAngle += angle;
}

//---------------------------------------------------------------------------------------
JointRotationCommand::JointRotationCommand(JointNode *node)
	: m_node(node), m_startAngle(0.0f), m_endAngle(0.0f)
	  
{
	m_startAngle = m_node->m_currentAngle;
}

//---------------------------------------------------------------------------------------
void JointRotationCommand::commit(float angle)
{
	m_endAngle = angle;
}

//---------------------------------------------------------------------------------------
void JointRotationCommand::undo()
{
	m_node->rotate_joint(-m_node->m_currentAngle);
	m_node->rotate_joint(m_startAngle);
}

//---------------------------------------------------------------------------------------
void JointRotationCommand::redo()
{
	m_node->rotate_joint(-m_node->m_currentAngle);
	m_node->rotate_joint(m_endAngle);
}
