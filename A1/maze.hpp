// Termm--Fall 2020

#pragma once

class Maze
{
public:
	Maze( size_t dim );
	~Maze();

	void reset();

	size_t getDim() const;

	int getValue( int x, int y ) const;

	void setValue( int x, int y, int h );

	int getStartColumnIdx() const;

	void digMaze();
	void printMaze(); // for debugging
private:
	size_t m_dim;
	int *m_values;
	int m_start_column_idx;
	void recDigMaze(int r, int c);
	int numNeighbors(int r, int c);
};
