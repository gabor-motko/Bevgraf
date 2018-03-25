#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
#include <string>
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
ENVIRONMENT env = { 200.0f, {0.0f, 0.6f, 0.0f}, {0.0f, 0.5f, 1.0f} };

struct 
{
	vec3 black = vec3(0.0f, 0.0f, 0.0f);
	vec3 darkblue = vec3(0.0f, 0.0f, 0.5f);
	vec3 darkred = vec3(0.5f, 0.0f, 0.0f);
	vec3 red = vec3(1.0f, 0.0f, 0.0f);
	vec3 orange = vec3(1.0f, 0.5f, 0.0f);
	vec3 yellow = vec3(1.0f, 1.0f, 0.0f);
	vec3 blue = vec3(0.0f, 0.0f, 1.0f);
	vec3 gray = vec3(0.35f, 0.35f, 0.35f);
} colors;

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();
bool * const mousePreviousStates = new bool[5]();

vec2 * selectedCP = NULL;
#define CP_R 7.0f

double delta, currentTime, previousTime;

#define V_CP (char)0b00000001
#define V_B_CPOLY (char)0b00000010
#define V_H_CPOLY (char)0b00000100
#define V_DC_CPOLY (char)0b00010000
#define V_DC_SWEEP (char)0b01000000

char visualize = V_CP;
float visualize_t = 0.0f;

std::vector<vec2> bernsteinPoints;	//Bernstein-Bezier görbe pontjai
std::vector<vec2> dcPoints;			//de-Casteljau-Bezier görbe pontjai
std::vector<vec2> hermitePoints;	//Hermite görbe pontjai
std::vector<vec2> dcVisPoints;		//de-Casteljau algoritmus osztópontjai

vec4 hermiteT = { -1.0f, -0.5f, 0.5f, 1.0f };

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

	if (out != NULL)
		out->clear();

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

vec4 getHermiteT(float t, bool tangent = false)
{
	if (tangent)
		return vec4(3 * pow(t, 2), 2 * t, 1, 0);
	return vec4(pow(t, 3), pow(t, 2), t, 1);
}

#pragma endregion

#pragma region GRAPHICS

void colorV3(vec3 rgb)
{
	glColor3f(rgb.x, rgb.y, rgb.z);
}

void drawTextLine(float x, float y, const char * s)
{
	glRasterPos2f(x, y);
	for (int i = 0; s[i] != '\0'; ++i)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i]);
}

void drawTextLine(float x, float y, char * s)
{
	glRasterPos2f(x, y);
	for (int i = 0; s[i] != '\0'; ++i)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i]);
}

void drawText()
{
	colorV3(colors.black);
	float left = 30.0f;
	float top = win.height - 20.0f;
	const char * title = "Vizualizacio:";
	const char * strings[] = { "Kontrollpontok", "Bernstein kontrollpoligon", "Hermite kontrollpoligon", "de-Casteljau kontrollpoligon", "de-Casteljau felezok" };
	std::string str;
	glRasterPos2f(left, top);
	drawTextLine(left, top, title);

	left += 10.0f;
	top -= 14.0f;
	str = (visualize & V_CP) ? "1: [x] " : "1: [ ] ";
	str += "Kontrollpontok";
	drawTextLine(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_B_CPOLY) ? "2: [x] " : "2: [ ] ";
	str += "Bernstein kontrollpoligon";
	drawTextLine(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_H_CPOLY) ? "3: [x] " : "3: [ ] ";
	str += "Hermite kontrollpoligon";
	drawTextLine(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_DC_CPOLY) ? "4: [x] " : "4: [ ] ";
	str += "de-Casteljau kontrollpoligon";
	drawTextLine(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_DC_SWEEP) ? "5: [x] " : "5: [ ] ";
	str += "de-Casteljau felezok";
	str += " (t = " + std::to_string(visualize_t).substr(0, 5) + ")";
	drawTextLine(left, top, str.c_str());
}

void drawBackground()
{
	colorV3(env.groundColor);
	glRectf(0.0f, 0.0f, win.width, env.ground);
}

