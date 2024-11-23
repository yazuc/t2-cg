// **********************************************************************
// PUCRS/Escola Polit�cnica
// COMPUTA��O GR�FICA
//
// Programa b�sico para criar aplicacoes 3D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************

#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;


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

#include "Temporizador.h"
#include "ListaDeCoresRGB.h"
#include "Ponto.h"
#include "Instancia.h"
#include "Tools.h"
#include "ImageClass.h"

enum ModoExibicao {
    Jogador,
    Superior,
    SuperiorCompleto,
    FreeCam,
    PlayerCam
    };

Temporizador T;
double AccumDeltaT=0;


GLfloat AspectRatio, angulo=0, anguloPrincipal = 90.0f;

// Controle do modo de projecao
// 0: Projecao Paralela Ortografica; 1: Projecao Perspectiva
// A funcao "PosicUser" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'p'
int ModoDeProjecao = 1;


// Controle do modo de projecao
// 0: Wireframe; 1: Faces preenchidas
// A funcao "Init" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'e'
int ModoDeExibicao = 1;
int xObs, yObs, zObs = 0;
void DesenhaCuboComTextura (float tamAresta);
void atualizaCamera();
double nFrames=0;
double TempoTotal=0;
bool freecam = false;
Ponto CantoEsquerdo = Ponto(-20,0,-10);
Ponto OBS;
Ponto ALVO;
Ponto VetorAlvo;
GLfloat CameraMatrix[4][4];
GLfloat InvCameraMatrix[4][4];
GLuint TEX1, TEX2, TEX, TEX3;
GLfloat anguloCanhao = 0.0f;
GLfloat anguloCanhaod = 0.0f;
Ponto PosicaoDoObjeto(0,0,4);
Ponto DirecaoDoObjeto(1,0,0);
Ponto DirecaoDoCanhao(1,0,0);
ModoExibicao tipoVista {PlayerCam};
float projX = 0.0f, projY = 0.0f, projZ = -10.0f; // Posição inicial do projétil
float projXd = 0.0f, projYd = 0.0f, projZd = -10.0f; // Posição inicial do projétil

float velocidadeProj = 0.5f; // Velocidade do projétil
const float velocidadeObj = 0.5f; // Velocidade do objeto
float pontaX, pontaY, pontaZ, dirProjX, dirProjY, dirProjZ = 0.0;
float pontaXd, pontaYd, pontaZd, dirProjXd, dirProjYd, dirProjZd = 0.0;
bool disparado = false; // Flag para indicar se o projétil foi disparado
bool disparado1 = false; // Flag para indicar se o projétil foi disparado
const int largura = 40;  // Número de blocos na largura do paredão
const int altura = 15;   // Número de blocos na altura do paredão
bool paredao[altura][largura]; // Matriz de blocos do paredão (true = ativo)
const float LIMITE_MIN_X = -45; // Limite mínimo do mapa no eixo X
const float LIMITE_MAX_X = 45; // Limite máximo do mapa no eixo X
const float LIMITE_MIN_Z = -15; // Limite mínimo do mapa no eixo Z
const float LIMITE_MAX_Z = 35; // Limite máximo do mapa no eixo Z


typedef struct  // Struct para armazenar um ponto
{
    float X,Y,Z;
    void Set(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }
    void Imprime()
    {
        cout << "X: " << X << " Y: " << Y << " Z: " << Z;
    }
} TPoint;


typedef struct // Struct para armazenar um triângulo
{
    TPoint P1, P2, P3; // Pontos do triângulo
    float R, G, B;     // Cores RGB da face

    void imprime()
    {
        cout << "P1 "; P1.Imprime(); cout << endl;
        cout << "P2 "; P2.Imprime(); cout << endl;
        cout << "P3 "; P3.Imprime(); cout << endl;
        cout << "Cor: (" << R << ", " << G << ", " << B << ")" << endl;
    }
} TTriangle;


// Classe para armazenar um objeto 3D
class Objeto3D
{
    TTriangle *faces; // vetor de faces
    unsigned int nFaces; // Variavel que armazena o numero de faces do objeto
public:
    Objeto3D()
    {
        nFaces = 0;
        faces = NULL;
    }
    unsigned int getNFaces()
    {
        return nFaces;
    }
    void LeObjeto (char *Nome); // implementado fora da classe
    void ExibeObjeto(); // implementado fora da classe
};


Objeto3D *MundoVirtual;

void Objeto3D::LeObjeto (char *Nome)
{
    // ***************
    // Exercicio
    //      complete esta rotina fazendo a leitura do objeto
    // ***************
    
    ifstream arq;
    arq.open(Nome, ios::in);
    if (!arq)
    {
        cout << "Erro na abertura do arquivo " << Nome << "." << endl;
        exit(1);
    }
    arq >> nFaces;
    faces = new TTriangle[nFaces];
    float x,y,z;
    for (int i=0;i<nFaces;i++)
    {
        // Le os trs vŽrtices
        arq >> x >> y >> z; // Vertice 1
        faces[i].P1.Set(x,y,z);
        arq >> x >> y >> z; // Vertice 2
        faces[i].P2.Set(x,y,z);
        arq >> x >> y >> z; // Vertice 3
        faces[i].P3.Set(x,y,z);
        cout << i << ": ";
        faces[i].imprime();
        // Falta ler o RGB da face....
    }
}

void Objeto3D::ExibeObjeto()
{
    if (!faces || nFaces == 0)
    {
        cout << "Objeto vazio ou não carregado!" << endl;
        return;
    }

    glBegin(GL_TRIANGLES); // Início do desenho de triângulos
    for (unsigned int i = 0; i < nFaces; i++)
    {
        // Desenhar o triângulo i
        glVertex3f(faces[i].P1.X, faces[i].P1.Y, faces[i].P1.Z); // Vértice 1
        glVertex3f(faces[i].P2.X, faces[i].P2.Y, faces[i].P2.Z); // Vértice 2
        glVertex3f(faces[i].P3.X, faces[i].P3.Y, faces[i].P3.Z); // Vértice 3
    }
    glEnd(); // Fim do desenho
}

