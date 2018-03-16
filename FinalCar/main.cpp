#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
#include <vector>
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
ENVIRONMENT env = { 100.0f, {0.0f, 0.6f, 0.0f}, {0.0f, 0.5f, 1.0f} };

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

#pragma endregion

#pragma region GRAPHICS

void drawBackground()
{
	
}

void drawConnectedBernstein();

void visualizeBernstein();

void drawDeCasteljau();

void visualizeDeCasteljau();

void drawLines();

void drawWheels();

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

void drawScene()
{
	getDelta();
	glClear(GL_COLOR_BUFFER_BIT);

	//TODO: drawing

	glutSwapBuffers();
}

void init()
{
	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, win.width, 0, win.height);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.0);
	glLineWidth(1.0);
}

void initVectors()
{

}

#pragma endregion


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

	glutTimerFunc(10, keyProcess, 0);

	glutMainLoop();
	return 0;
}