void drawCPs()
{
	colorV3(colors.darkblue);
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

void drawConnectedBernstein()
{
	int n = 3;
	vec2 * points;
	vec2 qt;
	glBegin(GL_LINE_STRIP);

	colorV3(colors.darkred);
	points = &bernsteinPoints[0];
	for (float t = 0.0f; t <= 1.0f; t += 1.0f / 64.0f)
	{
		qt = vec2();
		for (int i = 0; i <= n; ++i)
		{
			float b = bernsteinPolynomial(i, n, t);
			qt.x += b * points[i].x;
			qt.y += b * points[i].y;
		}
		glVertex2f(qt.x, qt.y);
	}
	colorV3(colors.red);
	points = &bernsteinPoints[3];
	for (float t = 0.0f; t <= 1.0f; t += 1.0f / 64.0f)
	{
		qt = vec2();
		for (int i = 0; i <= n; ++i)
		{
			float b = bernsteinPolynomial(i, n, t);
			qt.x += b * points[i].x;
			qt.y += b * points[i].y;
		}
		glVertex2f(qt.x, qt.y);
	}
	colorV3(colors.orange);
	points = &bernsteinPoints[6];
	for (float t = 0.0f; t <= 1.0f; t += 1.0f / 64.0f)
	{
		qt = vec2();
		for (int i = 0; i <= n; ++i)
		{
			float b = bernsteinPolynomial(i, n, t);
			qt.x += b * points[i].x;
			qt.y += b * points[i].y;
		}
		glVertex2f(qt.x, qt.y);
	}

	glEnd();
}

void visualizeBernstein()
{
	colorV3(colors.black);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < bernsteinPoints.size(); ++i)
		glVertex2f(bernsteinPoints[i].x, bernsteinPoints[i].y);
	glEnd();
}

void drawHermite()
{
	colorV3(colors.yellow);
}

void visualizeHermite()
{
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < hermitePoints.size(); ++i)
		glVertex2f(hermitePoints[i].x, hermitePoints[i].y);
	glEnd();
}

void drawDeCasteljau()
{
	colorV3(colors.blue);
	glBegin(GL_LINE_STRIP);
	for (float t = 0.0f; t <= 1.0f; t += 1.0f / 64)
	{
		vec2 p = dcPoint(dcPoints, t);
		glVertex2f(p.x, p.y);
	}
	glEnd();
}

void visualizeDeCasteljau()
{
	colorV3(colors.blue);
	if (visualize & V_DC_CPOLY || visualize & V_DC_SWEEP)
	{
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < dcPoints.size(); ++i)
			glVertex2f(dcPoints[i].x, dcPoints[i].y);
		glEnd();
	}
	if (visualize & V_DC_SWEEP)
	{
		vec2 p = dcPoint(dcPoints, visualize_t, &dcVisPoints);
		colorV3(colors.blue);
		glBegin(GL_LINES);
		for (int i = 0; i < dcVisPoints.size(); ++i)
		{
			glVertex2f(dcVisPoints[i].x, dcVisPoints[i].y);
		}
		glEnd();
		colorV3(colors.red);
		glBegin(GL_POINTS);
		glVertex2f(p.x, p.y);
		p = lerpv2(dcPoints[0], dcPoints[4], visualize_t);
		glVertex2f(p.x, p.y);
		glEnd();
	}
}

void drawLines()
{
	colorV3(colors.blue);
	glBegin(GL_LINES);
	glVertex2f(bernsteinPoints[9].x, bernsteinPoints[9].y);
	glVertex2f(hermitePoints[0].x, hermitePoints[0].y);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(dcPoints[0].x, dcPoints[0].y);
	glVertex2f(dcPoints[4].x, dcPoints[4].y);
	glEnd();
	colorV3(colors.black);
	glBegin(GL_LINES);
	glVertex2f(hermitePoints[2].x, hermitePoints[2].y);
	glVertex2f(bernsteinPoints[0].x, bernsteinPoints[0].y);
	glEnd();
}

