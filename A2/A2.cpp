// Term--Winter 2024

#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f)),
	  m_currentInteractionMode(InteractionMode::ROTATE_MODEL),
	  m_near_plane(1.0f),
	  m_far_plane(10.0f),
	  m_last_xPos(0),
	  m_mouse_move_slowdown_factor(0.01f)
{

}

//----------------------------------------------------------------------------------------
// Constructor
Transform::Transform()
{
	S = glm::mat4(1.0f);
	R = glm::mat4(1.0f);
	T = glm::mat4(1.0f);
	m_is_viewing_transform = false;
}
//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

	m_camera = glm::lookAt(
		glm::vec3( 0.0f, 0.0f, 5.0f ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	m_view_transform.m_is_viewing_transform = true;
	
	// m_proj = glm::perspective(
	// 	glm::radians( 30.0f ),
	// 	float( m_framebufferWidth ) / float( m_framebufferHeight ),
	// 	1.0f, 1000.0f );

	m_cube_points.push_back(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f)); // Bottom left front vertex (from camera pov)
	m_cube_points.push_back(glm::vec4( 1.0f, -1.0f, 1.0f, 1.0f)); // Bottom right front
	m_cube_points.push_back(glm::vec4( 1.0f,  1.0f, 1.0f, 1.0f)); // Top right front
	m_cube_points.push_back(glm::vec4(-1.0f,  1.0f, 1.0f, 1.0f)); // Top left front

	m_cube_points.push_back(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f)); // Bottom left back vertex
	m_cube_points.push_back(glm::vec4( 1.0f, -1.0f, -1.0f, 1.0f)); // Bottom right back
	m_cube_points.push_back(glm::vec4( 1.0f,  1.0f, -1.0f, 1.0f)); // Top right back
	m_cube_points.push_back(glm::vec4(-1.0f,  1.0f, -1.0f, 1.0f)); // Top left back

	m_gnomon_scaler = glm::mat4(0.3f);
	m_gnomon_scaler[3][3] = 1.0f; // The 1/0 coord that defines this as a point and not a vector

	m_viewport_start_x = 0.0f;
	m_viewport_end_x = 0.0f;
	m_viewport_start_y = 0.0f;
	m_viewport_end_y = 0.0f;

	m_viewport_left_boundary = floor(0.05f * m_framebufferWidth);
	m_viewport_right_boundary = floor(0.95f * m_framebufferWidth);
	m_viewport_bottom_boundary = floor(0.05f * m_framebufferHeight);
	m_viewport_top_boundary = floor(0.95f * m_framebufferHeight);
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//---------------------------------------------------------------------------------------- Spring 2020
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & V0,   // Line Start (NDC coordinate)
		const glm::vec2 & V1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
void A2::drawCubeGnomon(std::vector<glm::vec4> cube_points)
{
	glm::vec4 center_of_cube = getCenterOfCube(cube_points);
	glm::vec4 front_face_of_cube = getCenterOfFrontFaceOfCube(cube_points);
	glm::vec4 right_face_of_cube = getCenterOfRightFaceOfCube(cube_points);
	glm::vec4 top_face_of_cube = getCenterOfTopFaceOfCube(cube_points);
	
	glm::vec4 x_gnomon_point = center_of_cube + (m_gnomon_scaler * glm::normalize(right_face_of_cube - center_of_cube));
	glm::vec4 y_gnomon_point = center_of_cube + (m_gnomon_scaler * glm::normalize(top_face_of_cube - center_of_cube));
	glm::vec4 z_gnomon_point = center_of_cube + (m_gnomon_scaler * glm::normalize(front_face_of_cube - center_of_cube));

	setLineColour(vec3(1.0f, 0.0f, 0.0f)); // Red for x axis
	drawLine(
		vec2(mapXPointToViewport(center_of_cube.x), mapYPointToViewport(center_of_cube.y)),
		vec2(mapXPointToViewport(x_gnomon_point.x), mapYPointToViewport(x_gnomon_point.y))
	);
	setLineColour(vec3(0.0f, 1.0f, 0.0f)); // Green for y axis
	drawLine(
		vec2(mapXPointToViewport(center_of_cube.x), mapYPointToViewport(center_of_cube.y)),
		vec2(mapXPointToViewport(y_gnomon_point.x), mapYPointToViewport(y_gnomon_point.y))
	);
	setLineColour(vec3(0.0f, 0.0f, 1.0f)); // Blue for z axis
	drawLine(
		vec2(mapXPointToViewport(center_of_cube.x), mapYPointToViewport(center_of_cube.y)),
		vec2(mapXPointToViewport(z_gnomon_point.x), mapYPointToViewport(z_gnomon_point.y))
	);
}

