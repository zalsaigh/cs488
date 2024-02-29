// Termm-Fall 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "JointNode.hpp"

#include <glm/glm.hpp>
#include <memory>

struct LightSource
{
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

enum MenuGui
{
	APPLICATION,
	EDIT,
	OPTIONS
};

enum InteractionMode
{
	POSITION_ORIENTATION,
	JOINTS
};


class A3 : public CS488Window
{
public:
	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void recurseRenderNode(SceneNode &node);
	void renderSceneGraph(SceneNode &node);
	void renderArcCircle();

	//-- Trackball methods
	void updateTrackballDimensions();

	//-- Joint Rotations
	void applyJointRotations(SceneNode *node, float angleOfRotation);
	void applyJointRotationsToTree(std::shared_ptr<SceneNode> root, float angleOfRotation); // Wrapper for root node
	void applyJointRotationsToHead(SceneNode *node, float angleOfRotation);
	void wrapperApplyJointRotationsToHead(std::shared_ptr<SceneNode> root, float angleOfRotation);

	glm::mat4 m_perspective;
	glm::mat4 m_view;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;

	// Booleans for user options on the GUI or keyboard
	bool m_useZBuffer;
	bool m_useBackfaceCulling;
	bool m_useFrontfaceCulling;
	bool m_showCircle;

	// Booleans to show the undo and redo too far messages
	bool m_showUndoMessage;
	bool m_showRedoMessage;

	// Variable for which GUI to show
	MenuGui m_currentMenu;

	// Variable for current interaction mode
	InteractionMode m_currentInteractionMode;

	// Booleans for mouse input
	bool m_isLeftButtonPressed;
	bool m_isMiddleButtonPressed;
	bool m_isRightButtonPressed;

	// Tracking mouse movement
	glm::vec2 m_lastMousePos;

	// Trackball-related variables
	float m_aspectRatio;
	float m_trackballRadius;
	glm::vec2 m_trackballCenter;

	// For undo/redo stack
	void getAllJoints();

	std::vector<JointNode *> m_allJointNodes;
	std::vector<std::vector<JointRotationCommand>> m_undoStack;
	std::vector<JointRotationCommand> m_currentUndoFrame;
	int m_currentUndoIndex;
	bool m_handleRightClickJointRotation;

	// Picking
	bool m_picking;
};
