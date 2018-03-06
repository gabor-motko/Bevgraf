#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
#include <vector>

#pragma region GLOBALS

#define STARTSPEED 150
#define MAXSPEED 500
#define MINSPEED 10
#define ACCELERATION 200

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
AREA area = { 0, 800, 0, 600 };
typedef struct TMouse
{
	int x, y;
	TMouse(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
} MOUSE;
MOUSE mouse;

//vec2 * points = new vec2[4]();
std::vector<vec2> points;
int selectedPoint = -1;
float handleRadius = 5;

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();	//0: LBM, 1: wheel, 2: RMB, 3: up, 4: down
bool * const mousePreviousStates = new bool[5]();

double currentTime, previousTime, delta;
bool visualize = false;


#pragma endregion

#pragma region GEOMETRY

//2x2 mátrix determinánsa
float det2x2(float a, float b, float c, float d)
{
	return a * d - b * c;
}

//p1-p2 egyenes normálvektora
vec2 getNormal(vec2 p1, vec2 p2, bool unit = true)
{
	vec2 dir = p2 - p1;
	if (unit)
		return normalize(vec2(dir.y, -dir.x));
	else
		return vec2(dir.y, -dir.x);
}

//--- CIRCLE
//Kör rajzolása üres vagy teli sokszögként
void drawCircle(vec2 pos, float _radius, bool filled, int detail = 18)
{
	if (filled)
		glBegin(GL_POLYGON);
	else
		glBegin(GL_LINE_LOOP);
	for (double t = 0; t <= 2 * pi(); t += pi() / detail)
		glVertex2d(pos.x + _radius * cos(t), pos.y + _radius * sin(t));
	glEnd();
}

#pragma endregion

#pragma region INPUT

void mouseButtonPressed(int button, int state, int x, int y)
{
	mouseStates[button] = !state;
	mouse.x = x;
	mouse.y = win.height - y;
}

void mouseMove(int x, int y)
{
	mouse.x = x;
	mouse.y = win.height - y;
}

void keyOps(int val)
{
	if (keyStates['x'])
		exit(0);

	if (mouseStates[GLUT_LEFT_BUTTON] == GLUT_DOWN && mousePreviousStates[GLUT_LEFT_BUTTON] == GLUT_UP)
	{
		for (int i = 0; i < points.size() && points.size() > 0; ++i)
		{

		}
	}

	memcpy(keyPreviousStates, keyStates, 256 * sizeof(bool));
	memcpy(mousePreviousStates, mouseStates, 5 * sizeof(bool));

	glutPostRedisplay();
	glutTimerFunc(5, keyOps, 0);
}

void keyDown(unsigned char key, int x, int y)
{
	keyStates[key] = true;
}

void keyUp(unsigned char key, int x, int y)
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

	//Fizika
	getDelta();
	ball.pos += ball.vel * delta;
	checkBounces();

	//Objektumok kirajzolása

	glutSwapBuffers();
}

#pragma endregion

int main(int argc, char * argv[])
{
	for (int i = 0; argc > 1 && argv[1][i] != '\0'; ++i)
	{
		if (argv[1][i] == 'v')
			visualize = true;
	}
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