//----------------------------------------------------------------------------------------
void A2::drawWorldGnomon()
{
	glm::vec4 world_origin = m_view_transform.getTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 x_gnomon_point = m_view_transform.getTransform() * m_gnomon_scaler * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 y_gnomon_point = m_view_transform.getTransform() * m_gnomon_scaler * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 z_gnomon_point = m_view_transform.getTransform() * m_gnomon_scaler * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	setLineColour(vec3(1.0f, 0.0f, 0.0f)); // Red for x axis
	drawLine(
		vec2(mapXPointToViewport(world_origin.x), mapYPointToViewport(world_origin.y)),
		vec2(mapXPointToViewport(x_gnomon_point.x), mapYPointToViewport(x_gnomon_point.y))
	);
	setLineColour(vec3(0.0f, 1.0f, 0.0f)); // Green for y axis
	drawLine(
		vec2(mapXPointToViewport(world_origin.x), mapYPointToViewport(world_origin.y)),
		vec2(mapXPointToViewport(y_gnomon_point.x), mapYPointToViewport(y_gnomon_point.y))
	);
	setLineColour(vec3(0.0f, 0.0f, 1.0f)); // Blue for z axis
	drawLine(
		vec2(mapXPointToViewport(world_origin.x), mapYPointToViewport(world_origin.y)),
		vec2(mapXPointToViewport(z_gnomon_point.x), mapYPointToViewport(z_gnomon_point.y))
	);
}

//----------------------------------------------------------------------------------------
void A2::drawViewportDrag(glm::vec2 top_left, glm::vec2 top_right, glm::vec2 bot_left, glm::vec2 bot_right)
{
	setLineColour(vec3(1.0f, 1.0f, 1.0f)); // White drag boundary
	drawLine(bot_left, bot_right);
	drawLine(bot_right, top_right);
	drawLine(top_right, top_left);
	drawLine(top_left, bot_left);
}

//----------------------------------------------------------------------------------------
float A2::mapXPointToViewport(float x)
{
	// Followed textbook, but it turns out adding by the left boundary adds by a number from  0 - 768, so I needed to get that on a scale from -1 to 1 (window coords)
	// However, after that, I noticed that the bottom right corner of my cube was spawning in the middle, so I needed to further move the cube entirely by the diagonal defined by
	// 1/2 a movement to the right (so half the distance from L to R) and then half a movement down (half the distance from B to T), resulting in the below addition
	return float((m_viewport_right_boundary - m_viewport_left_boundary) / (m_framebufferWidth))*x + ((m_viewport_left_boundary + m_viewport_right_boundary) / (m_framebufferWidth / 2)) - 2;
}

