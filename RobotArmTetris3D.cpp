/*=======CMPT 361 Assignment 2 - 3D Tetris With Robot Arm implementation=======*/

/*Name: Ivan Jonathan Hoo*/
/*Student #: 301251368*/
/*Email: ihoo@sfu.ca*/

#include "include/Angel.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>

using namespace std;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

/*********************************Global Arrays and Variables*********************************/

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200*6];

// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game 1920x1080
int xsize = 1000; 
int ysize = 768;

//Projection, View, and Model matrices 
mat4 projectionMat,viewMat,modelMat;

//VAO
enum VAO_IDs
{
	VAO_Grid,
	VAO_Board,
	VAO_Tile
};
GLuint vaoIDs[3];

//VBO
enum VBO_IDs
{
	VBO_GridPos,
	VBO_GridColor,
	VBO_BoardPos,
	VBO_BoardColor,
	VBO_CurrTilePos,
	VBO_CurrTileColor
};
GLuint vboIDs[6];

bool gameOver;
bool freezeTime;
float score;
float timer;

bool board[10][20]; // board[x][y] represents whether the cell (x,y) is occupied

vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilePos = vec2(5,20-1); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

int tileType;
vec4 currTileColors[4];

const vec2 allTileShapes[7][4] =
{
	{vec2(-2, 0), vec2(-1, 0), vec2(0,0), vec2( 1,0)},  // I
	{vec2(-1,-1), vec2( 0,-1), vec2(0,0), vec2(-1,0)},  // O
	{vec2(-1, 0), vec2( 1, 0), vec2(0,0), vec2( 0,1)},  // T
	{vec2(-1,-1), vec2( 0,-1), vec2(0,0), vec2( 1,0)},  // S
	{vec2(-1,-1), vec2(-1, 0), vec2(0,0), vec2( 1,0)},  // L
	{vec2( 1,-1), vec2( 0,-1), vec2(0,0), vec2(-1,0)},  // S-reverse
	{vec2(-1, 1), vec2(-1, 0), vec2(0,0), vec2( 1,0)},  // L-reverse
};

/*===================fruit colors===================*/
const vec4 purple = vec4(1.0,0.0,1.0,1.0);
const vec4 red = vec4(1.0,0.0,0.0,1.0);
const vec4 yellow = vec4(1.0,1.0,0.0,1.0);
const vec4 green = vec4(0.0,1.0,0.0,1.0);
const vec4 orange = vec4(1.0,0.5,0.0,1.0);
/*==================================================*/
const vec4 grey = vec4(0.5,0.5,0.5,1.0);
const vec4 gridColor = vec4(0.8,0.8,0.8,0.8);
const vec4 black = vec4(0.0,0.0,0.0,1.0);
const vec4 emptyCellColor = vec4(0.0,0.0,0.0,0.0);

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// location of uniform variables in shader program
GLuint uniformVars;

/*********************************************************************************************/

//============================================ROBOT ARM============================================

/********Robot Arm Global Variables********/
GLuint r_vao;
GLuint r_buffer;
vec3 r_pos;
mat4 r_MVPmat;
const int NumVertices = 36;
point4 points[NumVertices];
color4 colors[NumVertices];
/******************************************/

point4 vertices[8] = {
	point4( -0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5, -0.5, -0.5, 1.0 ),
	point4( -0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5, -0.5, -0.5, 1.0 )
};

color4 vertex_colors[8] = {
	color4( 0.0, 0.5, 0.9, 1.0 ),
	color4( 0.0, 0.7, 0.8, 1.0 ),
	color4( 0.0, 0.6, 0.9, 1.0 ),
	color4( 0.0, 0.9, 0.9, 1.0 ),
	color4( 0.0, 0.8, 0.9, 1.0 ),
	color4( 0.0, 0.5, 0.8, 1.0 ),
	color4( 0.0, 0.6, 0.8, 1.0 ),
	color4( 0.0, 0.7, 0.9, 1.0 )
};

// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 12.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 11.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;

// Array of rotation angles (in degrees) for each rotation axis
enum
{
	Base,
	LowerArm,
	UpperArm,
	NumAngles
};
int     Axis = Base;
GLfloat Theta[NumAngles] = { 0.0 };

int Index = 0;

