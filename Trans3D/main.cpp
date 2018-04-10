#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
#include <string>
#include <vector>
#include <fstream>

void initVectors();

#pragma region GLOBALS

typedef struct TWindow
{
	GLsizei width = 800;
	GLsizei height = 600;
	TWindow(int w, int h)
	{
		width = w;
		height = h;
	}
} WINDOW;
WINDOW win = { 800, 600 };

struct
{
	vec3 black = vec3(0.0f, 0.0f, 0.0f);
	vec3 darkblue = vec3(0.0f, 0.0f, 0.7f);
	vec3 darkred = vec3(0.7f, 0.0f, 0.0f);
	vec3 red = vec3(1.0f, 0.2f, 0.0f);
	vec3 orange = vec3(1.0f, 0.5f, 0.0f);
	vec3 yellow = vec3(1.0f, 1.0f, 0.0f);
	vec3 blue = vec3(0.0f, 0.0f, 1.0f);
	vec3 gray = vec3(0.35f, 0.35f, 0.35f);
	vec3 sky = vec3(0.0f, 0.5f, 1.0f);
	vec3 ground = vec3(0.0f, 0.6f, 0.0f);
} colors;

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();
bool * const mousePreviousStates = new bool[5]();


double delta, currentTime, previousTime;
float visualize_t = 0.0f;

std::vector<vec3> points;
std::vector<int[]> edges;

#pragma endregion

#pragma region INPUT

bool keyPress(int key)
{
	return keyStates[key] && !keyPreviousStates[key];
}

bool mousePress(int button)
{
	return mouseStates[button] && !mousePreviousStates[button];
}

void keyProcess(int x)
{
	if (keyStates['x'])
	{
		exit(0);
	}
	if (keyStates['w'])
	{
	}
	if (keyStates[GLUT_KEY_LEFT])
	{
		if (visualize_t > 0.0f)
		{
			visualize_t -= 0.5f * delta;
			if (visualize_t < 0.0f)
				visualize_t = 0.0f;
		}
	}
	if (keyStates[GLUT_KEY_RIGHT])
	{
		if (visualize_t < 1.0f)
		{
			visualize_t += 0.5f * delta;
			if (visualize_t > 1.0f)
				visualize_t = 1.0f;
		}
	}

	memcpy(keyPreviousStates, keyStates, 256 * sizeof(bool));
	memcpy(mousePreviousStates, mouseStates, 5 * sizeof(bool));

	glutPostRedisplay();
	glutTimerFunc(10, keyProcess, 0);
}

void keyDown(unsigned char key, int x, int y)
{
	keyStates[key] = true;
}

void keyDown(int key, int x, int y)
{
	keyStates[key] = true;
}

void keyUp(unsigned char key, int x, int y)
{
	keyStates[key] = false;
}

void keyUp(int key, int x, int y)
{
	keyStates[key] = false;
}

#pragma endregion

#pragma region SYSTEM

void getDelta()
{
	currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	delta = currentTime - previousTime;
	previousTime = currentTime;
}

void display()
{
	getDelta();
	glClear(GL_COLOR_BUFFER_BIT);

	vec3 * ppoints = new vec3[8];
	mat4 tr = windowToViewport3(vec2(-2, -2), vec2(4, 4), vec2(), vec2(win.width, win.height)) * perspective(1.0);

	glBegin(GL_POINTS);
	for (int i = 0; i < points.size(); ++i)
	{
		ppoints[i] = hToIh(tr * points[i]);
		glVertex2f(ppoints[i].x, ppoints[i].y);
	}
	glEnd();

	glBegin(GL_LINES);
	for (int i = 0; i < edges.size(); ++i)
	{
	}
	glEnd();

	glutSwapBuffers();
}

void init()
{
	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	glClearColor(colors.sky.x, colors.sky.y, colors.sky.z, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, win.width, 0, win.height);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
}

void initVectors()
{
	points.push_back(vec3(1.0, 1.0, 1.0));		//0
	points.push_back(vec3(1.0, 1.0, -1.0));		//1
	points.push_back(vec3(1.0, -1.0, 1.0));		//2
	points.push_back(vec3(1.0, -1.0, -1.0));	//3
	points.push_back(vec3(-1.0, 1.0, 1.0));		//4
	points.push_back(vec3(-1.0, 1.0, -1.0));	//5
	points.push_back(vec3(-1.0, -1.0, 1.0));	//6
	points.push_back(vec3(-1.0, -1.0, -1.0));	//7
	edges.push_back({ 0, 1 });
	edges.push_back({ 0, 2 });
	edges.push_back({ 1, 3 });
	edges.push_back({ 2, 3 });
	edges.push_back({ 4, 5 });
	edges.push_back({ 4, 6 });
	edges.push_back({ 5, 7 });
	edges.push_back({ 7, 6 });
	edges.push_back({ 0, 4 });
	edges.push_back({ 1, 5 });
	edges.push_back({ 2, 6 });
	edges.push_back({ 3, 7 });
}

int main(int argc, char * argv[])
{
	initVectors();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
	glutInitWindowSize(win.width, win.height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutSpecialFunc(keyDown);
	glutSpecialUpFunc(keyUp);

	glutTimerFunc(10, keyProcess, 0);

	glutMainLoop();
	return 0;
}

#pragma endregion