void inicializarParedao()
{
    for (int i = 0; i < altura; i++)
    {
        for (int j = 0; j < largura; j++)
        {
            paredao[i][j] = true;
        }
    }
}
GLuint LoadTexture (const char *nomeTex)
{
    GLenum errorCode;
    GLuint IdTEX;
    // Habilita o uso de textura
    glEnable ( GL_TEXTURE_2D );

    // Define a forma de armazenamento dos pixels na textura (1= alihamento por byte)
    glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

    // Gera um identificar para a textura
    glGenTextures (1, &IdTEX); //  vetor que guardas os n�meros das texturas
    errorCode =  glGetError();
    if (errorCode == GL_INVALID_OPERATION)
    {
        cout << "Erro: glGenTextures chamada entre glBegin/glEnd." << endl;
        return -1;
    }

    // Define que tipo de textura ser� usada
    // GL_TEXTURE_2D ==> define que ser� usada uma textura 2D (bitmaps)
    // texture_id[OBJETO_ESQUERDA]  ==> define o n�mero da textura
    glBindTexture ( GL_TEXTURE_2D, IdTEX );

    // Carrega a imagem
    ImageClass Img;

    int r = Img.Load(nomeTex);
    if (!r)
    {
        cout << "Erro lendo imagem " << nomeTex << endl;
        exit(1);
    }

    int level = 0;
    int border = 0;

    // Envia a textura para OpenGL, usando o formato apropriado
    int formato;
    formato = GL_RGB;
    if (Img.Channels() == 4)
        formato = GL_RGBA;
    //if (Img.Channels() == 3)
    //   formato = GL_RGB;

    glTexImage2D (GL_TEXTURE_2D, level, formato,
                  Img.SizeX(), Img.SizeY(),
                  border, formato,
                  GL_UNSIGNED_BYTE, Img.GetImagePtr());
    errorCode = glGetError();
    if (errorCode == GL_INVALID_OPERATION)
    {
        cout << "Erro: glTexImage2D chamada entre glBegin/glEnd." << endl;
        return -1;
    }
    if (errorCode != GL_NO_ERROR)
    {
        cout << "Houve algum erro na criacao da textura." << endl;
        return -1;
    }
    // Ajusta os filtros iniciais para a textura
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Img.Delete();
    
    cout << "Carga de textura OK." << endl;
    return IdTEX;
}
void initTexture (void)
{
    TEX3 = LoadTexture ("./MonadoTilt.jpg");
    TEX2 = LoadTexture ("./grass_texture.jpg");
    TEX1 = LoadTexture ("./igordinho.png");
    TEX = LoadTexture ("./shulkxenoblade.png");
}

// **********************************************************************
//  void init(void)
//        Inicializa os parametros globais de OpenGL
// **********************************************************************
void init(void)
{
    glClearColor(0.4f, 1.0f, 0.0f, 1.0f); // Fundo de tela preto

    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable (GL_CULL_FACE );
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    //glShadeModel(GL_FLAT);

    glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    if (ModoDeExibicao) // Faces Preenchidas??
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    ALVO = Ponto(0, 0, 0);
    OBS = Ponto(0,3,10);
    VetorAlvo = ALVO - OBS;        
    initTexture();
    inicializarParedao();
}

// **********************************************************************
//
// **********************************************************************
void animate()
{
    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    if (AccumDeltaT > 1.0/30) // fixa a atualiza��o da tela em 30
    {
        AccumDeltaT = 0;
        angulo+= 1;
        glutPostRedisplay();
    }
    if (TempoTotal > 5.0)
    {
        cout << "Tempo Acumulado: "  << TempoTotal << " segundos. " ;
        cout << "Nros de Frames sem desenho: " << nFrames << endl;
        cout << "FPS(sem desenho): " << nFrames/TempoTotal << endl;
        TempoTotal = 0;
        nFrames = 0;
    }
}

// **********************************************************************
//  void DesenhaCubo()
// **********************************************************************
bool verificarColisao()
{
    // Posições do paredão: O paredão vai de x = 6 até x = -7, de y = 0 e de z = -11 até z = 11
    const float paredaoXMin = 6.0f;
    const float paredaoXMax = 7.0f;
    const float paredaoZMin = 4.0f; 
    const float paredaoZMax = 16.0f; 

    // Verifica se a posição do projétil está dentro dos limites do paredão
    if (projX >= paredaoXMin && projX <= paredaoXMax && projZ >= paredaoZMin && projZ <= paredaoZMax)
    {
        return true; // Colisão detectada
    }
    
    return false; // Sem colisão
}

bool verificarColisao2()
{
    // Posições do paredão: O paredão vai de x = 6 até x = -7, de y = 0 e de z = -11 até z = 11
    const float paredaoXMin = 6.0f;
    const float paredaoXMax = 7.0f;
    const float paredaoZMin = 4.0f; 
    const float paredaoZMax = 16.0f; 

    // Verifica se a posição do projétil está dentro dos limites do paredão
    if (projX >= paredaoXMin && projX <= paredaoXMax && projZ >= paredaoZMin && projZ <= paredaoZMax)
    {
        return true; // Colisão detectada
    }
    
    return false; // Sem colisão
}

void quebrarBloco(float projX, float projY, float projZ)
{
    // Primeiro, converta a posição do projétil para o sistema de coordenadas do paredão
    // Aqui você já tem as transformações necessárias (inversão de translação e rotação)

    // Definindo as transformações aplicadas ao paredão
    const float transX = -18.0f; // Translação em X
    const float transY = -1.0f;  // Translação em Y
    const float transZ = 7.0f;   // Translação em Z
    const float rotAngulo = 90.0f; // Ângulo de rotação

    // Inversão da translação
    projX -= transX;
    projY -= transY + 7;
    projZ -= transZ + 3;

    // Agora aplicamos a inversa da rotação. A rotação foi feita no eixo Y, então aplicamos a rotação inversa no eixo Y.
    float radAngulo = rotAngulo * M_PI / 180.0f; // Converte o ângulo para radianos
    float cosAngulo = cos(-radAngulo);  // Cálculo para rotação inversa
    float sinAngulo = sin(-radAngulo);

    // Aplica a inversão da rotação para o eixo Y
    float rotatedX = cosAngulo * projX + sinAngulo * projZ;
    float rotatedZ = sinAngulo * projX - cosAngulo * projZ;

    // Agora, o projX e projZ estão no sistema de coordenadas local do paredão
    projX = rotatedX;
    projZ = rotatedZ;

    // Ajustando o valor de projY para garantir que o cálculo de blocoY esteja correto
    // Se o paredão está centrado na origem, o cálculo de Y precisa ser ajustado
    int blocoY = int(projY + altura / 2.0f);   // Converte a posição para índice da matriz

    // Calcula as coordenadas do bloco atingido
    int blocoX = int((projX + largura / 2.0f));  // Converte a posição para índice da matriz

    // Verifica se a posição calculada está dentro dos limites do paredão
    if (blocoX >= 0 && blocoX < largura && blocoY >= 0 && blocoY < altura)
    {
        paredao[blocoY][blocoX] = false; // Marca o bloco como destruído
    }
}

