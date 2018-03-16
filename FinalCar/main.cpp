#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#define BYTE_TO_BINARY(byte) (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'), (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'), (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'), (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0') 

#pragma region MAT23



#pragma endregion


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

typedef struct TEnvironment
{
	float ground;
	vec3 skyColor;
	vec3 groundColor;
	TEnvironment(float _ground, vec3 _groundColor, vec3 _skyColor)
	{
		ground = _ground;
		groundColor = _groundColor;
		skyColor = _skyColor;
	}
} ENVIRONMENT;
ENVIRONMENT env = { 200.0f, {0.0f, 0.6f, 0.0f}, {0.0f, 0.5f, 1.0f} };

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

std::vector<vec2> bernsteinPoints;	//Bernstein-Bezier görbe pontjai
std::vector<vec2> dcPoints;			//de-Casteljau-Bezier görbe pontjai
std::vector<vec2> hermitePoints;	//Hermite görbe pontjai

vec3 hermiteT = { 0.0f, 0.5f, 1.0f };

#pragma endregion

#pragma region MATH

//Lineáris interpoláció két vektor között
vec2 lerpv2(vec2 a, vec2 b, float t)
{
	assert(t >= 0.0 && t <= 1.0);
	return (1 - t) * a + t * b;
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

float bernsteinPolynomial(int i, int n, float t)
{
	return binomialCoefficient(n, i) * pow(t, i) * pow(1.0f - t, n - i);
}

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

vec2 hermitePoint()
{

}

#pragma endregion

#pragma region GRAPHICS


void drawBackground()
{
	glColor3f(env.groundColor.x, env.groundColor.y, env.groundColor.z);
	glRectf(0.0f, 0.0f, win.width, env.ground);
}

void drawCPs()
{
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_POINTS);
	for (int i = 0; i < bernsteinPoints.size(); ++i)
	{
		glVertex2f(bernsteinPoints[i].x, bernsteinPoints[i].y);
	}
	for (int i = 0; i < dcPoints.size(); ++i)
	{
		glVertex2f(dcPoints[i].x, dcPoints[i].y);
	}
	for (int i = 0; i < hermitePoints.size(); ++i)
	{
		glVertex2f(hermitePoints[i].x, hermitePoints[i].y);
	}
	glEnd();
}

void drawConnectedBernstein();

void visualizeBernstein()
{
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_STRIP);
	for(int i = 0; i < bernsteinPoints.size(); ++i)
		glVertex2f(bernsteinPoints[i].x, bernsteinPoints[i].y);
	glEnd();
}

void drawHermite()
{
	mat3 hermiteM = mat3(vec3(pow(hermiteT.x, 2), hermiteT.x, 1), vec3(pow(hermiteT.y, 2), hermiteT.y, 1), vec3(pow(hermiteT.z, 2), hermiteT.z, 1), true);
	for (float t = hermiteT.x; t <= hermiteT.z; t += (hermiteT.z - hermiteT.x) / 64)
	{
		vec2 p = hermitePoint(hermitePoints, )
	}
}

void drawDeCasteljau();

void visualizeDeCasteljau();

void drawLines()
{
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex2f(bernsteinPoints[9].x, bernsteinPoints[9].y);
	glVertex2f(hermitePoints[0].x, hermitePoints[0].y);
	glEnd();
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2f(hermitePoints[2].x, hermitePoints[2].y);
	glVertex2f(bernsteinPoints[0].x, bernsteinPoints[0].y);
	glEnd();
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex2f(dcPoints[0].x, dcPoints[0].y);
	glVertex2f(dcPoints[4].x, dcPoints[4].y);
	glEnd();
}

