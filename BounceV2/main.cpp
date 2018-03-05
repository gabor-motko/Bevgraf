#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>
//#include <spaghetti.h>

#pragma region GLOBALS

#define STARTSPEED 150
#define MAXSPEED 500
#define MINSPEED 10
#define ACCELERATION 200

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
WINDOW win = { 800, 600 };
AREA area = { 50, 750, 50, 550 };


#define HANDLE_BOTTOMLEFT 0
#define HANDLE_TOPRIGHT 1
#define HANDLE_P1 2
#define HANDLE_P2 3
vec2 * points = new vec2[4]();
int selectedPoint = -1;
int handleRadius = 7;

bool * const keyStates = new bool[256]();
bool * const keyPreviousStates = new bool[256]();
bool * const mouseStates = new bool[5]();	//0: LBM, 1: wheel, 2: RMB, 3: up, 4: down
bool * const mousePreviousStates = new bool[5]();

double currentTime, previousTime, delta;
bool visualize = false;


#pragma endregion

#pragma region GEOMETRY

//2x2 mátrix determinánsa
float det2x2(float a, float b, float c, float d)
{
	return a * d - b * c;
}

//Merõleges vektor pointból a p1-p2 egyenesre
vec2 vecToLine(vec2 p1, vec2 p2, vec2 point)
{
	vec2 v = normalize(p2 - p1);	//p1-p2 egyenes irányvektora
	vec2 p = p1 + (dot(point - p1, v) * v);
	return p - point;
}

//p1-p2 egyenes normálvektora
vec2 getNormal(vec2 p1, vec2 p2, bool unit = true)
{
	vec2 dir = p2 - p1;
	if (unit)
		return normalize(vec2(dir.y, -dir.x));
	else
		return vec2(dir.y, -dir.x);
}

//p1-p2 és p3-p4 egyenesek metszéspontja - hiba, ha párhuzamosak.
vec2 getIntersect(vec2 p1, vec2 p2, vec2 p3, vec2 p4)
{
	float denom = det2x2(p1.x - p2.x, p1.y - p2.y, p3.x - p4.x, p3.y - p4.y);
	assert(denom != 0);
	vec2 out = vec2();
	out.x = det2x2(det2x2(p1.x, p1.y, p2.x, p2.y), p1.x - p2.x, det2x2(p3.x, p3.y, p4.x, p4.y), p3.x - p4.x) / denom;
	out.y = det2x2(det2x2(p1.x, p1.y, p2.x, p2.y), p1.y - p2.y, det2x2(p3.x, p3.y, p4.x, p4.y), p3.y - p4.y) / denom;
	return out;
}

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

//--- LINES

//p1 és p2 ponton átmenõ szakasz vagy egyenes rajzolása
void drawLineByPoints(vec2 p1, vec2 p2, bool infinite, bool handles = true)
{
	vec2 end1, end2;
	if (infinite)
	{
		if (p1.x == p2.x)	//Függõleges
		{
			end1 = vec2(p1.x, area.bottom);
			end2 = vec2(p2.x, area.top);
		}
		else if (p1.y == p2.y)	//Vízszintes
		{
			end1 = vec2(area.left, p1.y);
			end2 = vec2(area.right, p2.y);
		}
		else
		{
			vec2 intersect[4];
			//bal
			intersect[0] = getIntersect(p1, p2, vec2(area.left, 0), vec2(area.left, 1));
			//jobb
			intersect[1] = getIntersect(p1, p2, vec2(area.right, 0), vec2(area.right, 1));
			//alsó
			intersect[2] = getIntersect(p1, p2, vec2(0, area.bottom), vec2(1, area.bottom));
			//felsõ
			intersect[3] = getIntersect(p1, p2, vec2(0, area.top), vec2(1, area.top));

			vec2 dir = p2 - p1;
			//end1 ha skalárszorzat pozitív, end2 ha negatív
			end1 = end2 = NULL;
			for (int i = 0; i <= 3; ++i)
			{
				if (dot(dir, intersect[i] - p1) > 0)
				{
					if (end1 == NULL || (dist(intersect[i], p1) < dist(end1, p1)))
						end1 = intersect[i];
				}
				if (dot(dir, intersect[i] - p1) < 0)
				{
					if (end2 == NULL || (dist(intersect[i], p2) < dist(end2, p2)))
						end2 = intersect[i];
				}
			}
		}
	}
	else
	{
		end1 = p1;
		end2 = p2;
	}
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2i(end1.x, end1.y);
	glVertex2i(end2.x, end2.y);
	glEnd();
	if (handles)
	{
		glColor3f(0.0f, 0.0f, 1.0f);
		drawCircle(p1, handleRadius, true, 8);
		drawCircle(p2, handleRadius, true, 8);
		if (infinite)
		{
			glColor3f(1.0f, 0.0f, 1.0f);
			drawCircle(end1, 3, true);
			drawCircle(end2, 3, true);
		}
	}
}