void quad( int a, int b, int c, int d )
{
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

void colorcube()
{
	quad( 1, 0, 3, 2 );
	quad( 2, 3, 7, 6 );
	quad( 3, 0, 4, 7 );
	quad( 6, 5, 1, 2 );
	quad( 4, 5, 6, 7 );
	quad( 5, 4, 0, 1 );
}

void base(const mat4 &vp)
{
	mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *
			  Scale( BASE_WIDTH,
				 BASE_HEIGHT,
				 BASE_WIDTH ) );
	
	glUniformMatrix4fv( uniformVars, 1, GL_TRUE, vp * r_MVPmat * instance );
	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

void upper_arm(const mat4 &vp)
{
	mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
			  Scale( UPPER_ARM_WIDTH,
				 UPPER_ARM_HEIGHT,
				 UPPER_ARM_WIDTH ) );
	
	glUniformMatrix4fv( uniformVars, 1, GL_TRUE, vp * r_MVPmat * instance );
	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

void lower_arm(const mat4 &vp)
{
	mat4 instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
			  Scale( LOWER_ARM_WIDTH,
				 LOWER_ARM_HEIGHT,
				 LOWER_ARM_WIDTH ) );
	
	glUniformMatrix4fv( uniformVars, 1, GL_TRUE, vp * r_MVPmat * instance );
	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

void initRobot( void )
{
	Index = 0;
	r_pos = vec3(-10, 0, 0);
	colorcube();
	
	// Create a vertex array object
	glGenVertexArrays( 1, &r_vao );
	glBindVertexArray( r_vao );
	
	// Create and initialize a buffer object
	glGenBuffers( 1, &r_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, r_buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_DYNAMIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
	
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	
	glEnableVertexAttribArray( vColor );
	glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
}

vec2 getRobotHandCoor() //get coordinate of tile in robot hand
{	
	return vec2(round(r_pos.x/2 + LOWER_ARM_HEIGHT * -sin(3.14159/180 * Theta[LowerArm]) + (UPPER_ARM_HEIGHT-0.5) * -cos(3.14159/180 * (90 - Theta[LowerArm] - Theta[UpperArm]))),
				round(r_pos.y + BASE_HEIGHT + LOWER_ARM_HEIGHT * cos(-3.14159/180* Theta[LowerArm]) + (UPPER_ARM_HEIGHT-0.5) * sin(3.14159/180 * (90 - Theta[LowerArm] - Theta[UpperArm])))
				);
}

//========================================END OF ROBOT ARM=========================================

//-------------------------------------------------------------------------------------------------------------------

//********Helper Functions********

bool isAboveBoard(int x,int y) 
{
	if(x<0 || x>9 || y<0)
		return false;
	return true;
}

bool isInBoard(int x,int y) 
{
	if(x<0 || x>9 || y<0 || y>19)
		return false;
	return true;
}

bool isOccupied(int x,int y)
{
	if(!isInBoard(x,y))
		return false;
	return board[x][y];
}

int canRelease()
{
	int cellsInBoard=0,cellsOccupied=0;
	for(int i=0; i<4; i++)
	{
		vec2 coor = tilePos + tile[i];
		if( isOccupied((int)coor.x,(int)coor.y) )
			cellsOccupied++;
		if( isAboveBoard((int)coor.x,(int)coor.y) )
			cellsInBoard++;
	}
	return (cellsInBoard==4 && cellsOccupied==0);
}

bool noTileBelow(const vec2 &p)
{
	for(int i=0; i<4; i++)
	{
		vec2 coor = p + tile[i];
		if( (int)(coor.y-1)<0 || isOccupied((int)coor.x,(int)(coor.y-1.0)) )
			return false;
	}
	return true;
}

//********************************

//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	if(gameOver)
		return;
	
	if(!freezeTime)
		tilePos = getRobotHandCoor();

	if(canRelease())
	{
		//update the color VBO of current tile
		vec4 newcolours[24*6];
		for (int i=0; i<24*6; i++)
			newcolours[i] = currTileColors[i/36];
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_CurrTileColor]); // Bind the VBO containing current tile vertex colours
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else
	{
		vec4 newcolours[24*6];
		for (int i=0; i<24*6; i++)
			newcolours[i] = grey;
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_CurrTileColor]); // Bind the VBO containing current tile vertex colours
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	}

	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_CurrTilePos]); 

	// For each of the 4 'cells' of the tile,
	for (int i=0; i<4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilePos.x + tile[i].x; 
		GLfloat y = tilePos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0),  16.5, 1); // front left bottom
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0),  16.5, 1); // front left top
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0),  16.5, 1); // front right bottom
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0),  16.5, 1); // front right top
		vec4 p5 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), -16.5, 1); // back left bottom
		vec4 p6 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), -16.5, 1); // back left top
		vec4 p7 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), -16.5, 1); // back right bottom
		vec4 p8 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), -16.5, 1); // back right top

		// Two points are used by two triangles each
		vec4 newpoints[36] = { p1, p2, p3, p2, p3, p4,
							   p5, p6, p7, p6, p7, p8,
							   p1, p2, p5, p2, p5, p6,
							   p3, p4, p7, p4, p7, p8,
							   p2, p4, p6, p4, p6, p8,
							   p1, p3, p5, p3, p5, p7 };

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*sizeof(newpoints), sizeof(newpoints), newpoints); 
	}
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile
void rotate()
{
	vec2 nextRotation[4];
	for(int i=0; i<4; i++)
		nextRotation[i]=vec2(-tile[i].y,tile[i].x);

	if(freezeTime)
		for(int i=0; i<4; i++)
		{
			vec2 coor=tilePos+nextRotation[i];
			if( (!isInBoard((int)coor.x,(int)coor.y) && freezeTime) || isOccupied((int)coor.x,(int)coor.y) )
				return;
		}

	//actually do rotation
	for(int i=0; i<4; i++)
		tile[i] = nextRotation[i];
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
	if(gameOver)
		return;

	//check for game over
	for(int i=0; i<10; i++)
		if(board[i][19]==true)
		{
			gameOver=true;
			return;
		}

	timer = 5;
	freezeTime = false;

	tilePos = getRobotHandCoor();

	//update the geometry VBO of current tile

	tileType = rand()%7;

	vector<int> colorIntVector; //this not the same as mathematical vector (this is C++ vector data structure)
	for(int i=0; i<5; i++)
		colorIntVector.push_back(i);

	for(int i=0; i<4; i++)
	{
		//make sure each box have different colors
		int selectedIndex = rand()%(int)(colorIntVector.size());
		int cellColorInt = colorIntVector[selectedIndex];

		//assign color
		if(cellColorInt==0)
			currTileColors[i] = purple;
		if(cellColorInt==1)
			currTileColors[i] = red;
		if(cellColorInt==2)
			currTileColors[i] = yellow;
		if(cellColorInt==3)
			currTileColors[i] = green;
		if(cellColorInt==4)
			currTileColors[i] = orange;

		//remove selected vector element
		colorIntVector.erase(colorIntVector.begin()+selectedIndex);

		tile[i] = allTileShapes[tileType][i];
	}

	for(int i=0; i<rand()%5; i++) //randomize orientation
		rotate();

	updatetile();

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	//64*2+21*11*2=590
	vec4 gridpoints[590];
	vec4 gridcolors[590];

	// Draw grid lines
	for (int i=0; i<11; i++)
	{
		gridpoints[2*i]    = vec4((33.0+(33.0*i)),33.0,  16.5,1);
		gridpoints[2*i+1]  = vec4((33.0+(33.0*i)),693.0, 16.5,1);
		gridpoints[2*i+64] = vec4((33.0+(33.0*i)),33.0, -16.5,1);
		gridpoints[2*i+65] = vec4((33.0+(33.0*i)),693.0,-16.5,1);
	}

	for (int i=0; i<21; i++)
	{
		gridpoints[22+2*i]    = vec4(33.0, (33.0+(33.0*i)), 16.5,1);
		gridpoints[22+2*i+1]  = vec4(363.0,(33.0+(33.0*i)), 16.5,1);
		gridpoints[22+2*i+64] = vec4(33.0, (33.0+(33.0*i)),-16.5,1);
		gridpoints[22+2*i+65] = vec4(363.0,(33.0+(33.0*i)),-16.5,1);
	}

	for (int i=0; i<21; i++)
		for (int j=0; j<11; j++) 
		{
			gridpoints[128+22*i+2*j]   = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0),  16.5, 1); // front left bottom
			gridpoints[128+22*i+2*j+1] = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), -16.5, 1); // back left bottom
		}

	for (int i=0; i<590; i++)
		gridcolors[i] = gridColor;

	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[VAO_Grid]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_GridPos]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, (590)*sizeof(vec4), gridpoints, GL_DYNAMIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_GridColor]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, (590)*sizeof(vec4), gridcolors, GL_DYNAMIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}

