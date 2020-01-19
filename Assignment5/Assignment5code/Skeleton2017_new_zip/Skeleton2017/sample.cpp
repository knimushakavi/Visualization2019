#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


#include <windows.h>
//#include <freeglut.h>
#include "GL\include\GL.H"
#include "GL\include\glu.h"
#include "GL\include\glut.h"
#include "GL\include\glui.h"

#include "Skeleton.h"
#include <time.h>
//#include "glui.h"

// title of these windows:

const char *WINDOWTITLE = { "OpenGL / GLUT / Assignment1 -- Kusuma Nimushakavi" };
const char *GLUITITLE   = { "User Interface Window" };

// what the glui package defines as true and false:
const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

// the escape key:
#define ESCAPE		0x1b

// initial window size:
const int INIT_WINDOW_SIZE = { 600 };
// size of the box:
const float BOXSIZE = { 2.f };

// multiplication factors for input interaction:
//  (these are known from previous experience)
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };
// able to use the left mouse for either rotation or scaling,
// in case have only a 2-button mouse:
enum LeftButton
{
	ROTATE,
	SCALE
};
// minimum allowable scale factor:
const float MINSCALE = { 0.05f };
// active mouse buttons (or them together):
const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };
// which projection:
enum Projections
{
	ORTHO,
	PERSP
};

// which button:
enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):
const float BACKCOLOR[] = { 0., 0., 0., 0. };

// line width for the axes:
const GLfloat AXES_WIDTH   = { 3. };

int XYplane = 0, YZplane = 0, ZXplane = 0, wireframe = 0, Bilinear = 0, volumeRendering = 1;
float s_min =10000 , s_max=0 , white_range = 0.5,iso_value = 25;
int x_value = 20, y_value = 20, z_value = 20;
int contours = 1;
float rgb[3] = {0.,0.,0.};
const int NX = 64, NY = 64, NZ = 64;
float x_min = -1.0, x_max = 1.0, y_min = -1.0, y_max = 1.0, z_min = -1.0, z_max = 1.0, grad_min = 20, grad_max = 0;
float g_min = 0.0, g_max = 445.0, MaxAlpha = 1.;
const float TEMPMIN = { 0.f };
const float TEMPMAX = { 100.f };
float t_min = TEMPMIN, t_max = TEMPMAX;

enum Colors
{
	RAINBOW,
	BWR,
	HEATMAP,
	DISCRETE,
	NONLINEAR,
	WHITE,
};



const GLfloat Colors[1][3] = { { 1., 1., 1. } };

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };

//
// non-constant global variables:
//

int	ActiveButton;		// current button that is down
GLuint	AxesList;		// list to hold the axes
int	AxesOn;			// != 0 means to draw the axes
int	DebugOn;			// != 0 means to print debugging info
int	DepthCueOn;		// != 0 means to use intensity depth cueing
GLUI *	Glui;			// instance of glui window
int	GluiWindow;		// the glut id for the glui window
int	LeftButton;		// either ROTATE or SCALE
GLuint	BoxList;		// object display list
int	MainWindow;		// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichColor;		// index into Colors[]
int	WhichProjection;	// ORTHO or PERSP
int	Xmouse, Ymouse;		// mouse values
float	Xrot, Yrot;		// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets

//
// function prototypes:
//

void	Animate( void );
void	Buttons( int );
void	Display( void );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( void );
void	InitGlui( void );
void	InitGraphics( void );
void	InitLists( void );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( void );
void	Resize( int, int );
void	Visibility( int );
void	Arrow( float [3], float [3] );
void	Cross( float [3], float [3], float [3] );
float	Dot( float [3], float [3] );
float	Unit( float [3], float [3] );
void	Axes( float );
void	HsvRgb( float[3], float [3] );


void    Display_Model(void);
void    set_view(GLenum mode);
void    set_scene(GLenum mode);
void    display_shape(GLenum mode);
void    Choose_Object();

double radius_factor = 0.9;
int display_mode = 0; 
int edge = 0;
int square = 0;


struct node {
	float x, y, z;
	float s;
	float r, g, b;
	float rad;
	float dTdx, dTdy, dTdz;
	float grad;
};

node Nodes[NX][NY][NZ];

/*
typedef struct node {
	float x, y, z, s;
};*/

typedef struct lineseg {
	int n1, n2;
};

typedef struct intersect_points {
	float x, y,z,s;
};

typedef struct intersects {
	intersect_points start, end;
};

typedef struct quad {
	int verts[4];
	int edges[4];
};

struct sources {
	double xc, yc, zc;
	double a;
} Sources[] = {
	{1.00f,0.00f,0.00f,90.00f},
	{-1.00f,-0.30f,0.00f,120.00f},
	{0.00f,1.00f,0.00f,120.00f},
	{0.00f,0.40f,1.00f,170.00f},
};
std::vector<node> grid_pts;
//std::vector<node> iso_pts;
std::vector<intersect_points> iso_pts;
std::vector<intersect_points> iso_stack;
std::vector<intersects> isoList;
std::vector<lineseg> edgeList;
std::vector<quad> quadList;


void rainbow_color(float s, float s_max, float s_min, float rgb[3])
{
	float t = (s - s_min) / (s_max - s_min);
	// make sure t is between 0 and 1, if not, rgb should be black
	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}
	float hsv[3] = { 1. };
	// map the scalar value linearly to the hue channel of the HSV
	hsv[0] = (1. - t) * 240;
	hsv[1] = hsv[2] = 1.; // set the saturation and value as 1
	// Call the HSV to RGB conversion function
	HsvRgb(hsv, rgb);
}

void bwr(float s, float s_max, float s_min, float rgb[3]) {

	float t = (s - s_min) / (s_max - s_min);
	// make sure t is between 0 and 1, if not, rgb should be black
	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}
	float hsv[3] = { 1. };
	float smid = (s_min + s_max) * white_range;

	if (s < smid) {
		hsv[0] = 240.;
		hsv[1] = 1 - ((s - s_min) / (smid - s_min));
	}
	else if (s > smid) {
		hsv[0] = 0;
		hsv[1] = ((s - smid) / (s_max - smid));
	}
	hsv[2] = 1.;
	HsvRgb(hsv, rgb);

}

void nonlinear(float s, float s_max, float s_min, float rgb[3]) {

	float t = (s - s_min) / (s_max - s_min);
	// make sure t is between 0 and 1, if not, rgb should be black
	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}
	float hsv[3] = { 1. };
	float smid = (s_min + s_max) / 2;
	float p = (s - smid) / (smid - s_min);
	// map the scalar value linearly to the hue channel of the HSV
	if (s < smid) {
		hsv[0] = (1. - p) * 240;
		hsv[1] = hsv[2] = 1.;

	}
	else {
		hsv[0] = 1;
		hsv[1] = hsv[2] = 1;
	}

	HsvRgb(hsv, rgb);
}

void heatmap(float s, float s_max, float s_min, float rgb[3]) {
	float t = (s - s_min) / (s_max - s_min);
	// make sure t is between 0 and 1, if not, rgb should be black
	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}
	float hsv[3] = { 1. };
	rgb[0] = rgb[1] = rgb[2] = 0.;
	float scale = (s_max - s_min) / 3;
	float s_one = s_min + scale;
	float s_two = s_one + scale;

	if (s >= s_min && s < s_one) {
		rgb[0] = s;
		rgb[1] = 0;
		rgb[2] = 0;
	}
	else if (s <= s_two) {
		rgb[0] = 1;
		rgb[1] = s;
		rgb[2] = 0;
	}
	else if (s <= s_max) {
		rgb[0] = 1;
		rgb[1] = 1;
		rgb[2] = s;
	}
}