void DesenhaProjetil()
{
    if (disparado) 
    {
        glPushMatrix();
            glTranslatef(projX, projY, projZ); // Posiciona o projétil
            glutSolidSphere(0.5, 10, 5);// Desenha o projétil como uma esfera
        glPopMatrix();
                        
        // Atualiza a posição do projétil
        projX -= velocidadeProj; // Move o projétil para frente (em direção ao paredão)

        printf("X: %f Y: %f Z: %f",projX, projY, projZ);
        // Verifica se o projétil atingiu o paredão (aproximadamente)
        if (verificarColisao()) // Considera a posição do paredão
        {
            const int alcance = 1; // Alcance da destruição ao redor do impacto

            // Itera sobre os blocos ao redor (em 3 dimensões)
            for (int dx = -alcance; dx <= alcance; ++dx)
            {
                for (int dy = -alcance; dy <= alcance; ++dy)
                {
                    for (int dz = -alcance; dz <= alcance; ++dz)
                    {
                        int blocoX = projX + dx;
                        int blocoY = projY + dy;
                        int blocoZ = projZ + dz;

                        // Quebra o bloco na posição calculada
                        quebrarBloco(blocoX, blocoY, blocoZ);
                    }
                }
            }
            disparado = false; // O projétil parou
            // Adicionar lógica para efeito de colisão, como mudança de cor ou efeito sonoro
            std::cout << "Colisão com o paredão!" << std::endl;
        }
    }

    if(disparado1){      
        glPushMatrix();
            glTranslatef(projXd, projYd, projZd); // Posiciona o projétil
            glutSolidSphere(0.5, 10, 5);// Desenha o projétil como uma esfera
        glPopMatrix();

        projXd -= velocidadeProj; // Move o projétil para frente (em direção ao paredão)

        if (verificarColisao2()) // Considera a posição do paredão
        {
            const int alcance = 1; // Alcance da destruição ao redor do impacto

            // Itera sobre os blocos ao redor (em 3 dimensões)
            for (int dx = -alcance; dx <= alcance; ++dx)
            {
                for (int dy = -alcance; dy <= alcance; ++dy)
                {
                    for (int dz = -alcance; dz <= alcance; ++dz)
                    {
                        int blocoX = projXd + dx;
                        int blocoY = projYd + dy;
                        int blocoZ = projZd + dz;

                        // Quebra o bloco na posição calculada
                        quebrarBloco(blocoX, blocoY, blocoZ);
                    }
                }
            }

            disparado1 = false; // O projétil parou
            std::cout << "Colisão com o paredão!" << std::endl;
        }
    }
    
}
void DesenhaCubo(float tamAresta)
{
    glBegin ( GL_QUADS );
    // Front Face
    glNormal3f(0,0,1);
    glVertex3f(-tamAresta/2, -tamAresta/2,  tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2,  tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2,  tamAresta/2);
    glVertex3f(-tamAresta/2,  tamAresta/2,  tamAresta/2);
    // Back Face
    glNormal3f(0,0,-1);
    glVertex3f(-tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f(-tamAresta/2,  tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2, -tamAresta/2);
    // Top Face
    glNormal3f(0,1,0);
    
    glVertex3f(-tamAresta/2,  tamAresta/2, -tamAresta/2);
    
    glVertex3f(-tamAresta/2,  tamAresta/2,  tamAresta/2);
    
    glVertex3f( tamAresta/2,  tamAresta/2,  tamAresta/2);
    
    glVertex3f( tamAresta/2,  tamAresta/2, -tamAresta/2);
    // Bottom Face
    glNormal3f(0,-1,0);
    glVertex3f(-tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2,  tamAresta/2);
    glVertex3f(-tamAresta/2, -tamAresta/2,  tamAresta/2);
    // Right face
    glNormal3f(1,0,0);
    glVertex3f( tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2,  tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2,  tamAresta/2);
    // Left Face
    glNormal3f(-1,0,0);
    glVertex3f(-tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f(-tamAresta/2, -tamAresta/2,  tamAresta/2);
    glVertex3f(-tamAresta/2,  tamAresta/2,  tamAresta/2);
    glVertex3f(-tamAresta/2,  tamAresta/2, -tamAresta/2);
    glEnd();

}

// **********************************************************************
// void DesenhaLadrilho(int corBorda, int corDentro)
// Desenha uma c�lula do piso.
// Eh possivel definir a cor da borda e do interior do piso
// O ladrilho tem largula 1, centro no (0,0,0) e est� sobre o plano XZ
// **********************************************************************
void DesenhaLadrilho(int corBorda, int corDentro)
{
    // Aplica a textura para o quadrado
    glBindTexture(GL_TEXTURE_2D, TEX2);  // Aplica a textura TEX2

    // Desenha o quadrado com a textura
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);

        // Coordenadas de textura para o ladrilho
        glTexCoord2f(0.0f, 0.0f); // Coordenada (0,0) para o vértice (-0.5f, -0.5f)
        glVertex3f(-0.5f,  0.0f, -0.5f);

        glTexCoord2f(0.0f, 1.0f); // Coordenada (0,1) para o vértice (-0.5f, 0.5f)
        glVertex3f(-0.5f,  0.0f,  0.5f);

        glTexCoord2f(1.0f, 1.0f); // Coordenada (1,1) para o vértice (0.5f, 0.5f)
        glVertex3f( 0.5f,  0.0f,  0.5f);

        glTexCoord2f(1.0f, 0.0f); // Coordenada (1,0) para o vértice (0.5f, -0.5f)
        glVertex3f( 0.5f,  0.0f, -0.5f);
    glEnd();

    // Desenha as bordas do ladrilho
    //defineCor(corBorda); // Define a cor das bordas (presumindo que esta função altera a cor)

    glBegin(GL_LINE_STRIP);
        glNormal3f(0, 1, 0);
        glVertex3f(-0.5f,  0.0f, -0.5f);
        glVertex3f(-0.5f,  0.0f,  0.5f);
        glVertex3f( 0.5f,  0.0f,  0.5f);
        glVertex3f( 0.5f,  0.0f, -0.5f);
        glVertex3f(-0.5f,  0.0f, -0.5f);  // Fecha a linha (volta ao ponto inicial)
    glEnd();
}


// **********************************************************************
//
//
// **********************************************************************
void DesenhaPiso()
{
    srand(100); // usa uma semente fixa para gerar sempre as mesma cores no piso
    glPushMatrix();
    glTranslated(CantoEsquerdo.x, CantoEsquerdo.y, CantoEsquerdo.z);
    for(int x=-20; x<20;x++)
    {
        glPushMatrix();
        for(int z=-20; z<20;z++)
        {
            DesenhaLadrilho(MediumGoldenrod, rand()%40);
            glTranslated(0, 0, 1);
        }
        glPopMatrix();
        glTranslated(1, 0, 0);
    }
    glPopMatrix();
}

void DesenhaLimitesMapa() {
    glBegin(GL_LINE_LOOP);
        glVertex3f(LIMITE_MIN_X, 0.0f, LIMITE_MIN_Z);
        glVertex3f(LIMITE_MAX_X, 0.0f, LIMITE_MIN_Z);
        glVertex3f(LIMITE_MAX_X, 0.0f, LIMITE_MAX_Z);
        glVertex3f(LIMITE_MIN_X, 0.0f, LIMITE_MAX_Z);
    glEnd();
}

void DesenhaQuadrado()
{
    glBindTexture(GL_TEXTURE_2D, TEX1);  // Aplica a textura
    glBegin(GL_QUADS);
    
    glColor3f(1.0, 1.0, 1.0); // Usar cor branca para manter a textura

    // Vértices do quadrado com coordenadas de textura
    glTexCoord2f(0.0, 0.0);  // Coordenada de textura para o vértice (0,0)
    glVertex3f(0, 0, 0);
    
    glTexCoord2f(1.0, 0.0);  // Coordenada de textura para o vértice (1,0)
    glVertex3f(1, 0, 0);
    
    glTexCoord2f(1.0, 1.0);  // Coordenada de textura para o vértice (1,1)
    glVertex3f(1, 1, 0);
    
    glTexCoord2f(0.0, 1.0);  // Coordenada de textura para o vértice (0,1)
    glVertex3f(0, 1, 0);
    
    glEnd();
    
    glDisable(GL_TEXTURE_2D);  // Desativa o mapeamento de textura
}

void DesenhaParedao()
<<<<<<< HEAD
{
    // Altura do paredão
    const int altura = 15; // 15 metros
    
    // Largura do paredão (ajustável conforme a cena)
    const int largura = 15; // 15 metros

    // Empilha a transformação atual da matriz
=======
{    
>>>>>>> refs/remotes/origin/main
    glPushMatrix();

    // Gira o paredão para que fique perpendicular ao chão, se necessário
    glRotatef(90, 0, 1, 0); // Ajustando rotação no eixo Y para uma melhor visualização

<<<<<<< HEAD
    // Posiciona o paredão no local desejado dentro do cenário
    glTranslatef(-18.0f, -1.0f, 7.0f); // Translação para a posição do paredão
=======
    // Posiciona o paredão no meio do cenário
    glTranslatef(-29.5, -1, 7);
>>>>>>> refs/remotes/origin/main

    // Desenha os quadrados de 1m x 1m
    for (int i = 0; i < altura; i++) // Para cada linha (vertical)
    {
        for (int j = 0; j < largura; j++) // Para cada coluna (horizontal)
        {
            if (paredao[i][j]) // Desenha apenas os blocos ativos
            {
                glPushMatrix(); // Empilha a matriz para cada bloco individual

                // Move para a posição do bloco
                glTranslatef(j, i, 0); // Ajusta a posição com base na coordenada (j, i)

                // Aplica a textura ao bloco
                glBindTexture(GL_TEXTURE_2D, TEX1);

                // Inicia o desenho do quadrado (bloco)
                glBegin(GL_QUADS);
                glColor3f(1.0f, 1.0f, 1.0f); // Cor branca para a textura

                // Definição dos vértices do quadrado e as coordenadas de textura
                glTexCoord2f(j / float(largura), i / float(altura)); // (0, 0) coordenada de textura
                glVertex3f(0, 0, 0);

                glTexCoord2f((j + 1) / float(largura), i / float(altura)); // (1, 0)
                glVertex3f(1, 0, 0);

                glTexCoord2f((j + 1) / float(largura), (i + 1) / float(altura)); // (1, 1)
                glVertex3f(1, 1, 0);

                glTexCoord2f(j / float(largura), (i + 1) / float(altura)); // (0, 1)
                glVertex3f(0, 1, 0);

                glEnd(); // Finaliza o desenho do quadrado
                glPopMatrix(); // Restaura a matriz
            }
        }
    }

    // Restaura as transformações
    glPopMatrix();
}



void DesenhaChao()
{
    glPushMatrix();
        glTranslated(-20, 0, 0);
        DesenhaPiso();
    glPopMatrix();
    glPushMatrix();
        glTranslated(20, 0, 0);
        DesenhaPiso();
    glPopMatrix();
}
// **********************************************************************
//  void DefineLuz(void)
// **********************************************************************
void DefineLuz(void)
{
  // Define cores para um objeto dourado
  //GLfloat LuzAmbiente[]   = {0.0, 0.0, 0.0 } ;
  GLfloat LuzAmbiente[]   = {0.4, 0.4, 0.4} ;
  GLfloat LuzDifusa[]   = {0.7, 0.7, 0.7};
  //GLfloat LuzDifusa[]   = {0, 0, 0};
  GLfloat PosicaoLuz0[]  = {0.0f, 3.0f, 5.0f };  // Posi��o da Luz
  GLfloat LuzEspecular[] = {0.9f, 0.9f, 0.9 };
  //GLfloat LuzEspecular[] = {0.0f, 0.0f, 0.0 };
 
  GLfloat Especularidade[] = {1.0f, 1.0f, 1.0f};

   // ****************  Fonte de Luz 0

 glEnable ( GL_COLOR_MATERIAL );

   // Habilita o uso de ilumina��o
  glEnable(GL_LIGHTING);

  // Ativa o uso da luz ambiente
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LuzAmbiente);
  // Define os parametros da luz n�mero Zero
  glLightfv(GL_LIGHT0, GL_AMBIENT, LuzAmbiente);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, LuzDifusa  );
  glLightfv(GL_LIGHT0, GL_SPECULAR, LuzEspecular  );
  glLightfv(GL_LIGHT0, GL_POSITION, PosicaoLuz0 );
  glEnable(GL_LIGHT0);

  // Ativa o "Color Tracking"
  glEnable(GL_COLOR_MATERIAL);

  // Define a reflectancia do material
  glMaterialfv(GL_FRONT,GL_SPECULAR, Especularidade);

  // Define a concentra��oo do brilho.
  // Quanto maior o valor do Segundo parametro, mais
  // concentrado ser� o brilho. (Valores v�lidos: de 0 a 128)
  glMateriali(GL_FRONT,GL_SHININESS,128);

}
// **********************************************************************

