#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
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
typedef struct TArea
{
	GLint left = 50;
	GLint right = 750;
	GLint bottom = 50;
	GLint top = 550;
	TArea(int l, int r, int b, int t)
	{
		left = l;
		right = r;
		top = t;
		bottom = b;
	}
} AREA;
AREA area = { 0, 800, 0, 600 };
typedef struct TMouse
{
	int x, y;
	TMouse(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
	vec2 pos()
	{
		return vec2(x, y);
	}
} MOUSE;
MOUSE mouse = { 0, 0 };

std::vector<vec2> points;
int selectedPoint = -1;
float handleRadius = 5.0f;

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();	//0: LBM, 1: wheel, 2: RMB, 3: up, 4: down
bool * const mousePreviousStates = new bool[5]();

double currentTime, previousTime, delta;	//idõzítés
bool visualize = false;	//Ha igaz, kirajzolódik a kontrollpoligon

#define ANIM_OFF 0		//Animáció nem lehetséges - nincs kirajzolva az Hermite-ív
#define ANIM_READY 1	//Az animáció kezdeti helyzete
#define ANIM_PLAYING 2	//A tanuló hazafelé megy
#define ANIM_FINISHED 3	//A tanuló hazaért
#define ANIM_PAUSED 4	//A tanuló megállt egy zebránál
int animStatus = ANIM_OFF;
float animT = 0.0f;
float animEnd = 1.0f;
float animStep = 0.1f;

const vec3 skyDay = { 0.0f, 0.6f, 1.0f };
const vec3 skyNight = { 0.0f, 0.0f, 0.4f };
float starTimer = 0.0f;
std::vector<vec2> stars = std::vector<vec2>(20);

#pragma endregion

#pragma region MATHS

//Lineáris interpoláció két vec3 között, t: R[0, 1] intervallumon
vec3 lerpv3(vec3 a, vec3 b, float t)
{
	return (1 - t) * a + t * b;
}

#pragma endregion


#pragma region GEOMETRY

//p1-p2 egyenes normálvektora
vec2 getNormal(vec2 p1, vec2 p2, bool unit = true)
{
	vec2 dir = p2 - p1;
	if (unit)
		return normalize(vec2(dir.y, -dir.x));
	else
		return vec2(dir.y, -dir.x);
}

//Hermite-ív
class HermiteCurve
{
public:
	int detail = 64;
	mat24 G;
	mat4 M;
	float t0 = -1.0f;
	float t1 = 0.0f;
	float t2 = 1.0f;
	float t3 = 2.0f;
	//t-hez tartozó T vagy T' vektor
	vec4 tVector(float t, bool tangent = false)
	{
		if (tangent)
			return vec4(3 * pow(t, 2), 2 * t, 1, 0);
		return vec4(pow(t, 3), pow(t, 2), t, 1);
	}
	//A görbe t pontjának helye, vagy a hozzá tartozó érintõ
	vec2 point(float t, bool tangent = false)
	{
		vec4 T = tVector(t, tangent);
		return G * M * T;
	}
	//Kirajzolás G vektorai alapján
	void draw(vec2 p1, vec2 p2, vec2 p3, vec2 p4)
	{
		this->G = mat24(p1, p2, p3, p4);
		this->M = inverse(mat4(tVector(t0), tVector(t1), tVector(t2), tVector(t3), true));
		glBegin(GL_LINE_STRIP);
		for (float t = t0; t <= t3; t += 1.0f / detail)
		{
			vec2 v = this->point(t);
			glVertex2f(v.x, v.y);
		}
		glEnd();
	}
	//G és M beállítása rajzolás nélkül
	void set(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int _detail)
	{
		this->G = mat24(p1, p2, p3, p4);
		this->M = inverse(mat4(tVector(t0), tVector(t1), tVector(t2), tVector(t3), true));
		this->detail = _detail;
	}
};
HermiteCurve curve;

//--- CIRCLE
//Kör rajzolása üres vagy teli sokszögként
void drawCircle(vec2 pos, float _radius, bool filled, int detail = 18)
{
	if (filled)
		glBegin(GL_POLYGON);
	else
		glBegin(GL_LINE_LOOP);
	for (double t = 0; t <= 2 * pi(); t += pi() / detail)
		glVertex2d(pos.x + _radius * cos(t), pos.y + _radius * sin(t));
	glEnd();
}

void vertexv2(vec2 pos)
{
	glVertex2f(pos.x, pos.y);
}

#pragma endregion

#pragma region GAME

//--- ANIMÁCIÓ
void drawPerson(float t)
{
	float ts = t * (curve.t3 - curve.t0) + curve.t0;
	vec2 pos = curve.point(ts, false);
	vec2 tan = normalize(curve.point(ts, true));
	vec2 nrm = vec2(-tan.y, tan.x);

	float head_r = 4.0f;
	vec2 head = pos + nrm * 20;
	vec2 hand1 = pos + nrm * 15;
	vec3 hand2 = pos + nrm * 15 + tan * 3;
	vec2 leg1 = pos + nrm * 10;
	vec2 leg2 = curve.point(ts + 0.03f, false);

	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2f(pos.x, pos.y);
	glVertex2f(head.x, head.y);
	glVertex2f(hand1.x, hand1.y);
	glVertex2f(hand2.x, hand2.y);
	glVertex2f(leg1.x, leg1.y);
	glVertex2f(leg2.x, leg2.y);
	glEnd();
	drawCircle(head + nrm * head_r, head_r, false, 6);
}

void drawHouse(bool lights)
{
	vec2 pos = curve.point(curve.t3);
	vec2 tan = -normalize(curve.point(curve.t3, true));
	vec2 nrm = vec2(tan.y, -tan.x);

	float h = 25.0f;
	float w = 40.0f;

	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_POLYGON);
	vertexv2(pos + (w * tan + h * nrm));
	vertexv2(pos + w * tan);
	vertexv2(pos);
	vertexv2(pos + h * nrm);
	glEnd();
	glColor3f(0.8f, 0.3f, 0.3f);
	glBegin(GL_POLYGON);
	vertexv2(pos + (w * tan + h * nrm));
	vertexv2(pos + ((w / 2.0f) * tan + 1.5f * h * nrm));
	vertexv2(pos + h * nrm);
	glEnd();
	if (lights)
	{
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_POLYGON);
		vertexv2(pos + (0.75f * w * tan + 0.75f * h * nrm));
		vertexv2(pos + (0.25f * w * tan + 0.75f * h * nrm));
		vertexv2(pos + (0.25f * w * tan + 0.25f * h * nrm));
		vertexv2(pos + (0.75f * w * tan + 0.25f * h * nrm));
		glEnd();
	}
}

