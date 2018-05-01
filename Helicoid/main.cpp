//Motkó Gábor BZ79WI

#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

//#define USE_LOCAL_AXES
//#define ENABLE_LIGHT

#pragma region GLOBALS
struct
{
	int width = 800;
	int height = 600;
	vec2 size()
	{
		return vec2(this->width, this->height);
	}
	float fps;
	bool ortho = true;
} win;

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

} colors;

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();
bool * const mousePreviousStates = new bool[5]();
vec2 mousePos = vec2();
vec2 relativePos = vec2();
bool selectedDivider = false;

double delta, currentTime, previousTime;
float visualize_t = 0.0f;

float projCenter = 5.0f;


class Quad
{
public:
	//Alapértelmezett szín
	static vec3 defaultColor;
	//Merõleges tengelyek körüli szögek
	static vec3 rotation;
	//Forgatási mátrix
	static mat4 getRotMatrix()
	{
		//return rotateX(rotation.x) * rotateY(rotation.y) * rotateZ(rotation.z);
		return rotateZ(Quad::rotation.z) * rotateY(Quad::rotation.y) * rotateX(Quad::rotation.x);
	}
	vec3 points[4];
	vec3 color;
	//Normál vektor
	vec3 getNormal()
	{
		return cross(points[1] - points[0], points[2] - points[0]);
	}
	//Középpont
	vec3 getCenter()
	{
		return (points[0] + points[1] + points[2] + points[3]) / 4.0f;
	}
	//Konstruktor: négy pont és szín
	Quad(vec3 p0, vec3 p1, vec3 p2, vec3 p3, vec3 color)
	{
		this->points[0] = p0;
		this->points[1] = p1;
		this->points[2] = p2;
		this->points[3] = p3;
		this->color = color;
	}
	//Konstruktor: négy pont
	Quad(vec3 p0, vec3 p1, vec3 p2, vec3 p3)
	{
		this->points[0] = p0;
		this->points[1] = p1;
		this->points[2] = p2;
		this->points[3] = p3;
		this->color = Quad::defaultColor;
	}

	//Összehasonlító függvény merõleges vetítéshez
	static bool compareOrtho(Quad a, Quad b)
	{
		return a.getCenter().z < b.getCenter().z;
	}
	//Összehasonlító függvény centrális vetítéshez
	static bool comparePerspective(Quad a, Quad b)
	{
		return fabs(a.getCenter().z - projCenter) > fabs(b.getCenter().z - projCenter);
	}
};

vec3 Quad::rotation = vec3();
vec3 Quad::defaultColor = colors.orange;

class Helicoid
{
public:
	vec3 rotation;
	mat4 getRotationMatrix()
	{
		//return rotateZ(this->rotation.z) * rotateY(this->rotation.y) * rotateX(this->rotation.x);
		return rotateX(this->rotation.x) * rotateY(this->rotation.y) * rotateZ(this->rotation.z);
	}

	float alpha = 10.0f;
	float height = 1.0f;
	float radius = 0.5f;

	float uStep = 0.1f;
	float vStep = 0.01f;

	std::vector<Quad> quads;
	void build()
	{
		quads.clear();
		for (float theta = 0.0f; theta <= this->height; theta += this->vStep)
		{
			if (theta > height)
				theta = height;
			for (float rho = 0.0f; rho <= this->radius; rho += this->uStep)
			{
				if (rho > radius)
					rho = radius;
				vec3 p0 = Helicoid::helicoPoint(rho, this->alpha, theta);
				vec3 p1 = Helicoid::helicoPoint(rho + this->uStep, this->alpha, theta);
				vec3 p2 = Helicoid::helicoPoint(rho + this->uStep, this->alpha, theta + this->vStep);
				vec3 p3 = Helicoid::helicoPoint(rho, this->alpha, theta + this->vStep);
				this->quads.push_back(Quad(p0, p1, p2, p3, colors.red));
			}
		}
	}
	static vec3 helicoPoint(float rho, float alpha, float theta)
	{
		return vec3(rho * cosf(alpha * theta), rho * sinf(alpha * theta), theta);
		//return vec3(rho * cosf(alpha * theta), theta, rho * sinf(alpha * theta));
	}
} helicoid;

class Light
{
public:
	vec3 pos;
	vec3 color;
	vec3 calculate(vec3 src, vec3 pos, vec3 nrm)
	{
		vec3 l = this->pos - pos;
		float mult = (dot(normalize(nrm), normalize(l)) + 1) / 2.0f;
		return src * mult;
	}
	Light()
	{
		this->pos = vec3(0.0f);
		this->color = vec3(1.0f);
	}
	Light(vec3 pos)
	{
		this->pos = pos;
		this->color = vec3(1.0f);
	}
	Light(vec3 pos, vec3 color)
	{
		this->pos = pos;
		this->color = color;
	}
};

Light light = Light(vec3(2.0f, 2.0f, 0.0f), colors.white);

void colorv3(vec3 rgb)
{
	glColor3f(rgb.x, rgb.y, rgb.z);
}

void printv2(vec2 v)
{
	printf("(%.2f, %.2f)\n", v.x, v.y);
}

#pragma endregion

#pragma region GRAPHICS

