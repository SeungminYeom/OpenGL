#include <windows.h>
#include <gl/glut.h>
#include <ctime>
#include <random>


//카메라 조작 변수
GLfloat xAngle, yAngle, zAngle;
GLfloat Near = 1, Far = 10;
GLfloat fov = 45;
int Projection;
int Object = 1;
GLsizei lastWidth, lastHeight;

//불꽃의 발사 시작 위치 설정 변수
GLfloat fireStartPosX = 0;
GLfloat fireStartPosY = -0.45f;
GLfloat fireStartPosZ = 0;
//불꽃의 타겟 위치 설정 변수
GLfloat targetPosX, targetPosY, targetPosZ;
//발사 중인 불꽃의 위치 설정 변수
GLfloat firePosX, firePosY, firePosZ;
//2차 베지어 곡선을 그리기 위한 변수
GLfloat bezierAX, bezierAY, bezierAZ;
GLfloat bezierBX, bezierBY, bezierBZ;

//불꽃이 발사 중인지 확인하는 변수
bool isFire = false;
//불꽃을 발사한 시간을 저장하는 변수
int fireStartTime;
//불꽃 확산 시 회전 각도를 저장하는 배열변수
int fireRotX[480];
int fireRotY[480];

//스파클라를 체크하는 변수
bool sparkler = false;
//스파클라의 반짝일 횟수
int maxSparkler = 20;
//스파클라가 반짝인 횟수 저장 변수
int sparklerValue = 0;

//불꽃이 확산할 반지름 저장 변수
float fireLength = 0.4f;
//불꽃 발사 중, 확산 중 불꽃의 크기 저장 변수
int fireScale1 = 2, fireScale2 = 3;
//선형보간 값 저장 변수
float u1(1), u2(1);
//팔레트에서 가져올 인덱스를 저장하는 변수
int colorValue;
//RGBA 저장 변수
float colorR, colorG, colorB, colorA;
//불꽃의 색상 테이블
float palette[] = { 1,		0,		0,
                    0,		1,		0,
                    0,		0,		1,
                    1,		1,		0,
                    0,		1,		1,
                    1,		0,		1,
                    0.5f,	0.5f,	1,
                    0.5f,	1,		0.5f,
                    1,		0.5f,	0.5f };

bool needMenuUpdate = false;
bool menuClicked = false;


void DoDisplay();
void FrameTimer(int);
void DoReshape(GLsizei width, GLsizei height);
void DoKeyboard(unsigned char key, int x, int y);
void DoMenu(int value);
void DoMouse(int, int, int, int);
void DrawPlatform();
void DrawMortar();
void DrawFire();
void ResetValue();
void UpdateMenu();
void FireRising();
void FireDiffusion();
void FireExtinction();
void FireSparkler();

int main(int a, char** c) {
    glutInit(&a, c);
    srand((unsigned int)time(NULL));
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("HANABI");
    glutDisplayFunc(DoDisplay);
    glutReshapeFunc(DoReshape);
    glutKeyboardFunc(DoKeyboard);
    glutMouseFunc(DoMouse);
    glutIdleFunc(UpdateMenu);
    glutCreateMenu(DoMenu);
    glutAddMenuEntry("Fire Scale  80%", 1);
    glutAddMenuEntry("Fire Scale 100%", 2);
    glutAddMenuEntry("Fire Scale 120%", 3);
    glutAddMenuEntry("Sparkler On", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glEnable(GL_DEPTH_TEST);
    FrameTimer(0);
    glutMainLoop();
    return 0;
}

void DoDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH);
    glShadeModel(GL_FLAT);
    glBlendFunc(GL_ONE, GL_ZERO);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLfloat aspect = (GLfloat)lastWidth / (GLfloat)lastHeight;
    gluPerspective(fov, aspect, Near, Far);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(0, 0, -2);
    glRotatef(xAngle, 1.0f, 0.0f, 0.0f);
    glRotatef(yAngle, 0.0f, 1.0f, 0.0f);
    glRotatef(zAngle, 0.0f, 0.0f, 1.0f);

    DrawPlatform();
    DrawMortar();
    if (isFire)
        DrawFire();

    glPopMatrix();
    glutSwapBuffers();
}

//60프레임으로 화면 출력
void FrameTimer(int value) {
    glutPostRedisplay();
    glutTimerFunc(1000/60, FrameTimer, 0);
}

void DoKeyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':yAngle += 2; break;
    case 'd':yAngle -= 2; break;
    case 'w':xAngle += 2; break;
    case 's':xAngle -= 2; break;
    case 'r':xAngle = yAngle = zAngle = 0.0; break;
    }
    xAngle = std::clamp((int)xAngle, -10, 90);
    glutPostRedisplay();
}

void DoMouse(int button, int state, int x, int y) {
    if (!isFire && button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        if (!menuClicked) ResetValue();
        menuClicked = false;
    }
}

//변수 값 초기화
void ResetValue() {
    fireStartTime = clock();

    sparklerValue = 2;
    isFire = true;

    targetPosX = (rand() % 11) * 0.05f - 0.25f;
    targetPosZ = (rand() % 11) * 0.05f - 0.25f;
    targetPosY = (rand() % 11) * 0.02f + 0.2f;

    firePosX = fireStartPosX;
    firePosY = fireStartPosY;
    firePosZ = fireStartPosZ;

    bezierAX = fireStartPosX;
    bezierAY = fireStartPosY;
    bezierAZ = fireStartPosZ;
    bezierBX = bezierBZ = 0;
    bezierBY = targetPosY;

    u1 = u2 = 1;

    colorValue = rand() % 9;
    colorA = 1;

    colorR = palette[colorValue * 3];
    colorG = palette[colorValue * 3 + 1];
    colorB = palette[colorValue * 3 + 2];

    for (int i = 0; i < 480; i++) {
        fireRotX[i] = rand() % 181;
        fireRotY[i] = rand() % 360;
    }
}