void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200*6];
	for (int i=0; i<1200*6; i++)
		boardcolours[i] = emptyCellColor;
		
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i=0; i<20; i++)
		for (int j=0; j<10; j++)
		{
			//Vertices of a board cell (a cube)
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0),  16.50, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0),  16.50, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0),  16.50, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0),  16.50, 1);
			vec4 p5 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), -16.50, 1);
			vec4 p6 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), -16.50, 1);
			vec4 p7 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), -16.50, 1);
			vec4 p8 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), -16.50, 1);

			vec4 cubeVertices[36] = { p1, p2, p3, p2, p3, p4,
									  p5, p6, p7, p6, p7, p8,
									  p1, p2, p5, p2, p5, p6,
									  p3, p4, p7, p4, p7, p8,
									  p2, p4, p6, p4, p6, p8,
									  p1, p3, p5, p3, p5, p7 };

			for(int k=0;k<36;k++)
				boardpoints[36*(10*i+j)+k]=cubeVertices[k];
		}

	// Initially no cell is occupied
	for (int i=0; i<10; i++)
		for (int j=0; j<20; j++)
			board[i][j] = false;

	// *** set up buffer objects
	glBindVertexArray(vaoIDs[VAO_Board]);
	glGenBuffers(2, &vboIDs[VBO_BoardPos]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_BoardPos]);
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_BoardColor]);
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrTile()
{
	glBindVertexArray(vaoIDs[VAO_Tile]);
	glGenBuffers(2, &vboIDs[VBO_CurrTilePos]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_CurrTilePos]);
	glBufferData(GL_ARRAY_BUFFER, 24*6*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_CurrTileColor]);
	glBufferData(GL_ARRAY_BUFFER, 24*6*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	//load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	//get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	//create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	//initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrTile();
	initRobot();

	//reset robot arm angles
	Theta[LowerArm] = 0;
	Theta[UpperArm] = -80;

	uniformVars = glGetUniformLocation(program, "MVPmat");

	//setup camera
	vec3 eye(0,26.5,27);
	vec3 center(0,10,0); //Center of board height=20/2=10
	vec3 up(0,1,0);
	viewMat = LookAt(eye,center,up);

	//reset variables
	gameOver = false;
	freezeTime = false;
	score = 0;
	timer = 5;

	newtile(); //create new tile

	//blend
   	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);
	//anti-alias
	glEnable(GL_MULTISAMPLE);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

