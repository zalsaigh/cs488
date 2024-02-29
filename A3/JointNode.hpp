// Termm-Fall 2020

#pragma once

#include "SceneNode.hpp"

class JointNode : public SceneNode {
public:
	JointNode(const std::string & name);
	virtual ~JointNode();

	void set_joint_x(double min, double init, double max);
	void set_joint_y(double min, double init, double max);
	void set_joint_z(double min, double init, double max);
	void rotate_joint(float angle);

	struct JointRange {
		double min, init, max;
	};


	JointRange m_joint_x, m_joint_y, m_joint_z;
	float m_currentAngle;
	char m_axis;
};

class JointRotationCommand {
public:
	JointRotationCommand(JointNode *node);

	void commit(float angle);
	void undo();
	void redo();

	JointNode* m_node;
	float m_startAngle;
	float m_endAngle;
};
