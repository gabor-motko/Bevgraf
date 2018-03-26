#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
#include <string>
#include <vector>
#include <fstream>

void initVectors();

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

struct 
{
	vec3 black = vec3(0.0f, 0.0f, 0.0f);
	vec3 darkblue = vec3(0.0f, 0.0f, 0.7f);
	vec3 darkred = vec3(0.7f, 0.0f, 0.0f);
	vec3 red = vec3(1.0f, 0.2f, 0.0f);
	vec3 orange = vec3(1.0f, 0.5f, 0.0f);
	vec3 yellow = vec3(1.0f, 1.0f, 0.0f);
	vec3 blue = vec3(0.0f, 0.0f, 1.0f);
	vec3 gray = vec3(0.35f, 0.35f, 0.35f);
	vec3 sky = vec3(0.0f, 0.5f, 1.0f);
	vec3 ground = vec3(0.0f, 0.6f, 0.0f);
} colors;

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();
bool * const mousePreviousStates = new bool[5]();

vec2 * selectedCP = NULL;
#define CP_R 4.0f

double delta, currentTime, previousTime;

#define V_B_CP			(char)0b00000001
#define V_B_POLY		(char)0b00000010
#define V_H_CP			(char)0b00000100
#define V_H_POLY		(char)0b00001000
#define V_DC_CP			(char)0b00010000
#define V_DC_POLY		(char)0b00100000
#define V_DC_SEGMENTS	(char)0b01000000

char visualize = V_B_CP | V_H_CP | V_DC_CP;
float visualize_t = 0.0f;

std::vector<vec2> bernsteinPoints;	//Bernstein-Bezier görbe pontjai
std::vector<vec2> dcPoints;			//de-Casteljau-Bezier görbe pontjai
std::vector<vec2> hermitePoints;	//Hermite görbe pontjai
std::vector<vec2> dcVisPoints;		//de-Casteljau algoritmus osztópontjai

const vec3 hermiteT = { -1.0f, -0.5f, 1.0f};
mat4 hermiteM;
mat24 hermiteG;

#pragma endregion

#pragma region MATH

//Lineáris interpoláció két vektor között
vec2 lerpv2(vec2 a, vec2 b, float t)
{
	assert(t >= 0.0 && t <= 1.0);
	return (1 - t) * a + t * b;
}

//Binomiális együttható
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

//Bernstein polinóm
float bernsteinPolynomial(int i, int n, float t)
{
	return binomialCoefficient(n, i) * pow(t, i) * pow(1.0f - t, n - i);
}

//Pont kiszámítása de-Casteljau algoritmusával
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

//Hermite-görbe T vektora
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

void drawText(float x, float y, const char * s)
{
	glRasterPos2f(x, y);
	for (int i = 0; s[i] != '\0'; ++i)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i]);
}

void drawTextOverlay()
{
	colorV3(colors.black);
	float left = 30.0f;
	float top = win.height - 20.0f;
	std::string str;
	glRasterPos2f(left, top);
	drawText(left, top, "Vizualizacio:");

	left = 45.0f;
	top -= 14.0f;
	str = (visualize & V_B_CP) ? "1: [x] " : "1: [ ] ";
	str += "Bernstein kontrollpontok";
	drawText(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_B_POLY) ? "2: [x] " : "2: [ ] ";
	str += "Bernstein kontrollpoligon";
	drawText(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_H_CP) ? "3: [x] " : "3: [ ] ";
	str += "Hermite kontrollpontok";
	drawText(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_H_POLY) ? "4: [x] " : "4: [ ] ";
	str += "Hermite kontrollpoligon";
	drawText(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_DC_CP) ? "5: [x] " : "5: [ ] ";
	str += "de-Casteljau kontrollpontok";
	drawText(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_DC_POLY) ? "6: [x] " : "6: [ ] ";
	str += "de-Casteljau kontrollpoligon";
	drawText(left, top, str.c_str());

	top -= 14.0f;
	str = (visualize & V_DC_SEGMENTS) ? "7: [x] " : "7: [ ] ";
	str += "de-Casteljau szakaszok";
	str += " (t = " + std::to_string(visualize_t).substr(0, 4) + ")";
	drawText(left, top, str.c_str());

	top = win.height - 20.0f;
	left = 500.0f;
	drawText(left, top, "<- ->: P(t) mozgatasa");
	top -= 14.0f;
	drawText(left, top, "X: kilepes");
}

void drawBackground()
{
	colorV3(colors.ground);
	glRectf(0.0f, 0.0f, win.width, 160.0f);
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
	if (visualize & V_B_CP)
	{
		colorV3(colors.darkblue);
		glBegin(GL_POINTS);
		for (int i = 0; i < bernsteinPoints.size(); ++i)
		{
			glVertex2f(bernsteinPoints[i].x, bernsteinPoints[i].y);
		}
		glEnd();
	}
	if (visualize &V_B_POLY)
	{
		colorV3(colors.black);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < bernsteinPoints.size(); ++i)
			glVertex2f(bernsteinPoints[i].x, bernsteinPoints[i].y);
		glEnd();
	}
}

void drawHermite()
{
	vec4 tv;
	vec2 p;
	colorV3(colors.yellow);
	glBegin(GL_LINE_STRIP);
	for (float t = hermiteT[0]; t <= hermiteT[2]; t += (hermiteT[2] - hermiteT[0]) / 64.0f)
	{
		tv = getHermiteT(t);
		p = hermiteG * hermiteM * tv;
		glVertex2f(p.x, p.y);
	}
	glEnd();
}