void DoMenu(int value) {
    switch (value) {
    case 1:
        fireLength = 0.3;
        break;
    case 2:
        fireLength = 0.4;
        break;
    case 3:
        fireLength = 0.5;
        break;
    case 4:
        sparkler = !sparkler;
        needMenuUpdate = true;
        break;
    }
    menuClicked = true;
}

void UpdateMenu() {
    if (needMenuUpdate) {
        if (sparkler) glutChangeToMenuEntry(4, "Sparkler Off", 4);
        else glutChangeToMenuEntry(4, "Sparkler On", 4);

        needMenuUpdate = false;
    }
}

void DoReshape(GLsizei width, GLsizei height) {
    glViewport(0, 0, width, height);

    lastWidth = width;
    lastHeight = height;
}

//바닥 그리기
void DrawPlatform() {
    glColor3f(1, 1, 1);

    glBegin(GL_POLYGON);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glEnd();
}

//박격포 그리기
void DrawMortar() {
    GLUquadricObj* pQuad;
    pQuad = gluNewQuadric();
    gluQuadricDrawStyle(pQuad, GLU_LINE);

    glPushMatrix();
    glColor3f(0.5, 0.5, 0.5);
    glTranslatef(0, -0.448f, 0);
    glRotatef(90, 1, 0, 0);
    gluCylinder(pQuad, 0.01, 0.01, 0.05, 20, 20);
    glPopMatrix();
}

//불꽃 그리기
void DrawFire() {
    //발사 후 1초가 지나기 전일 떄
    if ((float)(clock() - fireStartTime) < 1000)
        FireRising();

    //발사 후 3초가 지나기 전일 때
    else if ((float)(clock() - fireStartTime) < 3000)
        FireDiffusion();

    //발사 후 4.5초가 지나기 전일 때
    else if ((float)(clock() - fireStartTime) < 4500)
        FireExtinction();

    //스파클라 발사가 필요할 경우
    else if (sparkler && sparklerValue < maxSparkler)
        FireSparkler();
    
    else isFire = false;
}

void FireRising() {
    bezierAY = (targetPosY - fireStartPosY) * (1 - u1) + fireStartPosY;
    bezierBX = (targetPosX - fireStartPosX) * (1 - u1) + fireStartPosX;
    bezierBZ = (targetPosZ - fireStartPosZ) * (1 - u1) + fireStartPosZ;
    firePosX = (bezierBX - bezierAX) * (1 - u1) + fireStartPosX;
    firePosY = (bezierBY - bezierAY) * (1 - u1) + bezierAY;
    firePosZ = (bezierBZ - bezierAZ) * (1 - u1) + fireStartPosZ;
    glPointSize(fireScale1 * (1 - u1));
    glColor3f(colorR, colorG, colorB);
    glBegin(GL_POINTS);
    glVertex3f(firePosX, firePosY, firePosZ);
    glEnd();
    u1 *= 0.97f;
}

void FireDiffusion() {
    for (int i = 0; i < 480; i++) {
        glPushMatrix();
        glTranslatef(firePosX, firePosY, firePosZ);
        glRotatef(fireRotY[i], 0, 1, 0);
        glRotatef(fireRotX[i], 1, 0, 0);
        glTranslatef(-firePosX, -firePosY, -firePosZ);
        glColor3f(colorR, colorG, colorB);
        glPointSize(fireScale2 * (1 - u2));
        glBegin(GL_POINTS);
        glVertex3f(firePosX, fireLength * (1 - u2) + firePosY, firePosZ);
        glEnd();
        glPopMatrix();
    }
    u2 *= pow(0.99f, 2);
}

void FireExtinction() {
    for (int i = 0; i < 480; i++) {
        glPushMatrix();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTranslatef(firePosX, firePosY, firePosZ);
        glRotatef(fireRotY[i], 0, 1, 0);
        glRotatef(fireRotX[i], 1, 0, 0);
        glTranslatef(-firePosX, -firePosY, -firePosZ);
        glColor4f(colorR, colorG, colorB, colorA);
        glBegin(GL_POINTS);
        glVertex3f(firePosX, fireLength * (1 - u2) + firePosY, firePosZ);
        glEnd();
        glPopMatrix();
    }
    u2 *= pow(0.99f, 2);
    firePosY -= 0.007f * u2;
    colorA *= 0.965;
}

void FireSparkler() {
    for (int i = 0; i < 480; i++) {
        if (i % sparklerValue != 0) continue;
        glPushMatrix();
        glTranslatef(firePosX, firePosY, firePosZ);
        glRotatef(fireRotY[i], 0, 1, 0);
        glRotatef(fireRotX[i], 1, 0, 0);
        glTranslatef(-firePosX, -firePosY, -firePosZ);
        glPointSize(1);
        glColor3f(colorR + 0.3f, colorG + 0.3f, colorB + 0.3f);
        glBegin(GL_POINTS);
        glVertex3f(firePosX, fireLength * (1 - u2) + firePosY, firePosZ);
        glEnd();
        glPopMatrix();
    }
    Sleep(20);
    sparklerValue++;
}