void discrete(float s, float s_max, float s_min, float rgb[3])
{
	float t = (s - s_min) / (s_max - s_min);
	// make sure t is between 0 and 1, if not, rgb should be black
	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}
	rgb[0] = rgb[1] = rgb[2] = 0.;
	float scale = (s_max - s_min) / 6;
	float s_one = s_min + scale;
	float s_two = s_one + scale;
	float s_three = s_two + scale;
	float s_four = s_three + scale;
	float s_five = s_four + scale;

	if (s >= s_min && s < s_one) {
		rgb[0] = 0.5;
		rgb[1] = rgb[2] = 0;
	}
	else if (s <= s_two) {
		rgb[0] = 1;
		rgb[2] = 0;
		rgb[1] = 0;
	}
	else if (s <= s_three) {
		rgb[0] = 1;
		rgb[1] = 0.5;
		rgb[2] = 0;
	}
	else if (s <= s_four) {
		rgb[0] = 1;
		rgb[1] = 1;
		rgb[2] = 0;
	}
	else if (s <= s_five) {
		rgb[0] = 1;
		rgb[1] = 1;
		rgb[2] = 0.5;
	}
	else if (s <= s_max) {
		rgb[0] = 1;
		rgb[1] = 1;
		rgb[2] = 1;
	}

}

double Temperature(double x, double y, double z) {
	double t = 0.0;

	for (int i = 0; i < 4; i++) {
		double dx = x - Sources[i].xc;
		double dy = y - Sources[i].yc;
		double dz = z - Sources[i].zc;
		double rsqd = dx * dx + dy * dy + dz * dz;
		t += Sources[i].a *exp(-5.*rsqd);
	}
	if (t > TEMPMAX)
		t = TEMPMAX;
	return t;
}

int count = 0;
void data_generator() {
	int i, j, k;
	double x=-1.0f, y=-1.0f, z=-1.0f;
	float scale = float(2./NX);
	printf("%f\n", scale);
	for (i = 0; i < NX; i++) {
		for (j = 0; j < NY; j++) {
			for (k = 0; k < NZ; k++) {
				Nodes[i][j][k].x = x;
				Nodes[i][j][k].y = y;
				Nodes[i][j][k].z = z;
				Nodes[i][j][k].s = Temperature(x, y, z);
				rgb[0] = rgb[1] = rgb[2];
				z += scale;
			}
			z = -1;
			y += scale;
		}
		y = -1;
		x += scale;
	}
}
void build_edge_list() {
	int i, j;
	int cur = 0;
	edgeList.clear();
	lineseg temp;
	for (j = 0; j < NY - 1; j++) {
		cur = j * NX;
		for (i = 0; i < NX - 1; i++) {
			temp.n1 = cur;
			temp.n2 = cur + 1;
			edgeList.push_back(temp);
			edge++;
			temp.n1 = cur;
			temp.n2 = cur + NX;
			edgeList.push_back(temp);
			edge++;
			cur++;
		}
		temp.n1 = cur;
		temp.n2 = cur + NX;
		edgeList.push_back(temp);
		edge++;
	}
	cur = (NY - 1)*NX;
	for (i = 0; i < NX - 1; i++) {
		temp.n1 = cur;
		temp.n2 = cur + 1;
		edgeList.push_back(temp);
		edge++;
		cur++;
	}
}
void build_face_list() {
	int i, j, p;
	int cur = 0;
	quadList.clear();
	square = 0;
	quad temp;
	for (j = 0; j < NY - 1; j++) {
		cur = j * NX;
		for (i = 0; i < NX - 1; i++) {
			//adding Vertices
			temp.verts[0] = cur;
			temp.verts[1] = cur + 1;
			temp.verts[2] = cur + NX + 1;
			temp.verts[3] = cur + NX;
			square++;
			quadList.push_back(temp);
			cur++;
		}
	}
}

int intersection_counter; 
int lines;
const int MINUS = { 0 };
const int PLUS = { 1 };
#define X 0
#define Y 1
#define Z 2
int Major;
int Xside, Yside, Zside;

unsigned char TextureXY[NZ][NY][NX][4];
unsigned char TextureXZ[NY][NZ][NX][4];
unsigned char TextureYZ[NX][NZ][NY][4];

void marching_squares(float iso)
{
	int i, j, l;
	int t;
	float nx, ny, nz, savg;
	iso_pts.clear();
	intersection_counter = 0;
	//printf("contours: %f", iso);
	int counter;
	quad quads;
	for (i = 0; i < square; i++) {
		counter = 0;
		savg = 0;
		iso_stack.clear();
		quads = quadList[i];
		for (j = 0; j < 4; j++) {
			node v0, v1;
			intersect_points point;
			v0 = grid_pts[quads.verts[j]];
			//printf("V0: %f\t%f\t%f\t%f%\n", v0.x, v0.y, v0.z, v0.s);
			if (j + 1 < 4)
				v1 = grid_pts[quads.verts[j + 1]];
			else
				v1 = grid_pts[quads.verts[0]];
			savg += v0.s;
			if ((v0.s == iso == v1.s)) {
				point.x = v0.x;
				point.y = v0.y;
				point.z = v0.z;
				iso_pts.push_back(point);
				iso_stack.push_back(point);
				counter++;
				intersection_counter++;
				point.x = v1.x;
				point.y = v1.y;
				point.z = v1.z;
				iso_pts.push_back(point);
				iso_stack.push_back(point);
				counter++;
				intersection_counter++;
				printf("intersection:", intersection_counter);

			}
			if ((v0.s < iso < v1.s) || (v1.s < iso < v0.s)) {
				t = ((iso - v0.s) / (v1.s - v0.s));
				if ((0 <= t) && (t <= 1))
				{
					nx = ((1 - t) * v0.x) + (t * v1.x);
					ny = ((1 - t) * v0.y) + (t * v1.y);
					nz = ((1 - t) * v0.z) + (t * v1.z);
					point.x = nx;
					point.y = ny;
					point.z = nz;
					iso_pts.push_back(point);
					iso_stack.push_back(point);
					counter++;
					intersection_counter++;
					//printf("intersection:", intersection_counter);
				}
			}
		}
		intersects inter;
		if (counter == 1) {
			iso_pts.pop_back();
			iso_stack.clear();
			intersection_counter--;
		}
		else if (counter == 2) {
			int i;
			intersect_points s, e;
			int len = iso_stack.size();
			s = iso_stack[0];
			e = iso_stack[1];/*glBegin(GL_LINES);
			glColor3f(1, 1, 1);
			glVertex3d(s.x, s.y, s.z);
			glVertex3d(e.x, e.y, e.z);
			glEnd();*/
			
			iso_stack.clear();
		}
		else if (counter == 3) {
			int a;
			int flag = 3;
			intersect_points v0, v1, v2;
			v0 = iso_stack[0];
			v1 = iso_stack[1];
			v2 = iso_stack[2];
			for (j = 0; j < 4; j++) {
				node v;
				intersect_points point;
				v = grid_pts[quads.verts[j]];
				if (v.x == v0.x && v.y == v0.y && v.z == v0.z) {
					flag = 0;
					break;
				}
				if (v.x == v1.x && v.y == v1.y && v.z == v1.z) {
					flag = 1;
					break;
				}
				if (v.x == v2.x && v.y == v2.y && v.z == v2.z) {
					flag = 2;
					break;
				}
			}
			if (flag == 3) {
				for (a = 0; a < counter; a++) {
					iso_pts.pop_back();
					iso_stack.clear();
					intersection_counter--;
				}
			}
			else {
				if (flag == 0) {
					int i;
					intersect_points s, e;
					int len = iso_stack.size();
					s = iso_stack[1];
					e = iso_stack[2];
					/*	glBegin(GL_LINES);
					glColor3f(1, 1, 1);
					glVertex3d(s.x, s.y, s.z);
					glVertex3d(e.x, e.y, e.z);
					glEnd();*/
				
					iso_stack.clear();
				}
				if (flag == 1) {

					int i;
					intersect_points s, e;
					int len = iso_stack.size();
					s = iso_stack[0];
					e = iso_stack[2];
					/*glBegin(GL_LINES);
					glColor3f(1, 1, 1);
					glVertex3d(s.x, s.y, s.z);
					glVertex3d(e.x, e.y, e.z);
					glEnd();*/
					
					iso_stack.clear();

				}
				if (flag == 2) {
					int i;
					intersect_points s, e;
					int len = iso_stack.size();
					s = iso_stack[0];
					e = iso_stack[1];
					/*glBegin(GL_LINES);
					glColor3f(1, 1, 1);
					glVertex3d(s.x, s.y, s.z);
					glVertex3d(e.x, e.y, e.z);
					glEnd();*/
					
					iso_stack.clear();
				}
			}
		}
		else if (counter == 4) {
			savg = savg / 4;
			intersect_points s, e, l, r;
			int len = iso_stack.size();
			s = iso_stack[0];
			e = iso_stack[1];
			l = iso_stack[1];
			r = iso_stack[1];
			/*	glBegin(GL_LINES);
			glColor3f(1, 1, 1);
			glVertex3d(s.x, s.y, s.z);
			glVertex3d(e.x, e.y, e.z);
			glEnd();
			glBegin(GL_LINES);
			glColor3f(1, 1, 1);
			glVertex3d(l.x, l.y, l.z);
			glVertex3d(r.x, r.y, r.z);
			glEnd();*/
		
			iso_stack.clear();


		}
	}
}