//-------------------------------------------------------------------------------------------------------------------

void setcolor(int x,int y,const vec4 &color)
{
	for(int i=0; i<36; i++)
	{
		int index = 36*(10*y+x)+i;
		boardcolours[index] = color;
	}
}

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile(const vec2 &p)
{
	for(int i=0; i<4; i++)
	{
		int XX = p.x+tile[i].x;
		int YY = p.y+tile[i].y;

		if(YY>19)
		{
			gameOver=true;
			return;
		}

		board[XX][YY] = true;
		setcolor(XX,YY,currTileColors[i]);
	}
}

//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	init();
}

//-------------------------------------------------------------------------------------------------------------------

//function to display text on GUI
template<class T>
void displayTxt(T str,float x,float y)
{
	stringstream ss(str);
	glRasterPos2f(x,y);
	char ch;
	while(ss>>noskipws>>ch)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,ch);
}

// Draws the game
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4f(1.0,0.0,0.0,1.0);

	projectionMat = Perspective(45,1.0*xsize/ysize,10,200);

	//draw robot arm
	glBindVertexArray(r_vao);
	mat4 M = projectionMat * viewMat * Translate(r_pos);
	r_MVPmat = RotateY(Theta[Base] );
	base(M);
	r_MVPmat *= Translate(0.0, BASE_HEIGHT, 0.0);
	r_MVPmat *= RotateZ(Theta[LowerArm]);
	lower_arm(M);
	r_MVPmat *= Translate(0.0, LOWER_ARM_HEIGHT, 0.0);
	r_MVPmat *= RotateZ(Theta[UpperArm]);
	upper_arm(M);

	r_MVPmat *= Translate(0.0, UPPER_ARM_HEIGHT, 0.0);

	//scale everything to unit length
	modelMat = mat4();
	modelMat *= Translate(0,10.0,0);
	modelMat *= Scale(1.0/33.0,1.0/33.0,1.0/33.0);  // scale to unit length
	modelMat *= Translate(-198.0,-363.0,0); // move to origin

	mat4 MVPmat = projectionMat * viewMat * modelMat;
	glUniformMatrix4fv(uniformVars,1,GL_TRUE,MVPmat);

	glBindVertexArray(vaoIDs[VAO_Board]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES,0,1200*6); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[VAO_Tile]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES,0,24*6); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[VAO_Grid]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES,0,590);

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_BoardColor]); 
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	
	//when game over...
	if(gameOver)
	{
		//display Game Over text on GUI
		glColor4f(1.0,0.0,0.0,1.0);
		displayTxt("GAME OVER!",-0.075,0.95);
		stringstream ssGUI;
		ssGUI.clear(); 
		ssGUI.str("");
		ssGUI << noskipws << "FINAL SCORE: " << score;
		displayTxt(ssGUI.str(),-0.075,-0.75);
		displayTxt("Press \"Q\" to quit!!!",-0.075,-0.85);
		displayTxt("Press \"R\" to restart!!!",-0.085,-0.95);
		//update the color VBO of current tile
		vec4 newcolours[24*6];
		for (int i=0; i<24*6; i++)
			newcolours[i] = currTileColors[i/36];
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_CurrTileColor]); // Bind the VBO containing current tile vertex colours
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//display score and timer on GUI
	if(!gameOver)
	{
		glColor4f(1.0,0.0,0.0,1.0);
		stringstream ssGUI;
		ssGUI << noskipws << "Score: " << score << "   Time Remaining: " << ceil(timer) << "s";
		displayTxt(ssGUI.str(),-0.023,-0.7);
		ssGUI.clear(); 
		ssGUI.str("");
	}

	//check timer
	if(timer>0 && freezeTime==false)
		timer-=1.0/60.0;
	else if(freezeTime==false) //if time is up replace tile by new one
	{
		if(score>0)
			score=(score-50)>=0?(score-50):0; //score penalty if time is up
		newtile();
	}

	glutSwapBuffers();
}

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{
	//Check if there is any empty cells in the specified row
	for(int i=0;i<10;i++)
		if(board[i][row]==false)
			return;

	score+=10;

	//shift everything down one row
	for(int b=row; b<19; b++)
		for(int a=0; a<10; a++)
		{
			board[a][b]=board[a][b+1];
			setcolor(a,b,boardcolours[36*(10*(b+1)+a)]);
		}
	for(int a=0; a<10; a++)
	{
		board[a][19]=false;
		setcolor(a,19,emptyCellColor);
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_BoardColor]); 
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
}