void drawWheels()
{
	vec2 front = lerpv2(bernsteinPoints[0], hermitePoints[2], 0.25f);
	vec2 rear = lerpv2(bernsteinPoints[0], hermitePoints[2], 0.75f);
	const float r1 = 50.0f;
	const float r2 = 30.0f;
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_POLYGON);
	for (float t = 0.0f; t <= 2 * pi(); t += 2 * pi() / 32)
		glVertex2f(front.x + r1 * cos(t), front.y + r1 * sin(t));
	glEnd();
	glBegin(GL_POLYGON);
	for (float t = 0.0f; t <= 2 * pi(); t += 2 * pi() / 32)
		glVertex2f(rear.x + r1 * cos(t), rear.y + r1 * sin(t));
	glEnd();
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_POLYGON);
	for (float t = 0.0f; t <= 2 * pi(); t += 2 * pi() / 10)
		glVertex2f(front.x + r2 * cos(t), front.y + r2 * sin(t));
	glEnd();
	glBegin(GL_POLYGON);
	for (float t = 0.0f; t <= 2 * pi(); t += 2 * pi() / 10)
		glVertex2f(rear.x + r2 * cos(t), rear.y + r2 * sin(t));
	glEnd();
}

#pragma endregion

#pragma region INPUT

bool keyPress(int key)
{
	return keyStates[key] && !keyPreviousStates[key];
}
bool mousePress(int button)
{
	return mouseStates[button] && !mousePreviousStates[button];
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
void mouseChange(int button, int state, int x, int y)
{
	mouseStates[button] = state;
	if (mousePress(GLUT_LEFT_BUTTON))
	{
		/*dcPoints.push_back(vec2(x, win.height - y));
		printf("point: %.0f %.0f\n", dcPoints[dcPoints.size()-1].x, dcPoints[dcPoints.size() - 1].y);
		out << "Points.push_back(vec2(" << dcPoints[dcPoints.size() - 1].x << ".0f, " << dcPoints[dcPoints.size() - 1].y << ".0f));\n";*/
	}
}

#pragma endregion

#pragma region SYSTEM

void getDelta()
{
	currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	delta = currentTime - previousTime;
	previousTime = currentTime;
}

void drawScene()
{
	getDelta();
	glClear(GL_COLOR_BUFFER_BIT);

	//TODO: drawing
	drawBackground();
	drawLines();
	drawWheels();

	if (visualize & V_B_CPOLY)
		visualizeBernstein();
	if(visualize & V_CP)
		drawCPs();

	glutSwapBuffers();
}

void init()
{
	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	glClearColor(env.skyColor.x, env.skyColor.y, env.skyColor.z, 0.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, win.width, 0, win.height);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(7.0f);
	glLineWidth(2.0f);
}

void initVectors()
{
	bernsteinPoints.push_back(vec2(180.0f, 205.0f));
	bernsteinPoints.push_back(vec2(180.0f, 234.0f));
	bernsteinPoints.push_back(vec2(180.0f, 247.0f));
	bernsteinPoints.push_back(vec2(223.0f, 247.0f));
	bernsteinPoints.push_back(vec2(253.0f, 253.0f));
	bernsteinPoints.push_back(vec2(229.0f, 280.0f));
	bernsteinPoints.push_back(vec2(249.0f, 327.0f));
	bernsteinPoints.push_back(vec2(270.0f, 375.0f));
	bernsteinPoints.push_back(vec2(376.0f, 465.0f));
	bernsteinPoints.push_back(vec2(532.0f, 438.0f));
	hermitePoints.push_back(vec2(706.0f, 291.0f));
	hermitePoints.push_back(vec2(752.0f, 252.0f));
	hermitePoints.push_back(vec2(750.0f, 205.0f));
	dcPoints.push_back(vec2(360.0f, 329.0f));
	dcPoints.push_back(vec2(366.0f, 368.0f));
	dcPoints.push_back(vec2(431.0f, 402.0f));
	dcPoints.push_back(vec2(496.0f, 398.0f));
	dcPoints.push_back(vec2(534.0f, 351.0f));
}

int main(int argc, char * argv[])
{
	initVectors();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(win.width, win.height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutSpecialFunc(keyDown);
	glutSpecialUpFunc(keyUp);
	glutMouseFunc(mouseChange);

	glutTimerFunc(10, keyProcess, 0);

	glutMainLoop();
	return 0;
}

#pragma endregion