void drawWheels()
{
	vec2 front = lerpv2(bernsteinPoints[0], hermitePoints[2], 0.25f);
	vec2 rear = lerpv2(bernsteinPoints[0], hermitePoints[2], 0.75f);
	const float r1 = 50.0f;
	const float r2 = 30.0f;
	colorV3(colors.gray);
	glBegin(GL_POLYGON);
	for (float t = 0.0f; t <= 2 * pi(); t += pi() / 32.0f)
		glVertex2f(front.x + r1 * cos(t), front.y + r1 * sin(t));
	glEnd();
	glBegin(GL_POLYGON);
	for (float t = 0.0f; t <= 2 * pi(); t += pi() / 32.0f)
		glVertex2f(rear.x + r1 * cos(t), rear.y + r1 * sin(t));
	glEnd();

	colorV3(colors.black);
	glBegin(GL_POLYGON);
	for (float t = 0.0f; t <= 2 * pi(); t += 2 * pi() / 10.0f)
		glVertex2f(front.x + r2 * cos(t), front.y + r2 * sin(t));
	glEnd();
	glBegin(GL_POLYGON);
	for (float t = 0.0f; t <= 2 * pi(); t += 2 * pi() / 10.0f)
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
	}
	if (keyPress('2'))
	{
		visualize ^= V_B_CPOLY;
	}
	if (keyPress('3'))
	{
		visualize ^= V_H_CPOLY;
	}
	if (keyPress('4'))
	{
		visualize ^= V_DC_CPOLY;
	}
	if (keyPress('5'))
	{
		visualize ^= V_DC_SWEEP;
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
	if (!mouseStates[GLUT_LEFT_BUTTON])
	{
		for (int i = 0; i < bernsteinPoints.size(); ++i)
		{
			if (dist(vec2(x, win.height - y), bernsteinPoints[i]) <= CP_R)
			{
				selectedCP = &bernsteinPoints[i];
				break;
			}
		}
		if (selectedCP == NULL)
		{
			for (int i = 0; i < dcPoints.size(); ++i)
			{
				if (dist(vec2(x, win.height - y), dcPoints[i]) <= CP_R)
				{
					selectedCP = &dcPoints[i];
					break;
				}
			}
		}
		if (selectedCP == NULL)
		{
			for (int i = 0; i < hermitePoints.size(); ++i)
			{
				if (dist(vec2(x, win.height - y), hermitePoints[i]) <= CP_R)
				{
					selectedCP = &hermitePoints[i];
					break;
				}
			}
		}
	}
	if (mouseStates[GLUT_LEFT_BUTTON])
	{
		selectedCP = NULL;
	}
}

void mouseMove(int x, int y)
{
	if (selectedCP != NULL)
	{
		*selectedCP = vec2(x, win.height - y);
	}
}

#pragma endregion

#pragma region SYSTEM

void getDelta()
{
	currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	delta = currentTime - previousTime;
	previousTime = currentTime;
}

void drawScene()
{
	getDelta();
	glClear(GL_COLOR_BUFFER_BIT);

	//Bernstein-Bezier C1 folytonosság
	bernsteinPoints[4] = bernsteinPoints[3] + (bernsteinPoints[3] - bernsteinPoints[2]);
	bernsteinPoints[7] = bernsteinPoints[6] + (bernsteinPoints[6] - bernsteinPoints[5]);

	//Kirajzolás
	drawBackground();
	glColor3f(0.0f, 0.0f, 0.0f);

	drawLines();
	drawWheels();
	drawHermite();
	drawDeCasteljau();
	drawConnectedBernstein();

	//Vizualizáció
	if (visualize & V_B_CPOLY)
		visualizeBernstein();
	if (visualize & V_DC_SWEEP || visualize & V_DC_CPOLY)
		visualizeDeCasteljau();
	if (visualize & V_H_CPOLY)
		visualizeHermite();
	if (visualize & V_CP)
		drawCPs();
	drawText();
	glutSwapBuffers();
}

void init()
{
	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	glClearColor(env.skyColor.x, env.skyColor.y, env.skyColor.z, 0.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, win.width, 0, win.height);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(CP_R);
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
	hermitePoints.push_back(vec2(600.0f, 350.0f));
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
	glutMotionFunc(mouseMove);

	glutTimerFunc(10, keyProcess, 0);

	glutMainLoop();
	return 0;
}

#pragma endregion