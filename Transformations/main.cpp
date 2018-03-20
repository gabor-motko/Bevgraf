#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <vector>

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
double currentTime, previousTime, delta;	//idõzítés

mat3 M;
std::vector<vec2> quads;
vec2 center = { 0.0f, 0.0f };

void initCircle()
{

}

void getDelta()
{
	currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	delta = currentTime - previousTime;
	previousTime = currentTime;
}

void init() {
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(-(win.width / 2), (win.width / 2), -(win.height / 2), (win.height / 2));
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.0);
	glLineWidth(1.0);

	M = mat3();

	initCircle();
	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void drawQuads(std::vector<vec2> points)
{
	glBegin(GL_QUADS);
	for (int i = 0; i < points.size(); ++i)
		glVertex2f(points[i].x, points[i].y);
	glEnd();
}


void display() {
	getDelta();

	M = rotate((2 * pi() * delta) / 5);
	for (int i = 0; i < quads.size(); ++i)
	{
		vec3 h = ihToH(quads[i]);
		quads[i] = hToIh(M * h);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(0, 0, 0);

	drawQuads(quads);
	glBegin(GL_POINTS);
	glVertex2f(center.x, center.y);
	glEnd();

	glutSwapBuffers();
}

void update(int v)
{

	glutPostRedisplay();
	glutTimerFunc(10, update, 0);
}

int main(int argc, char** argv)
{
	quads = std::vector<vec2>(8);
	quads.push_back(vec2(50, 150));
	quads.push_back(vec2(150, 150));
	quads.push_back(vec2(150, 50));
	quads.push_back(vec2(50, 50));
	quads.push_back(vec2(150, -50));
	quads.push_back(vec2(200, -50));
	quads.push_back(vec2(250, -100));
	quads.push_back(vec2(200, -100));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(win.width, win.height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("2D Transformation");

	init();
	glutDisplayFunc(display);
	glutTimerFunc(10, update, 0);

	glutMainLoop();
	return 0;
}



