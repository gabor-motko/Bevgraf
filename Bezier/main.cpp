#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <vector>

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

//vec2 points[4] = { { -250.0f, 250.0f },{ -100.0f, -200.0f },{ 100.0f, -200.0f },{ 250.0f, 250.0f } };
std::vector<vec2> points;
#define DRAW_STEP 0.001f

#pragma endregion

#pragma region MATHS

//Két vec2 lineáris interpolációja i: R[0..1] intervallumon
vec2 lerpv2(vec2 a, vec2 b, float i)
{
	assert(i >= 0.0 && i <= 1.0);
	return vec2((1 - i) * a.x + i * b.x, (1 - i) * a.y + i * b.y);
}

double binomialCoefficient(int n, int k)
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

double bernsteinPolynomial(int n, int i, double t)
{
	return binomialCoefficient(n, i) * pow(t, i) * pow(1 - t, n - i);
}

//t: [0..1]-hez tartozó pont kiszámítása de Casteljau algoritmusával
vec2 dcPoint(std::vector<vec2> p, float t, std::vector<vec2>* segmentPoints = NULL)
{
	int n = p.size();
	std::vector<vec2> out;
	int r = 0;
	for (int i = 0; p.size() > 1; ++i)
	{
		out.clear();
		for (int j = 1; j < n - i; ++j)
		{
			out.push_back(lerpv2(p[j], p[j - 1], t));
		}
		p.swap(out);
	}
	return p[0];
}

#pragma endregion

void drawDC()
{
	glBegin(GL_LINE_STRIP);
	for (double t = 0.0; t <= 1.0; t += 0.01)
	{
		vec2 p = dcPoint(points, t);
		glVertex2f(p.x, p.y);
	}
	glEnd();
}

void drawBernstein()
{
	glColor3f(0.0, 0.0, 1.0);

	glBegin(GL_POINTS);
	for (int i = 0; i < 4; i++)
		glVertex2f(points[i].x, points[i].y);
	glEnd();

	glColor3f(0.0, 0.0, 0.0);

	//bezier görbe kontrollpontjait összekötõ vonal
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; ++i)
		glVertex2f(points[i].x, points[i].y);
	glEnd();
	glColor3f(1.0, 0.0, 0.0);

	//TODO bezier görbe pontjainak kiszámolása bernstein polinommal
	glBegin(GL_LINE_STRIP);
	for (float t = 0.0f; t <= 1.0f; t += 0.01f)
	{
		int n = points.size() - 1;
		vec2 qt = vec2();
		for (int i = 0; i <= n; ++i)
		{
			qt.x += bernsteinPolynomial(n, i, t) * points[i].x;
			qt.y += bernsteinPolynomial(n, i, t) * points[i].y;
		}
		glVertex2f(qt.x, qt.y);
		printf("point: %d %d\n", qt.x, qt.y);
	}
	glEnd();
}

#pragma region SYSTEM

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0, 0.0, 0.0);

	//drawBernstein();
	drawDC();

	glutSwapBuffers();
}

void init()
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(-win.width / 2, win.width / 2, -win.height / 2, win.height / 2);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.0);
	glLineWidth(1.0);
}

#pragma endregion


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(win.width, win.height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();

	glutDisplayFunc(display);

	glutMainLoop();
	return 0;
}