class Circle
{
public:
	vec2 pos = { 0, 0 };
	vec2 vel = { 0, 0 };
	int radius = 40;
	int detail = 18;
	float color[3] = { 0.0f, 0.0f, 0.0f };
	//Véletlenszerû, érvényes pozíció generálása
	void randomizePosition()
	{
		srand(time(NULL));
		pos.x = rand() % ((area.right - radius / 2) - (area.left + radius / 2) + 1) + area.left;
		pos.y = rand() % ((area.top - radius / 2) - (area.bottom + radius / 2) + 1) + area.bottom;
	}
	//Konstruktor véletlenszerû kezdõ pozícióval és sebességgel. r = 20, detail = 18.
	Circle()
	{
		srand(time(NULL));
		vel = normalize(vec2(rand(), rand())) * STARTSPEED;
		this->randomizePosition();
	}
	//Konstruktor adott sugárral és lépésközzel, de véletlenszerû kezdõ pozícióval és sebességgel
	Circle(int radius, int detail = 18)
	{
		this->radius = radius;
		this->detail = detail;
		Circle();
	}
	//Sebesség nagysága
	float speed()
	{
		return length(vel);
	}
	//Sebességvektor iránya (egység)
	vec2 direction()
	{
		return normalize(vel);
	}
	//Kör kirajzolása
	void draw(bool filled = false)
	{
		glColor3f(color[0], color[1], color[2]);
		drawCircle(pos, radius, filled);
		if (visualize)
		{
			drawLineByPoints(ball.pos, ball.pos + vecToLine(points[HANDLE_P1], points[HANDLE_P2], ball.pos), false, false);
			drawLineByPoints(ball.pos, ball.pos + ball.direction() * ball.radius, false, false);
		}
	}
	//Azonnali visszaverõdés adott normálvektorról
	void bounce(vec2 normal, bool isUnit = false)
	{
		vec2 n = normal;
		if (!isUnit)
			n = normalize(n);
		this->vel = this->vel - 2 * dot(this->vel, n) * n;
	}
} ball;

//Ellenõrzi, hogy a labdának vissza kell-e pattannia egy egyenesrõl.
void checkBounces()
{
	//P1-P2 egyenesrõl csak akkor verõdjön vissza, ha metszi azt és a zöld területre nézõ oldala felé halad
	vec2 nrm = getNormal(points[HANDLE_P1], points[HANDLE_P2], true);
	vec2 distv = vecToLine(points[HANDLE_P1], points[HANDLE_P2], ball.pos);
	float speedToLine = dot(nrm, ball.direction());
	if (length(distv) <= ball.radius && speedToLine < 0)
		ball.bounce(nrm, false);
	
	if(ball.pos.x + ball.radius >= area.right && ball.vel.x > 0)
		ball.bounce(vec2(-1, 0), false);
	if (ball.pos.x - ball.radius <= area.left && ball.vel.x < 0)
		ball.bounce(vec2(1, 0), false);
	if (ball.pos.y + ball.radius >= area.top && ball.vel.y > 0)
		ball.bounce(vec2(0, -1), false);
	if (ball.pos.y - ball.radius <= area.bottom && ball.vel.y < 0)
		ball.bounce(vec2(0, 1), false);
}

//--- BOUNDARIES
//Határoló téglalap és sarokpontok rajzolása
void drawBoundaries()
{
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2i(area.left, area.bottom);
	glVertex2i(area.left, area.top);
	glVertex2i(area.right, area.top);
	glVertex2i(area.right, area.bottom);
	glEnd();
	drawCircle(points[HANDLE_BOTTOMLEFT], handleRadius, true, 8);
	drawCircle(points[HANDLE_TOPRIGHT], handleRadius, true, 8);
}