void planexy() {
	unsigned int i, j, k;
	float s;
	grid_pts.clear();
	for (i = 0; i < NX; i++) {
		for (j = 0; j < NY; j++) {
			k = z_value;
				node temp;
				temp.x = Nodes[i][j][k].x;
				temp.y = Nodes[i][j][k].y;
				temp.z = Nodes[i][j][k].z;
				temp.s = Nodes[i][j][k].s;
				temp.grad = Nodes[i][j][k].grad;
				grid_pts.push_back(temp);
			
		}
	}
	build_edge_list();
	build_face_list();
	node temp;
	for (i = 0; i < NX*NY; i++) {
		temp = grid_pts[i];
		s = temp.s;
		if (s > s_max) {
			s_max = s;
		}
		else if (s < s_min) {
			s_min = s;
		}
	}

	int p;
	float x, y;
	quad quads;
	node v0, v1, v2, v3;
	int cur = 0;
	rgb[0] = rgb[1] = rgb[2] = 1;
	for (p = 0; p < square; p++) {
		int intersection_counter = 0;
		quads = quadList[p];
		v0 = grid_pts[quads.verts[0]];
		v1 = grid_pts[quads.verts[1]];
		v2 = grid_pts[quads.verts[2]];
		v3 = grid_pts[quads.verts[3]];
		glBegin(GL_QUADS);
		if (v0.x > x_min && v0.x < x_max && v0.y > y_min && v0.y < y_max && v0.s >= t_min && v0.s <= t_max && v0.grad >= g_min && v0.grad <=g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v0.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v0.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v0.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v0.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v0.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);	
			glVertex3d(v0.x, v0.y, v0.z);
		}
		if (v1.x > x_min && v1.x < x_max && v1.y > y_min && v1.y < y_max  && v1.s >= t_min && v1.s <= t_max && v1.grad >= g_min && v1.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v1.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v1.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v1.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v1.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v1.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v1.x, v1.y, v1.z);
		}
		if (v2.x > x_min && v2.x < x_max && v2.y > y_min && v2.y < y_max && v2.s >= t_min && v2.s <= t_max && v2.grad >= g_min && v2.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v2.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v2.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v2.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v2.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v2.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v2.x, v2.y, v2.z);
		}
		if (v3.x > x_min && v3.x < x_max && v3.y > y_min && v3.y < y_max && v3.s >= t_min && v3.s <= t_max && v3.grad >= g_min && v3.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v3.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v3.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v3.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v3.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v3.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);	
			glVertex3d(v3.x, v3.y, v3.z);
		}
		glEnd();
	}
}

void planeyz() {
	unsigned int i, j, k;
	float s;
	grid_pts.clear();
	for (j = 0; j < NY; j++) {
		for (k = 0; k < NZ; k++) {
			i = x_value;
			node temp;
			temp.x = Nodes[i][j][k].x;
			temp.y = Nodes[i][j][k].y;
			temp.z = Nodes[i][j][k].z;
			temp.s = Nodes[i][j][k].s;
			temp.grad = Nodes[i][j][k].grad;
			grid_pts.push_back(temp);
		}
	}
	build_edge_list();
	build_face_list();
	node temp;
	for (i = 0; i < NX*NY; i++) {
		temp = grid_pts[i];
		s = temp.s;
		if (s > s_max) {
			s_max = s;
		}
		else if (s < s_min) {
			s_min = s;
		}
	}

	int p;
	float x, y;
	quad quads;
	node v0, v1, v2, v3;
	int cur = 0;
	rgb[0] = rgb[1] = rgb[2] = 1;
	for (p = 0; p < square; p++) {
		int intersection_counter = 0;
		quads = quadList[p];
		v0 = grid_pts[quads.verts[0]];
		v1 = grid_pts[quads.verts[1]];
		v2 = grid_pts[quads.verts[2]];
		v3 = grid_pts[quads.verts[3]];
		glBegin(GL_QUADS);
		if (v0.z > z_min && v0.z < z_max && v0.y > y_min && v0.y < y_max && v0.s >= t_min && v0.s <= t_max && v0.grad >= g_min && v0.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v0.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v0.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v0.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v0.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v0.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v0.x, v0.y, v0.z);
		}
		if (v1.z > z_min && v1.z < z_max && v1.y > y_min && v1.y < y_max && v1.s >= t_min && v1.s <= t_max && v1.grad >= g_min && v1.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v1.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v1.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v1.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v1.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v1.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v1.x, v1.y, v1.z);
		}
		if (v2.z > z_min && v2.z < z_max && v2.y > y_min && v2.y < y_max && v2.s >= t_min && v2.s <= t_max && v2.grad >= g_min && v2.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v2.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v2.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v2.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v2.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v2.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v2.x, v2.y, v2.z);
		}
		if (v3.z > z_min && v3.z < z_max && v3.y > y_min && v3.y < y_max && v3.s >= t_min && v3.s <= t_max && v3.grad >= g_min && v3.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v3.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v3.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v3.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v3.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v3.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v3.x, v3.y, v3.z);
		}
		glEnd();
	}
}

void planezx() {
	unsigned int i, j, k;
	float s;
	grid_pts.clear();
	for (k = 0; k < NZ; k++) {
		for (i = 0; i < NX; i++) {
			j = y_value;
			node temp;
			temp.x = Nodes[i][j][k].x;
			temp.y = Nodes[i][j][k].y;
			temp.z = Nodes[i][j][k].z;
			temp.s = Nodes[i][j][k].s;
			temp.grad = Nodes[i][j][k].grad;
			grid_pts.push_back(temp);
		}
	}
	build_edge_list();
	build_face_list();
	node temp;
	for (i = 0; i < NX*NZ; i++) {
		temp = grid_pts[i];
		s = temp.s;
		if (s > s_max) {
			s_max = s;
		}
		else if (s < s_min) {
			s_min = s;
		}
	}

	int p;
	float x, y;
	quad quads;
	node v0, v1, v2, v3;
	int cur = 0;
	rgb[0] = rgb[1] = rgb[2] = 1;
	for (p = 0; p < square; p++) {
		int intersection_counter = 0;
		quads = quadList[p];
		v0 = grid_pts[quads.verts[0]];
		v1 = grid_pts[quads.verts[1]];
		v2 = grid_pts[quads.verts[2]];
		v3 = grid_pts[quads.verts[3]];
		glBegin(GL_QUADS);
		if (v0.z > z_min && v0.z < z_max && v0.x > x_min && v0.x < x_max && v0.s >= t_min && v0.s <= t_max && v0.grad >= g_min && v0.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v0.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v0.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v0.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v0.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v0.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v0.x, v0.y, v0.z);
		}
		if (v1.z > z_min && v1.z < z_max && v1.x > x_min && v1.x < x_max && v1.s >= t_min && v1.s <= t_max && v1.grad >= g_min && v1.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v1.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v1.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v1.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v1.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v1.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v1.x, v1.y, v1.z);
		}
		if (v2.z > z_min && v2.z < z_max && v2.x > x_min && v2.x < x_max && v2.s >= t_min && v2.s <= t_max && v2.grad >= g_min && v2.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v2.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v2.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v2.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v2.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v2.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v2.x, v2.y, v2.z);
		}
		if (v3.z > z_min && v3.z < z_max && v3.x > x_min && v3.x < x_max && v3.s >= t_min && v3.s <= t_max && v3.grad >= g_min && v3.grad <= g_max) {
			switch (WhichColor) {
			case RAINBOW:
				rainbow_color(v3.s, s_max, s_min, rgb);
				break;
			case BWR:
				bwr(v3.s, s_min, s_max, rgb);
				break;
			case HEATMAP:
				heatmap(v3.s, s_min, s_max, rgb);
				break;
			case DISCRETE:
				discrete(v3.s, s_min, s_max, rgb);
				break;
			case NONLINEAR:
				nonlinear(v3.s, s_min, s_max, rgb);
				break;
			case WHITE:
				rgb[0] = rgb[1] = rgb[2] = 1;
				break;
			}
			glColor3fv(rgb);
			glVertex3d(v3.x, v3.y, v3.z);
		}
		glEnd();
	}
}