void visualizeHermite()
{
	if (visualize & V_H_CP)
	{
		colorV3(colors.darkblue);
		glBegin(GL_POINTS);
		for (int i = 0; i < hermitePoints.size(); ++i)
		{
			glVertex2f(hermitePoints[i].x, hermitePoints[i].y);
		}
		glEnd();
	}
	if (visualize & V_H_POLY)
	{
		colorV3(colors.black);
		glBegin(GL_LINE_STRIP);
		glVertex2f(hermitePoints[0].x, hermitePoints[0].y);
		glVertex2f(hermitePoints[1].x, hermitePoints[1].y);
		glVertex2f(hermitePoints[2].x, hermitePoints[2].y);
		glEnd();
	}
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
	if (visualize & V_DC_CP)
	{
		colorV3(colors.darkblue);
		glBegin(GL_POINTS);
		for (int i = 0; i < dcPoints.size(); ++i)
		{
			glVertex2f(dcPoints[i].x, dcPoints[i].y);
		}
		glEnd();
	}
	colorV3(colors.blue);
	if (visualize & V_DC_POLY || visualize & V_DC_SEGMENTS)
	{
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < dcPoints.size(); ++i)
			glVertex2f(dcPoints[i].x, dcPoints[i].y);
		glEnd();
	}
	if (visualize & V_DC_SEGMENTS)
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
	glVertex2f(hermitePoints[2].x, hermitePoints[2].y);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(dcPoints[0].x, dcPoints[0].y);
	glVertex2f(dcPoints[4].x, dcPoints[4].y);
	glEnd();
	colorV3(colors.black);
	glBegin(GL_LINES);
	glVertex2f(hermitePoints[0].x, hermitePoints[0].y);
	glVertex2f(bernsteinPoints[0].x, bernsteinPoints[0].y);
	glEnd();
}

void drawWheels()
{
	vec2 front = lerpv2(bernsteinPoints[0], hermitePoints[0], 0.25f);
	vec2 rear = lerpv2(bernsteinPoints[0], hermitePoints[0], 0.75f);
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
		visualize ^= V_B_CP;
	}
	if (keyPress('2'))
	{
		visualize ^= V_B_POLY;
	}
	if (keyPress('3'))
	{
		visualize ^= V_H_CP;
	}
	if (keyPress('4'))
	{
		visualize ^= V_H_POLY;
	}
	if (keyPress('5'))
	{
		visualize ^= V_DC_CP;
	}
	if (keyPress('6'))
	{
		visualize ^= V_DC_POLY;
	}
	if (keyPress('7'))
	{
		visualize ^= V_DC_SEGMENTS;
	}
	if (keyStates[GLUT_KEY_LEFT])
	{
		if (visualize_t > 0.0f)
		{
			visualize_t -= 0.5f * delta;
			if (visualize_t < 0.0f)
				visualize_t = 0.0f;
		}
	}
	if (keyStates[GLUT_KEY_RIGHT])
	{
		if (visualize_t < 1.0f)
		{
			visualize_t += 0.5f * delta;
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
			if (dist2(vec2(x, win.height - y), bernsteinPoints[i]) <= CP_R * CP_R)
			{
				selectedCP = &bernsteinPoints[i];
				break;
			}
		}
		if (selectedCP == NULL)
		{
			for (int i = 0; i < dcPoints.size(); ++i)
			{
				if (dist2(vec2(x, win.height - y), dcPoints[i]) <= CP_R * CP_R)
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
				if (dist2(vec2(x, win.height - y), hermitePoints[i]) <= CP_R * CP_R)
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

	//Hermite G mátrix frissítése
	hermiteG = mat24(hermitePoints[0], hermitePoints[1], bernsteinPoints[9] - hermitePoints[2], hermitePoints[2]);

	//Kirajzolás

	drawBackground();
	drawTextOverlay();

	glLineWidth(2.0f);
	drawLines();
	drawWheels();
	drawHermite();
	drawDeCasteljau();
	drawConnectedBernstein();

	//Vizualizáció
	glLineWidth(1.0f);
	visualizeBernstein();
	visualizeDeCasteljau();
	visualizeHermite();


	glutSwapBuffers();
}

void init()
{
	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	glClearColor(colors.sky.x, colors.sky.y, colors.sky.z, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, win.width, 0, win.height);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(CP_R * 2);
	hermiteM = inverse(mat4(getHermiteT(hermiteT[0]), getHermiteT(hermiteT[1]), getHermiteT(1.0f, true), getHermiteT(hermiteT[2], false), true));
}

void initVectors()
{
	bernsteinPoints.push_back(vec2(85.0f, 200.0f));
	bernsteinPoints.push_back(vec2(85.0f, 240.0f));
	bernsteinPoints.push_back(vec2(85.0f, 250.0f));
	bernsteinPoints.push_back(vec2(133.0f, 250.0f));
	bernsteinPoints.push_back(vec2(198.0f, 250.0f));
	bernsteinPoints.push_back(vec2(152.0f, 300.0f));
	bernsteinPoints.push_back(vec2(193.0f, 361.0f));
	bernsteinPoints.push_back(vec2(230.0f, 418.0f));
	bernsteinPoints.push_back(vec2(358.0f, 472.0f));
	bernsteinPoints.push_back(vec2(502.0f, 437.0f));
	hermitePoints.push_back(vec2(740.0f, 200.0f));
	hermitePoints.push_back(vec2(740.0f, 250.0f));
	hermitePoints.push_back(vec2(592.0f, 344.0f));
	dcPoints.push_back(vec2(270.0f, 310.0f));
	dcPoints.push_back(vec2(292.0f, 385.0f));
	dcPoints.push_back(vec2(369.0f, 424.0f));
	dcPoints.push_back(vec2(477.0f, 418.0f));
	dcPoints.push_back(vec2(535.0f, 320.0f));


}

int main(int argc, char * argv[])
{
	initVectors();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
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