void drawSky()
{
	//Csillagok rajzolása csak ha a tanuló félúton jár, randomizálás másodpercenként
	starTimer += delta;
	if (animT > 0.5f)
	{
		if (starTimer >= 1.0f)
		{
			starTimer = 0.0f;
			//Nagyobb lépésközû görbe a legmagasabb pont megbecsléséhez
			HermiteCurve testCurve = HermiteCurve();
			testCurve.set(points[0], points[1], points[2], points[3], 16);
			int highest = 0;
			for (float t = testCurve.t0; t <= testCurve.t3; t += (testCurve.t3 - testCurve.t0) / testCurve.detail)
			{
				int p = testCurve.point(t).y;
				if (p > highest)
					highest = p;
			}
			for (int i = 0; i < stars.size(); ++i)
			{
				stars[i] = vec2(rand() % win.width, (rand() % (win.height - highest)) + highest);
			}
		}
		glPointSize(2.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_POINTS);
		for (int i = 0; i < stars.size(); ++i)
		{
			glVertex2f(stars[i].x, stars[i].y);
		}
		glEnd();
	}
}

void processAnimation()
{
	//Animáció rajzolása csak akkor, ha az ív is létezik
	if (animStatus != ANIM_OFF)
	{
		drawSky();
		drawHouse(animStatus == ANIM_FINISHED);
		if (animT >= animEnd)
		{
			animStatus = ANIM_FINISHED;
			animT = animEnd;
		}
		if (animStatus != ANIM_FINISHED)
		{
			drawPerson(animT);
		}
		if (animStatus == ANIM_PLAYING)
		{
			animT += animStep * delta;
		}
	}
}

