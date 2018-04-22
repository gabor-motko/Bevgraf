#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#pragma region GLOBALS
struct
{
	int width = 800;
	int height = 600;
	float divider = 400.0f;
	vec2 size()
	{
		return vec2(this->width, this->height);
	}
	bool wireframe = true;
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
	static vec3 rotation;
	static mat4 getRotMatrix()
	{
		return rotateX(rotation.x) * rotateY(rotation.y) * rotateZ(rotation.z);
	}
	vec3 points[4];
	vec3 color;
	vec3 normal()
	{
		return cross(points[1] - points[0], points[2] - points[0]);
	}
	vec3 center()
	{
		return (points[0] + points[1] + points[2] + points[3]) / 4.0;
	}
	Quad(vec3 p0, vec3 p1, vec3 p2, vec3 p3, vec3 color)
	{
		this->points[0] = p0;
		this->points[1] = p1;
		this->points[2] = p2;
		this->points[3] = p3;
		this->color = color;
	}

	static bool compareOrtho(Quad a, Quad b)
	{
		return a.center().z < b.center().z;
	}
	static bool comparePerspective(Quad a, Quad b)
	{
		return fabs(a.center().z - projCenter) > fabs(b.center().z - projCenter);
	}
};

vec3 Quad::rotation = vec3();

std::vector<vec3> points;
std::vector<Quad> faces;

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

void drawCube(std::vector<Quad> quads, bool useOrtho)
{
	mat4 w2v = windowToViewport3(vec2(-2, -2), vec2(4.0, 4.0), view.pos(), view.size());
	mat4 proj;
	if (useOrtho)
	{
		std::sort(quads.begin(), quads.end(), Quad::compareOrtho);
		proj = ortho();
	}
	else
	{
		std::sort(quads.begin(), quads.end(), Quad::comparePerspective);
		proj = perspective(projCenter);
	}
	vec3 * projected = new vec3[4];
	for (int i = 0; i < quads.size(); ++i)
	{
		Quad f = quads[i];
		for (int j = 0; j < 4; ++j)
		{
			projected[j] = hToIh(w2v * proj * ihToH(f.points[j]));
		}

		if (win.wireframe)
		{
			colorv3(colors.white);
			glBegin(GL_LINE_LOOP);
		}
		else
		{
			colorv3(f.color);
			glBegin(GL_QUADS);
		}
		glVertex2f(projected[0].x, projected[0].y);
		glVertex2f(projected[1].x, projected[1].y);
		glVertex2f(projected[2].x, projected[2].y);
		glVertex2f(projected[3].x, projected[3].y);
		glEnd();
	}
}