//-------------------------------------------------------------------------------------------------------------------

//function to make tile falls automatically
void fallingTileAuto(int data)
{
	if(noTileBelow(tilePos))
	{
		tilePos.y--;
		updatetile();
		glutTimerFunc(100, fallingTileAuto, data);
	} 
	else
	{
		settile(tilePos);

		for(int x=0; x<20; x++)
			for(int i=0; i<20; i++)
				checkfullrow(i);

		newtile();
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VBO_BoardColor]); 
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP:
			rotate();
			updatetile();
			break;
		case GLUT_KEY_RIGHT:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				viewMat *= RotateY(10);
			break;
		case GLUT_KEY_LEFT:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				viewMat *= RotateY(-10);
			break;
		default:
			break;
	}
}

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case ' ':
			if(freezeTime==true) //stop multiple calls to glutTimerFunc() if user holds the space key
				return;
			if(canRelease()) 
			{
				freezeTime=true;
				glutTimerFunc(0,fallingTileAuto,-1);
			}
			break;
		case 'w':
			Theta[UpperArm]+=5;
			updatetile();
			break;
		case 'a':
			Theta[LowerArm]+=5;
			updatetile();
			break;
		case 's':
			Theta[UpperArm]-=5;
			updatetile();
			break;
		case 'd':
			Theta[LowerArm]-=5;
			updatetile();
			break;
	}
	glutPostRedisplay();
}

void idle(void)
{
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_MULTISAMPLE | GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize,ysize);
	glutInitWindowPosition(0.5*(glutGet(GLUT_SCREEN_WIDTH)-xsize), 0.5*(glutGet(GLUT_SCREEN_HEIGHT)-ysize)); //center position relative to screen size
	glutCreateWindow("3D Tetris With Robot Arm");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
