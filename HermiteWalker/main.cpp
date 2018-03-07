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
WINDOW win = { 800, 600 };
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
AREA area = { 0, 800, 0, 600 };
typedef struct TMouse
{
	int x, y;
	TMouse(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
	vec2 pos()
	{
		return vec2(x, y);
	}
} MOUSE;
MOUSE mouse = { 0, 0 };

//vec2 * points = new vec2[4]();
std::vector<vec2> points;
int selectedPoint = -1;
float handleRadius = 5.0f;

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();	//0: LBM, 1: wheel, 2: RMB, 3: up, 4: down
bool * const mousePreviousStates = new bool[5]();

double currentTime, previousTime, delta;
bool visualize = false;

#define ANIM_OFF 0
#define ANIM_READY 1
#define ANIM_PLAYING 2
#define ANIM_FINISHED 3
#define ANIM_PAUSED 4
int animStatus = ANIM_OFF;
float animT = 0.0f;
float animEnd = 1.0f;
float animStep = 0.1f;

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

//Hermite-ív
class HermiteCurve
{
public:
	int detail = 48;
	mat24 G;
	mat4 M;
	float t0 = -1.0f;
	float t1 = 0.0f;
	float t2 = 1.0f;
	float t3 = 2.0f;
	vec4 tVector(float t, bool tangent = false)
	{
		if (tangent)
			return vec4(3 * pow(t, 2), 2 * t, 1, 0);
		return vec4(pow(t, 3), pow(t, 2), t, 1);
	}
	vec2 point(float t, bool tangent = false)
	{
		vec4 T = tVector(t, tangent);
		return G * M * T;
	}
	void draw(vec2 p1, vec2 p2, vec2 p3, vec2 p4)
	{
		this->G = mat24(p1, p2, p3, p4);
		this->M = inverse(mat4(tVector(t0), tVector(t1), tVector(t2), tVector(t3), true));
		glBegin(GL_LINE_STRIP);
		for (float t = t0; t <= t3; t += 1.0f / detail)
		{
			vec2 v = this->point(t);
			glVertex2f(v.x, v.y);
		}
		glEnd();
	}
};
HermiteCurve curve;

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

#pragma region GAME

//--- ANIMÁCIÓ
void drawAnimation(float t)
{
	float ts = t * (curve.t3 - curve.t0) + curve.t0;
	vec2 pos = curve.point(ts, false);
	vec2 dir = normalize(curve.point(ts, true));
	vec2 nrm = vec2(-dir.y, dir.x);
	glColor3f(0.0f, 1.0f, 0.0f);
	drawCircle(pos, 3, true, 6);
	glBegin(GL_LINES);
	glVertex2f(pos.x, pos.y);
	glVertex2f(pos.x + nrm.x * 20, pos.y + nrm.y * 20);
	glEnd();
}

void processAnimation()
{
	if (animStatus != ANIM_OFF)
	{
		if (animT >= animEnd)
		{
			animStatus = ANIM_FINISHED;
		}
		if (animStatus == ANIM_FINISHED)
		{

		}
		else
			drawAnimation(animT);
		if(animStatus == ANIM_PLAYING)
			animT += animStep * delta;
	}
}

#pragma endregion


#pragma region INPUT

void mouseButtonPressed(int button, int state, int x, int y)
{
	mouseStates[button] = state;
	mouse.x = x;
	mouse.y = win.height - y;
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			int i;
			for (i = 0; i < points.size() && points.size() > 0; ++i)
			{
				if (dist(points[i], mouse.pos()) < handleRadius)
				{
					selectedPoint = i;
					break;
				}
			}
			if (i >= points.size())
			{
				selectedPoint = -1;
				if (points.size() < 4)
				{
					points.push_back(mouse.pos());
					if(points.size() >= 4)
						animStatus = ANIM_READY;
				}
			}
		}
		if (button == GLUT_RIGHT_BUTTON)
		{
			if (points.size() > 0)
			{
				points.pop_back();
				animStatus = ANIM_OFF;
			}
		}
	}
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
	if (keyStates[' '] && !keyPreviousStates[' '])
	{
		if (points.size() >= 4)
		{
			if (animStatus == ANIM_READY || animStatus == ANIM_PAUSED)
				animStatus = ANIM_PLAYING;
			else if (animStatus == ANIM_PLAYING)
				animStatus = ANIM_PAUSED;
			if (animStatus == ANIM_FINISHED)
			{
				animT = 0.0f;
				animStatus = ANIM_PLAYING;
			}
		}
		else
		{
			animStatus = ANIM_OFF;
			animT = 0.0f;
		}
	}

	if (mouseStates[GLUT_LEFT_BUTTON] == GLUT_DOWN)
	{
		if (selectedPoint >= 0)
		{
			points[selectedPoint] = mouse.pos();
		}
	}
	if (mouseStates[GLUT_LEFT_BUTTON] == GLUT_UP)
	{
		selectedPoint = -1;
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

	//Fizika
	getDelta();

	//Objektumok kirajzolása
	for (vec2& cp : points)
	{
		drawCircle(cp, handleRadius, true, 6);
	}
	if (visualize)
	{
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < points.size(); ++i)
			glVertex2f(points[i].x, points[i].y);
		glEnd();
	}
	if (points.size() >= 4)
	{
		curve.draw(points[0], points[1], points[2], points[3]);
	}

	//Animáció
	processAnimation();

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
	glutSpecialFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutSpecialUpFunc(keyUp);
	glutMouseFunc(mouseButtonPressed);
	glutMotionFunc(mouseMove);
	glutTimerFunc(5, keyOps, 0);
	glutMainLoop();
}