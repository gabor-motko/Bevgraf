#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <vector>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte) (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'), (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'), (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'), (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0') 

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

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();
bool * const mousePreviousStates = new bool[5]();

double delta, currentTime, previousTime;

#define V_CP (char)0b00000001
#define V_B_CPOLY (char)0b00000010
#define V_H_CPOLY (char)0b00000100
#define V_DC_CPOLY (char)0b00010000
#define V_DC_SEGMENTS (char)0b00100000
#define V_DC_SWEEP (char)0b01000000

char visualize = V_CP;
float visualize_t = 0.0f;

std::vector<vec2> points;
#define DRAW_STEP 0.01f

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

double bernsteinPolynomial(int i, int n, double t)
{
	return binomialCoefficient(n, i) * pow(t, i) * pow(1 - t, n - i);
}

//t: [0..1]-hez tartozó pont kiszámítása de Casteljau algoritmusával
vec2 dcPoint(std::vector<vec2> p, float t, std::vector<vec2> * out = NULL)
{
	assert(!p.empty());

	int n = p.size() - 1;
	std::vector<vec2> q = std::vector<vec2>(n + 1);
	q = p;

	for (int i = 0; i <= n; ++i)
	{
		for (int j = 0; j < n - i; ++j)
		{
			if (out != NULL && i >= 1)
			{
				out->push_back(q[j]);
				out->push_back(q[j + 1]);
			}
			q[j] = (1 - t) * q[j] + t * q[j + 1];
		}
	}
	return q[0];
}

#pragma endregion

#pragma region GRAPHICS

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

void visualizeDC()
{
	if ((visualize & V_DC_CPOLY) || (visualize & V_DC_SEGMENTS) || (visualize & V_DC_SWEEP))
	{
		std::vector<vec2> s = std::vector<vec2>();
		vec2 p = dcPoint(points, visualize_t, &s);
		if (visualize & V_DC_CPOLY)
		{
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i < points.size(); ++i)
			{
				glVertex2f(points[i].x, points[i].y);
			}
			glEnd();
		}
		if (visualize & V_DC_SEGMENTS)
		{
			glBegin(GL_LINES);
			for (int i = 0; i < s.size(); ++i)
			{
				glVertex2f(s[i].x, s[i].y);
			}
			glEnd();
		}
		if (visualize & V_DC_SWEEP)
		{
			glBegin(GL_POINTS);
			glVertex2f(p.x, p.y);
			glEnd();
		}
	}
}

void drawBernstein()
{

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
			qt.x += bernsteinPolynomial(i, n, t) * points[i].x;
			qt.y += bernsteinPolynomial(i, n, t) * points[i].y;
		}
		glVertex2f(qt.x, qt.y);
	}
	glEnd();
}

#pragma endregion

#pragma region INPUT

bool keyPress(int key)
{
	return keyStates[key] && !keyPreviousStates[key];
}

void keyProcess(int x)
{
	if (keyStates['x'])
	{
		exit(0);
	}
	if (keyPress('1'))
	{
		visualize ^= V_CP;
		printf("Visualize: %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY(visualize));
	}
	if (keyPress('2'))
	{
		visualize ^= V_B_CPOLY;
		printf("Visualize: %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY(visualize));
	}
	if (keyPress('3'))
	{
		visualize ^= V_H_CPOLY;
		printf("Visualize: %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY(visualize));
	}
	if (keyPress('4'))
	{
		visualize ^= V_DC_CPOLY;
		printf("Visualize: %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY(visualize));
	}
	if (keyPress('5'))
	{
		visualize ^= V_DC_SEGMENTS;
		printf("Visualize: %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY(visualize));
	}
	if (keyPress('6'))
	{
		visualize ^= V_DC_SWEEP;
		printf("Visualize: %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY(visualize));
	}
	if (keyStates[GLUT_KEY_LEFT])
	{
		if (visualize_t > 0.0f)
		{
			visualize_t -= 0.25f * delta;
			if (visualize_t < 0.0f)
				visualize_t = 0.0f;
		}
	}
	if (keyStates[GLUT_KEY_RIGHT])
	{
		if (visualize_t < 1.0f)
		{
			visualize_t += 0.25f * delta;
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
	currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	delta = currentTime - previousTime;
	previousTime = currentTime;
}

void display()
{
	getDelta();
	glClear(GL_COLOR_BUFFER_BIT);

	if (visualize & V_CP)
	{
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_POINTS);
		for (int i = 0; i < 4; i++)
			glVertex2f(points[i].x, points[i].y);
		glEnd();
	}

	glColor3f(0.0, 0.0, 0.0);
	//drawBernstein();
	drawDC();

	visualizeDC();

	glutSwapBuffers();
}

void init()
{
	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
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
	points = std::vector<vec2>();
	points.push_back(vec2(-250.0f, -250.0f));
	points.push_back(vec2(-100.0f, 200.0f));
	points.push_back(vec2(100.0f, -200.0f));
	points.push_back(vec2(250.0f, 250.0f));

	printf("1: kontrollpontok\n2: Dernstein-Bezier kontrollpoligon\n3: Hermite kontrollpoligon\n4: de-Casteljau-Bezier kontrollpoligon\n5: de-Casteljau szakaszok\n6: de-Casteljau szemléltetés\n\n");

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
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