void gradient_calculation() {
	int i, j, k;
	for (i = 0; i < NX; i++) {
		for (j = 0; j < NY; j++) {
			for (k = 0; k < NZ; k++) {

				if (i == 0) {
					Nodes[i][j][k].dTdx = (Nodes[i + 1][j][k].s)/(Nodes[i + 1][j][k].x);
					Nodes[i][j][k].dTdy = (Nodes[i][j + 1][k].s - Nodes[i][j - 1][k].s) / (Nodes[i][j + 1][k].y - Nodes[i][j - 1][k].y);
					Nodes[i][j][k].dTdz = (Nodes[i][j][k + 1].s - Nodes[i][j][k - 1].s) / (Nodes[i][j][k + 1].z - Nodes[i][j][k - 1].z);
					
				}
				else if (j == 0) {
					Nodes[i][j][k].dTdx = ((Nodes[i + 1][j][k].s) - (Nodes[i - 1][j][k].s)) / (Nodes[i + 1][j][k].x - Nodes[i - 1][j][k].x);
					Nodes[i][j][k].dTdy = (Nodes[i][j + 1][k].s ) / (Nodes[i][j + 1][k].y);
					Nodes[i][j][k].dTdz = (Nodes[i][j][k + 1].s - Nodes[i][j][k - 1].s) / (Nodes[i][j][k + 1].z - Nodes[i][j][k - 1].z);
					
				}
				else if (k == 0) {
					Nodes[i][j][k].dTdx = ((Nodes[i + 1][j][k].s) - (Nodes[i - 1][j][k].s)) / (Nodes[i + 1][j][k].x - Nodes[i - 1][j][k].x);
					Nodes[i][j][k].dTdy = (Nodes[i][j + 1][k].s - Nodes[i][j - 1][k].s) / (Nodes[i][j + 1][k].y - Nodes[i][j - 1][k].y);
					Nodes[i][j][k].dTdz = (Nodes[i][j][k + 1].s ) / (Nodes[i][j][k + 1].z );
					
				}
				else if (i == NX-1) {
					Nodes[i][j][k].dTdx = ((Nodes[i - 1][j][k].s)) / ( Nodes[i - 1][j][k].x);
					Nodes[i][j][k].dTdy = (Nodes[i][j + 1][k].s - Nodes[i][j - 1][k].s) / (Nodes[i][j + 1][k].y - Nodes[i][j - 1][k].y);
					Nodes[i][j][k].dTdz = (Nodes[i][j][k + 1].s - Nodes[i][j][k - 1].s) / (Nodes[i][j][k + 1].z - Nodes[i][j][k - 1].z);
					
				}
				else if (j == NY-1) {
					Nodes[i][j][k].dTdx = ((Nodes[i + 1][j][k].s) - (Nodes[i - 1][j][k].s)) / (Nodes[i + 1][j][k].x - Nodes[i - 1][j][k].x);
					Nodes[i][j][k].dTdy = ( Nodes[i][j - 1][k].s) / ( Nodes[i][j - 1][k].y);
					Nodes[i][j][k].dTdz = (Nodes[i][j][k + 1].s - Nodes[i][j][k - 1].s) / (Nodes[i][j][k + 1].z - Nodes[i][j][k - 1].z);
				
				}
				else if (k == NZ -1) {
					Nodes[i][j][k].dTdx = ((Nodes[i + 1][j][k].s) - (Nodes[i - 1][j][k].s)) / (Nodes[i + 1][j][k].x - Nodes[i - 1][j][k].x);
					Nodes[i][j][k].dTdy = (Nodes[i][j + 1][k].s - Nodes[i][j - 1][k].s) / (Nodes[i][j + 1][k].y - Nodes[i][j - 1][k].y);
					Nodes[i][j][k].dTdz = ( Nodes[i][j][k - 1].s) / ( Nodes[i][j][k - 1].z);
					
				}
				else {
					Nodes[i][j][k].dTdx = ((Nodes[i + 1][j][k].s) - (Nodes[i - 1][j][k].s)) / (Nodes[i + 1][j][k].x - Nodes[i - 1][j][k].x);
					Nodes[i][j][k].dTdy = (Nodes[i][j + 1][k].s - Nodes[i][j - 1][k].s) / (Nodes[i][j + 1][k].y - Nodes[i][j - 1][k].y);
					Nodes[i][j][k].dTdz = (Nodes[i][j][k + 1].s - Nodes[i][j][k - 1].s) / (Nodes[i][j][k + 1].z - Nodes[i][j][k - 1].z);
				
				}
				float sum = (Nodes[i][j][k].dTdx * Nodes[i][j][k].dTdx) + (Nodes[i][j][k].dTdy * Nodes[i][j][k].dTdy) + (Nodes[i][j][k].dTdy * Nodes[i][j][k].dTdy);
				Nodes[i][j][k].grad = sqrt(sum);
				sum = 0;
				
				if (Nodes[i][j][k].grad > grad_max) {
					grad_max = Nodes[i][j][k].grad;
				}
				else if (Nodes[i][j][k].grad < grad_min) {
					grad_min = Nodes[i][j][k].grad;
				}
			}
		}
	}
	//printf("min gradient: %f, max gradient : %f \n ", min, max);
}