void MygluPerspective(float fieldOfView, float aspect, float zNear, float zFar )
{
    //https://stackoverflow.com/questions/2417697/gluperspective-was-removed-in-opengl-3-1-any-replacements/2417756#2417756
    // The following code is a fancy bit of math that is equivilant to calling:
    // gluPerspective( fieldOfView/2.0f, width/height , 0.1f, 255.0f )
    // We do it this way simply to avoid requiring glu.h
    //GLfloat zNear = 0.1f;
    //GLfloat zFar = 255.0f;
    //GLfloat aspect = float(width)/float(height);
    GLfloat fH = tan( float(fieldOfView / 360.0f * 3.14159f) ) * zNear;
    GLfloat fW = fH * aspect;
    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}
// **********************************************************************
//  void PosicUser()
// **********************************************************************
void PosicUser()
{

    // Define os par�metros da proje��o Perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define o volume de visualiza��o sempre a partir da posicao do
    // observador
    if (ModoDeProjecao == 0)
        glOrtho(-12, 12, -12, 12, 1, 18); // Projecao paralela Orthografica
    else MygluPerspective(90, AspectRatio, 0.1, 50); // Projecao perspectiva

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if(tipoVista == Jogador){
        OBS = Ponto(0,0,10);
        ALVO = Ponto(0, 0, 0);
        gluLookAt(OBS.x, OBS.y, OBS.z,   // Posi��o do Observador
            ALVO.x, ALVO.y, ALVO.z,     // Posi��o do Alvo
            0.0,1.0,0.0);
    }
    if(tipoVista == FreeCam){
        OBS = Ponto(xObs,yObs,zObs);
        gluLookAt(OBS.x, OBS.y, OBS.z,   // Posi��o do Observador
            ALVO.x, ALVO.y, ALVO.z,     // Posi��o do Alvo
            0.0,1.0,0.0);
    }
    if(tipoVista == Superior){
        OBS = Ponto(0,30,0);
        gluLookAt(OBS.x, OBS.y, OBS.z,   // Posi��o do Observador
            ALVO.x, ALVO.y, ALVO.z,     // Posi��o do Alvo
            0.0,0.0,-1.0);
    }
    if(tipoVista == PlayerCam){
         // Converter o ângulo principal para radianos
        float rad = anguloPrincipal * M_PI / 180.0f;

        // Calcular a posição da câmera ao redor do jogador
        float distancia = 10.0f; // Distância horizontal da câmera ao jogador
        float altura = 5.0f;     // Altura da câmera em relação ao jogador

        OBS = Ponto(
            PosicaoDoObjeto.x - distancia * sin(rad),  // Coordenada X
            PosicaoDoObjeto.y + altura,               // Coordenada Y (altura)
            PosicaoDoObjeto.z - distancia * cos(rad)  // Coordenada Z
        );

        // Focar no jogador
        ALVO = Ponto(PosicaoDoObjeto.x, PosicaoDoObjeto.y, PosicaoDoObjeto.z);

        // Configurar a visão com gluLookAt
        gluLookAt(OBS.x, OBS.y, OBS.z,   // Posição do Observador
                ALVO.x, ALVO.y, ALVO.z, // Posição do Alvo
                0.0, 1.0, 0.0);    
    }
    ALVO = Ponto(0, 0, 0);

    
    
    glGetFloatv(GL_MODELVIEW_MATRIX,&CameraMatrix[0][0]);
    InverteMatriz(CameraMatrix, InvCameraMatrix);
    SalvaMatrizDaCamera(InvCameraMatrix);
    
    //ImprimeMatriz(CameraMatrix);
    //cout << "Inversa:\n";
    //ImprimeMatriz(InvCameraMatrix);

}
// **********************************************************************
//  void reshape( int w, int h )
//		trata o redimensionamento da janela OpenGL
//
// **********************************************************************
void reshape( int w, int h )
{

	// Evita divis�o por zero, no caso de uam janela com largura 0.
	if(h == 0) h = 1;
    // Ajusta a rela��o entre largura e altura para evitar distor��o na imagem.
    // Veja fun��o "PosicUser".
	AspectRatio = 1.0f * w / h;
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Seta a viewport para ocupar toda a janela
    glViewport(0, 0, w, h);
    //cout << "Largura" << w << endl;

	PosicUser();

}

