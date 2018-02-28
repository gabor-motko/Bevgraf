#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>

#pragma region GLOBALS

#define STARTSPEED 150
#define MAXSPEED 500

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

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();	//0: LBM, 1: wheel, 2: RMB, 3: up, 4: down
bool * const mousePreviousStates = new bool[5]();

double currentTime, previousTime, delta;

#pragma endregion

#pragma region SHAPES

//--- CIRCLE
//Kör rajzolása üres vagy teli sokszögként
void drawSemicircle(vec2 pos, float radius, float arc, float rotation, bool sector, bool filled, int detail = 18)
{
	float arcRad = degToRad(arc);
	if (filled)
		glBegin(GL_POLYGON);
	else
		glBegin(GL_LINE_LOOP);
	for (double t = 0; t <= arcRad; t += arcRad / detail)
		glVertex2d(pos.x + radius * cos(t), pos.y + radius * sin(t));
	if (sector)
		glVertex2d(pos.x, pos.y);
	glEnd();
}

void drawCircle(vec2 pos, float radius, bool filled, int detail = 18)
{
	drawSemicircle(pos, radius, 2 * pi(), 0, filled, detail);
}

void drawEllipse(vec2 f1, vec2 f2, float a, float b, bool filled, int detail)
{
	if (filled)
		glBegin(GL_POLYGON);
	else
		glBegin(GL_LINE_LOOP);
	glEnd();
}



#pragma endregion

#pragma region INPUT

void mouseButtonPressed(int button, int state, int x, int y)
{
	mouseStates[button] = !state;
	if (state == GLUT_DOWN)
	{

	}
	if (state == GLUT_UP)
	{

	}
}

void mouseMove(int x, int y)
{
	if (mouseStates[GLUT_LEFT_BUTTON])
	{

	}
}

void keyOps(int val)
{
	if (keyStates['x'])
		exit(0);

	memcpy(keyPreviousStates, keyStates, 256 * sizeof(bool));
	memcpy(mousePreviousStates, mouseStates, 5 * sizeof(bool));
	glutPostRedisplay();
	glutTimerFunc(5, keyOps, 0);
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
	currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	delta = currentTime - previousTime;
	previousTime = currentTime;
}

void init()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0f, win.width, 0.0f, win.height);

	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

}
void drawScene()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0, 0.0, 0.0);

	//Process physics
	getDelta();

	//Draw
	drawSemicircle(vec2(200, 200), 100, 180, 0, true, false);

	glutSwapBuffers();
}

#pragma endregion

int main(int argc, char * argv[])
{
	srand(time(NULL));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(50, 100);
	glutInitWindowSize(win.width, win.height);
	glutCreateWindow(argv[0]);

	init();
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutMouseFunc(mouseButtonPressed);
	glutMotionFunc(mouseMove);
	glutTimerFunc(5, keyOps, 0);
	glutMainLoop();
}