void generate_wireframe() {
   
	unsigned int i, j, k,p;
	float s;
	for (k = 0; k < NZ; k++) {
		for (i = 0; i < NX-1; i++) {
			for (j = 0; j < NY-1; j++) {
				node temp;
				temp.x = Nodes[i][j][k].x;
				temp.y = Nodes[i][j][k].y;
				temp.z = Nodes[i][j][k].z;
				temp.s = Nodes[i][j][k].s;
				grid_pts.push_back(temp);
			}
		}
		node temp;
		for (i = 0; i < NX*NY; i++) {
			temp = grid_pts[i];
			s = temp.s;
			if (s > s_max) {
				s_max = s;
			}
			else if (s < s_min) {
				s_min = s;
			}
		}
		build_edge_list();
		build_face_list();
		for (p = 0; p < square; p++) {
			int intersection_counter = 0;
			//marching_squares(iso_value);
		}
		
	}
	for (j = 0; j < NY; j++) {
		for (k = 0; k < NZ-1; k++) {
			for (i = 0; i < NX-1; i++) {
				node temp;
				temp.x = Nodes[i][j][k].x;
				temp.y = Nodes[i][j][k].y;
				temp.z = Nodes[i][j][k].z;
				temp.s = Nodes[i][j][k].s;
				grid_pts.push_back(temp);
			}
		}
		node temp;
		for (i = 0; i < NX*NY; i++) {
			temp = grid_pts[i];
			s = temp.s;
			if (s > s_max) {
				s_max = s;
			}
			else if (s < s_min) {
				s_min = s;
			}
		}
		build_edge_list();
		build_face_list();
		for (p = 0; p < square; p++) {
			int intersection_counter = 0;
			//marching_squares(iso_value);
		}
		
	}
	for (i = 0; i < NX; i++) {
		for (j = 0; j < NY-1; j++) {
			for (k = 0; k < NZ-1; k++) {
				node temp;
				temp.x = Nodes[i][j][k].x;
				temp.y = Nodes[i][j][k].y;
				temp.z = Nodes[i][j][k].z;
				temp.s = Nodes[i][j][k].s;
				grid_pts.push_back(temp);
			}
		}
		node temp;
		for (i = 0; i < NX*NY; i++) {
			temp = grid_pts[i];
			s = temp.s;
			if (s > s_max) {
				s_max = s;
			}
			else if (s < s_min) {
				s_min = s;
			}
		}
		build_edge_list();
		build_face_list();
		for (p = 0; p < square; p++) {
			int intersection_counter = 0;
			//marching_squares(iso_value);
		}	
	}
}

void draw_wireframe() {
    /*intersect_points point;
	int q;
	glBegin(GL_POINTS);
	glColor3f(1, 1, 1);
	//printf("intersection:%d", intersection_counter);
	for (q = 0; q < intersection_counter; q++) {
		point = iso_pts[q];
		printf("%f", point.x);
		glVertex3d(point.x, point.y, point.z);
	}
	glEnd();*/
	
	printf("intersect counter", intersection_counter);

}