//Kocka kirajzolása helyi tengelyeken forgatva
void drawGeometry(Helicoid helicoid, bool useOrtho = false)
{
	mat4 rot = helicoid.getRotationMatrix();
	mat4 w2v = windowToViewport3(vec2(-2, -2), vec2(4.0, 4.0), view.pos(), view.size());
	mat4 proj;
	std::vector<Quad> rotated = std::vector<Quad>(helicoid.quads);
	for (int i = 0; i < rotated.size(); ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			rotated[i].points[j] = hToIh(rot * ihToH(rotated[i].points[j]));
		}
	}
	if (useOrtho)
	{
		std::sort(rotated.begin(), rotated.end(), Quad::compareOrtho);
		proj = ortho();
	}
	else
	{
		std::sort(rotated.begin(), rotated.end(), Quad::comparePerspective);
		proj = perspective(projCenter);
	}

	vec3 * projected = new vec3[4];
	for (int i = 0; i < rotated.size(); ++i)
	{
		Quad f = rotated[i];
		for (int j = 0; j < 4; ++j)
		{
			projected[j] = hToIh(w2v * proj * ihToH(f.points[j]));
		}

		colorv3(f.color);
		//colorv3(light.calculate(f.color, projected[0], f.getNormal()));
		glBegin(GL_QUADS);
		glVertex2f(projected[0].x, projected[0].y);
		glVertex2f(projected[1].x, projected[1].y);
		glVertex2f(projected[2].x, projected[2].y);
		glVertex2f(projected[3].x, projected[3].y);
		glEnd();

		colorv3(colors.black);
		glBegin(GL_LINE_LOOP);
		glVertex2f(projected[0].x, projected[0].y);
		glVertex2f(projected[1].x, projected[1].y);
		glVertex2f(projected[2].x, projected[2].y);
		glVertex2f(projected[3].x, projected[3].y);
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
	str = "C = " + std::to_string(projCenter);
	drawText(left, top, str.c_str());
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
	if (keyPress('c'))
	{
		win.ortho = !win.ortho;
	}
	if (keyStates['w'])
	{
		helicoid.rotation.x -= degToRad(60 * delta);
	}
	if (keyStates['s'])
	{
		helicoid.rotation.x += degToRad(60 * delta);
	}
	if (keyStates['a'])
	{
		helicoid.rotation.y += degToRad(60 * delta);
	}
	if (keyStates['d'])
	{
		helicoid.rotation.y -= degToRad(60 * delta);
	}
	if (keyStates['q'])
	{
		helicoid.rotation.z += degToRad(60 * delta);
	}
	if (keyStates['e'])
	{
		helicoid.rotation.z -= degToRad(60 * delta);
	}
	if (keyStates['r'])
	{
		projCenter += 1.0f * delta;
	}
	if (keyStates['f'])
	{
		projCenter -= 1.0f * delta;
		if (projCenter < 1.8f)
			projCenter = 1.8f;
	}
	if (keyStates['1'])
	{
		helicoid.height += delta;
		helicoid.build();
	}
	if (keyStates['2'])
	{
		helicoid.height -= delta;
		helicoid.build();
	}
	if (keyStates['3'])
	{
		helicoid.radius += delta;
		helicoid.build();
	}
	if (keyStates['4'])
	{
		helicoid.radius -= delta;
		helicoid.build();
	}
	if (keyStates['5'])
	{
		helicoid.alpha += 10 * delta;
		helicoid.build();
	}
	if (keyStates['6'])
	{
		helicoid.alpha -= 10 * delta;
		helicoid.build();
	}

	if (mouseStates[GLUT_LEFT_BUTTON] == GLUT_DOWN)
	{
		//if (selectedDivider)
		//{
		//	win.divider = mousePos.x;
		//}
		//else
		//{
		//	view.setPos(mousePos + relativePos);
		//}
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

void mouseButton(int button, int state, int x, int y)
{
	mouseStates[button] = state;
	//if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	//{
	//	relativePos = view.pos() - vec2((float)x, (float)win.height - (float)y);
	//	printf("relativePos: %f %f\n", relativePos.x, relativePos.y);
	//}
	//printf("%d %d\n", button, state);
	//selectedDivider = (fabs(x - win.divider) < 2.0f);
}

void mouseMove(int x, int y)
{
	mousePos.x = (float)x;
	mousePos.y = (float)(win.height - y);
}

#pragma endregion

#pragma region SYSTEM

void getDelta()
{
	currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	delta = currentTime - previousTime;
	previousTime = currentTime;
	win.fps = 1.0 / delta;
}

void display()
{
	getDelta();
	glClear(GL_COLOR_BUFFER_BIT);

	//Kirajzolás centrális vetítéssel
	drawGeometry(helicoid, win.ortho);

	//Szöveg
	drawTextOverlay();

	printf("%f\n", delta);
	glutSwapBuffers();
}

void init()
{
	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	vec3 cc = colors.mediumgreen;
	glClearColor(cc.x, cc.y, cc.z, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, win.width, 0, win.height);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glPointSize(3.0f);
	view.left = (win.width - view.width) / 2.0f;
	view.bottom = (win.height - view.height) / 2.0f;
	mouseStates[GLUT_LEFT_BUTTON] = GLUT_UP;
	helicoid.build();
}

int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(win.width, win.height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();

	glutDisplayFunc(display);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutSpecialFunc(keyDown);
	glutSpecialUpFunc(keyUp);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	glutPassiveMotionFunc(mouseMove);

	glutTimerFunc(10, keyProcess, 0);

	glutMainLoop();
	return 0;
}

#pragma endregion