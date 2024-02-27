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
	  m_currentUndoIndex(0),
	  m_lastMousePos(glm::vec2(0.0f)),
	  m_useZBuffer(true),
	  m_useBackfaceCulling(false),
	  m_useFrontfaceCulling(false),
	  m_isLeftButtonPressed(false),
	  m_isMiddleButtonPressed(false),
	  m_isRightButtonPressed(false),
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
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	// glClearColor(0.85, 0.85, 0.85, 1.0);
	glClearColor(0.471, 0.318, 0.663, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

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


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.25f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
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
				if( ImGui::Button( "Quit Application (Q)" ) )
				{
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				break;
			case (MenuGui::EDIT):
				break;
			case (MenuGui::OPTIONS):
				if (ImGui::Checkbox("Z-buffer (Z)", &m_useZBuffer)) {}
				if (ImGui::Checkbox("Backface Culling (B)", &m_useBackfaceCulling)) {}
				if (ImGui::Checkbox("Frontface Culling (F)", &m_useFrontfaceCulling)) {}
				break;
			default:
				break;
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

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

		if (node.m_nodeType == NodeType::GeometryNode)
		{
			//-- Set Material values:
			location = shader.getUniformLocation("material.kd");
			const GeometryNode *geometryNode = static_cast<const GeometryNode *>(&node);
			vec3 kd = geometryNode->material.kd;
			glUniform3fv(location, 1, value_ptr(kd));
			CHECK_GL_ERRORS;
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
	if (m_useZBuffer) {
		glEnable( GL_DEPTH_TEST );
	}

	// Check for culling
	if (!(m_useBackfaceCulling || m_useFrontfaceCulling)) {
		glDisable( GL_CULL_FACE );
	} else {
		glEnable( GL_CULL_FACE );
		if (m_useBackfaceCulling && m_useFrontfaceCulling) {
			glCullFace( GL_FRONT_AND_BACK );
		} else if (m_useBackfaceCulling) {
			glCullFace( GL_BACK );
		} else {
			glCullFace( GL_FRONT );
		}
	}

	renderSceneGraph(*m_rootNode);


	// Disable z-buffer for the trackball because it "supersedes" the scene
	glDisable( GL_DEPTH_TEST );
	renderArcCircle();
}


//----------------------------------------------------------------------------------------
void A3::recurseRenderNode(const SceneNode &node)
{
	for (const SceneNode * child : node.children) {
		updateShaderUniforms(m_shader, *child, m_view);
		if (child->m_nodeType == NodeType::GeometryNode)
		{
			const GeometryNode * geometryNode = static_cast<const GeometryNode *>(child);


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
void A3::renderSceneGraph(const SceneNode & root)
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
) {
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
				* Referenced from https://www.xarg.org/2021/07/trackball-rotation-using-quaternions/
				*/
				lastCursorPosVecOnTrackball /= m_trackballRadius;
				currentCursorPosVecOnTrackball /= m_trackballRadius;
				float lastPosXSquaredPlusYSquared = (lastCursorPosVecOnTrackball.x * lastCursorPosVecOnTrackball.x) + (lastCursorPosVecOnTrackball.y * lastCursorPosVecOnTrackball.y);
				float currentPosXSquaredPlusYSquared = (currentCursorPosVecOnTrackball.x * currentCursorPosVecOnTrackball.x) + (currentCursorPosVecOnTrackball.y * currentCursorPosVecOnTrackball.y);
				float radiusSquared = m_trackballRadius * m_trackballRadius;

				// float cosAngleOfRotation = (glm::dot(lastCursorPosVecOnTrackball, currentCursorPosVecOnTrackball)) / 
				// 		(glm::length(lastCursorPosVecOnTrackball) * glm::length(currentCursorPosVecOnTrackball));
				// float angleOfRotation = glm::acos(glm::clamp(cosAngleOfRotation, -1.0f, 1.0f));
				
				if (currentPosXSquaredPlusYSquared <= 1.0f && lastPosXSquaredPlusYSquared <= 1.0f)
				// if (currentPosXSquaredPlusYSquared <= radiusSquared && lastPosXSquaredPlusYSquared <= radiusSquared)
				{
					// float currentPosZ = glm::sqrt(radiusSquared - currentPosXSquaredPlusYSquared);
					// float lastPosZ = glm::sqrt(radiusSquared - lastPosXSquaredPlusYSquared);
					float currentPosZ = glm::sqrt(1.0f - currentPosXSquaredPlusYSquared);
					float lastPosZ = glm::sqrt(1.0f - lastPosXSquaredPlusYSquared);
					// glm::vec3 axisOfRotation = glm::normalize(glm::cross(glm::vec3(lastCursorPosVecOnTrackball, lastPosZ), glm::vec3(currentCursorPosVecOnTrackball, currentPosZ)));
					glm::vec3 axisOfRotation = glm::cross(glm::vec3(lastCursorPosVecOnTrackball, lastPosZ), glm::vec3(currentCursorPosVecOnTrackball, currentPosZ));
					axisOfRotation.x = -axisOfRotation.x;
					float angleOfRotation = glm::length(axisOfRotation);
					if (0.000001f < angleOfRotation || -0.000001f > angleOfRotation)
					{
						m_rootNode->rotate(axisOfRotation/angleOfRotation, angleOfRotation);
						std::cout << "Axis: " << axisOfRotation << ", Angle: " << angleOfRotation << "\n";
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
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			m_isRightButtonPressed = true;
			eventHandled = true;
		}
	}
	if (actions == GLFW_RELEASE)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			m_isLeftButtonPressed = false;
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			m_isMiddleButtonPressed = false;
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			m_isRightButtonPressed = false;
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
	}
	// Fill in with event handling code...

	return eventHandled;
}