void CompositeXY() {
	int x, y, z, zz;
	float alpha;
	float r, g, b;
	for (x = 0; x < NX; x++) {
		for (y = 0; y < NY; y++) {
			r = g = b = 0;
			for (zz = 0; zz < NZ; zz++) {
				//which direction to composite
				if (Zside == PLUS)
					z = zz;
				else
					z = (NZ - 1) - zz;
				node v = Nodes[x][y][z];
				float s = Nodes[x][y][z].s;
				rgb[0] = rgb[1] = rgb[2] = 1;
				switch (WhichColor) {
				case RAINBOW:
					rainbow_color(s, TEMPMAX,TEMPMIN, rgb);
					break;
				case BWR:
					bwr(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case HEATMAP:
					heatmap(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case DISCRETE:
					discrete(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case NONLINEAR:
					nonlinear(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case WHITE:
					rgb[0] = rgb[1] = rgb[2] = 1;
					break;
				}				
			
				Nodes[x][y][z].r = rgb[0];
				Nodes[x][y][z].g = rgb[1];
				Nodes[x][y][z].b = rgb[2];
				if (v.x >= x_min && v.x <= x_max && v.y >= y_min && v.y <= y_max && v.z >= z_min && v.z <= z_max && v.s >= t_min && v.s <= t_max && v.grad >= g_min && v.grad <= g_max)
				{
					r = Nodes[x][y][z].r;
					g = Nodes[x][y][z].g;
					b = Nodes[x][y][z].b;
					alpha = MaxAlpha;	
				}
				else {
					r = g = b = 0;
					alpha = 0.0;
				}
				//printf("r:%f,g:%f,b:%f\n",r,g,b);
				TextureXY[zz][x][y][0] = (unsigned char)(255.*r + .5);
				TextureXY[zz][x][y][1] = (unsigned char)(255.*g + .5);
				TextureXY[zz][x][y][2] = (unsigned char)(255.*b + .5);
				TextureXY[zz][x][y][3] = (unsigned char)(255.*alpha + .5);
			}
		}
	}
}

void CompositeXZ() {
	int x, y, z, yy;
	float alpha;
	float r, g, b;
	
	for (x = 0; x < NX; x++) {
		for (z = 0; z < NZ; z++)  
		{
			r = g = b = 0;
			for (yy = 0; yy < NY; yy++) {
				//which direction to composite
				if (Yside == PLUS)
					y = yy;
				else
					y = (NY - 1) - yy;
				node v = Nodes[x][y][z];
				float s = v.s;
				rgb[0] = rgb[1] = rgb[2] = 1;
				switch (WhichColor) {
				case RAINBOW:
					rainbow_color(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case BWR:
					bwr(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case HEATMAP:
					heatmap(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case DISCRETE:
					discrete(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case NONLINEAR:
					nonlinear(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case WHITE:
					rgb[0] = rgb[1] = rgb[2] = 1;
					break;
				}
				Nodes[x][y][z].r = rgb[0];
				Nodes[x][y][z].g = rgb[1];
				Nodes[x][y][z].b = rgb[2];
				if (v.x >= x_min && v.x <= x_max && v.z >= z_min && v.z <= z_max && v.y >= y_min && v.y <= y_max && v.s >= t_min && v.s <= t_max && v.grad >= g_min && v.grad <= g_max) {
					r = Nodes[x][y][z].r;
					g = Nodes[x][y][z].g;
					b = Nodes[x][y][z].b;
					alpha = MaxAlpha;
				}
				else {
					
					r = g = b = 0;
					alpha = 0.0;
				}
				TextureXZ[yy][z][x][0] = (unsigned char)(255.*r + .5);
				TextureXZ[yy][z][x][1] = (unsigned char)(255.*g + .5);
				TextureXZ[yy][z][x][2] = (unsigned char)(255.*b + .5);
				TextureXZ[yy][z][x][3] = (unsigned char)(255.*alpha + .5);

			}
		}
	}
}

void CompositeYZ() {
	int x, y, z, xx;
	float alpha;
	float r, g, b;
	
	for (y = 0; y < NY; y++) {
		for (z = 0; z < NZ; z++) {
			r = g = b = 0;
			for (xx = 0; xx < NX; xx++) {
				//which direction to composite
				if (Xside == PLUS)
					x = xx;
				else
					x = (NX - 1) - xx;
				node v = Nodes[x][y][z];
				float s = v.s;
				rgb[0] = rgb[1] = rgb[2] = 1;
				switch (WhichColor) {
				case RAINBOW:
					rainbow_color(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case BWR:
					bwr(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case HEATMAP:
					heatmap(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case DISCRETE:
					discrete(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case NONLINEAR:
					nonlinear(s, TEMPMAX, TEMPMIN, rgb);
					break;
				case WHITE:
					rgb[0] = rgb[1] = rgb[2] = 1;
					break;
				}
			
				Nodes[x][y][z].r = rgb[0];
				Nodes[x][y][z].g = rgb[1];
				Nodes[x][y][z].b = rgb[2];
				if (v.x >= x_min && v.x <= x_max && v.z >= z_min && v.z <= z_max && v.y >= y_min && v.y <= y_max && v.s >= t_min && v.s <= t_max && v.grad >= g_min && v.grad <= g_max) {
					r = Nodes[x][y][z].r;
					g = Nodes[x][y][z].g;
					b = Nodes[x][y][z].b;
					alpha = MaxAlpha;
				}
				else {
					r = g = b = 0;
					alpha = 0.0;
				}
				TextureYZ[xx][z][y][0] = (unsigned char)(255.*r + .5);
				TextureYZ[xx][z][y][1] = (unsigned char)(255.*g + .5);
				TextureYZ[xx][z][y][2] = (unsigned char)(255.*b + .5);
				TextureYZ[xx][z][y][3] = (unsigned char)(255.*alpha + .5);

			}
		}
	}
}

void DetermineVisibility() {
	float xr, yr;
	float cx, sx;
	float cy, sy;
	float nzx, nzy, nzz;

	xr = Xrot * (M_PI / 180.);
	yr = Yrot * (M_PI / 180.);
	cx = cos(xr);
	sx = sin(xr);
	cy = cos(yr);
	sy = sin(yr);

	nzx = -sy;
	nzy = sx * cy;
	nzz = cx * cy;

	Xside = (nzx > 0. ? PLUS : MINUS);
	Yside = (nzy > 0. ? PLUS : MINUS);
	Zside = (nzz > 0. ? PLUS : MINUS);

	if (fabs(nzx) > fabs(nzy) && fabs(nzx) > fabs(nzz))
		Major = X;
	else if (fabs(nzy) > fabs(nzx) && fabs(nzy) > fabs(nzz))
		Major = Y;
	else
		Major = Z;
}

void Render_Texture() {
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	float filter = GL_NEAREST;
	if (Bilinear)
		filter = GL_LINEAR;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if (Major == Z) {
		float z0, dz, zcoord;
		int z;
		if (Zside == PLUS) {
			z0 = -1.;
			dz = 2. / (float)(NZ - 1);
		}
		else {
			z0 = 1.;
			dz = -2. / (float)(NZ - 1);
		}
		for (z = 0, zcoord = z0; z < NZ; z++, zcoord += dz) 
		{
			
			glTexImage2D(GL_TEXTURE_2D, 0, 4, NX, NY, 0, GL_RGBA, GL_UNSIGNED_BYTE, &TextureXY[z][0][0][0]);
			glBegin(GL_QUADS);
			glTexCoord2f(0., 0.);
			glVertex3f(-1., -1., zcoord);
			glTexCoord2f(1., 0.);
			glVertex3f(1., -1., zcoord);
			glTexCoord2f(1., 1.);
			glVertex3f(1., 1., zcoord);
			glTexCoord2f(0., 1.);
			glVertex3f(-1., 1., zcoord);
			glEnd();
		}
	}
	if (Major == Y) {
		float y0, dy, ycoord;
		int y;
		if (Yside == PLUS) {
			y0 = -1.;
			dy = 2. / (float)(NZ - 1);
		}
		else {
			y0 = 1.;
			dy = -2. / (float)(NY - 1);
		}
		
		for (y = 0, ycoord = y0; y < NY; y++, ycoord += dy) {
			
			glTexImage2D(GL_TEXTURE_2D, 0, 4, NZ, NX, 0, GL_RGBA, GL_UNSIGNED_BYTE, &TextureXZ[y][0][0][0]);
			glBegin(GL_QUADS);
			glTexCoord2f(0., 0.);
			glVertex3f(-1.,  ycoord, -1.);
			glTexCoord2f(1., 0.);
			glVertex3f(1., ycoord, -1.);
			glTexCoord2f(1., 1.);
			glVertex3f(1.,ycoord, 1. );
			glTexCoord2f(0., 1.);
			glVertex3f(-1., ycoord, 1.);
			glEnd();
		}
	}
	if (Major == X) {

		float x0, dx, xcoord;
		int x;
		if (Xside == PLUS) {
			x0 = -1.;
			dx = 2. / (float)(NX - 1);
		}
		else {
			x0 = 1.;
			dx = -2. / (float)(NX - 1);
		}
		for (x = 0, xcoord = x0; x < NX; x++, xcoord += dx) {	
			glTexImage2D(GL_TEXTURE_2D, 0, 4, NZ, NY, 0, GL_RGBA, GL_UNSIGNED_BYTE, &TextureYZ[x][0][0][0]);
			glBegin(GL_QUADS);
			glTexCoord2f(0., 0.);
			glVertex3f(xcoord ,-1., -1. );
			glTexCoord2f(1., 0.);
			glVertex3f(xcoord,1., -1.);
			glTexCoord2f(1., 1.);
			glVertex3f( xcoord, 1., 1.);
			glTexCoord2f(0., 1.);
			glVertex3f(xcoord ,-1., 1.);
			glEnd();
		}
	}
	glDisable(GL_TEXTURE_2D);
}
//extern PlyFile *in_ply;
//extern FILE *this_file;

//
// main program:
//

int
main( int argc, char *argv[] )
{
	unsigned int i, j ,k;
	float s;
	int p;
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	data_generator();
	gradient_calculation();

	InitGraphics();
	// create the display structures that will not change:
	InitLists();
	// init all the global variables used by Display():
	// this will also post a redisplay
	// it is important to call this before InitGlui()
	// so that the variables that glui will control are correct
	// when each glui widget is created
	Reset();
	// setup all the user interface stuff:
	InitGlui();
	// draw the scene once and wait for some interaction:
	// (will never return)
	glutMainLoop();
	// finalize the object if loaded
	// this is here to make the compiler happy:
	return 0;
}


//
// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display() from here -- let glutMainLoop() do it
//

void
Animate( void )
{
	// put animation stuff in here -- change some global variables
	// for Display() to find:
	// force a call to Display() next time it is convenient:
	glutSetWindow( MainWindow );
	glutPostRedisplay();
}

//
// glui buttons callback:
//

void
Buttons( int id )
{
	switch( id )
	{
		case RESET:
			Reset();
			Glui->sync_live();
			glutSetWindow( MainWindow );
			glutPostRedisplay();
			break;

		case QUIT:
			// gracefully close the glui window:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:

			Glui->close();
			glutSetWindow( MainWindow );
			glFinish();
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Button ID %d\n", id );
	}

}

//
// draw the complete scene:
//

void
Display( void )
{
	GLsizei vx, vy, v;		// viewport dimensions
	GLint xl, yb;		// lower-left corner of viewport
	GLfloat scale2;		// real glui scale factor

	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}

	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );

	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	glEnable(GL_COLOR_MATERIAL);
	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to a square centered in the window:

	vx = glutGet( GLUT_WINDOW_WIDTH );
	vy = glutGet( GLUT_WINDOW_HEIGHT );
	v = vx < vy ? vx : vy;			// minimum dimension
	xl = ( vx - v ) / 2;
	yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );

	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D() IF YOU ARE DOING 2D !
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );

	// place the objects into the scene:
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !

	gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );

	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !
	glTranslatef( (GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2] );

	// rotate the scene:
	// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !
	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );
	glMultMatrixf( (const GLfloat *) RotMatrix );

	// uniformly scale the scene:

	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );
	scale2 = 1. + Scale2;		// because glui translation starts at 0.
	if( scale2 < MINSCALE )
		scale2 = MINSCALE;
	glScalef( (GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2 );

	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// Let us disable lighting right now
	glDisable(GL_LIGHTING);

	// possibly draw the axes:
	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[0][0] );
		glCallList( AxesList );
	}

	// set the color of the object:

	//glColor3fv( Colors[0] );

	// Render the loaded object
	set_view(GL_RENDER);
	
	glTranslatef(0.0, 0.0, -3.0);

	glTranslatef( (GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2] );
	
	//glTranslatef(poly->center.entry[0], poly->center.entry[1], poly->center.entry[2]);

	glScalef( (GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2 );
	
	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );
	glMultMatrixf( (const GLfloat *) RotMatrix );
	
	//glScalef(1.0/poly->radius, 1.0/poly->radius, 1.0/poly->radius);
	//glTranslatef(-poly->center.entry[0], -poly->center.entry[1], -poly->center.entry[2]);

	display_shape(GL_RENDER);

	// swap the double-buffered framebuffers:
	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush() here, not glFinish() !
	glFlush();
}

//
// use glut to display a string of characters using a raster font:
//

void
DoRasterString( float x, float y, float z, char *s )
{
	char c;			// one character to print

	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


//
// use glut to display a string of characters using a stroke font:
//

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	char c;			// one character to print
	float sf;		// the scale factor

	glPushMatrix();
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		sf = ht / ( 119.05 + 33.33 );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix();
}

//
// return the number of seconds since the start of the program:
//

float
ElapsedSeconds( void )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.;
}

//
// initialize the glui window:
//

void
InitGlui( void )
{
	GLUI_Panel *panel;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;

	GLUI_Spinner *spinner;
	// setup the glui window:

	glutInitWindowPosition( INIT_WINDOW_SIZE + 50, 0 );
	Glui = GLUI_Master.create_glui( (char *) GLUITITLE );


	Glui->add_statictext( (char *) GLUITITLE );
	Glui->add_separator();

	Glui->add_checkbox( "Axes", &AxesOn );

	Glui->add_checkbox( "Perspective", &WhichProjection );

	Glui->add_checkbox( "Intensity Depth Cue", &DepthCueOn );

	spinner = new GLUI_Spinner(Glui, "White Threshold value:", &white_range);
	spinner->set_int_limits(0, 1);

	spinner = new GLUI_Spinner(Glui, "iso value value:", &iso_value);
	spinner->set_int_limits(0,100);

	panel = Glui->add_panel("Volume Rendering");
	Glui->add_checkbox_to_panel(panel, "VolumeRendering", &volumeRendering);
	
	Glui->add_checkbox_to_panel(panel, "Bilinear", &Bilinear);

	spinner = new GLUI_Spinner(Glui, "alpha value:", &MaxAlpha);
	spinner->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);

	Glui->add_separator();
	panel = Glui->add_panel("Cutting Plane");
	Glui->add_checkbox_to_panel(panel,"XY", &XYplane);
	Glui->add_column_to_panel(panel, GLUIFALSE);
	Glui->add_checkbox_to_panel(panel,"YZ", &YZplane);
	Glui->add_column_to_panel(panel, GLUIFALSE);
	Glui->add_checkbox_to_panel(panel,"ZX", &ZXplane);
	Glui->add_column_to_panel(panel, GLUIFALSE);
	Glui->add_checkbox_to_panel(panel,"wireframe", &wireframe);

	Glui->add_separator();

	//Glui->add_spinner_to_panel(panel, "z min value:",z_min);
	//spinner->set_float_limits(-1.0, 1.0, GLUI_LIMIT_CLAMP);
	spinner = new GLUI_Spinner(Glui, "z min value:", &z_min);
	spinner->set_float_limits(-1.0, 1.0, GLUI_LIMIT_CLAMP);
	
	spinner = new GLUI_Spinner(Glui, "z max value:", &z_max);
	spinner->set_float_limits(-1.0, 1.0, GLUI_LIMIT_CLAMP);

	Glui->add_separator();

	spinner = new GLUI_Spinner(Glui, "y min value:", &y_min);
	spinner->set_float_limits(-1.0, 1.0, GLUI_LIMIT_CLAMP);

	spinner = new GLUI_Spinner(Glui, "y max value:", &y_max);
	spinner->set_float_limits(-1.0, 1.0, GLUI_LIMIT_CLAMP);

	Glui->add_separator();

	spinner = new GLUI_Spinner(Glui, "x min value:", &x_min);
	spinner->set_float_limits(-1.0, 1.0, GLUI_LIMIT_CLAMP);

	spinner = new GLUI_Spinner(Glui, "x max value:", &x_max);
	spinner->set_float_limits(-1.0, 1.0, GLUI_LIMIT_CLAMP);

	Glui->add_separator();

	spinner = new GLUI_Spinner(Glui, "t max value:", &t_max);
	spinner->set_int_limits(0, TEMPMAX, GLUI_LIMIT_CLAMP);

	spinner = new GLUI_Spinner(Glui, "t min value:", &t_min);
	spinner->set_int_limits(0, TEMPMAX, GLUI_LIMIT_CLAMP);

	Glui->add_separator();
	
	spinner = new GLUI_Spinner(Glui, "gradient max value:", &g_max);
	spinner->set_int_limits(grad_min, grad_max, GLUI_LIMIT_CLAMP);

	spinner = new GLUI_Spinner(Glui, "gradient min value:", &g_min);
	spinner->set_int_limits(grad_min, grad_max, GLUI_LIMIT_CLAMP);

	Glui->add_separator();

	spinner = new GLUI_Spinner(Glui, "x value:", &x_value);
	spinner->set_int_limits(0, NX-1, GLUI_LIMIT_CLAMP);

	spinner = new GLUI_Spinner(Glui, "y value:", &y_value);
	spinner->set_int_limits(0, NY-1, GLUI_LIMIT_CLAMP);

	spinner = new GLUI_Spinner(Glui, "z value:", &z_value);
	spinner->set_int_limits(0, NZ-1, GLUI_LIMIT_CLAMP);

	Glui->add_separator();

	/*
	spinner = new GLUI_Spinner(Glui, "number of contours:", &contours);
	spinner->set_int_limits(1, 10);
   */

	// Add a rollout for the axes color
	GLUI_Rollout *rollout = Glui->add_rollout("Color Map ", 0);

	panel = Glui->add_panel(  "Axes Color" );
	//GLUI_Rollout *rollout = Glui->add_rollout_to_panel(panel,  "Axes Color", 1 );
		group = Glui->add_radiogroup_to_panel( panel, &WhichColor );
	
		group = Glui->add_radiogroup_to_panel( rollout, &WhichColor );
		Glui->add_radiobutton_to_group(group, "Rainbow");
		Glui->add_radiobutton_to_group(group, "BWR");
		Glui->add_radiobutton_to_group(group, "HeatMap");
		Glui->add_radiobutton_to_group(group, "Discrete");
		Glui->add_radiobutton_to_group(group, "NonLinear");
		Glui->add_radiobutton_to_group(group, "White");


		rot = Glui->add_rotation_to_panel( panel, "Rotation", (float *) RotMatrix );
		// allow the object to be spun via the glui rotation widget:
		rot->set_spin( 1.0 );
		Glui->add_column_to_panel( panel, GLUIFALSE );
		scale = Glui->add_translation_to_panel( panel, "Scale",  GLUI_TRANSLATION_Y , &Scale2 );
		scale->set_speed( 0.005f );

		Glui->add_column_to_panel( panel, GLUIFALSE );
		trans = Glui->add_translation_to_panel( panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0] );
		trans->set_speed( 0.05f );

		Glui->add_column_to_panel( panel, GLUIFALSE );
		trans = Glui->add_translation_to_panel( panel, "Trans Z",  GLUI_TRANSLATION_Z , &TransXYZ[2] );
		trans->set_speed( 0.05f );

	Glui->add_checkbox( "Debug", &DebugOn );


	panel = Glui->add_panel( "", GLUIFALSE );

	Glui->add_button_to_panel( panel, "Reset", RESET, (GLUI_Update_CB) Buttons );

	Glui->add_column_to_panel( panel, GLUIFALSE );

	Glui->add_button_to_panel( panel, "Quit", QUIT, (GLUI_Update_CB) Buttons );


	// tell glui what graphics window it needs to post a redisplay to:

	Glui->set_main_gfx_window( MainWindow );


	// set the graphics window's idle function:

	GLUI_Master.set_glutIdleFunc( NULL );
}


//
// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
//

void
InitGraphics( void )
{
	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow() )
	// ask for color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );


	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );


	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );


	// setup the clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );


	// setup the callback routines:


	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	//glutDisplayFunc( Display_Model );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( 0, NULL, 0 );

	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	// glutIdleFunc( NULL );
	// let glui take care of it in InitGlui()
}