//----------------------------------------------------------------------------------------
float A2::mapYPointToViewport(float y)
{
	return float((m_viewport_top_boundary - m_viewport_bottom_boundary) / (m_framebufferHeight))*y + 2 - ((m_viewport_bottom_boundary + m_viewport_top_boundary) / (m_framebufferHeight / 2));
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// P = PROJ * VIEW * MODEL * position
	// Here, VIEW is split into VIEW_TRANSFORMATIONS * CAMERA, simply because the lookAt function returns one matrix
	// and I use it to initialize my camera. Thus, m_camera technically never changes after initialization and only
	// the view transformation matrix does.
	glm::mat4 transform = m_view_transform.getTransform() * m_camera * m_model_transform.getTransform();

	std::vector<glm::vec4> converted_points;

	for (auto cube_point : m_cube_points) {
		converted_points.push_back(transform * cube_point);
	}

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	// Draw front square:
	setLineColour(vec3(1.0f, 0.7f, 0.8f));

	drawLine(
		vec2(mapXPointToViewport(converted_points[0].x), mapYPointToViewport(converted_points[0].y)),
		vec2(mapXPointToViewport(converted_points[1].x), mapYPointToViewport(converted_points[1].y))
	);

	drawLine(
		vec2(mapXPointToViewport(converted_points[1].x), mapYPointToViewport(converted_points[1].y)),
		vec2(mapXPointToViewport(converted_points[2].x),mapYPointToViewport(converted_points[2].y))
	);

	drawLine(
		vec2(mapXPointToViewport(converted_points[2].x), mapYPointToViewport(converted_points[2].y)),
		vec2(mapXPointToViewport(converted_points[3].x), mapYPointToViewport(converted_points[3].y))
	);

	drawLine(
		vec2(mapXPointToViewport(converted_points[3].x), mapYPointToViewport(converted_points[3].y)),
		vec2(mapXPointToViewport(converted_points[0].x), mapYPointToViewport(converted_points[0].y))
	);

	// Draw back square:
	setLineColour(vec3(0.2f, 1.0f, 1.0f));

	drawLine(
		vec2(mapXPointToViewport(converted_points[4].x), mapYPointToViewport(converted_points[4].y)),
		vec2(mapXPointToViewport(converted_points[5].x), mapYPointToViewport(converted_points[5].y))
	);

	drawLine(
		vec2(mapXPointToViewport(converted_points[5].x), mapYPointToViewport(converted_points[5].y)),
		vec2(mapXPointToViewport(converted_points[6].x), mapYPointToViewport(converted_points[6].y))
	);

	drawLine(
		vec2(mapXPointToViewport(converted_points[6].x), mapYPointToViewport(converted_points[6].y)),
		vec2(mapXPointToViewport(converted_points[7].x), mapYPointToViewport(converted_points[7].y))
	);

	drawLine(
		vec2(mapXPointToViewport(converted_points[7].x), mapYPointToViewport(converted_points[7].y)),
		vec2(mapXPointToViewport(converted_points[4].x), mapYPointToViewport(converted_points[4].y))
	);

	// Draw connectors (top and bottom face)
	setLineColour(vec3(1.0f, 1.0f, 1.0f));

	drawLine(
		vec2(mapXPointToViewport(converted_points[0].x), mapYPointToViewport(converted_points[0].y)),
		vec2(mapXPointToViewport(converted_points[4].x), mapYPointToViewport(converted_points[4].y))
	);

	drawLine(
		vec2(mapXPointToViewport(converted_points[1].x), mapYPointToViewport(converted_points[1].y)),
		vec2(mapXPointToViewport(converted_points[5].x), mapYPointToViewport(converted_points[5].y))
	);

	drawLine(
		vec2(mapXPointToViewport(converted_points[2].x), mapYPointToViewport(converted_points[2].y)),
		vec2(mapXPointToViewport(converted_points[6].x), mapYPointToViewport(converted_points[6].y))
	);

	drawLine(
		vec2(mapXPointToViewport(converted_points[3].x), mapYPointToViewport(converted_points[3].y)),
		vec2(mapXPointToViewport(converted_points[7].x), mapYPointToViewport(converted_points[7].y))
	);


	drawCubeGnomon(converted_points);
	drawWorldGnomon();

	if (m_currentInteractionMode == InteractionMode::VIEWPORT)
	{
		if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			float lesser_viewport_x = glm::min(m_viewport_start_x, m_viewport_end_x);
			float lesser_viewport_y = glm::min(m_viewport_start_y, m_viewport_end_y);

			float greater_viewport_x = glm::max(m_viewport_start_x, m_viewport_end_x);
			float greater_viewport_y = glm::max(m_viewport_start_y, m_viewport_end_y);

			float window_width_midpoint = m_framebufferWidth / 2;
			float window_height_midpoint = m_framebufferHeight / 2;
			lesser_viewport_x = (lesser_viewport_x / window_width_midpoint) - 1;
			greater_viewport_x = (greater_viewport_x / window_width_midpoint) - 1;
			lesser_viewport_y = 1 - (lesser_viewport_y / window_height_midpoint); // (1 - height) fixes orientation of the drag. Issue is due to viewport (x,y) mapping
			greater_viewport_y = 1 - (greater_viewport_y / window_height_midpoint);

			glm::vec2 top_left_viewport = glm::vec2(lesser_viewport_x, greater_viewport_y);
			glm::vec2 top_right_viewport = glm::vec2(greater_viewport_x, greater_viewport_y);
			glm::vec2 bot_left_viewport = glm::vec2(lesser_viewport_x, lesser_viewport_y);
			glm::vec2 bot_right_viewport = glm::vec2(greater_viewport_x, lesser_viewport_y);

			drawViewportDrag(top_left_viewport, top_right_viewport, bot_left_viewport, bot_right_viewport);
		}
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button( "Reset Application" ) ) {
			reset();
		}

		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "Rotate View", &m_currentInteractionMode, InteractionMode::ROTATE_VIEW ) ) {}
		if( ImGui::RadioButton( "Translate View", &m_currentInteractionMode, InteractionMode::TRANSLATE_VIEW ) ) {}
		if( ImGui::RadioButton( "Perspective", &m_currentInteractionMode, InteractionMode::PERSPECTIVE) ) {}
		if( ImGui::RadioButton( "Rotate Model", &m_currentInteractionMode, InteractionMode::ROTATE_MODEL ) ) {}
		if( ImGui::RadioButton( "Translate Model", &m_currentInteractionMode, InteractionMode::TRANSLATE_MODEL ) ) {}
		if( ImGui::RadioButton( "Scale Model", &m_currentInteractionMode, InteractionMode::SCALE_MODEL) ) {}
		if( ImGui::RadioButton( "Viewport", &m_currentInteractionMode, InteractionMode::VIEWPORT ) ) {}
		ImGui::PopID();

		ImGui::Text( "Near: %.1f, Far: %.1f", m_near_plane, m_far_plane);

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
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
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	double difference_x = xPos - m_last_xPos;
	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		switch (m_currentInteractionMode) {
			case InteractionMode::PERSPECTIVE:
				break;
			case InteractionMode::ROTATE_MODEL:
				m_model_transform.rotateX((m_mouse_move_slowdown_factor * float(difference_x)));
				break;
			case InteractionMode::ROTATE_VIEW:
				m_view_transform.rotateX((m_mouse_move_slowdown_factor * float(difference_x)));
				break;
			case InteractionMode::SCALE_MODEL: 
				m_model_transform.scale(1.0f + (m_mouse_move_slowdown_factor * float(difference_x)), 1.0f, 1.0f);
				break;
			case InteractionMode::TRANSLATE_MODEL:
				m_model_transform.translate((m_mouse_move_slowdown_factor * float(difference_x)), 0.0f, 0.0f);
				break;
			case InteractionMode::TRANSLATE_VIEW:
				m_view_transform.translate((m_mouse_move_slowdown_factor * float(difference_x)), 0.0f, 0.0f);
				break;
			case InteractionMode::VIEWPORT:
				m_viewport_end_x = glm::clamp(ImGui::GetMousePos().x, 0.0f, float(m_framebufferWidth));
				m_viewport_end_y = glm::clamp(ImGui::GetMousePos().y, 0.0f, float(m_framebufferHeight));
				break;
			default:
				break;
		};
		eventHandled = true;
	}
	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
	{
		switch (m_currentInteractionMode) {
			case InteractionMode::PERSPECTIVE:
				break;
			case InteractionMode::ROTATE_MODEL:
				m_model_transform.rotateY((m_mouse_move_slowdown_factor * float(difference_x)));
				break;
			case InteractionMode::ROTATE_VIEW:
				m_view_transform.rotateY((m_mouse_move_slowdown_factor * float(difference_x)));
				break;
			case InteractionMode::SCALE_MODEL:
				m_model_transform.scale(1.0f, 1.0f + (m_mouse_move_slowdown_factor * float(difference_x)), 1.0f);
				break;
			case InteractionMode::TRANSLATE_MODEL:
				m_model_transform.translate(0.0f, (m_mouse_move_slowdown_factor * float(difference_x)), 0.0f);
				break;
			case InteractionMode::TRANSLATE_VIEW:
				m_view_transform.translate(0.0f, (m_mouse_move_slowdown_factor * float(difference_x)), 0.0f);
				break;
			case InteractionMode::VIEWPORT:
				break;
			default:
				break;
		};
		eventHandled = true;
	}
	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		switch (m_currentInteractionMode) {
			case InteractionMode::PERSPECTIVE:
				break;
			case InteractionMode::ROTATE_MODEL:
				m_model_transform.rotateZ((m_mouse_move_slowdown_factor * float(difference_x)));
				break;
			case InteractionMode::ROTATE_VIEW:
				m_view_transform.rotateZ((m_mouse_move_slowdown_factor * float(difference_x)));
				break;
			case InteractionMode::SCALE_MODEL:
				m_model_transform.scale(1.0f, 1.0f, 1.0f + (m_mouse_move_slowdown_factor * float(difference_x)));
				break;
			case InteractionMode::TRANSLATE_MODEL:
				m_model_transform.translate(0.0f, 0.0f, (m_mouse_move_slowdown_factor * float(difference_x)));
				break;
			case InteractionMode::TRANSLATE_VIEW:
				m_view_transform.translate(0.0f, 0.0f, (m_mouse_move_slowdown_factor * float(difference_x)));
				break;
			case InteractionMode::VIEWPORT:
				break;
			default:
				break;
		};
		eventHandled = true;
	}
	m_last_xPos = xPos;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (m_currentInteractionMode == InteractionMode::VIEWPORT)
		{
			if (actions == GLFW_PRESS) 
			{
				if (button == GLFW_MOUSE_BUTTON_LEFT)
				{
					m_viewport_start_x = glm::clamp(ImGui::GetMousePos().x, 0.0f, float(m_framebufferWidth));
					m_viewport_start_y = glm::clamp(ImGui::GetMousePos().y, 0.0f, float(m_framebufferHeight));
					
					m_viewport_end_x = m_viewport_start_x;
					m_viewport_end_y = m_viewport_start_y;
					eventHandled = true;
				}
			}
			if (actions == GLFW_RELEASE)
			{
				if (button == GLFW_MOUSE_BUTTON_LEFT)
				{
					m_viewport_end_x = glm::clamp(ImGui::GetMousePos().x, 0.0f, float(m_framebufferWidth));
					m_viewport_end_y = glm::clamp(ImGui::GetMousePos().y, 0.0f, float(m_framebufferHeight));

					m_viewport_left_boundary = glm::min(m_viewport_start_x, m_viewport_end_x);
					m_viewport_right_boundary = glm::max(m_viewport_start_x, m_viewport_end_x);
					m_viewport_top_boundary = glm::max(m_viewport_start_y, m_viewport_end_y);
					m_viewport_bottom_boundary = glm::min(m_viewport_start_y, m_viewport_end_y);

					eventHandled = true;
				}
			}
		}

	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
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
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_A)
		{
			reset();
		} else if (key == GLFW_KEY_Q)
		{
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		} else if (key == GLFW_KEY_O)
		{
			m_currentInteractionMode = InteractionMode::ROTATE_VIEW;
			eventHandled = true;
		} else if (key == GLFW_KEY_E)
		{
			m_currentInteractionMode = InteractionMode::TRANSLATE_VIEW;
			eventHandled = true;
		} else if (key == GLFW_KEY_P)
		{
			m_currentInteractionMode = InteractionMode::PERSPECTIVE;
			eventHandled = true;
		} else if (key == GLFW_KEY_R)
		{
			m_currentInteractionMode = InteractionMode::ROTATE_MODEL;
			eventHandled = true;
		} else if (key == GLFW_KEY_T)
		{
			m_currentInteractionMode = InteractionMode::TRANSLATE_MODEL;
			eventHandled = true;
		} else if (key == GLFW_KEY_S)
		{
			m_currentInteractionMode = InteractionMode::SCALE_MODEL;
			eventHandled = true;
		} else if (key == GLFW_KEY_V)
		{
			m_currentInteractionMode = InteractionMode::VIEWPORT;
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
void A2::reset()
{
	m_currentInteractionMode = InteractionMode::ROTATE_MODEL;

	m_model_transform.R = glm::mat4(1.0f);
	m_model_transform.S = glm::mat4(1.0f);
	m_model_transform.T = glm::mat4(1.0f);

	m_view_transform.R = glm::mat4(1.0f);
	m_view_transform.S = glm::mat4(1.0f);
	m_view_transform.T = glm::mat4(1.0f);

	m_camera = glm::lookAt(
		glm::vec3( 0.0f, 0.0f, -5.0f ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	m_viewport_start_x = 0.0f;
	m_viewport_end_x = 0.0f;
	m_viewport_start_y = 0.0f;
	m_viewport_end_y = 0.0f;

	m_viewport_left_boundary = floor(0.05f * m_framebufferWidth);
	m_viewport_right_boundary = floor(0.95f * m_framebufferWidth);
	m_viewport_bottom_boundary = floor(0.05f * m_framebufferHeight);
	m_viewport_top_boundary = floor(0.95f * m_framebufferHeight);
}

//----------------------------------------------------------------------------------------
glm::vec4 A2::getCenterOfCube(std::vector<glm::vec4> cube_points)
{
	glm::vec4 sum_of_cube = glm::vec4(0.0f);
	for (auto cube_point : cube_points)
	{
		sum_of_cube += cube_point;
	}

	sum_of_cube /= 8.0f;

	return sum_of_cube;
}

//----------------------------------------------------------------------------------------
glm::vec4 A2::getCenterOfFrontFaceOfCube(std::vector<glm::vec4> cube_points)
{
	// The face that faces the camera
	glm::vec4 sum_of_cube = cube_points[0] + cube_points[1] + cube_points[2] + cube_points[3];
	sum_of_cube /= 4.0f;

	return sum_of_cube;
}

//----------------------------------------------------------------------------------------
glm::vec4 A2::getCenterOfRightFaceOfCube(std::vector<glm::vec4> cube_points)
{
	// Right face from camera's POV (anatomical left from cube's POV)
	glm::vec4 sum_of_cube = cube_points[1] + cube_points[2] + cube_points[5] + cube_points[6];
	sum_of_cube /= 4.0f;

	return sum_of_cube;
}

//----------------------------------------------------------------------------------------
glm::vec4 A2::getCenterOfTopFaceOfCube(std::vector<glm::vec4> cube_points)
{
	glm::vec4 sum_of_cube = cube_points[2] + cube_points[3] + cube_points[6] + cube_points[7];
	sum_of_cube /= 4.0f;

	return sum_of_cube;
}

//----------------------------------------------------------------------------------------
void Transform::rotateX(float theta)
{
	glm::mat4 rotation_matrix = glm::mat4(1.0f);
	rotation_matrix[1][1] = cos(theta);
	rotation_matrix[2][1] = -sin(theta);
	rotation_matrix[1][2] = sin(theta);
	rotation_matrix[2][2] = cos(theta);

	R = m_is_viewing_transform ? (glm::inverse(rotation_matrix) * R): (R * rotation_matrix);
}

//----------------------------------------------------------------------------------------
void Transform::rotateY(float theta)
{
	glm::mat4 rotation_matrix = glm::mat4(1.0f);
	rotation_matrix[0][0] = cos(theta);
	rotation_matrix[2][0] = sin(theta);
	rotation_matrix[0][2] = -sin(theta);
	rotation_matrix[2][2] = cos(theta);

	R = m_is_viewing_transform ? (glm::inverse(rotation_matrix) * R): (R * rotation_matrix);
}

//----------------------------------------------------------------------------------------
void Transform::rotateZ(float theta)
{
	glm::mat4 rotation_matrix = glm::mat4(1.0f);
	rotation_matrix[0][0] = cos(theta);
	rotation_matrix[1][0] = -sin(theta);
	rotation_matrix[0][1] = sin(theta);
	rotation_matrix[1][1] = cos(theta);

	R = m_is_viewing_transform ? (glm::inverse(rotation_matrix) * R): (R * rotation_matrix);
}

//----------------------------------------------------------------------------------------
void Transform::scale(float s_x, float s_y, float s_z)
{
	glm::mat4 scale_matrix = glm::mat4(1.0f);
	scale_matrix[0][0] = s_x;
	scale_matrix[1][1] = s_y;
	scale_matrix[2][2] = s_z;

	S = m_is_viewing_transform ? (glm::inverse(scale_matrix) * S): (S * scale_matrix);
}

//----------------------------------------------------------------------------------------
void Transform::translate(float delta_x, float delta_y, float delta_z)
{
	glm::mat4 translation_matrix = glm::mat4(1.0f);
	translation_matrix[3][0] = delta_x;
	translation_matrix[3][1] = delta_y;
	translation_matrix[3][2] = delta_z;

	T = m_is_viewing_transform ? (glm::inverse(translation_matrix) * T ): (translation_matrix * T);
}

//----------------------------------------------------------------------------------------
glm::mat4 Transform::getTransform()
{
	return R * T * S; // Rotation happens after translation to get translations on the local axes.
}