#pragma endregion


#pragma region INPUT

void mouseButtonPressed(int button, int state, int x, int y)
{
	mouseStates[button] = state;
	mouse.x = x;
	mouse.y = win.height - y;
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			int i;
			for (i = 0; i < points.size() && points.size() > 0; ++i)
			{
				if (dist(points[i], mouse.pos()) < handleRadius)
				{
					selectedPoint = i;
					break;
				}
			}
			if (i >= points.size())
			{
				selectedPoint = -1;
				if (points.size() < 4)
				{
					points.push_back(mouse.pos());
					if (points.size() >= 4)
						animStatus = ANIM_READY;
				}
			}
		}
		if (button == GLUT_RIGHT_BUTTON)
		{
			if (points.size() > 0)
			{
				points.pop_back();
				animStatus = ANIM_OFF;
			}
		}
	}
}

void mouseMove(int x, int y)
{
	mouse.x = x;
	mouse.y = win.height - y;
}

void keyOps(int val)
{
	if (keyStates['x'])
		exit(0);
	if (keyStates[' '] && !keyPreviousStates[' '])
	{
		if (points.size() >= 4)
		{
			if (animStatus == ANIM_READY || animStatus == ANIM_PAUSED)
				animStatus = ANIM_PLAYING;
			else if (animStatus == ANIM_PLAYING)
				animStatus = ANIM_PAUSED;
			if (animStatus == ANIM_FINISHED)
			{
				animT = 0.0f;
				animStatus = ANIM_PLAYING;
			}
		}
		else
		{
			animStatus = ANIM_OFF;
			animT = 0.0f;
		}
	}
	if (keyStates['v'] && !keyPreviousStates['v'])
	{
		visualize = !visualize;
	}

	if (mouseStates[GLUT_LEFT_BUTTON] == GLUT_DOWN)
	{
		if (selectedPoint >= 0)
		{
			points[selectedPoint] = mouse.pos();
		}
	}
	if (mouseStates[GLUT_LEFT_BUTTON] == GLUT_UP)
	{
		selectedPoint = -1;
	}

	memcpy(keyPreviousStates, keyStates, 256 * sizeof(bool));
	memcpy(mousePreviousStates, mouseStates, 5 * sizeof(bool));

	glutPostRedisplay();
	glutTimerFunc(5, keyOps, 0);
}

void keyDown(unsigned char key, int x, int y)
{
	keyStates[key] = true;
}

void keyUp(unsigned char key, int x, int y)
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

void init()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0f, win.width, 0.0f, win.height);

	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void drawScene()
{
	vec3 skyColor = lerpv3(skyDay, skyNight, animT);
	glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Fizika
	getDelta();

	//Objektumok kirajzolása
	glColor3f(0.0, 0.0, 0.0);
	if (visualize)
	{
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < points.size(); ++i)
			glVertex2f(points[i].x, points[i].y);
		glEnd();
	}
	if (points.size() >= 4)
	{
		curve.draw(points[0], points[1], points[2], points[3]);
	}
	glColor3f(0.0f, 1.0f, 0.0f);
	for (vec2& cp : points)
	{
		drawCircle(cp, handleRadius, true, 6);
	}

	//Animáció
	processAnimation();

	glutSwapBuffers();
}

#pragma endregion

int main(int argc, char * argv[])
{
	srand(time(NULL));

	puts("Space: indulás/megállás\nV: kontrollpoligon kirajzolása\n");

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(50, 100);
	glutInitWindowSize(win.width, win.height);
	glutCreateWindow(argv[0]);

	init();
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutMouseFunc(mouseButtonPressed);
	glutMotionFunc(mouseMove);
	glutTimerFunc(5, keyOps, 0);
	glutMainLoop();
}