//
// initialize the display lists that will not change:
//

void
InitLists( void )
{
	float dx = BOXSIZE / 2.;
	float dy = BOXSIZE / 2.;
	float dz = BOXSIZE / 2.;

	// create the object:

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );

		glBegin( GL_QUADS );

			glColor3f( 0., 0., 1. );
			glNormal3f( 0., 0.,  1. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f( -dx,  dy,  dz );

			glNormal3f( 0., 0., -1. );
				glTexCoord2f( 0., 0. );
				glVertex3f( -dx, -dy, -dz );
				glTexCoord2f( 0., 1. );
				glVertex3f( -dx,  dy, -dz );
				glTexCoord2f( 1., 1. );
				glVertex3f(  dx,  dy, -dz );
				glTexCoord2f( 1., 0. );
				glVertex3f(  dx, -dy, -dz );

			glColor3f( 1., 0., 0. );
			glNormal3f(  1., 0., 0. );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f(  dx,  dy,  dz );

			glNormal3f( -1., 0., 0. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f( -dx,  dy, -dz );
				glVertex3f( -dx, -dy, -dz );

			glColor3f( 0., 1., 0. );
			glNormal3f( 0.,  1., 0. );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f( -dx,  dy, -dz );

			glNormal3f( 0., -1., 0. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx, -dy, -dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx, -dy,  dz );

		glEnd();

	glEndList();


	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList();
}


//
// the keyboard callback:
//

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			Buttons( QUIT );	// will not return here
			break;			// happy compiler

		case 'r':
		case 'R':
			LeftButton = ROTATE;
			break;

		case 's':
		case 'S':
			LeftButton = SCALE;
			break;

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}


	// synchronize the GLUI display with the variables:

	Glui->sync_live();


	// force a call to Display():

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}


