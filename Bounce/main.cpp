#include <stdio.h>
#include <GL/glut.h>
#include <bevgrafmath2017.h>
#include <math.h>
#include <time.h>

#pragma region GLOBALS

GLsizei winWidth = 800, winHeight = 600;			//Ablak mérete és a játékterület határai
bool * const keyStates = new bool[256]();			//Jelenlegi billentyûállapotok
bool * const keyPreviousStates = new bool[256]();	//Az elõzõ ciklus billentyûállapotai

int currentTime, previousTime;						//Idõzítõ változói
const float velStepSize = 10.0f;					//Gyorsulás és minimum sebesség
const float speedMax = 500.0f;						//Maximum sebesség
const float startSpeed = 200.0f;					//Kezdõ sebesség
GLint radius, targetRadius = 10;					//Kör és cél sugarai
vec2 circlePos, circleVel, targetPos;				//Kör középpontja, kör sebessége, cél középpontja
int score = 0;
const int winScore = 10;

#pragma endregion

#pragma region CIRCLE

//Kör tulajdonságainak inicializálása, biztosítva hogy a kör a játékterületen belül legyen
void initCircle(int _radius)
{
	radius = _radius;
	circleVel = { startSpeed, startSpeed };
	circlePos = { 0, 0 };

	circlePos.x = rand() % winWidth;
	if (circlePos.x <= radius)
		circlePos.x += -(circlePos.x) + radius;
	if (circlePos.x >= winWidth - radius)
		circlePos.x -= (circlePos.x - winWidth) - radius;

	circlePos.y = rand() % winHeight;
	if (circlePos.y <= radius)
		circlePos.y += -(circlePos.y) + radius;
	if (circlePos.y >= winHeight - radius)
		circlePos.y -= (circlePos.y - winHeight) - radius;
}

//Kör rajzolása üres vagy teli sokszögként
void drawCircle(int cx, int cy, float _radius, bool filled)
{
	if (filled)
		glBegin(GL_POLYGON);
	else
		glBegin(GL_LINE_LOOP);
	for (double t = 0; t <= 2 * pi(); t += pi() / 18)
		glVertex2d(cx + _radius * cos(t), cy + _radius * sin(t));
	glEnd();
}

//vec2 overload
void drawCircle(vec2 pos, float radius, bool filled)
{
	drawCircle(pos.x, pos.y, radius, filled);
}

//Eldönti, hogy a körnek vissza kell-e pattannia
void bounce(bool bounceOnX, bool bounceOnY)
{
	if (bounceOnX || circlePos.x < radius || circlePos.x > winWidth - radius)
		circleVel.x = -circleVel.x;
	if (bounceOnY || circlePos.y < radius || circlePos.y > winHeight - radius)
		circleVel.y = -circleVel.y;
}

#pragma endregion

#pragma region TARGET

//Keres egy elérhetõ helyet a célnak
vec2 placeTarget()
{
	vec2 pos = { 0, 0 };
	bool invalid;
	do
	{
		invalid = false;
		pos.x = rand() % winWidth;
		pos.y = rand() % winHeight;

		//A cél nem lehet a körön belül
		if (dist(pos, circlePos) <= (radius + targetRadius))
		{
			invalid = true;
		}
		//A cél nem lehet a sarkokban lévõ r oldalú négyzetekben
		if (pos.x <= radius && pos.y <= radius)	//bal alsó
		{
			invalid = true;
		}
		if (pos.x <= radius && pos.y >= winHeight - radius)	//bal felsõ
		{
			invalid = true;
		}
		if (pos.x >= winWidth - radius && pos.y <= radius)	//jobb alsó
		{
			invalid = true;
		}
		if (pos.x >= winWidth - radius && pos.y >= winHeight - radius)	//jobb felsõ
		{
			invalid = true;
		}
	} while (invalid);
	return pos;
}

//Ha a kör célba talál
void hit()
{
	score++;
	if (score >= winScore)
	{
		printf("Vége\n");
		exit(0);
	}
	printf("%d pont\n", score);
	targetPos = placeTarget();
}

#pragma endregion

#pragma region SYSTEM

void init(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, winWidth, 0.0, winHeight);
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
	keyPreviousStates[key] = false;
}

void keyUp(int key, int x, int y)
{
	keyStates[key] = false;
	keyPreviousStates[key] = false;
}

//Billentyûállapotok kezelése
void keyOperations(int value)
{
	if ((keyStates[GLUT_KEY_LEFT] && !keyPreviousStates[GLUT_KEY_LEFT]) || (keyStates[GLUT_KEY_RIGHT] && !keyPreviousStates[GLUT_KEY_RIGHT]))	//Csak akkor pattanjon vissza a labda, ha a billentyû le van nyomva, de elõzõleg nem volt
		bounce(true, false);
	if ((keyStates[GLUT_KEY_DOWN] && !keyPreviousStates[GLUT_KEY_DOWN]) || (keyStates[GLUT_KEY_UP] && !keyPreviousStates[GLUT_KEY_UP]))
		bounce(false, true);
	if (keyStates['w'])
	{
		if (length(circleVel) <= speedMax)
			circleVel += velStepSize * normalize(circleVel);
	}
	if (keyStates['s'])
	{
		if (length(circleVel) >= velStepSize)
			circleVel -= velStepSize * normalize(circleVel);
	}
	if (keyStates['x'])
		exit(0);

	memcpy(keyPreviousStates, keyStates, 256 * sizeof(char));
	glutPostRedisplay();
	glutTimerFunc(10, keyOperations, 0);
}

void reshapeWindow(int width, int height)
{
	winWidth = width;
	winHeight = height;
	if (circlePos.x > winWidth || circlePos.y > winHeight)
		initCircle(radius);
	if (targetPos.x > winWidth || targetPos.y > winHeight)
		targetPos = placeTarget();
}

void drawScene()
{
	glClear(GL_COLOR_BUFFER_BIT);
	//Idõzítõ
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	double delta = (currentTime - previousTime) / 1000.0f;	//Idõ delta két ciklus között
	previousTime = currentTime;

	//Animáció
	bounce(false, false);			//Visszapattanás
	circlePos += circleVel * delta;	//Állandó mozgás

									//Találat érzékelése
	if (dist(circlePos, targetPos) <= radius + targetRadius)
		hit();

	glColor3f(0.0f, 0.0f, 0.0f);
	drawCircle(circlePos, radius, false);
	glColor3f(0.0f, 0.5f, 1.0f);
	drawCircle(targetPos, targetRadius, true);
	glutSwapBuffers();
}

#pragma endregion

int main(int argc, char * argv[])
{
	printf("Nyilak - visszapattanás\nW/S - gyorsítás\nX - kilépés\n");
	srand(time(NULL));

	targetPos = placeTarget();
	initCircle(25);

	glutInit(&argc, argv);
	previousTime = glutGet(GLUT_ELAPSED_TIME);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(50, 100);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("Bounce");

	init();
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(keyDown);		//Betûbillentyûk - unsigned char
	glutKeyboardUpFunc(keyUp);
	glutSpecialFunc(keyDown);		//Speciális billentyûk - int
	glutSpecialUpFunc(keyUp);
	glutReshapeFunc(reshapeWindow);
	glutTimerFunc(10, keyOperations, 0);
	glutMainLoop();

	return 0;
}


