// Motkó Gábor BZ79WI
// 10. házi

#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();
bool * const mousePreviousStates = new bool[5]();

// Színek
struct
{
	vec3 black = vec3(0.0f);
	vec3 white = vec3(1.0);
	vec3 darkblue = vec3(0.0f, 0.0f, 0.7f);
	vec3 darkred = vec3(0.7f, 0.0f, 0.0f);
	vec3 red = vec3(1.0f, 0.2f, 0.0f);
	vec3 orange = vec3(1.0f, 0.5f, 0.0f);
	vec3 yellow = vec3(1.0f, 1.0f, 0.0f);
	vec3 blue = vec3(0.0f, 0.0f, 1.0f);
	vec3 gray = vec3(0.35f, 0.35f, 0.35f);
	vec3 sky = vec3(0.0f, 0.5f, 1.0f);
	vec3 green = vec3(0.0f, 1.0f, 0.0f);
	vec3 mediumgreen = vec3(0.0f, 0.70f, 0.0f);
	vec3 darkgreen = vec3(0.0f, 0.5f, 0.0f);
	void v3(vec3 rgb)
	{
		glColor3f(rgb.x, rgb.y, rgb.z);
	}
} color;

// Idõzítés
struct
{
	double delta;
	int current;
	int previous;
	void getDelta()
	{
		current = glutGet(GLUT_ELAPSED_TIME);
		delta = (current - previous) / 1000.0;
		previous = current;
	}
} t;

// Window
struct
{
	int width = 800;
	int height = 600;
	vec2 size()
	{
		return vec2(this->width, this->height);
	}
	float fps;
	float center;
} win;

// Viewport
struct
{
	float bottom = 0;
	float left = 100;
	vec2 pos()
	{
		return vec2(this->left, this->bottom);
	}
	void setPos(vec2 xy)
	{
		this->left = xy.x;
		this->bottom = xy.y;
	}
	int width = 500;
	int height = 500;
	vec2 size()
	{
		return vec2(this->width, this->height);
	}
} view;

struct
{
	float height = 0;
	float radius = 5;
	float bearing = 0;
	mat4 getMatrix()
	{
		vec3 eye = vec3(radius * cos(bearing), height, radius * sin(bearing));
		vec3 target = vec3();
		vec3 up = vec3(0, 1, 0);

		vec3 mz = normalize(eye - target);
		vec3 mx = normalize(cross(eye, mz));
		vec3 my = normalize(cross(mz, mx));

		return coordinateTransform(eye, mx, my, mz);
	}
} cam;

class Quad
{
public:
	static vec3 defaultColor;

	vec3 points[4];
	vec3 color;
	vec3 getNormal()
	{
		return cross(points[1] - points[0], points[2] - points[0]);
	}
	vec3 getCenter()
	{
		return (points[0] + points[1] + points[2] + points[3]) / 4.0f;
	}
	bool isBackfacing()
	{
		vec3 v = normalize(win.center - points[0]);
		vec3 n = normalize(getNormal());
		return dot(n, v) > 0;
	}
	Quad(vec3 p0, vec3 p1, vec3 p2, vec3 p3)
	{
		points[0] = p0;
		points[1] = p1;
		points[2] = p2;
		points[3] = p3;
		color = Quad::defaultColor;
	}

	static bool compare(Quad a, Quad b)
	{
		return fabs(a.getCenter().z - win.center) > fabs(b.getCenter().z - win.center);
	}
};
vec3 Quad::defaultColor = vec3(1.0f);

class BezierCurve
{
public:
	static double binomialCoefficient(int n, int k)
	{
		if (k > n - k)
			k = n - k;
		double c = 1.0;
		for (int i = 0; i < k; i++)
		{
			c = c * (n - i);
			c = c / (i + 1);
		}
		return c;
	}
	vec2 points[4];
	float bernsteinPoly(int i, int n, float t)
	{
		return binomialCoefficient(n, i) * pow(t, i) * pow(1.0f - t, n - i);
	}
	vec2 point(float t)
	{
		vec2 qt = vec2();
		for (int i = 0; i <= 3; ++i)
		{
			float b = bernsteinPoly(i, 3, t);
			qt.x += b * points[i].x;
			qt.y += b * points[i].y;
		}
		return qt;
	}
	BezierCurve(vec2 p0, vec2 p1, vec2 p2, vec2 p3)
	{
		points[0] = p0;
		points[1] = p1;
		points[2] = p2;
		points[3] = p3;
	}
};
BezierCurve shape = BezierCurve(vec2(0.3, 0), vec2(1, 0.25f), vec2(0, 0.7), vec2(0.3, 1));