void drawCubeLocal(std::vector<Quad> quads, bool useOrtho)
{
	mat4 rot = Quad::getRotMatrix();
	mat4 w2v = windowToViewport3(vec2(-2, -2), vec2(4.0, 4.0), view.pos(), view.size());
	mat4 proj;
	std::vector<Quad> rotated = std::vector<Quad>(quads);
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

		if (win.wireframe)
		{
			colorv3(colors.white);
			glBegin(GL_LINE_LOOP);
		}
		else
		{
			colorv3(f.color);
			glBegin(GL_QUADS);
		}
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

	str = "Center = " + std::to_string(projCenter);
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
		win.wireframe = !win.wireframe;
	}
	if (keyStates['w'])
	{
		mat4 rot = rotateX(degToRad(-60 * delta));
		for (int i = 0; i < faces.size(); ++i)
		{
			Quad * f = &faces[i];
			for (int j = 0; j < 4; ++j)
			{
				f->points[j] = hToIh(rot * ihToH(f->points[j]));
			}
		}
	}
	if (keyStates['s'])
	{
		mat4 rot = rotateX(degToRad(60 * delta));
		for (int i = 0; i < faces.size(); ++i)
		{
			Quad * f = &faces[i];
			for (int j = 0; j < 4; ++j)
			{
				f->points[j] = hToIh(rot * ihToH(f->points[j]));
			}
		}
	}
	if (keyStates['a'])
	{
		mat4 rot = rotateY(degToRad(60 * delta));
		for (int i = 0; i < faces.size(); ++i)
		{
			Quad * f = &faces[i];
			for (int j = 0; j < 4; ++j)
			{
				f->points[j] = hToIh(rot * ihToH(f->points[j]));
			}
		}
	}
	if (keyStates['d'])
	{
		mat4 rot = rotateY(degToRad(-60 * delta));
		for (int i = 0; i < faces.size(); ++i)
		{
			Quad * f = &faces[i];
			for (int j = 0; j < 4; ++j)
			{
				f->points[j] = hToIh(rot * ihToH(f->points[j]));
			}
		}
	}
	if (keyStates['q'])
	{
		mat4 rot = rotateZ(degToRad(60 * delta));
		for (int i = 0; i < faces.size(); ++i)
		{
			Quad * f = &faces[i];
			for (int j = 0; j < 4; ++j)
			{
				f->points[j] = hToIh(rot * ihToH(f->points[j]));
			}
		}
	}
	if (keyStates['e'])
	{
		mat4 rot = rotateZ(degToRad(-60 * delta));
		for (int i = 0; i < faces.size(); ++i)
		{
			Quad * f = &faces[i];
			for (int j = 0; j < 4; ++j)
			{
				f->points[j] = hToIh(rot * ihToH(f->points[j]));
			}
		}
	}
	if (keyStates['r'])
	{
		projCenter += 5.0f * delta;
	}
	if (keyStates['f'])
	{
		projCenter -= 5.0f * delta;
	}

	if (mouseStates[GLUT_LEFT_BUTTON] == GLUT_DOWN)
	{
		if (selectedDivider)
		{
			win.divider = mousePos.x;
		}
		else
		{
			view.setPos(mousePos + relativePos);
		}
	}

	memcpy(keyPreviousStates, keyStates, 256 * sizeof(bool));
	memcpy(mousePreviousStates, mouseStates, 5 * sizeof(bool));

	glutPostRedisplay();
	glutTimerFunc(10, keyProcess, 0);
}
void keyProcessLocal(int x)
{
	if (keyStates['x'])
	{
		exit(0);
	}
	if (keyStates['w'])
	{
		Quad::rotation.x -= degToRad(60 * delta);
	}
	if (keyStates['s'])
	{
		Quad::rotation.x += degToRad(60 * delta);
	}
	if (keyStates['a'])
	{
		Quad::rotation.y -= degToRad(60 * delta);
	}
	if (keyStates['d'])
	{
		Quad::rotation.y += degToRad(60 * delta);
	}
	if (keyStates['q'])
	{
		Quad::rotation.z -= degToRad(60 * delta);
	}
	if (keyStates['e'])
	{
		Quad::rotation.z += degToRad(60 * delta);
	}
	if (keyStates['r'])
	{
		projCenter += 5.0f * delta;
	}
	if (keyStates['f'])
	{
		projCenter -= 5.0f * delta;
	}

	if (mouseStates[GLUT_LEFT_BUTTON] == GLUT_DOWN)
	{
		if (selectedDivider)
		{
			win.divider = mousePos.x;
		}
		else
		{
			view.setPos(mousePos + relativePos);
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

void mouseButton(int button, int state, int x, int y)
{
	mouseStates[button] = state;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		relativePos = view.pos() - vec2((float)x, (float)win.height - (float)y);
		printf("relativePos: %f %f\n", relativePos.x, relativePos.y);
	}
	printf("%d %d\n", button, state);
	selectedDivider = (fabs(x - win.divider) < 2.0f);
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
}

void display()
{
	getDelta();
	glClear(GL_COLOR_BUFFER_BIT);

	colorv3(colors.white);
	drawCube(faces, true);

	colorv3(colors.sky);
	glScissor(0.0f, 0.0f, win.divider, win.height);
	glEnable(GL_SCISSOR_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
	glRectf(0.0f, 0.0f, win.divider, win.height);

	colorv3(colors.white);
	drawCube(faces, false);

	glDisable(GL_SCISSOR_TEST);

	colorv3(colors.black);
	glBegin(GL_LINES);
	glVertex2f(win.divider, 0.0f);
	glVertex2f(win.divider, win.height);
	glEnd();

	drawTextOverlay();

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
}

void initVectors()
{
	points.push_back(vec3(1.0, 1.0, 1.0));		//0
	points.push_back(vec3(1.0, 1.0, -1.0));		//1
	points.push_back(vec3(1.0, -1.0, 1.0));		//2
	points.push_back(vec3(1.0, -1.0, -1.0));	//3
	points.push_back(vec3(-1.0, 1.0, 1.0));		//4
	points.push_back(vec3(-1.0, 1.0, -1.0));	//5
	points.push_back(vec3(-1.0, -1.0, 1.0));	//6
	points.push_back(vec3(-1.0, -1.0, -1.0));	//7

	faces.push_back(Quad(points[7], points[3], points[1], points[5], colors.red));
	faces.push_back(Quad(points[2], points[6], points[4], points[0], colors.green));
	faces.push_back(Quad(points[3], points[2], points[0], points[1], colors.blue));
	faces.push_back(Quad(points[6], points[7], points[5], points[4], colors.yellow));
	faces.push_back(Quad(points[5], points[1], points[0], points[4], colors.orange));
	faces.push_back(Quad(points[6], points[2], points[3], points[7], colors.darkblue));
}

int main(int argc, char * argv[])
{
	initVectors();

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