void DesenhaCanhao() {
    glPushMatrix();

    // Ativa texturas no escopo local
    glBindTexture(GL_TEXTURE_2D, TEX3); // Vincula a textura TEX3

    // Desenha o primeiro canhão no lado positivo do eixo X
    glPushMatrix();
    glTranslatef(1.0f, 1.0f, -1.2f); // Move o canhão para o lado positivo do eixo X
    glRotatef(anguloCanhao, 0, 0, 1); // Rotaciona o canhão
    glScalef(2, 0.5, 0.5); // Escala o canhão
    glutSolidCube(1.0); // Desenha o canhão
    glPopMatrix(); // Restaura a matriz

    // Desenha o segundo canhão no lado negativo do eixo X
    glPushMatrix();
    glTranslatef(1.0f, 1.0f, 1.2f); // Move o canhão para o lado negativo do eixo X
    glRotatef(anguloCanhaod, 0, 0, 1); // Rotaciona o canhão
    glScalef(2, 0.5, 0.5); // Escala o canhão
    glutSolidCube(1.0); // Desenha o canhão
    glPopMatrix(); // Restaura a matriz

    glPopMatrix(); // Finaliza as transformações do canhão
}
// Função para mover o projétil
void AtualizarPosicaoProjetil() {
    if (disparado) {
        // Atualiza a posição com base na direção e velocidade
        projX += dirProjX * velocidadeProj; // Atualiza a posição no eixo X
        projY += dirProjY * velocidadeProj; // Atualiza a posição no eixo Y
        projZ += dirProjZ * velocidadeProj; // Atualiza a posição no eixo Z

        // Exibe a nova posição do projétil
        printf("Nova posição do projétil: x:%f y:%f z:%f\n", projX, projY, projZ);
    }

    if (disparado) {
        // Atualiza a posição com base na direção e velocidade
        projX += dirProjX * velocidadeProj; // Atualiza a posição no eixo X
        projY += dirProjY * velocidadeProj; // Atualiza a posição no eixo Y
        projZ += dirProjZ * velocidadeProj; // Atualiza a posição no eixo Z

        // Verificar se o projétil ultrapassou os limites do mapa
        if (projX < LIMITE_MIN_X || projX > LIMITE_MAX_X || 
            projZ < LIMITE_MIN_Z || projZ > LIMITE_MAX_Z) 
        {
            disparado = false; // Parar o projétil
            std::cout << "Projétil saiu do mapa!" << std::endl;
        }

        // Exibe a nova posição do projétil
        printf("Nova posição do projétil: x:%f y:%f z:%f\n", projX, projY, projZ);
    }
    if (disparado1) {
        // Atualiza a posição com base na direção e velocidade
        projXd += dirProjXd * velocidadeProj; // Atualiza a posição no eixo X
        projYd += dirProjYd * velocidadeProj; // Atualiza a posição no eixo Y
        projZd += dirProjZd * velocidadeProj; // Atualiza a posição no eixo Z

        // Exibe a nova posição do projétil
        printf("Nova posição do projétil: x:%f y:%f z:%f\n", projX, projY, projZ);
    }

    if (disparado1) {
        // Atualiza a posição com base na direção e velocidade
        projXd += dirProjXd * velocidadeProj; // Atualiza a posição no eixo X
        projYd += dirProjYd * velocidadeProj; // Atualiza a posição no eixo Y
        projZd += dirProjZd * velocidadeProj; // Atualiza a posição no eixo Z

        // Verificar se o projétil ultrapassou os limites do mapa
        if (projXd < LIMITE_MIN_X || projXd > LIMITE_MAX_X || 
            projZd < LIMITE_MIN_Z || projZd > LIMITE_MAX_Z) 
        {
            disparado1 = false; // Parar o projétil
            std::cout << "Projétil saiu do mapa!" << std::endl;
        }

        // Exibe a nova posição do projétil
        printf("Nova posição do projétil: x:%f y:%f z:%f\n", projX, projY, projZ);
    }
}

