//
//  Instancia.hpp
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 22/09/20.
//  Copyright © 2020 Márcio Sarroglia Pinho. All rights reserved.
//

#ifndef Instancia_hpp
#define Instancia_hpp
#include <iostream>
using namespace std;

#include "Poligono.h"
typedef void TipoFuncao();

class Instancia{
public:
    //Poligono *modelo;
    // Construtora
    Instancia();
    
    // Flag para definir se a instancia eh visivel ou nao
    // Pode ser usada para retirar a instancia do processo de
    // redesenho
    bool Visivel;
    
    // Referência para a função a ser usado para desenhar a instância
    TipoFuncao *modelo;
    
    int IdDoModelo;
    int QtdDeTiros;
    
    // Parâmetros de instanciamento aplicados ANTES de desenhar a instância
    Ponto Posicao, Escala;
    float Rotacao;  // Angulo de rotacao da instancia
    Ponto Pivot;    // Ponto ao redor do qual a instancia sobre rotacao e escala
    Ponto Direcao;  // Vetor de direcao do movimento - deve ser compativel com o atributo "Rotacao"
    float Velocidade; // Velocidade de deslocamento
    Ponto PosicaoDoPersonagem; // Posicao atualizada
    Ponto Envelope[4]; // Quatro vertices do envelope

    double r,g,b;
    
    // Método que desenha a instancia
    void desenha();
    
    // Método que calcula a nova posicao da instancia com base no
    // valor atual dos atributos
    void AtualizaPosicao(double tempoDecorrido);
    
    // Metodo que imprime os vertices do envelope OOBB da instacia
    void ImprimeEnvelope(char const *msg1, char const *msg2);

};

//Ponto InstanciaPonto(Ponto P);
//void InstanciaPonto(Ponto &p, Ponto &out);
Ponto InstanciaPonto(Ponto P, GLfloat InvMatrix[4][4]);
void InstanciaPonto(Ponto &p, Ponto &out, GLfloat InvMatrix[4][4]);
Ponto InstanciaPonto(Ponto P, Ponto OBS, Ponto ALVO);
void SalvaMatrizDaCamera(GLfloat M[4][4]);

#endif /* Instancia_hpp */
