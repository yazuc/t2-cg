//
//  Instancia.cpp
//
//  Created by Márcio Sarroglia Pinho on 22/09/20.
//  Copyright © 2024 Márcio Sarroglia Pinho. All rights reserved.
//

#include "Instancia.h"
#include "Tools.h"
GLfloat InvMatrix[4][4];

// ***********************************************************
//
// ***********************************************************
void SalvaMatrizDaCamera(GLfloat InvMatrix[4][4])
{
    //InvMatrix
}
// ***********************************************************
//  void InstanciaPonto(Ponto3D *p, Ponto3D *out)
//  Esta funcao calcula as coordenadas de um ponto no
//  sistema de referencia do universo (SRU), ou seja,
//  aplica as rotacoes, escalas e translacoes a um
//  ponto no sistema de referencia do objeto (SRO).
//
//  O parametro InvMatrix serve para programas 3D
//  em que se precisa remover a transformação da camera para
//  obter as coordenadas no SRU
//  Se InvMatrix for a matriz identidade, o ponto out
//  será retornado no sistema de referencia da Camera (SRC)
// ***********************************************************
void InstanciaPonto(Ponto &p, Ponto &out, GLfloat InvMatrix[4][4])
{
    GLfloat ponto_novo[4];
    GLfloat matriz_gl[4][4];
    GLfloat M[4][4];
    int  i;
    
    // Obtem da matriz ModelView que contem a transformacao da atual
    glGetFloatv(GL_MODELVIEW_MATRIX,&matriz_gl[0][0]);
    
    // Retira da matriz ModelView a transformacao da Camera
    MultiplicaMatriz(matriz_gl, InvMatrix, M);

    for(i=0;i<4;i++)
    {
        ponto_novo[i]= M[0][i] * p.x+
        M[1][i] * p.y+
        M[2][i] * p.z+
        M[3][i];
    }
    out.x=ponto_novo[0];
    out.y=ponto_novo[1];
    out.z=ponto_novo[2];
}

Ponto InstanciaPonto(Ponto P, GLfloat InvMatrix[4][4])
{
    Ponto temp;
    InstanciaPonto(P, temp, InvMatrix);
    return temp;
}
Ponto InstanciaPonto(Ponto P)
{
    Ponto temp;
    float Identity[4][4];
    criaIdentidade(Identity);
    InstanciaPonto(P, temp, Identity);
    return temp;
}
/*
Ponto InstanciaPonto(Ponto P, Ponto OBS, Ponto ALVO)
{
    glGetFloatv(GL_MODELVIEW_MATRIX,&CameraMatrix[0][0]);
    InverteMatriz(CameraMatrix, InvCameraMatrix);
}
*/

Instancia::Instancia()
{
    //cout << "Instanciado..." << endl;
    Rotacao = 0;
    Posicao = Ponto(0,0,0);
    Escala = Ponto(1,1,1);
    Pivot = Ponto(0,0,0);
}

void Instancia::desenha()
{
    // Aplica as transformacoes geometricas no modelo
    glPushMatrix();
        glTranslatef(Posicao.x, Posicao.y, 0);
        glTranslatef(Pivot.x, Pivot.y, Pivot.z);
        glRotatef(Rotacao, 0,0,1);
        glScalef(Escala.x, Escala.y, Escala.z);
        glTranslatef(-Pivot.x, -Pivot.y, -Pivot.z);

        // Obtem a posicao do ponto 0,0,0 no SRU
        // Nao eh usado aqui, mas eh util para detectar colisoes
        // PosicaoDoPersonagem = InstanciaPonto(Ponto (0,0,0)+Pivot);
        PosicaoDoPersonagem = InstanciaPonto(Ponto(0,0,0)+Pivot, InvMatrix);
        
        (*modelo)(); // desenha a instancia
        
    glPopMatrix();
}

void Instancia::AtualizaPosicao(double tempoDecorrido)
{
    Posicao = Posicao + Direcao*tempoDecorrido*Velocidade;
}
void Instancia::ImprimeEnvelope(char const *msg1, char const *msg2)
{
    cout << msg1;
    for(int i=0;i<4;i++)
        Envelope[i].imprime();
    cout << msg2;
}