//--- BACKGROUND
//Háttér
void drawBackground()
{
	vec2 shift = vec2(30, 30);
	vec2 nextPos = vec2(0, 0);
	int col = 0, row = 0;

	for (row = (int)(area.bottom / shift.y) + 1; row * shift.y < area.top; ++row)
	{
		for (col = (int)(area.left / shift.x) + 1; col * shift.x + (row % 2 ? 0.0f : shift.x / 2) < area.right; ++col)
		{
			nextPos = vec2(shift.x * col + (row % 2 ? 0.0f : shift.x / 2), shift.y * row);
			
			vec2 nrm = getNormal(points[HANDLE_P1], points[HANDLE_P2], true);
			vec2 p = vecToLine(points[HANDLE_P1], points[HANDLE_P2], nextPos);
			bool nPlus = dot(nrm, p) > 0;

			if (nPlus)
				glColor3f(1.0f, 0.0f, 0.0f);
			else
				glColor3f(0.0f, 1.0f, 0.0f);
			if (dist(nextPos, ball.pos) < ball.radius)
				glColor3f(0.0f, 1.0f, 1.0f);
			drawCircle(nextPos, 5, true);
		}
	}
}

#pragma endregion

#pragma region INPUT

void movePoint(int index, vec2 pos)
{
	points[selectedPoint] = pos;
	switch (index)
	{
	case HANDLE_BOTTOMLEFT:
		area.left = pos.x;
		area.bottom = pos.y;
		break;
	case HANDLE_TOPRIGHT:
		area.right = pos.x;
		area.top = pos.y;
		break;
	case HANDLE_P1:
		break;
	case HANDLE_P2:
		break;
	default:
		break;
	}
}

void mouseButtonPressed(int button, int state, int x, int y)
{
	mouseStates[button] = !state;
	if (state == GLUT_DOWN)
	{
		//Ha az egérgomb le van nyomva, kijelölöm a kurzor alatti pontot mozgatásra
		for (int i = 0; i < 6; ++i)
		{
			if (dist(points[i], vec2(x, win.height - y)) < handleRadius)
			{
				selectedPoint = i;
			}
		}
	}
	if (state == GLUT_UP)
	{
		selectedPoint = -1;
	}
}

void mouseMove(int x, int y)
{
	//Kiválasztott pont mozgatása, ha a bal egérgomb le van nyomva
	if (mouseStates[0])
	{
		movePoint(selectedPoint, vec2(x, win.height - y));
	}
}

void keyOps(int val)
{
	if (keyStates['x'])
		exit(0);
	if (keyStates['w'] && ball.speed() < MAXSPEED)
		ball.vel += ball.direction() * ACCELERATION * delta;
	if (keyStates['s'] && ball.speed() > MINSPEED)
		ball.vel -= ball.direction() * ACCELERATION * delta;

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
	points[HANDLE_BOTTOMLEFT] = vec2(area.left, area.bottom);
	points[HANDLE_TOPRIGHT] = vec2(area.right, area.top);
	points[HANDLE_P1] = vec2((rand() % (area.right - area.left + 1) + area.left), (rand() % (area.top - area.bottom + 1) + area.bottom));
	points[HANDLE_P2] = vec2((rand() % (area.right - area.left + 1) + area.left), (rand() % (area.top - area.bottom + 1) + area.bottom));

	//Ha a kör az egyenes rossz oldalán van, megcserélem a két pontját
	vec2 nrm = getNormal(points[HANDLE_P1], points[HANDLE_P2], true);
	vec2 p = vecToLine(points[HANDLE_P1], points[HANDLE_P2], ball.pos);
	if (dot(nrm, p) > 0)
	{
		vec2 temp = points[HANDLE_P1];
		points[HANDLE_P1] = points[HANDLE_P2];
		points[HANDLE_P2] = temp;
	}


	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0f, win.width, 0.0f, win.height);

	previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}
void drawScene()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0, 0.0, 0.0);

	//Fizika
	getDelta();
	ball.pos += ball.vel * delta;
	checkBounces();

	//Objektumok kirajzolása
	drawBoundaries();
	drawBackground();
	drawLineByPoints(points[HANDLE_P1], points[HANDLE_P2], true, true);
	ball.draw();

	glutSwapBuffers();
}

#pragma endregion

int main(int argc, char * argv[])
{
	for (int i = 0; argc > 1 && argv[1][i] != '\0'; ++i)
	{
		if (argv[1][i] == 'v')
			visualize = true;
	}
	srand(time(NULL));

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