void DesenhaParalelepipedoComTextura() {
    glBindTexture(GL_TEXTURE_2D, TEX); // Vincula a textura
    glEnable(GL_TEXTURE_2D); // Ativa a textura

    glPushMatrix();
    glTranslatef(0, 0, -1);
    glScalef(2, 1, 3);

    // Desenho manual do paralelepípedo
    glDisable(GL_CULL_FACE); 
    glBegin(GL_QUADS);
    // Frente
    glTexCoord2f(0, 0); glVertex3f(-1, -1, 1);
    glTexCoord2f(1, 0); glVertex3f( 1, -1, 1);
    glTexCoord2f(1, 1); glVertex3f( 1,  1, 1);
    glTexCoord2f(0, 1); glVertex3f(-1,  1, 1);

    // Trás
    glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f( 1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f( 1,  1, -1);
    glTexCoord2f(0, 1); glVertex3f(-1,  1, -1);

    // Esquerda
    glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f(-1, -1,  1);
    glTexCoord2f(1, 1); glVertex3f(-1,  1,  1);
    glTexCoord2f(0, 1); glVertex3f(-1,  1, -1);

    // Direita
    glTexCoord2f(0, 0); glVertex3f(1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f(1, -1,  1);
    glTexCoord2f(1, 1); glVertex3f(1,  1,  1);
    glTexCoord2f(0, 1); glVertex3f(1,  1, -1);

    // Topo
    glTexCoord2f(0, 0); glVertex3f(-1, 1, -1);
    glTexCoord2f(1, 0); glVertex3f( 1, 1, -1);
    glTexCoord2f(1, 1); glVertex3f( 1, 1,  1);
    glTexCoord2f(0, 1); glVertex3f(-1, 1,  1);

    // Base
    glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f( 1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f( 1, -1,  1);
    glTexCoord2f(0, 1); glVertex3f(-1, -1,  1);

    glEnd();

    glPopMatrix();
}



void DesenhaCuboComTextura(float tamAresta) {
    // Desativa o culling para garantir que todas as faces sejam desenhadas
    glDisable(GL_CULL_FACE);

    glBegin(GL_QUADS);
    
    // Front Face
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-tamAresta, -tamAresta, tamAresta);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-tamAresta, tamAresta, tamAresta);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(tamAresta, tamAresta, tamAresta);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(tamAresta, -tamAresta, tamAresta);
    
    // Back Face
    glNormal3f(0, 0, -1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-tamAresta, -tamAresta, -tamAresta);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-tamAresta, tamAresta, -tamAresta);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(tamAresta, tamAresta, -tamAresta);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(tamAresta, -tamAresta, -tamAresta);
    
    // Top Face
    glNormal3f(0, 1, 0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-tamAresta, tamAresta, -tamAresta);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-tamAresta, tamAresta, tamAresta);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(tamAresta, tamAresta, tamAresta);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(tamAresta, tamAresta, -tamAresta);
    
    // Bottom Face
    glNormal3f(0, -1, 0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-tamAresta, -tamAresta, -tamAresta);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(tamAresta, -tamAresta, -tamAresta);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(tamAresta, -tamAresta, tamAresta);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-tamAresta, -tamAresta, tamAresta);
    
    // Right Face
    glNormal3f(1, 0, 0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(tamAresta, -tamAresta, -tamAresta);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(tamAresta, tamAresta, -tamAresta);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(tamAresta, tamAresta, tamAresta);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(tamAresta, -tamAresta, tamAresta);
    
    // Left Face
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-tamAresta, -tamAresta, -tamAresta);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-tamAresta, -tamAresta, tamAresta);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-tamAresta, tamAresta, tamAresta);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-tamAresta, tamAresta, -tamAresta);
    glEnd();

    // Reativa o culling de faces, se necessário
    glEnable(GL_CULL_FACE);
}


