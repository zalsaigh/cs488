// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "maze.hpp"

enum Direction
{
	UP = 1,
	DOWN = 2,
	RIGHT = 3,
	LEFT = 4
};

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

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

private:
	void initGrid();
	void digMaze();
	void drawWalls(size_t mazeDim);
	void drawFloor(size_t mazeDim);
	void drawAvatar();
	void reset();

	void handleMove(Direction dir);

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Fields related to cube geometry.
	GLuint m_cube_vao; // Vertex Array Object
	GLuint m_cube_vbo; // Vertex Buffer Object

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	glm::mat4 transform;

	float m_floor_colour[3];
	float m_avatar_colour[3];
	float m_walls_colour[3];
	int current_col;

	Maze *m_maze;
	int m_height_added;

	int m_avatar_posn_row;
	int m_avatar_posn_column;

	bool m_maze_exists;

	double m_last_x_pos;
	double m_last_x_pos_while_dragging;
	double m_threshold_for_persistence;
	double m_persistence_speed;

	float m_camera_scroll_pos;
	float m_max_zoom;
	float m_min_zoom;

	bool m_persist;
	bool m_check_if_should_persist;
};
