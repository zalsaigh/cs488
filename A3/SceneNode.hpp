// Termm-Fall 2020

#pragma once

#include "Material.hpp"

#include <glm/glm.hpp>

#include <list>
#include <string>
#include <iostream>
#include <unordered_map>

enum class NodeType {
	SceneNode,
	GeometryNode,
	JointNode
};

class SceneNode {
public:
    SceneNode(const std::string & name);

	SceneNode(const SceneNode & other);

    virtual ~SceneNode();
    
	static int totalSceneNodes();
    
    const glm::mat4& get_transform() const;
    const glm::mat4& get_inverse() const;

    const glm::mat4& get_local_transform() const;
    const glm::mat4& get_inverse_local_transform() const;
    
    void set_transform(const glm::mat4& m);
    void set_local_transform(const glm::mat4& m);
    void set_local_transform();
    
    void add_child(SceneNode* child);
    
    void remove_child(SceneNode* child);

    //-- Selections
    void toggleSelection();
    const glm::vec3 getSelectionRGBColor() const;

	//-- Transformations:
    void rotate(char axis, float angle);
    void rotate(const glm::vec3& axisOfRotation, float angle);
    void rotateAboutView(const glm::vec3& axisOfRotation, float angle);
    void scale(const glm::vec3& amount);
    void translate(const glm::vec3& amount);

	friend std::ostream & operator << (std::ostream & os, const SceneNode & node);

	bool isSelected;

    // Initial Transformation Data
    glm::mat4 initialLocalRotations;
    glm::mat4 initialLocalScales;
    glm::mat4 initialLocalTranslations;
    
    // Transformation Data
    glm::mat4 localTrans; // Transformations that are local to just this node
    glm::mat4 invLocalTrans;
    glm::mat4 trans;
    glm::mat4 invtrans;

    glm::mat4 localRotations;
    glm::mat4 localScales;
    glm::mat4 localTranslations;
    glm::mat4 localViewRotations;
    
    std::list<SceneNode*> children;

	NodeType m_nodeType;
	std::string m_name;
	unsigned int m_nodeId;

    static std::unordered_map<unsigned int, SceneNode*> selectionMap; 

private:
	// The number of SceneNode instances.
	static unsigned int nodeInstanceCount;
};