//
// called when the mouse button transitions down or up:
//

void
MouseButton( int button, int state, int x, int y )
{
	int b;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}

//
// called when the mouse moves while a button is down:
//

void
MouseMotion( int x, int y )
{
	int dx, dy;		// change in mouse coordinates

	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	dx = x - Xmouse;		// change in mouse coords
	dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		switch( LeftButton )
		{
			case ROTATE:
				Xrot += ( ANGFACT*dy );
				Yrot += ( ANGFACT*dx );
				break;

			case SCALE:
				Scale += SCLFACT * (float) ( dx - dy );
				if( Scale < MINSCALE )
					Scale = MINSCALE;
				break;
		}
	}


	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}

//
// reset the transformations and the colors:
//
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
//

void
Reset( void )
{
	ActiveButton = 0;
	AxesOn = GLUITRUE;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	Scale  = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display()
	WhichColor = RAINBOW;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;

	                  RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0]                   = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1]                   = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3]                   = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;
}



//
// called when user resizes the window:
//

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display():

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}


//
// handle a change to the window's visibility:
//

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




//////////////////////////////////////////  EXTRA HANDY UTILITIES:  /////////////////////////////

// size of wings as fraction of length:

#define WINGS	0.10


// axes:

#define X	1
#define Y	2
#define Z	3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void
Arrow( float tail[3], float head[3] )
{
	float u[3], v[3], w[3];		// arrow coordinate system
	float d;			// wing distance
	float x, y, z;			// point to plot
	float mag;			// magnitude of major direction
	float f;			// fabs of magnitude
	int axis;			// which axis is the major


	// set w direction in u-v-w coordinate system:

	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];


	// determine major direction:

	axis = X;
	mag = fabs( w[0] );
	if( (f=fabs(w[1]))  > mag )
	{
		axis = Y;
		mag = f;
	}
	if( (f=fabs(w[2]))  > mag )
	{
		axis = Z;
		mag = f;
	}


	// set size of wings and turn w into a Unit vector:

	d = WINGS * Unit( w, w );


	// draw the shaft of the arrow:

	glBegin( GL_LINE_STRIP );
		glVertex3fv( tail );
		glVertex3fv( head );
	glEnd();

	// draw two sets of wings in the non-major directions:

	if( axis != X )
	{
		Cross( w, axx, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
	}


	if( axis != Y )
	{
		Cross( w, ayy, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
	}



	if( axis != Z )
	{
		Cross( w, azz, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
	}
}



float
Dot( float v1[3], float v2[3] )
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}



void
Cross( float v1[3], float v2[3], float vout[3] )
{
	float tmp[3];

	tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
	tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
	tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
Unit( float vin[3], float vout[3] )
{
	float dist, f ;

	dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];

	if( dist > 0.0 )
	{
		dist = sqrt( dist );
		f = 1. / dist;
		vout[0] = f * vin[0];
		vout[1] = f * vin[1];
		vout[2] = f * vin[2];
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}



// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[] = {
		1, 2, -3, 4
		};


static float yx[] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[] = {
		1, 2, 3, -2, 4
		};


static float zx[] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[] = {
		1, 2, 3, 4, -5, 6
		};


// fraction of the length to use as height of the characters:

const float LENFRAC = 0.10f;


// fraction of length to use as start location of the characters:

const float BASEFRAC = 1.10f;


//
//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)
//

void
Axes( float length )
{
	int i, j;			// counters
	float fact;			// character scale factor
	float base;			// character start location


	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd();
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd();

	fact = LENFRAC * length;
	base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( i = 0; i < 4; i++ )
		{
			j = xorder[i];
			if( j < 0 )
			{
				
				glEnd();
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd();

	glBegin( GL_LINE_STRIP );
		for( i = 0; i < 5; i++ )
		{
			j = yorder[i];
			if( j < 0 )
			{
				
				glEnd();
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd();

	glBegin( GL_LINE_STRIP );
		for( i = 0; i < 6; i++ )
		{
			j = zorder[i];
			if( j < 0 )
			{
				
				glEnd();
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd();

}




//
// routine to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//		"Computer Graphics Principles and Practices,"
//		Additon-Wesley, 1990, pp592-593.


void
HsvRgb( float hsv[3], float rgb[3] )
{
	float h, s, v;			// hue, sat, value
	float r, g, b;			// red, green, blue
	float i, f, p, q, t;		// interim values


	// guarantee valid input:

	h = hsv[0] / 60.;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;


	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}


	// get an rgb from the hue itself:
	
	i = floor( h );
	f = h - i;
	p = v * ( 1. - s );
	q = v * ( 1. - s*f );
	t = v * ( 1. - ( s * (1.-f) ) );

	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

// Set the view and the lighting properties
//

void set_view(GLenum mode)
{
	icVector3 up, ray, view;
	GLfloat light_ambient0[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat light_diffuse0[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat light_specular0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_ambient1[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse1[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_specular1[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_ambient2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular1);


    glMatrixMode(GL_PROJECTION);
	if (mode == GL_RENDER)
		glLoadIdentity();

	if (WhichProjection == ORTHO)
		glOrtho(-radius_factor, radius_factor, -radius_factor, radius_factor, 0.0, 40.0);
	else
		gluPerspective(45.0, 1.0, 0.1, 40.0);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	light_position[0] = 5.5;
	light_position[1] = 0.0;
	light_position[2] = 0.0;
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	light_position[0] = -0.1;
	light_position[1] = 0.0;
	light_position[2] = 0.0;
	glLightfv(GL_LIGHT2, GL_POSITION, light_position);
}

//
// Set the scene for the object based on the center of the object

void set_scene(GLenum mode)
{
	glTranslatef(0.0, 0.0, -3.0);
}


//
// OpenGL rendering of the object
void display_shape(GLenum mode)
{
	unsigned int i, j, k,p;
	float s;
	GLfloat mat_diffuse[4];

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1., 1.);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Set the material property (esp. color) of the object
	//mat_diffuse[0] = 0.6;
	//mat_diffuse[1] = 0.8;
	//mat_diffuse[2] = 0.7;
	//mat_diffuse[3] = 1.0;
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	if (XYplane != 0) {
		planexy();
	}
	if (YZplane != 0) {
		planeyz();
	}
	if (ZXplane != 0) {
		planezx();
	}
	if (wireframe != 0) {
		draw_wireframe();
	}		
	if (volumeRendering != 0) {
		DetermineVisibility();
		CompositeXY();
		CompositeXZ();
		CompositeYZ();
		Render_Texture();
	}
}


void Display_Model(void)
{
	GLint viewport[4];
	int jitter;

	glClearColor (1.0, 1.0, 1.0, 1.0);  // background for rendering color coding and lighting
	glGetIntegerv (GL_VIEWPORT, viewport);
 
	set_view(GL_RENDER);
	set_scene(GL_RENDER);
	display_shape(GL_RENDER);

	glFlush();
	glutSwapBuffers();
	glFinish();
}


void Choose_Object()
{
	Glui->sync_live();
	glutSetWindow( MainWindow );
	glutPostRedisplay(); 
}