class Light
{
public:
	vec3 pos;
	vec3 color;
	Light()
	{
		this->pos = vec3();
	}
	Light(vec3 _pos)
	{
		this->pos = _pos;
	}
	vec3 calculate(vec3 qpos, vec3 nrm, vec3 src)
	{
		vec3 l = this->pos - qpos;
		float mult = (dot(normalize(nrm), normalize(l)) + 1) / 2.0f;
		return src * mult;
	}
} light;

#pragma region GRAPHICS

std::vector<Quad> geometry = std::vector<Quad>();

void drawCurve()
{
	glBegin(GL_LINE_STRIP);
	for (float t = 0; t <= 1; t += 0.05)
	{
		vec2 p = shape.point(t) * 200;
		printMathObject(p);
		glVertex2f(p.x, p.y);
	}
	glEnd();
}

vec3 vasePoint(float phi, float h)
{
	vec2 p = shape.point(h);
	return vec3(cos(phi) * p.x, p.y, sin(phi) * p.x);
}

void buildVase()
{
	float dPhi = pi() / 4.0;
	float dH = 0.1f;
	geometry.clear();
	for (float phi = 0.0f; phi < 2 * pi(); phi += dPhi)
	{
		for (float h = 0.0f; h < 1.0f; h += dH)
		{
			vec3 p0 = vasePoint(phi, h);
			vec3 p1 = vasePoint(phi + dPhi, h);
			vec3 p2 = vasePoint(phi + dPhi, h + dH);
			vec3 p3 = vasePoint(phi, h + dH);
			geometry.push_back(Quad(p0, p1, p2, p3));
		}
	}
}

void drawGeometry()
{
	std::sort(geometry.begin(), geometry.end(), Quad::compare);
	for (int i = 0; i < geometry.size(); ++i)
	{
		Quad q = geometry[i];
	}
}

#pragma endregion


#pragma region INPUT

bool keyPress(char c)
{
	return keyStates[c] && !keyPreviousStates[c];
}
bool keyDown(char c)
{
	return keyStates[c];
}

void onKeyDown(unsigned char key, int x, int y)
{
	keyStates[key] = true;
}

void onKeyUp(unsigned char key, int x, int y)
{
	keyStates[key] = false;
}

void inputProcess(int x)
{
	if (keyPress('x'))
	{
		exit(0);
	}
	if (keyDown('w'))
	{
		cam.radius -= t.delta;
	}
	if (keyDown('s'))
	{
		cam.radius += t.delta;
	}
	if (keyDown('a'))
	{
		cam.bearing += t.delta;
	}
	if (keyDown('d'))
	{
		cam.bearing -= t.delta;
	}
	if (keyDown('r'))
	{
		cam.height += t.delta;
	}
	if (keyDown('f'))
	{
		cam.height -= t.delta;
	}

	memcpy(keyPreviousStates, keyStates, 256 * sizeof(bool));
	memcpy(mousePreviousStates, mouseStates, 5 * sizeof(bool));
	glutPostRedisplay();
	glutTimerFunc(10, inputProcess, 0);
}

#pragma endregion


#pragma region SYSTEM

void display()
{
	t.getDelta();
	win.fps = 1.0 / t.delta;
	glClear(GL_COLOR_BUFFER_BIT);

	color.v3(color.black);

	buildVase();

	glutSwapBuffers();
}

void init()
{
	t.previous = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	vec3 cc = color.mediumgreen;
	glClearColor(cc.x, cc.y, cc.z, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, win.width, 0, win.height);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glPointSize(3.0f);
	view.left = (win.width - view.width) / 2.0f;
	view.bottom = (win.height - view.height) / 2.0f;
	//mouseStates[GLUT_LEFT_BUTTON] = GLUT_UP;
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(win.width, win.height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Vase");
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(onKeyDown);
	glutKeyboardUpFunc(onKeyUp);

	glutTimerFunc(10, inputProcess, 0);
	glutMainLoop();
	return 0;
}

#pragma endregion