// Termm-Fall 2020

#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;
const float MOUSE_MOVEMENT_SLOWDOWN_FACTOR = 0.01f;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
	  m_currentUndoIndex(-1), // -1 when empty, 0 for 1 thing
	  m_lastMousePos(glm::vec2(0.0f)),
	  m_useZBuffer(true),
	  m_handleRightClickJointRotation(true),
	  m_showCircle(false),
	  m_useBackfaceCulling(false),
	  m_useFrontfaceCulling(false),
	  m_isLeftButtonPressed(false),
	  m_isMiddleButtonPressed(false),
	  m_isRightButtonPressed(false),
	  m_picking(false),
	  m_showUndoMessage(false),
	  m_showRedoMessage(false),
	  m_currentMenu(MenuGui::APPLICATION),
	  m_currentInteractionMode(InteractionMode::POSITION_ORIENTATION)

{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
// Initialize the initial local transformations. This will be used in reset()
void initNodeInitialTransformations(SceneNode *node)
{
	node->initialLocalRotations = node->localRotations;
	node->initialLocalScales = node->localScales;
	node->initialLocalTranslations = node->localTranslations;
	for (auto *child : node->children)
	{
		initNodeInitialTransformations(child);
	}
}

//----------------------------------------------------------------------------------------
void initTreeInitialTransformations(std::shared_ptr<SceneNode> root)
{
	root->initialLocalRotations = root->localRotations;
	root->initialLocalScales = root->localScales;
	root->initialLocalTranslations = root->localTranslations;
	for (auto *child : root->children)
	{
		initNodeInitialTransformations(child);
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.471, 0.318, 0.663, 1.0); // Royal purple

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	initTreeInitialTransformations(m_rootNode);

	getAllJoints();

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			// Default meshes
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj"),

			// Puppet meshes
			getAssetFilePath("beard.obj"),
			getAssetFilePath("eyeball.obj"),
			getAssetFilePath("eyebrows.obj"),
			getAssetFilePath("hairball.obj"),
			getAssetFilePath("hat_with_connectors.obj"),
			getAssetFilePath("head.obj"),
			getAssetFilePath("hoof.obj"),
			getAssetFilePath("leg_socket.obj"),
			getAssetFilePath("lips.obj"),
			getAssetFilePath("lower_leg.obj"),
			getAssetFilePath("neck.obj"),
			getAssetFilePath("tail.obj"),
			getAssetFilePath("tail_hair.obj"),
			getAssetFilePath("torso.obj"),
			getAssetFilePath("upper_leg.obj"),
			getAssetFilePath("wing.obj"),
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	updateTrackballDimensions();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();
	

	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	// float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perspective = glm::perspective(degreesToRadians(60.0f), m_aspectRatio, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(10.0f, 10.0f, 10.0f);
	m_light.rgbIntensity = vec3(1.0f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perspective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking");
		glUniform1i(location, m_picking ? 1 : 0);

		if (!m_picking)
		{
			//-- Set LightSource uniform for the scene:
			//{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
			//}

			//-- Set background light ambient intensity
			//{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.25f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
			//}
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Applies transforms recursively
 */
void applyTransformsToNode(SceneNode* node, glm::mat4 stackedTransforms)
{
	stackedTransforms *= node->get_local_transform();
	node->set_transform(stackedTransforms);

	for (auto* child : node->children)
	{
		applyTransformsToNode(child, stackedTransforms);
	}

}

//----------------------------------------------------------------------------------------
/*
 * Wrapper around applyTransformToNode above
 */
void applyTransformsToTree(std::shared_ptr<SceneNode> root)
{
	root->set_transform(root->get_local_transform());

	for (auto* child : root->children)
	{
		applyTransformsToNode(child, root->get_transform());
	}
	
}

//----------------------------------------------------------------------------------------
/*
 * Adjusts trackball dimensions
 */
void A3::updateTrackballDimensions()
{
	// We want diameter to be 1/2 of the smaller dimension
	float trackballDiameter = 0.5f * float(glm::min(m_framebufferHeight, m_framebufferWidth));
	m_trackballRadius = 0.5f * trackballDiameter;
	m_trackballCenter = glm::vec2(m_framebufferWidth / 2, m_framebufferHeight / 2);
	m_aspectRatio = float(m_framebufferWidth)/float(m_framebufferHeight);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...
	// Do the top down multiplication of all the matrices. The idea right now is that world transformations happen to rootNode, and rootNode's transformations are multiplied last
	// We have root --> cube
	//           |
	//            ----> sphere
	// Sphere and cube will have their own transforms applied before the root transform is applied. With this mind, create a recursive function called from here, using DFS
	applyTransformsToTree(m_rootNode);
	// Trackball stuff
	updateTrackballDimensions();

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
	float opacity(0.5f);

	ImGui::Begin("", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);

		if (ImGui::BeginMenuBar())
		{
			
			if (ImGui::Button("Application")) 
			{
				m_currentMenu = MenuGui::APPLICATION;
			}
			ImGui::SameLine();
			if (ImGui::Button("Edit"))
			{
				m_currentMenu = MenuGui::EDIT;
			}
			ImGui::SameLine();
			if (ImGui::Button("Options"))
			{
				m_currentMenu = MenuGui::OPTIONS;
			}
			ImGui::EndMenuBar();
		}

		ImGui::PushID(0);
		if (ImGui::RadioButton( "Position/Orientation (P)", (int*)&m_currentInteractionMode, InteractionMode::POSITION_ORIENTATION) ) {}
		if (ImGui::RadioButton( "Joints (J)", (int*)&m_currentInteractionMode, InteractionMode::JOINTS)) {}
		ImGui::PopID();

		switch(m_currentMenu) {
			case (MenuGui::APPLICATION):
				if( ImGui::Button( "Reset Position (I)" ) )
				{
					m_rootNode->localTranslations = m_rootNode->initialLocalTranslations;
					m_rootNode->set_local_transform();
				}
				if( ImGui::Button( "Reset Orientation (O)" ) )
				{
					m_rootNode->localRotations = m_rootNode->initialLocalRotations;
					m_rootNode->localViewRotations = glm::mat4();
					m_rootNode->set_local_transform();
				}
				if( ImGui::Button( "Reset Joints (S)" ) )
				{
					while (m_currentUndoIndex > -1)
					{
						for (auto &command : m_undoStack[m_currentUndoIndex])
						{
							command.undo();
						}
						m_currentUndoIndex--;
					}
					m_undoStack.clear();
					m_currentUndoFrame.clear();
				}
				if( ImGui::Button( "Reset All (A)" ) )
				{
					m_rootNode->localTranslations = m_rootNode->initialLocalTranslations;
					m_rootNode->localRotations = m_rootNode->initialLocalRotations;
					m_rootNode->localViewRotations = glm::mat4();
					m_rootNode->set_local_transform();

					while (m_currentUndoIndex > -1)
					{
						for (auto &command : m_undoStack[m_currentUndoIndex])
						{
							command.undo();
						}
						m_currentUndoIndex--;
					}
					m_undoStack.clear();
					m_currentUndoFrame.clear();
				}
				if( ImGui::Button( "Quit Application (Q)" ) )
				{
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				break;
			case (MenuGui::EDIT):
				if( ImGui::Button( "Undo (U)" ) )
				{
					if (m_currentUndoIndex == -1)
					{
						m_showUndoMessage = true;
					} else {
						m_showUndoMessage = false;
						for (auto &command : m_undoStack[m_currentUndoIndex])
						{
							command.undo();
						}
						m_currentUndoIndex--;
					}
				}
				if( ImGui::Button( "Redo (R)" ) )
				{
					if (m_currentUndoIndex == int(m_undoStack.size()) - 1)
					{
						m_showRedoMessage = true;
					}  else {
						m_showRedoMessage = false;
						m_currentUndoIndex++;
						for (auto &command : m_undoStack[m_currentUndoIndex])
						{
							command.redo();
						}
					}
				}
				break;
			case (MenuGui::OPTIONS):
				if (ImGui::Checkbox("Circle (C)", &m_showCircle)) {}
				if (ImGui::Checkbox("Z-buffer (Z)", &m_useZBuffer)) {}
				if (ImGui::Checkbox("Backface Culling (B)", &m_useBackfaceCulling)) {}
				if (ImGui::Checkbox("Frontface Culling (F)", &m_useFrontfaceCulling)) {}
				break;
			default:
				break;
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		if (m_showUndoMessage)
		{
			ImGui::Text( "You cannot UNDO any more!");
		}
		if (m_showRedoMessage)
		{
			ImGui::Text( "You cannot REDO any more!");
		}

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;


		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;
		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const SceneNode & node,
		const glm::mat4 & viewMatrix,
		bool picking
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;

		if (node.m_nodeType == NodeType::GeometryNode)
		{
			if (picking)
			{
				glm::vec3 rgb = node.getSelectionRGBColor();

				location = shader.getUniformLocation("material.kd");
				glUniform3f(location, float(rgb.r)/255.0f, float(rgb.g)/255.0f, float(rgb.b)/255.0f);
				CHECK_GL_ERRORS;
			} else {
				//-- Set Material values:
				location = shader.getUniformLocation("material.kd");
				const GeometryNode *geometryNode = static_cast<const GeometryNode *>(&node);
				vec3 kd = geometryNode->material.kd;
				glUniform3fv(location, 1, value_ptr(kd));
				CHECK_GL_ERRORS;
			}	
		}
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	// Enable z-buffer if the option is turned on
	if (m_useZBuffer)
	{
		glEnable( GL_DEPTH_TEST );
	}

	// Check for culling
	if (!(m_useBackfaceCulling || m_useFrontfaceCulling))
	{
		glDisable( GL_CULL_FACE );
	} else
	{
		glEnable( GL_CULL_FACE );
		if (m_useBackfaceCulling && m_useFrontfaceCulling)
		{
			glCullFace( GL_FRONT_AND_BACK );
		} else if (m_useBackfaceCulling)
		{
			glCullFace( GL_BACK );
		} else
		{
			glCullFace( GL_FRONT );
		}
	}

	renderSceneGraph(*m_rootNode);


	// Disable z-buffer for the trackball because it "supersedes" the scene
	glDisable( GL_DEPTH_TEST );
	if (m_showCircle)
	{
		renderArcCircle();
	}	
}


//----------------------------------------------------------------------------------------
void A3::recurseRenderNode(SceneNode &node)
{
	for (SceneNode * child : node.children) {
		updateShaderUniforms(m_shader, *child, m_view, m_picking);
		if (child->m_nodeType == NodeType::GeometryNode)
		{
			if (node.m_nodeType != NodeType::JointNode || m_currentInteractionMode != InteractionMode::JOINTS)
			{
				child->isSelected = false; // Do not allow picking unmoveable geometry nodes. Also reset colors on switching back to Position mode.
			}
			GeometryNode * geometryNode = static_cast<GeometryNode *>(child);
			if (geometryNode->isSelected)
			{
				geometryNode->material.kd = {1.0, 0.0, 0.0};
			} else {
				geometryNode->material.kd = geometryNode->originalMaterial.kd;
			}


			// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
			BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

			//-- Now render the mesh:
			m_shader.enable();
			glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
			m_shader.disable();
		
		}
		recurseRenderNode(*child);
	}
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(SceneNode & root)
{
	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	recurseRenderNode(root);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		glm::mat4 M;
		if( m_aspectRatio > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/m_aspectRatio, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*m_aspectRatio, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Apply joint rotations to each joint in the tree that has selected geometry node children.
void A3::applyJointRotations(SceneNode *node, float angleOfRotation)
{
	bool hasRotated = false; // Set to true so it doesn't rotate twice just because two of its kids are selected.
	// This should never happen anyways because every joint node has at max 1 child, but whatever
	for (auto *child : node->children)
	{
		// Redundant check (since only geometry node children of joint nodes can be selected), but just in case
		if (node->m_nodeType == NodeType::JointNode && child->isSelected && child->m_nodeType == NodeType::GeometryNode && !hasRotated) 
		{
			JointNode *joint = static_cast<JointNode *>(node);
			if (joint->m_name == "head_joint" && m_isRightButtonPressed)
			{
				// Handle this rotation in the right click
				continue;
			}
			if (joint->m_name == "left_wing_joint") // lol, just looks cooler that way
			{
				joint->rotate_joint(-angleOfRotation);
			} else {
				joint->rotate_joint(angleOfRotation);
			}
			hasRotated = true;
		}
		applyJointRotations(child, angleOfRotation);
	}
}

//----------------------------------------------------------------------------------------
// Wrapper around applyJointRotations so we can pass the root sharedNode
void A3::applyJointRotationsToTree(std::shared_ptr<SceneNode> root, float angleOfRotation)
{
	for (auto *child : root->children)
	{
		applyJointRotations(child, angleOfRotation);
	}
}

//----------------------------------------------------------------------------------------
// Wrapper around applyJointRotationsToHead so we can pass the root sharedNode
void A3::applyJointRotationsToHead(SceneNode* node, float angleOfRotation)
{
	for (auto *child : node->children)
	{
		if (node->m_name == "head_joint" && node->m_nodeType == NodeType::JointNode && child->isSelected)
		{
			JointNode *joint = static_cast<JointNode *>(node);
			joint->rotate_joint(angleOfRotation);
			return;
		} else {
			applyJointRotationsToHead(child, angleOfRotation);
		}
		
	}
}

//----------------------------------------------------------------------------------------
// Wrapper around applyJointRotationsToHead so we can pass the root sharedNode
void A3::wrapperApplyJointRotationsToHead(std::shared_ptr<SceneNode> root, float angleOfRotation)
{
	for (auto *child : root->children)
	{
		applyJointRotationsToHead(child, angleOfRotation);
	}
}

//----------------------------------------------------------------------------------------
void recurseGetAllJoints(SceneNode *node, std::vector<JointNode *>& outVec)
{
	for (auto *child : node->children)
	{
		if (node->m_nodeType == NodeType::JointNode)
		{
			JointNode *joint = static_cast<JointNode *>(node);
			outVec.push_back(joint);
		}
		recurseGetAllJoints(child, outVec);
	}
}

//----------------------------------------------------------------------------------------
void A3::getAllJoints()
{
	for (auto *child : m_rootNode->children)
	{
		recurseGetAllJoints(child, m_allJointNodes);
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) 
{
	bool eventHandled(false);
	
	float differenceX = xPos - m_lastMousePos.x;
	float differenceY = yPos - m_lastMousePos.y;

	// Fill in with event handling code...
	switch(m_currentInteractionMode)
	{
		case InteractionMode::POSITION_ORIENTATION:
			// Translate whole model on x and y axes
			if (m_isLeftButtonPressed)
			{
				glm::vec3 translateVec{differenceX * MOUSE_MOVEMENT_SLOWDOWN_FACTOR, -differenceY * MOUSE_MOVEMENT_SLOWDOWN_FACTOR, 0};
				m_rootNode->translate(translateVec);
				eventHandled = true;
			}
			// Translate whole model on z axis
			if (m_isMiddleButtonPressed)
			{
				glm::vec3 translateVec{0, 0, differenceY * MOUSE_MOVEMENT_SLOWDOWN_FACTOR};
				m_rootNode->translate(translateVec);
				eventHandled = true;
			}
			// Trackball rotation
			if (m_isRightButtonPressed)
			{
				glm::vec2 lastCursorPosVecOnTrackball = m_lastMousePos - m_trackballCenter;
				glm::vec2 currentCursorPosVecOnTrackball = glm::vec2(xPos, yPos) - m_trackballCenter;

				/* 
				* Equation of 3d sphere is x^2 + y^2 + z^2 = radius^2, so
				* z location is usually sqrt(radius^2 - x^2 - y^2) but if x^2 + y^2 > radius^2 then we should rotate around z-axis
				* If we normalize by radius, we get (x/radius)^2 + (y/radius)^2 <= 1 for the check
				* Referenced from https://www.xarg.org/2021/07/trackball-rotation-using-quaternions/
				*/
				lastCursorPosVecOnTrackball /= m_trackballRadius;
				currentCursorPosVecOnTrackball /= m_trackballRadius;
				float lastPosXSquaredPlusYSquared = (lastCursorPosVecOnTrackball.x * lastCursorPosVecOnTrackball.x) + (lastCursorPosVecOnTrackball.y * lastCursorPosVecOnTrackball.y);
				float currentPosXSquaredPlusYSquared = (currentCursorPosVecOnTrackball.x * currentCursorPosVecOnTrackball.x) + (currentCursorPosVecOnTrackball.y * currentCursorPosVecOnTrackball.y);
				
				if (currentPosXSquaredPlusYSquared <= 1.0f && lastPosXSquaredPlusYSquared <= 1.0f)
				{
					float currentPosZ = glm::sqrt(1.0f - currentPosXSquaredPlusYSquared);
					float lastPosZ = glm::sqrt(1.0f - lastPosXSquaredPlusYSquared);
					glm::vec3 axisOfRotation = glm::cross(glm::vec3(lastCursorPosVecOnTrackball, lastPosZ), glm::vec3(currentCursorPosVecOnTrackball, currentPosZ));
					axisOfRotation.x = -axisOfRotation.x;
					float angleOfRotation = glm::length(axisOfRotation);
					if (0.000001f < angleOfRotation || -0.000001f > angleOfRotation)
					{
						m_rootNode->rotate(axisOfRotation/angleOfRotation, angleOfRotation);
					}
					
				} else 
				{
					// Rotate around view z
					// zCross is the z component of cross product between last and current posn.
					// if zCross > 0, then current is clockwise from last, and we need a negative angle rotation
					// if zCross < 0, then we need positive angle rotation.

					float cosAngleOfRotation = (glm::dot(lastCursorPosVecOnTrackball, currentCursorPosVecOnTrackball)) / 
						(glm::length(lastCursorPosVecOnTrackball) * glm::length(currentCursorPosVecOnTrackball));
					float angleOfRotation = glm::acos(glm::clamp(cosAngleOfRotation, -1.0f, 1.0f));

					float zCross = lastCursorPosVecOnTrackball.x * currentCursorPosVecOnTrackball.y - lastCursorPosVecOnTrackball.y * currentCursorPosVecOnTrackball.x;
					angleOfRotation = (zCross < 0) ? angleOfRotation : -angleOfRotation;
					m_rootNode->rotateAboutView(glm::vec3(0,0,1), angleOfRotation); 
				}
				eventHandled = true;
			}
			break;
		case InteractionMode::JOINTS:
			// Change Angle of Selected Joints
			if (m_isMiddleButtonPressed)
			{
				float angleOfRotation = -differenceY;
				// Recurse through tree, see if child is selected - if so, (redundantly?)
				// check if current node is a joint node, and if so, apply rotation.
				applyJointRotationsToTree(m_rootNode, angleOfRotation);
				eventHandled = true;
			}
			// Rotate head left to right ONLY if it's selected
			if (m_isRightButtonPressed)
			{
				float angleOfRotation = -differenceY;
				wrapperApplyJointRotationsToHead(m_rootNode, angleOfRotation);
				eventHandled = true;
			}
			break;
		default:
			break;
	}

	m_lastMousePos = {float(xPos), float(yPos)};
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (actions == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			m_isLeftButtonPressed = true;
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			m_isMiddleButtonPressed = true;
			if (m_currentInteractionMode == InteractionMode::JOINTS)
			{
				std::vector<JointRotationCommand> commands;
				for (auto *node : m_allJointNodes)
				{
					JointRotationCommand newCommand = JointRotationCommand(node);
					commands.push_back(newCommand);
				}
				m_currentUndoFrame = commands;
			}
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			m_isRightButtonPressed = true;
			if (m_currentInteractionMode == InteractionMode::JOINTS && !m_isMiddleButtonPressed)
			{
				m_handleRightClickJointRotation = true;
				std::vector<JointRotationCommand> commands;
				for (auto *node : m_allJointNodes)
				{
					JointRotationCommand newCommand = JointRotationCommand(node);
					commands.push_back(newCommand);
				}
				m_currentUndoFrame = commands;
			}  else if (m_currentInteractionMode == InteractionMode::JOINTS && m_isMiddleButtonPressed)
			{
				m_handleRightClickJointRotation = false;
			}
			eventHandled = true;
		}
	}
	if (actions == GLFW_RELEASE)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			m_isLeftButtonPressed = false;
			// Picking
			if (m_currentInteractionMode == InteractionMode::JOINTS)
			{
				double xPos, yPos;
				glfwGetCursorPos(m_window, &xPos, &yPos);

				m_picking = true;

				uploadCommonSceneUniforms();
				glClearColor(1.0, 1.0, 1.0, 1.0 );
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
				glClearColor(0.471, 0.318, 0.663, 1.0); // Royal purple

				draw();

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				GLubyte pixelData[4] = {0, 0, 0, 0};

				glReadBuffer( GL_BACK );

				xPos *= double(m_framebufferWidth) / double(m_windowWidth);
				yPos = m_windowHeight - yPos;
				yPos *= double(m_framebufferHeight) / double(m_windowHeight);

				glReadPixels( int(xPos), int(yPos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixelData );
				CHECK_GL_ERRORS;

				int pickedId = pixelData[0] + (pixelData[1] << 8) + (pixelData[2] << 16);

				if (pickedId < SceneNode::totalSceneNodes())
				{
					SceneNode::selectionMap[pickedId]->toggleSelection();
				}

				m_picking = false;

				CHECK_GL_ERRORS;
			}
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			m_isMiddleButtonPressed = false;
			if (m_currentInteractionMode == InteractionMode::JOINTS && m_handleRightClickJointRotation)
			{
				for (auto &command : m_currentUndoFrame)
				{
					command.commit(command.m_node->m_currentAngle);
				}
				if (m_undoStack.size() > m_currentUndoIndex + 1)
				{
					m_undoStack.erase(m_undoStack.begin() + m_currentUndoIndex + 1, m_undoStack.end());
				}
				m_currentUndoIndex++;
				m_undoStack.push_back(m_currentUndoFrame);
			}
			
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			m_isRightButtonPressed = false;
			if (m_currentInteractionMode == InteractionMode::JOINTS)
			{
				for (auto &command : m_currentUndoFrame)
				{
					command.commit(command.m_node->m_currentAngle);
				}
				if (m_undoStack.size() > m_currentUndoIndex + 1)
				{
					m_undoStack.erase(m_undoStack.begin() + m_currentUndoIndex + 1, m_undoStack.end());
				}
				m_currentUndoIndex++;
				m_undoStack.push_back(m_currentUndoFrame);
			}
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_Q)
		{
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}
		if (key == GLFW_KEY_M)
		{
			show_gui = !show_gui;
			eventHandled = true;
		}
		if (key == GLFW_KEY_Z)
		{
			m_useZBuffer = !m_useZBuffer;
			eventHandled = true;
		}
		if (key == GLFW_KEY_B)
		{
			m_useBackfaceCulling = !m_useBackfaceCulling;
			eventHandled = true;
		}
		if (key == GLFW_KEY_F)
		{
			m_useFrontfaceCulling = !m_useFrontfaceCulling;
			eventHandled = true;
		}
		if (key == GLFW_KEY_C)
		{
			m_showCircle = !m_showCircle;
			eventHandled = true;
		}
		if (key == GLFW_KEY_P)
		{
			m_currentInteractionMode = InteractionMode::POSITION_ORIENTATION;
			eventHandled = true;
		}
		if (key == GLFW_KEY_J)
		{
			m_currentInteractionMode = InteractionMode::JOINTS;
			eventHandled = true;
		}
		if (key == GLFW_KEY_I)
		{
			m_rootNode->localTranslations = m_rootNode->initialLocalTranslations;
			m_rootNode->set_local_transform();
		}
		if (key == GLFW_KEY_O)
		{
			m_rootNode->localRotations = m_rootNode->initialLocalRotations;
			m_rootNode->localViewRotations = glm::mat4();
			m_rootNode->set_local_transform();
		}
		if (key == GLFW_KEY_A)
		{
			m_rootNode->localTranslations = m_rootNode->initialLocalTranslations;
			m_rootNode->localRotations = m_rootNode->initialLocalRotations;
			m_rootNode->localViewRotations = glm::mat4();
			m_rootNode->set_local_transform();

			while (m_currentUndoIndex > -1)
			{
				for (auto &command : m_undoStack[m_currentUndoIndex])
				{
					command.undo();
				}
				m_currentUndoIndex--;
			}
			m_undoStack.clear();
			m_currentUndoFrame.clear();
		}
		if (key == GLFW_KEY_S)
		{
			while (m_currentUndoIndex > -1)
			{
				for (auto &command : m_undoStack[m_currentUndoIndex])
				{
					command.undo();
				}
				m_currentUndoIndex--;
			}
			m_undoStack.clear();
			m_currentUndoFrame.clear();
		}
		if (key == GLFW_KEY_U)
		{
			if (m_currentUndoIndex == -1)
			{
				m_showUndoMessage = true;
			} else {
				m_showUndoMessage = false;
				for (auto &command : m_undoStack[m_currentUndoIndex])
				{
					command.undo();
				}
				m_currentUndoIndex--;
			}
		}
		if (key == GLFW_KEY_R)
		{
			if (m_currentUndoIndex == int(m_undoStack.size()) - 1)
			{
				m_showRedoMessage = true;
			}  else {
				m_showRedoMessage = false;
				m_currentUndoIndex++;
				for (auto &command : m_undoStack[m_currentUndoIndex])
				{
					command.redo();
				}
			}
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
