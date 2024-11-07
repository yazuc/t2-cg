

#ifndef __TOOLS__
#define __TOOLS__

#ifdef WIN32
#include <windows.h>
#include <glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <GL/glut.h>
#endif

void ImprimeMatriz(GLfloat M[4][4]);
void criaIdentidade(float m1[4][4]);
void InverteMatriz( float a[4][4], float b[4][4]);
void MultiplicaMatriz(float m1[4][4], float m2[4][4], float m[4][4]);
float calculaDistancia(float P1[3], float P2[3]);


#endif
