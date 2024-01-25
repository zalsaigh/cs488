// Termm--Fall 2020

#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <sys/types.h>
#include <unistd.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

using namespace glm;
using namespace std;

static const size_t DIM = 16;
static const float OFFSET_OFF_GRID = 0.01; // Offsets all objects off the grid
// This makes it so that the grid doesn't show through the floor, for example

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 ), m_height_added( 0 ), m_maze_exists( false ),
		m_avatar_posn_row( 0 ), m_avatar_posn_column( 0 ), m_last_x_pos( 0 ),
		m_camera_scroll_pos( 2.*float(DIM)*2.0*M_SQRT1_2 ), m_min_zoom(15.0f),
		m_max_zoom(75.0f), m_persist( false ), m_last_x_pos_while_dragging( 0 ),
		m_threshold_for_persistence(0.1), m_persistence_speed( 0 ), m_check_if_should_persist( false )
{

	// Floor dark brown
	m_floor_colour[0] = 0.239f;
	m_floor_colour[1] = 0.134f;
	m_floor_colour[2] = 0.067f;

	// Walls light brown
	m_walls_colour[0] = 0.768f;
	m_walls_colour[1] = 0.702f;
	m_walls_colour[2] = 0.612f;

	// Avatar white
	m_avatar_colour[0] = 1.0f;
	m_avatar_colour[1] = 1.0f;
	m_avatar_colour[2] = 1.0f;

	m_maze = new Maze(DIM);
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{
	delete m_maze;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Initialize random number generator
	int rseed=getpid();
	srandom(rseed);
	// Print random number seed in case we want to rerun with
	// same random numbers
	cout << "Random number seed = " << rseed << endl;

	// m_maze->digMaze();
	// m_maze->printMaze();
	//
	// m_avatar_posn_row = -1;
	// m_avatar_posn_column = m_maze->getStartColumnIdx();

	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt(
		glm::vec3( 0.0f, m_camera_scroll_pos, m_camera_scroll_pos * 0.5 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective(
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );

	transform = glm::translate( transform, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
}

void A1::initGrid()
{
	// DIM + 3 lines in each dimension
	// 2 * that to get all lines in both directions
	// 2 * that to get each two points that make up each line
	// 3 * that to get the coordinates that make up each point
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the grid vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my*
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
	if (m_persist) {
		glm::vec3 axis_of_rotation(0.0f, 1.0f, 0.0f);
		float rotation_factor = 200.0f;
		mat4 rotatedTransform = glm::rotate(mat4(), float(m_persistence_speed) / rotation_factor, axis_of_rotation);
		rotatedTransform *= transform;
		transform = rotatedTransform;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button( "Reset Application" ) ) {
			reset();
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.
		switch(current_col) {
			case 1:
				ImGui::ColorEdit3( "##Colour", m_avatar_colour );
				break;
			case 2:
				ImGui::ColorEdit3( "##Colour", m_walls_colour );
				break;
			default:
				ImGui::ColorEdit3( "##Colour", m_floor_colour );
				break;
		};

		// ImGui::SameLine();
		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "Select Floor Colour", &current_col, 0 ) ) {}
		if( ImGui::RadioButton( "Select Avatar Colour", &current_col, 1 ) ) {}
		if( ImGui::RadioButton( "Select Walls Colour", &current_col, 2 ) ) {}
		ImGui::PopID();

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
void A1::drawFloor(size_t mazeDim) {
	// We need two triangles, 3 vertices each, 3 floats each
	size_t sz = 2 * 3 * 3;

	float fMazeDim = (float)mazeDim;

	float verts[sz] = {
		0, 0 + OFFSET_OFF_GRID, 0,
		0, 0 + OFFSET_OFF_GRID, fMazeDim,
		fMazeDim, 0 + OFFSET_OFF_GRID, fMazeDim,

		fMazeDim, 0 + OFFSET_OFF_GRID, fMazeDim,
		fMazeDim, 0 + OFFSET_OFF_GRID, 0,
		0, 0 + OFFSET_OFF_GRID, 0
	};

	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	glUniform3f( col_uni, m_floor_colour[0], m_floor_colour[1], m_floor_colour[2] );
	glDrawArrays( GL_TRIANGLES, 0, sz / 3 ); // div by 3 to get number of vertices
}

//----------------------------------------------------------------------------------------
void A1::drawWalls(size_t mazeDim) {
	// Draw the cubes
	// 1 cube
	// 6 * that for each side of the cube
	// 2 * that for each triangle that makes  up a side
	// 3 * that for each vertex that makes up a triangle
	// 3 * that for the coordinates that make up a vertex
	size_t sz = 3 * 3 * 2 * 6;

	for (size_t i = 0 ; i < mazeDim; i++) {
		for (size_t j = 0; j < mazeDim; j++) {
			if (m_maze->getValue(i, j) == 1) {
				// If it has an X
				// i and j are flipped for x and z because device coords are different
				// from array coords.
				float verts[sz] = {
					// Bottom face
					0 + j, 0 + OFFSET_OFF_GRID, 0 + i,
					0 + j, 0 + OFFSET_OFF_GRID, 1 + i,
					1 + j, 0 + OFFSET_OFF_GRID, 0 + i,

					1 + j, 0 + OFFSET_OFF_GRID, 0 + i,
					1 + j, 0 + OFFSET_OFF_GRID, 1 + i,
					0 + j, 0 + OFFSET_OFF_GRID, 1 + i,

					// Face that faces away from us
					0 + j, 0 + OFFSET_OFF_GRID, 1 + i,
					0 + j, 1 + OFFSET_OFF_GRID, 1 + i,
					1 + j, 0 + OFFSET_OFF_GRID, 1 + i,

					1 + j, 0 + OFFSET_OFF_GRID, 1 + i,
					1 + j, 1 + OFFSET_OFF_GRID, 1 + i,
					0 + j, 1 + OFFSET_OFF_GRID, 1 + i,

					// Face that faces left
					0 + j, 1 + OFFSET_OFF_GRID, 0 + i,
					0 + j, 0 + OFFSET_OFF_GRID, 0 + i,
					0 + j, 0 + OFFSET_OFF_GRID, 1 + i,

					0 + j, 1 + OFFSET_OFF_GRID, 0 + i,
					0 + j, 1 + OFFSET_OFF_GRID, 1 + i,
					0 + j, 0 + OFFSET_OFF_GRID, 1 + i,

					// Face that faces right
					1 + j, 0 + OFFSET_OFF_GRID, 1 + i,
					1 + j, 0 + OFFSET_OFF_GRID, 0 + i,
					1 + j, 1 + OFFSET_OFF_GRID, 0 + i,

					1 + j, 1 + OFFSET_OFF_GRID, 0 + i,
					1 + j, 1 + OFFSET_OFF_GRID, 1 + i,
					1 + j, 0 + OFFSET_OFF_GRID, 1 + i,

					// Face that faces me
					0 + j, 0 + OFFSET_OFF_GRID, 0 + i,
					0 + j, 1 + OFFSET_OFF_GRID, 0 + i,
					1 + j, 0 + OFFSET_OFF_GRID, 0 + i,

					0 + j, 1 + OFFSET_OFF_GRID, 0 + i,
					1 + j, 1 + OFFSET_OFF_GRID, 0 + i,
					1 + j, 0 + OFFSET_OFF_GRID, 0 + i,

					// Face that faces up
					1 + j, 1 + OFFSET_OFF_GRID, 1 + i,
					1 + j, 1 + OFFSET_OFF_GRID, 0 + i,
					0 + j, 1 + OFFSET_OFF_GRID, 0 + i,

					0 + j, 1 + OFFSET_OFF_GRID, 0 + i,
					0 + j, 1 + OFFSET_OFF_GRID, 1 + i,
					1 + j, 1 + OFFSET_OFF_GRID, 1 + i

				};

				glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
					verts, GL_STATIC_DRAW );

				// Specify the means of extracting the position values properly.
				GLint posAttrib = m_shader.getAttribLocation( "position" );
				glEnableVertexAttribArray( posAttrib );
				glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

				glUniform3f( col_uni, m_walls_colour[0], m_walls_colour[1], m_walls_colour[2] );
				glDrawArrays( GL_TRIANGLES, 0, sz / 3 ); // div by 3 to get number of vertices

				for (size_t h = 0 ; h < m_height_added; h++) {
					for (size_t itr = 1; itr  < sz; itr += 3) { // Change every y coord
						verts[itr]++;
					}
					glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
						verts, GL_STATIC_DRAW );

					// Specify the means of extracting the position values properly.
					GLint posAttrib = m_shader.getAttribLocation( "position" );
					glEnableVertexAttribArray( posAttrib );
					glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

					glUniform3f( col_uni, m_walls_colour[0], m_walls_colour[1], m_walls_colour[2] );
					glDrawArrays( GL_TRIANGLES, 0, sz / 3 ); // div by 3 to get number of vertices
				}
			}
		}
	}

}

//----------------------------------------------------------------------------------------
void A1::drawAvatar() {
	// Sphere, 12 stacks x 24 sectors = 268 panels
	std::vector<float> verts;
	size_t numStacks = 12;
	size_t numSectors = 24;
	float radius = 0.5f;

	float fAvatarColumn = (float) m_avatar_posn_column;
	float fAvatarRow = (float) m_avatar_posn_row;

	for (int i = 0; i <= numStacks; i++) {
		float stackAngle = M_PI_2 - i * (M_PI / numStacks);
		for (int j = 0; j <= numSectors; j++) {
			float sectorAngle = j * 2 * M_PI / numSectors;
			float xCoord = radius * cos(stackAngle) * cos(sectorAngle);
			float yCoord = radius * cos(stackAngle) * sin(sectorAngle);
			float zCoord = radius * sin(stackAngle);
			verts.push_back(xCoord + fAvatarColumn + 0.5); // Add 0.5 to x and z to center the sphere in the grid
			verts.push_back(yCoord + OFFSET_OFF_GRID + 0.5); // Add 0.5 to y to get the sphere off the ground
			verts.push_back(zCoord + fAvatarRow + 0.5);
		}
	}

	std::vector<float> triangles;
	// Each sector looks like this
	/* a --- b
	*  |     |
	*  |     |
	*  |     |
	*  c --- d
	* so we want a-c-d triangles and d-b-a triangles
	*  a --- b
	*  | \   |
	*  |  \  |
	*  |   \ |
	*  c --- d
	* Except the top and bottom row of sectors
	*       a
	*      / \
	*     /   \
	*    c --- d
	*
	*		 a --- b
	*    \     /
	*     \   /
	*    		d
	*/
	for (int i = 0; i < numStacks; i++) {
		for (int j = 0; j < numSectors; j++) {
			int aVertexXCoordIdx = (i * (numSectors + 1) + j) * 3;
			int aVertexYCoordIdx = aVertexXCoordIdx + 1;
			int aVertexZCoordIdx = aVertexXCoordIdx + 2;

			int bVertexXCoordIdx = (i * (numSectors + 1) + (j + 1)) * 3;
			int bVertexYCoordIdx = bVertexXCoordIdx + 1;
			int bVertexZCoordIdx = bVertexXCoordIdx + 2;

			int cVertexXCoordIdx = ((i + 1) * (numSectors + 1) + j) * 3;
			int cVertexYCoordIdx = cVertexXCoordIdx + 1;
			int cVertexZCoordIdx = cVertexXCoordIdx + 2;

			int dVertexXCoordIdx = ((i + 1) * (numSectors + 1) + (j + 1)) * 3;
			int dVertexYCoordIdx = dVertexXCoordIdx + 1;
			int dVertexZCoordIdx = dVertexXCoordIdx + 2;

			if (i != 0)
			{
				// Add the a-c-d triangle
				triangles.push_back(verts[aVertexXCoordIdx]);
				triangles.push_back(verts[aVertexYCoordIdx]);
				triangles.push_back(verts[aVertexZCoordIdx]);

				triangles.push_back(verts[cVertexXCoordIdx]);
				triangles.push_back(verts[cVertexYCoordIdx]);
				triangles.push_back(verts[cVertexZCoordIdx]);

				triangles.push_back(verts[dVertexXCoordIdx]);
				triangles.push_back(verts[dVertexYCoordIdx]);
				triangles.push_back(verts[dVertexZCoordIdx]);

			}

			if (i != numStacks - 1)
			{
				// Add the d-b-a triangle

				triangles.push_back(verts[dVertexXCoordIdx]);
				triangles.push_back(verts[dVertexYCoordIdx]);
				triangles.push_back(verts[dVertexZCoordIdx]);

				triangles.push_back(verts[bVertexXCoordIdx]);
				triangles.push_back(verts[bVertexYCoordIdx]);
				triangles.push_back(verts[bVertexZCoordIdx]);

				triangles.push_back(verts[aVertexXCoordIdx]);
				triangles.push_back(verts[aVertexYCoordIdx]);
				triangles.push_back(verts[aVertexZCoordIdx]);

			}
		}
	}

	size_t sz = triangles.size();
	float finalArr[sz]; // Can't pass a vector to OpenGL

	for (size_t i = 0 ; i < sz; i++) {
		finalArr[i] = triangles[i];
	}

	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		finalArr, GL_STATIC_DRAW );
	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	glUniform3f( col_uni, m_avatar_colour[0], m_avatar_colour[1], m_avatar_colour[2] );
	glDrawArrays( GL_TRIANGLES, 0, sz / 3 ); // div by 3 to get number of vertices
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	// mat4 W;
	// W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		// glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( transform ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 ); // 2 vertices each, 2 dimensions, 3 + DIM total lines each dimension

		// Create the vertex array to record buffer assignments.
		glGenVertexArrays( 1, &m_cube_vao );
		glBindVertexArray( m_cube_vao );

		// Create the cube vertex buffer
		glGenBuffers( 1, &m_cube_vbo );
		glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );

		if (m_maze_exists) {
			size_t mazeDim = m_maze->getDim();

			drawFloor(mazeDim);
			if (m_height_added >= 0)
			{
				drawWalls(mazeDim); // m_height_added can be -1, meaning no walls
			}
		}

		drawAvatar();

		// Reset state to prevent rogue code from messing with *my*
		// stuff!
		glBindVertexArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
		// Highlight the active square.
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos)
{
	// Put some code here to handle rotations.  Probably need to
	// check whether we're *dragging*, not just moving the mouse.
	// Probably need some instance variables to track the current
	// rotation amount, and maybe the previous X position (so
	// that you can rotate relative to the *change* in X.
	bool eventHandled(false);
	double difference_x = xPos - m_last_x_pos;
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			glm::vec3 axis_of_rotation(0.0f, 1.0f, 0.0f);
			float rotation_factor = 100.0f;
			mat4 rotatedTransform = glm::rotate(mat4(), float(difference_x) / rotation_factor, axis_of_rotation);
			rotatedTransform *= transform;
			transform = rotatedTransform;
			m_last_x_pos_while_dragging = xPos;
			m_check_if_should_persist = true;
			eventHandled = true;
		}
	}
	m_last_x_pos = xPos;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if (actions == GLFW_PRESS) {
			m_persist = false;
		}

		if (actions == GLFW_RELEASE) {
			if (m_check_if_should_persist)
			{
				double xPos, yPos;
				glfwGetCursorPos(m_window, &xPos, &yPos);
				if (glm::abs(xPos - m_last_x_pos_while_dragging) > m_threshold_for_persistence)
				{
					m_persist = true;
					m_persistence_speed = xPos - m_last_x_pos_while_dragging;
				}
				m_check_if_should_persist = false;
			}
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.

	// yOffset is 1 when we zoom in, -1 when we zoom out
	if (yOffSet != 0) {
		if (m_camera_scroll_pos < m_min_zoom) {
			m_camera_scroll_pos = m_min_zoom;
		} else if (m_camera_scroll_pos > m_max_zoom) {
			m_camera_scroll_pos = m_max_zoom;
		} else {
			// This works because the z value lookFrom is half the y value lookFrom lol,
			// so i just scaled it at the same rate.
			view = glm::lookAt(
				glm::vec3( 0.0f, m_camera_scroll_pos - yOffSet, 0.5*(m_camera_scroll_pos - yOffSet)),
				glm::vec3( 0.0f, 0.0f, 0.0f ),
				glm::vec3( 0.0f, 1.0f, 0.0f )
			);
			m_camera_scroll_pos -= yOffSet;
		}
		eventHandled = true;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
bool isInOuterRing(int row, int column)
{
	// -1, -1 -> DIM (inclusive)
	// -1 -> DIM (inclusive), -1
	// DIM, -1 -> DIM (inclusive)
	// -1 -> DIM (inclusive), -1
	int intDim = int(DIM); // So I can compare int with int
	bool isInTopRow = (row == -1 && (-1 <= column && column <= intDim));
	bool isInBottomRow = (row == intDim && (-1 <= column && column <= intDim));
	bool isInLeftmostColumn = ((-1 <= row && row <= intDim) && column == -1);
	bool isInRightmostColumn = ((-1 <= row && row <= intDim) && column == intDim);
	return isInTopRow || isInBottomRow || isInLeftmostColumn || isInRightmostColumn;
}

//----------------------------------------------------------------------------------------
bool isInMaze(int row, int column)
{
	// 0 -> DIM - 1 (inclusive), 0 -> DIM - 1 (inclusive)
	int intDim = int(DIM); // So I can compare int with int
	return (0 <= row && row < intDim) && (0 <= column && column < intDim);
}

//----------------------------------------------------------------------------------------
/*
 * Handles avatar movement
 */
void A1::handleMove(Direction dir)
{
	int target_row = m_avatar_posn_row;
	int target_column = m_avatar_posn_column;
	switch(dir) {
		case Direction::UP:
			target_row = m_avatar_posn_row - 1;
			break;
		case Direction::DOWN:
			target_row = m_avatar_posn_row + 1;
			break;
		case Direction::RIGHT:
			target_column = m_avatar_posn_column + 1;
			break;
		case Direction::LEFT:
			target_column = m_avatar_posn_column - 1;
			break;
		default:
			break;
	}

	if (isInOuterRing(m_avatar_posn_row, m_avatar_posn_column) &&
			isInOuterRing(target_row, target_column))
	{
		m_avatar_posn_row = target_row;
		m_avatar_posn_column = target_column;
	} else if (isInMaze(m_avatar_posn_row, m_avatar_posn_column) &&
						 isInOuterRing(target_row, target_column))
	{
		// This case is for if the avatar is in the maze start or maze end spot
		m_avatar_posn_row = target_row;
		m_avatar_posn_column = target_column;
	} else if (isInMaze(target_row, target_column))
	{
		if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
				glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		{
			if (m_maze->getValue(target_row, target_column) == 1)
			{
				m_maze->setValue(target_row, target_column, 0);
			}
			m_avatar_posn_row = target_row;
			m_avatar_posn_column = target_column;
		} else
		{
			if (m_maze->getValue(target_row, target_column) == 0)
			{
				m_avatar_posn_row = target_row;
				m_avatar_posn_column = target_column;
			}
		}
	}
}


//----------------------------------------------------------------------------------------
/*
 *  Dig maze
 */
void A1::digMaze() {
	m_maze->reset();
	m_maze->digMaze();
	m_maze->printMaze();
	m_maze_exists = true;
	m_avatar_posn_row = 0;
	m_avatar_posn_column = m_maze->getStartColumnIdx();
}
//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.

		// Quit program.
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}

		// Reset program to default state
		if (key == GLFW_KEY_R) {
			reset();
			eventHandled = true;
		}

		// Dig a new maze
		if (key == GLFW_KEY_D) {
			digMaze();
			eventHandled = true;
		}

		if (m_maze_exists) {
			if (key == GLFW_KEY_SPACE) {
				m_height_added++;
				eventHandled = true;
			}

			if (key == GLFW_KEY_BACKSPACE) {
				if (m_height_added >= 0) {
					m_height_added--;
				}
				eventHandled = true;
			}
		}

		if (key == GLFW_KEY_UP) {
			handleMove(Direction::UP);
			eventHandled = true;
		}

		if (key == GLFW_KEY_DOWN) {
			handleMove(Direction::DOWN);
			eventHandled = true;
		}

		if (key == GLFW_KEY_RIGHT) {
			handleMove(Direction::RIGHT);
			eventHandled = true;
		}

		if (key == GLFW_KEY_LEFT) {
			handleMove(Direction::LEFT);
			eventHandled = true;
		}

	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Sets the maze back to original state
 * (empty grid, default view, default colours, reset avatar location)
 */
void A1::reset()
{

	// Floor dark brown
	m_floor_colour[0] = 0.239f;
	m_floor_colour[1] = 0.134f;
	m_floor_colour[2] = 0.067f;

	// Walls light brown
	m_walls_colour[0] = 0.768f;
	m_walls_colour[1] = 0.702f;
	m_walls_colour[2] = 0.612f;

	// Avatar white
	m_avatar_colour[0] = 1.0f;
	m_avatar_colour[1] = 1.0f;
	m_avatar_colour[2] = 1.0f;

	m_height_added = 0;
	m_maze_exists = false;
	m_maze->reset();

	m_avatar_posn_row = 0;
	m_avatar_posn_column = 0;

	m_persist = false;

	// Reset the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	m_camera_scroll_pos = 2.*float(DIM)*2.0*M_SQRT1_2;

	// Reset to initial view and projection and transform matrices
	view = glm::lookAt(
		glm::vec3( 0.0f, m_camera_scroll_pos, m_camera_scroll_pos * 0.5 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective(
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );

	transform = glm::translate( mat4(), vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
}
