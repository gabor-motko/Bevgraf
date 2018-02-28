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
typedef struct TArea
{
	GLint left = 50;
	GLint right = 750;
	GLint bottom = 50;
	GLint top = 550;
	TArea(int l, int r, int b, int t)
	{
		left = l;
		right = r;
		top = t;
		bottom = b;
	}
} AREA;
WINDOW win = { 800, 600 };
AREA area = { 50, 750, 50, 550 };


#define HANDLE_BOTTOMLEFT 0
#define HANDLE_TOPRIGHT 1
#define HANDLE_P1 2
#define HANDLE_P2 3
vec2 * points = new vec2[4]();
int selectedPoint = -1;
int handleRadius = 5;

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();	//0: LBM, 1: wheel, 2: RMB, 3: up, 4: down
bool * const mousePreviousStates = new bool[5]();

double currentTime, previousTime, delta;


#pragma endregion

#pragma region SHAPES

//--- CIRCLE
//Kör rajzolása üres vagy teli sokszögként
void drawCircle(int cx, int cy, int _radius, bool filled, int detail = 18)
{
	if (filled)
		glBegin(GL_POLYGON);
	else
		glBegin(GL_LINE_LOOP);
	for (double t = 0; t <= 2 * pi(); t += pi() / detail)
		glVertex2d(cx + _radius * cos(t), cy + _radius * sin(t));
	glEnd();
}

//vec2 overload
void drawCircle(vec2 pos, int radius, bool filled, int detail = 18)
{
	drawCircle(pos.x, pos.y, radius, filled, detail);
}

class Circle
{
public:
	vec2 pos = { 0, 0 };
	vec2 vel = { 0, 0 };
	int radius = 20;
	int detail = 18;
	Circle()
	{
		srand(time(NULL));
		vel = normalize(vec2(rand(), rand())) * STARTSPEED;
		printf("Velocity: %d, %d\n", vel.x, vel.y);
		pos.x = rand() % ((area.right - radius / 2) - (area.left + radius / 2) + 1) + area.left;
		pos.y = rand() % ((area.top - radius / 2) - (area.bottom + radius / 2) + 1) + area.bottom;
	}
	Circle(int radius, int detail = 18)
	{
		this->radius = radius;
		this->detail = detail;
		Circle();
	}
	vec2 speed()
	{
		return length(vel);
	}
	vec2 direction()
	{
		return normalize(vel);
	}
	void draw()
	{
		drawCircle(pos, radius, false);
	}
} ball;

//--- BOUNDARIES
void drawBoundaries()
{
	glBegin(GL_LINE_LOOP);
	glVertex2i(area.left, area.bottom);
	glVertex2i(area.left, area.top);
	glVertex2i(area.right, area.top);
	glVertex2i(area.right, area.bottom);
	glEnd();
	drawCircle(points[HANDLE_BOTTOMLEFT], handleRadius, true, 8);
	drawCircle(points[HANDLE_TOPRIGHT], handleRadius, true, 8);
}

//--- LINES
void drawLineByPoints(vec2 p1, vec2 p2)
{
	drawCircle(p1, handleRadius, true, 8);
	drawCircle(p2, handleRadius, true, 8);
	glBegin(GL_LINES);
	glVertex2i(p1.x, p1.y);
	glVertex2i(p2.x, p2.y);
	glEnd();
}

#pragma endregion

#pragma region INPUT

void movePoint(int index, vec2 pos)
{
	points[selectedPoint] = pos;
	switch (index)
	{
	case HANDLE_BOTTOMLEFT:
		area.left = pos.x;
		area.bottom = pos.y;
		break;
	case HANDLE_TOPRIGHT:
		area.right = pos.x;
		area.top = pos.y;
		break;
	case HANDLE_P1:
		break;
	case HANDLE_P2:
		break;
	default:
		break;
	}
}

void mouseButtonPressed(int button, int state, int x, int y)
{
	mouseStates[button] = !state;
	if (state == GLUT_DOWN)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (dist(points[i], vec2(x, win.height - y)) < handleRadius)
			{
				selectedPoint = i;
			}
		}
	}
	if (state == GLUT_UP)
	{
		selectedPoint = -1;
	}
}

void mouseMove(int x, int y)
{
	if (mouseStates[0])
	{
		movePoint(selectedPoint, vec2(x, win.height - y));
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
	ball.pos = vec2(200, 200);
	points[HANDLE_BOTTOMLEFT] = vec2(area.left, area.bottom);
	points[HANDLE_TOPRIGHT] = vec2(area.right, area.top);
	points[HANDLE_P1] = vec2((rand() % (area.right - area.left + 1) + area.left), (rand() % (area.top - area.bottom + 1) + area.bottom));
	points[HANDLE_P2] = vec2((rand() % (area.right - area.left + 1) + area.left), (rand() % (area.top - area.bottom + 1) + area.bottom));
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

	//ball.pos += ball.vel * delta;

	//Draw the boundaries
	drawBoundaries();
	ball.draw();
	drawLineByPoints(points[HANDLE_P1], points[HANDLE_P2]);

	glutSwapBuffers();
}

#pragma endregion

int main(int argc, char * argv[])
{
	srand(time(NULL));

	printf("left: %d\nright: %d\ntop: %d\nbottom: %d\n", area.left, area.right, area.top, area.bottom);

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