// No seu loop de renderização ou função de desenho:
void desenharLinhaDirecao(float projX, float projY, float projZ, float dirProjX, float dirProjY, float dirProjZ)
{
    // Escala para ajustar o comprimento da linha (exemplo: 10x)
    float escala = 100.0f;

    // Ponto final da linha com base na direção do projétil
    float linhaX = projX + dirProjX * escala;
    float linhaY = projY + dirProjY * escala;
    float linhaZ = projZ + dirProjZ * escala;

    // Desenhar a linha
    glBegin(GL_LINES); // Inicia o desenho de linhas
        glColor3f(1.0f, 0.0f, 0.0f); // Cor da linha (vermelho, exemplo)
        glVertex3f(projX, projY, projZ); // Posição inicial (projétil)
        glVertex3f(linhaX, linhaY, linhaZ); // Posição final
    glEnd(); // Finaliza o desenho
}

// **********************************************************************
//  void display( void )
// **********************************************************************
float PosicaoZ = -30;
void display( void )
{

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	DefineLuz();
	PosicUser();

	glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(0, -1, 0);
    DesenhaChao();
    glPopMatrix();
    
	// glPushMatrix();
	// 	glTranslatef ( 5.0f, 0.0f, 0.0f );
    //     glRotatef(angulo,0,1,0);
    //     //glBindTexture (GL_TEXTURE_2D, TEX1);
	// 	glColor3f(0.5f,0.0f, 0.0f); // Vermelho
    //     glutSolidCube(2);
    //     //DesenhaCuboComTextura(1);
	// glPopMatrix();

	// glPushMatrix();
	// 	glTranslatef ( -4.0f, 0.0f, 2.0f );
	// 	glRotatef(angulo,0,1,0);
	// 	glColor3f(0.6156862745, 0.8980392157, 0.9803921569); // Azul claro
    //     glutSolidCube(2);
	// glPopMatrix();
    
    glPushMatrix();
        glTranslatef ( PosicaoDoObjeto.x, PosicaoDoObjeto.y, PosicaoDoObjeto.z );
        glRotatef(anguloPrincipal,0,1,0);
        glBindTexture (GL_TEXTURE_2D, TEX);//glColor3f(0.8f,0.8f, 0.0f); // AMARELO
        DesenhaParalelepipedoComTextura();//glutSolidCube(2);
        glRotatef(270.0f, 0, 1, 0);
        DesenhaCanhao();    

        // Tamanho do canhão (distância da base à ponta)
        float comprimentoCanhao = 0.0f; // Ajuste esse valor conforme o comprimento do canhão

        // Converte o ângulo para radianos
        float radAngulo = anguloPrincipal * M_PI / 180.0f;
        float radAnguloCanhao = (anguloCanhao ) * M_PI / 180.0f;
        float radAnguloCanhaod = (anguloCanhaod ) * M_PI / 180.0f;

        // A posição base do cubo já é PosicaoDoObjeto
        // A posição da ponta do canhão será calculada a partir dessa base com o ângulo de rotação

        // Calcula a direção do canhão (vetor de deslocamento)
        float direcaoX = cos(radAngulo) * comprimentoCanhao; // Direção no eixo X
        float direcaoY = sin(radAnguloCanhao);                             // Direção no eixo Y (se necessário ajuste a rotação no Y)
        float direcaoZ = sin(radAngulo) * comprimentoCanhao; // Direção no eixo Z

        float direcaoXd = cos(radAngulo) * comprimentoCanhao; // Direção no eixo X
        float direcaoYd = sin(radAnguloCanhaod);                             // Direção no eixo Y (se necessário ajuste a rotação no Y)
        float direcaoZd = sin(radAngulo) * comprimentoCanhao; // Direção no eixo Z

        float deslocamentoLateral = 2.0f;
        // A posição do cubo já é PosicaoDoObjeto
        // Agora, calculamos a posição da ponta do canhão com o deslocamento calculado
        pontaX = PosicaoDoObjeto.x + direcaoX ;
        pontaY = PosicaoDoObjeto.y + direcaoY;
        pontaZ = PosicaoDoObjeto.z + direcaoZ + deslocamentoLateral;

        pontaXd = PosicaoDoObjeto.x + direcaoXd ;
        pontaYd = PosicaoDoObjeto.y + direcaoYd;
        pontaZd = PosicaoDoObjeto.z + direcaoZd - deslocamentoLateral; 
        //desenharLinhaDirecao(pontaX, pontaY, pontaZ, projX, projY, projZ);

        // Exibe a posição da ponta do canhão
        //printf("Posição da ponta do canhão: x: %f, y: %f, z: %f\n", pontaX, pontaY, pontaZ);
        Ponto P;
        P = InstanciaPonto(Ponto(0,0,0), InvCameraMatrix);
        //P = InstanciaPonto(Ponto(0,0,0), OBS, ALVO);

        PosicaoDoObjeto.imprime("Posicao do Objeto:", "\n");
        //P.imprime("Ponto Instanciado: ", "\n");
    glPopMatrix();

    glColor3f(0.8,0.8,0);    
    DesenhaParedao();
    AtualizarPosicaoProjetil();
    DesenhaProjetil();

    //Exibe vaca
    // glPushMatrix();
    //     glTranslatef ( 0,0,0 );
    //     glRotatef(65,0,0,1);
    //     glRotatef(30.0,1,0,0);
    //     //glColor3f(1.0f,0.3f,0.0f);
    //     MundoVirtual[0].ExibeObjeto();
    // glPopMatrix();

    DesenhaLimitesMapa();

	glutSwapBuffers();
}


// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
//
//
// **********************************************************************
float speed = 0.5f; // Velocidade do movimento
Ponto direcao(0, 0, -1); // Direção inicial do observador (para frente)
Ponto direita(1, 0, 0);  // Direção inicial para direita
void keyboard ( unsigned char key, int x, int y )
{
	switch ( key )
	{
    case 27:        // Termina o programa qdo
      exit ( 0 );   // a tecla ESC for pressionada
      break;
    case 'p':
            ModoDeProjecao = !ModoDeProjecao;
            glutPostRedisplay();
            break;
    case 'o':
            //ModoDeProjecao = !ModoDeProjecao;
            tipoVista = Superior;
            glutPostRedisplay();
            break;
    case 'l':
            tipoVista = Jogador;
            glutPostRedisplay();
            break;            
    case 'e':
            ModoDeExibicao = !ModoDeExibicao;
            init();
            glutPostRedisplay();
            break;
    case 'f':
            tipoVista = FreeCam;
            break;
    case 'b':
        tipoVista = PlayerCam;
        break;
    case 'w': yObs += 1; break; // Move para cima
    case 's': yObs -= 1; break; // Move para baixo
    case 'a': xObs -= 1; break; // Move para esquerda
    case 'd': xObs += 1; break; // Move para direita
    case 'n': 
        anguloCanhao += 5.0f;
        if(anguloCanhao > 360)
            anguloCanhao = 0;
        DirecaoDoCanhao.rotacionaY(anguloCanhao);
        break; // Move para direita
    case 'm':
        anguloCanhaod += 5.0f;
        if(anguloCanhaod > 360)
            anguloCanhaod = 0; 
        DirecaoDoCanhao.rotacionaY(anguloCanhao);
        break; // Move para direita
    case 'r': // Resetar a posição
        OBS = Ponto(0, 3, 10);
        ALVO = Ponto(0, 0, 0);
        break;  
    case ' ': // Resetar a posição
        if (!disparado) // Verifica se o projétil não foi disparado
        {
            // Converte os ângulos para radianos
            float radX = anguloPrincipal * M_PI / 180.0f; // Ângulo do canhão
            float radY = (anguloCanhao) * M_PI / 180.0f; // Ângulo principal (se necessário)

            disparado = true;

            dirProjX = cos(radY) * sin(radX); // Direção no eixo X
            dirProjY = sin(radY);             // Direção no eixo Y
            dirProjZ = cos(radY) * cos(radX); // Direção no eixo Z
            

            // A posição inicial do projétil é a posição da ponta do canhão
            projX = pontaX + dirProjX;
            projY = pontaY + dirProjY;
            projZ = pontaZ + dirProjZ;
            

            // Exibe a posição inicial do projétil
            printf("Posição inicial do projétil: x:%f y:%f z:%f\n", projX, projY, projZ);
        }
        if(!disparado1){
            float radX = anguloPrincipal * M_PI / 180.0f; // Ângulo do canhão            
            float radYd = (anguloCanhaod) * M_PI / 180.0f; // Ângulo principal (se necessário)
        
            disparado1 = true;

            dirProjXd = cos(radYd) * sin(radX); // Direção no eixo X
            dirProjYd = sin(radYd);             // Direção no eixo Y
            dirProjZd = cos(radYd) * cos(radX); // Direção no eixo Z
            projXd = pontaXd + dirProjXd;
            projYd = pontaYd + dirProjYd;
            projZd = pontaZd + dirProjZd;
        }
        break;   
    default:
            cout << key;
    break;
  }
}

void atualizaCamera()
{
    // Posição da câmera em relação ao jogador
    float eyeX = PosicaoDoObjeto.x;
    float eyeY = PosicaoDoObjeto.y + 5.0f; // Eleva a câmera acima do jogador
    float eyeZ = PosicaoDoObjeto.z + 10.0f; // Afasta a câmera atrás do jogador

    // Ponto para onde a câmera está olhando (o jogador)
    float centerX = PosicaoDoObjeto.x;
    float centerY = PosicaoDoObjeto.y;
    float centerZ = PosicaoDoObjeto.z;

    // Vetor "para cima" (normalmente o eixo Y positivo)
    float upX = 0.0f;
    float upY = 1.0f;
    float upZ = 0.0f;

    // Define a visão da câmera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ,  // Posição da câmera
              centerX, centerY, centerZ,  // Ponto para onde a câmera olha
              upX, upY, upZ);  // Vetor "para cima"
}

// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
	float novoX = PosicaoDoObjeto.x;
    float novoZ = PosicaoDoObjeto.z;

    switch ( a_keys )
    {
        case GLUT_KEY_UP:
            // Atualizar posição ao clicar seta para cima
            novoX += velocidadeObj * sin(anguloPrincipal * M_PI / 180.0f);
            novoZ += velocidadeObj * cos(anguloPrincipal * M_PI / 180.0f);
            break;
        case GLUT_KEY_DOWN:
            // Atualizar posição ao clicar seta para cima
            novoX -= velocidadeObj * sin(anguloPrincipal * M_PI / 180.0f);
            novoZ -= velocidadeObj * cos(anguloPrincipal * M_PI / 180.0f);
            break;
        case GLUT_KEY_RIGHT:
            // Alterar o ângulo ao clicar a seta da direita
            anguloPrincipal -= 5.0f;
            //DirecaoDoObjeto.rotacionaZ(anguloPrincipal);
            break;
        case GLUT_KEY_LEFT:
            // Alterar o ângulo ao clicar a seta da esquerda
            anguloPrincipal += 5.0f;
            //DirecaoDoObjeto.rotacionaZ(anguloPrincipal);
            break;
        default:
            break;
    }

    // Verificar limites antes de atualizar a posição
    if (novoX >= LIMITE_MIN_X && novoX <= LIMITE_MAX_X && 
        novoZ >= LIMITE_MIN_Z && novoZ <= LIMITE_MAX_Z) 
    {
        PosicaoDoObjeto.x = novoX;
        PosicaoDoObjeto.z = novoZ;
    }
}

// **********************************************************************
//  void main ( int argc, char** argv )
// **********************************************************************
int main ( int argc, char** argv )
{
	glutInit            ( &argc, argv );
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
	glutInitWindowPosition (0,0);
	glutInitWindowSize  ( 700, 700 );
	glutCreateWindow    ( "Computacao Grafica - Exemplo Basico 3D" );

	init ();
    //system("pwd");

	glutDisplayFunc ( display );
	glutReshapeFunc ( reshape );
	glutKeyboardFunc ( keyboard );
	glutSpecialFunc ( arrow_keys );
	glutIdleFunc ( animate );
    
    //char Nome[] = "Vaca.tri";
    //MundoVirtual = new Objeto3D[5];
    //carrega obj .tri
    //MundoVirtual[0].LeObjeto(Nome);
    //MundoVirtual[1].LeObjeto("watership.tri");
	
    glutMainLoop ( );
	return 0;
}



