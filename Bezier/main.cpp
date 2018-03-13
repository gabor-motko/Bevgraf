#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <vector>

GLsizei winWidth = 600, winHeight = 600;

vec2 points[4] = { { -250.0f, 250.0f },{ -100.0f, -200.0f },{ 100.0f, -200.0f },{ 250.0f, 250.0f } };
#define N 3
#define DRAW_STEP 0.001f


void init() {
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(-winWidth / 2, winWidth / 2, -winHeight / 2, winHeight / 2);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.0);
	glLineWidth(1.0);
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

void draBezierWithBernstein() {

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
		vec2 qt = vec2();
		for (int i = 0; i <= N; ++i)
		{
			qt.x += bernsteinPolynomial(N, i, t) * points[i].x;
			qt.y += bernsteinPolynomial(N, i, t) * points[i].y;
		}
		glVertex2f(qt.x, qt.y);
		printf("point: %d %d\n", qt.x, qt.y);
	}
	glEnd();
}


void display() {
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0, 0.0, 0.0);

	draBezierWithBernstein();

	glutSwapBuffers();
}

//TODO getActivePoint + processMouse + processMouseActiveMotion


int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Bezier");
	init();
	glutDisplayFunc(display);

	//TODO mouseFunc + motionFunc

	glutMainLoop();
	return 0;
}



