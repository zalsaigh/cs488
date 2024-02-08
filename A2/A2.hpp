// Termm--Fall 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;

enum InteractionMode {
	ROTATE_VIEW,
	TRANSLATE_VIEW,
	PERSPECTIVE,
	ROTATE_MODEL,
	TRANSLATE_MODEL,
	SCALE_MODEL,
	VIEWPORT
};


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class Transform {
public:
	Transform();

	glm::mat4 S; // Scale
	glm::mat4 R; // Rotate
	glm::mat4 T; // Translate

	bool m_is_viewing_transform; // False for model transform, true for viewing.

	void rotateX(float theta);
	void rotateY(float theta);
	void rotateZ(float theta);
	void scale(float s_x, float s_y, float s_z);
	void translate(float delta_x, float delta_y, float delta_z);

	glm::mat4 getTransform();
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	void reset();

	glm::vec4 getCenterOfCube(std::vector<glm::vec4> cube_points);
	glm::vec4 getCenterOfFrontFaceOfCube(std::vector<glm::vec4> cube_points);
	glm::vec4 getCenterOfRightFaceOfCube(std::vector<glm::vec4> cube_points);
	glm::vec4 getCenterOfTopFaceOfCube(std::vector<glm::vec4> cube_points);

	void drawCubeGnomon(std::vector<glm::vec4> cube_points);
	void drawWorldGnomon();
	void drawViewportDrag(glm::vec2 top_left, glm::vec2 top_right, glm::vec2 bot_left, glm::vec2 bot_right);

	float mapXPointToViewport(float x);
	float mapYPointToViewport(float y);

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

	int m_currentInteractionMode; // Casts to enum, is an int because RadioButton can only take int addresses
	float m_near_plane;
	float m_far_plane;

	double m_last_xPos;

	// glm::mat4 m_proj; // Projections

	Transform m_model_transform;
	Transform m_view_transform;

	std::vector<glm::vec4> m_cube_points; // Cube positioning relative to world coord sys
	glm::mat4 m_camera;

	float m_mouse_move_slowdown_factor;
	glm::mat4 m_gnomon_scaler;

	float m_viewport_start_x;
	float m_viewport_start_y;

	float m_viewport_end_x;
	float m_viewport_end_y;

	float m_viewport_left_boundary;
	float m_viewport_right_boundary;
	float m_viewport_bottom_boundary;
	float m_viewport_top_boundary;
};
