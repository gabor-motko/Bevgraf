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
	float center = 5.0f;
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
	int width = 800;
	int height = 800;
	vec2 size()
	{
		return vec2(this->width, this->height);
	}
} view;

struct
{
	float height = 0;
	float radius = 2;
	float bearing = 0;
	mat4 getMatrix()
	{
		vec3 eye = vec3(radius * cos(bearing), height, -radius * sin(bearing));
		vec3 target = vec3();
		vec3 up = vec3(0, 1, 0);

		vec3 mz = normalize(eye - target);
		vec3 mx = normalize(cross(up, mz));
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
	void print()
	{
		printf("Quad: %f, %f, %f | %f, %f, %f | %f, %f, %f | %f, %f, %f\n", points[0].x, points[0].y, points[0].z, points[1].x, points[1].y, points[1].z, points[2].x, points[2].y, points[2].z, points[3].x, points[3].y, points[3].z);
	}
	Quad(vec3 p0, vec3 p1, vec3 p2, vec3 p3, vec3 color)
	{
		points[0] = p0;
		points[1] = p1;
		points[2] = p2;
		points[3] = p3;
		this->color = color;
	}
	Quad(vec3 p0, vec3 p1, vec3 p2, vec3 p3)
	{
		points[0] = p0;
		points[1] = p1;
		points[2] = p2;
		points[3] = p3;
		color = Quad::defaultColor;
	}
	Quad()
	{
		points[0] = vec3();
		points[1] = vec3();
		points[2] = vec3();
		points[3] = vec3();
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
	vec2 points[6];
	float bernsteinPoly(int i, int n, float t)
	{
		return binomialCoefficient(n, i) * pow(t, i) * pow(1.0f - t, n - i);
	}
	vec2 point(float t)
	{
		vec2 qt = vec2();
		for (int i = 0; i <= 5; ++i)
		{
			float b = bernsteinPoly(i, 5, t);
			qt.x += b * points[i].x;
			qt.y += b * points[i].y;
		}
		return qt;
	}
	BezierCurve(vec2 p0, vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5)
	{
		points[0] = p0;
		points[1] = p1;
		points[2] = p2;
		points[3] = p3;
		points[4] = p4;
		points[5] = p5;
	}
};
BezierCurve shape = BezierCurve(vec2(0, 0), vec2(1, 0), vec2(1, 0.7), vec2(0.3, 0.7), vec2(0.3, 1.0), vec2(1, 1));

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
		vec3 p = hToIh(cam.getMatrix() * ihToH(this->pos));
		vec3 l = p - qpos;
		float mult = (dot(normalize(nrm), normalize(l)) + 1) / 2.0f;
		return src * mult;
	}
};

Light light = Light(vec3(2));

#pragma region GRAPHICS

std::vector<Quad> geometry = std::vector<Quad>();

void drawCurve()
{
	glBegin(GL_LINE_STRIP);
	for (float t = 0; t <= 1; t += 0.05)
	{
		vec2 p = shape.point(t) * 200;
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
		geometry.push_back(Quad(vasePoint(phi, 0), vasePoint(phi + dPhi, 0), vec3(), vec3()));
		for (float h = 0.0f; h < 1.0f; h += dH)
		{
			vec3 p0 = vasePoint(phi, h);
			vec3 p1 = vasePoint(phi + dPhi, h);
			vec3 p2 = vasePoint(phi + dPhi, h + dH);
			vec3 p3 = vasePoint(phi, h + dH);
			geometry.push_back(Quad(p0, p1, p2, p3));
		}
	}
	//std::vector<vec3> points = std::vector<vec3>();
	//points.push_back(vec3(1.0, 1.0, 1.0));		//0
	//points.push_back(vec3(1.0, 1.0, -1.0));		//1
	//points.push_back(vec3(1.0, -1.0, 1.0));		//2
	//points.push_back(vec3(1.0, -1.0, -1.0));	//3
	//points.push_back(vec3(-1.0, 1.0, 1.0));		//4
	//points.push_back(vec3(-1.0, 1.0, -1.0));	//5
	//points.push_back(vec3(-1.0, -1.0, 1.0));	//6
	//points.push_back(vec3(-1.0, -1.0, -1.0));	//7

	//geometry.push_back(Quad(points[7], points[3], points[1], points[5], color.red));
	//geometry.push_back(Quad(points[2], points[6], points[4], points[0], color.green));
	//geometry.push_back(Quad(points[3], points[2], points[0], points[1], color.blue));
	//geometry.push_back(Quad(points[6], points[7], points[5], points[4], color.yellow));
	//geometry.push_back(Quad(points[5], points[1], points[0], points[4], color.orange));
	//geometry.push_back(Quad(points[6], points[2], points[3], points[7], color.darkblue));
}

void drawGeometry()
{
	mat4 worldViewProj = cam.getMatrix();
	std::vector<Quad> tr = std::vector<Quad>(geometry.size());
	for (int i = 0; i < geometry.size(); ++i)
	{
		vec3 p0 = hToIh(worldViewProj * ihToH(geometry[i].points[0]));
		vec3 p1 = hToIh(worldViewProj * ihToH(geometry[i].points[1]));
		vec3 p2 = hToIh(worldViewProj * ihToH(geometry[i].points[2]));
		vec3 p3 = hToIh(worldViewProj * ihToH(geometry[i].points[3]));
		//tr[i] = Quad(p0, p1, p2, p3, light.calculate(geometry[i].points[0], geometry[i].getNormal(), geometry[i].color));
		tr[i] = Quad(p0, p1, p2, p3);
	}
	std::sort(tr.begin(), tr.end(), Quad::compare);
	mat4 proj = perspective(win.center);
	mat4 w2v = windowToViewport3(vec2(-1), vec2(2), view.pos(), view.size());

	for (int i = 0; i < tr.size(); ++i)
	{
		Quad q = tr[i];
		color.v3(light.calculate(q.points[0], (q.isBackfacing() ? (-1 * q.getNormal()) : q.getNormal()), q.color));
		//color.v3(q.color);
		glBegin(GL_QUADS);
		for (int j = 0; j < 4; ++j)
		{
			vec3 p = hToIh(w2v * proj * ihToH(q.points[j]));
			glVertex2f(p.x, p.y);
		}
		glEnd();
	}
}

void drawText(float x, float y, const char * s)
{
	glRasterPos2f(x, y);
	for (int i = 0; s[i] != '\0'; ++i)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i]);
}

void drawTextOverlay()
{
	float left = 30.0f;
	float top = win.height - 20.0f;
	std::string str;

	str = "Framerate: " + std::to_string((int)win.fps);
	drawText(left, top, str.c_str());
	top -= 14.0f;
	str = "C = " + std::to_string(win.center);
	drawText(left, top, str.c_str());
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
		cam.radius -= 10 * t.delta;
	}
	if (keyDown('s'))
	{
		cam.radius += 10 * t.delta;
	}
	if (keyDown('a'))
	{
		cam.bearing += 10 * t.delta;
	}
	if (keyDown('d'))
	{
		cam.bearing -= 10 * t.delta;
	}
	if (keyDown('r'))
	{
		cam.height += 10* t.delta;
	}
	if (keyDown('f'))
	{
		cam.height -= 10 * t.delta;
	}
	if (keyDown('t'))
	{
		win.center -= t.delta;
	}
	if (keyDown('g'))
	{
		win.center += t.delta;
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
	drawCurve();
	drawGeometry();

	drawTextOverlay();

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
	printf("begin\n");
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