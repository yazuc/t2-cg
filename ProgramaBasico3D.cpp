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
    FreeCam
    };

Temporizador T;
double AccumDeltaT=0;


GLfloat AspectRatio, angulo=0;

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
double nFrames=0;
double TempoTotal=0;
bool freecam = false;
Ponto CantoEsquerdo = Ponto(-20,0,-10);
Ponto OBS;
Ponto ALVO;
Ponto VetorAlvo;
GLfloat CameraMatrix[4][4];
GLfloat InvCameraMatrix[4][4];
GLuint TEX1, TEX2;
Ponto PosicaoDoObjeto(0,0,4);
ModoExibicao tipoVista {Jogador};

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
    TEX2 = LoadTexture ("./grass_texture.jpg");
    TEX1 = LoadTexture ("./igordinho.png");
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
void DesenhaParalelepipedo()
{
    glPushMatrix();
        glTranslatef(0,0,-1);
        glScalef(1,1,2);
        glutSolidCube(2);
        //DesenhaCubo(1);
    glPopMatrix();
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
{
    // Altura do paredão
    const int altura = 15; // 15m
    
    // Largura do paredão (meio do cenário)
    const int largura = 15; // Exemplo: ajusta conforme o tamanho maior do cenário
    
    glPushMatrix();

    // Gira o paredão para ficar perpendicular ao chão
    glRotatef(90, 0, 0, 1);

    // Posiciona o paredão no meio do cenário
    glTranslatef(-1, -7, -10);

    // Desenha os quadrados de 1m x 1m que compõem o paredão
    for (int i = 0; i < altura; i++) // Altura (vertical)
    {
        for (int j = 0; j < largura; j++) // Largura (horizontal)
        {
            glPushMatrix();
            glTranslatef(j, i, 0); // Move para a posição do quadrado

            // Ajusta as coordenadas de textura para que a textura se repita
            glBindTexture(GL_TEXTURE_2D, TEX1);  // Aplica a textura

            glBegin(GL_QUADS);
            glColor3f(1.0, 1.0, 1.0); // Cor branca para a textura

            // Vértices com coordenadas de textura ajustadas para repetição
            glTexCoord2f(j / float(largura), i / float(altura)); // Coordenada de textura para o vértice (0,0)
            glVertex3f(0, 0, 0);

            glTexCoord2f((j + 1) / float(largura), i / float(altura)); // Coordenada de textura para o vértice (1,0)
            glVertex3f(1, 0, 0);

            glTexCoord2f((j + 1) / float(largura), (i + 1) / float(altura)); // Coordenada de textura para o vértice (1,1)
            glVertex3f(1, 1, 0);

            glTexCoord2f(j / float(largura), (i + 1) / float(altura)); // Coordenada de textura para o vértice (0,1)
            glVertex3f(0, 1, 0);

            glEnd();
            glPopMatrix();
        }
    }
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
    
	glPushMatrix();
		glTranslatef ( 5.0f, 0.0f, 3.0f );
        glRotatef(angulo,0,1,0);
        //glBindTexture (GL_TEXTURE_2D, TEX1);
		glColor3f(0.5f,0.0f, 0.0f); // Vermelho
        glutSolidCube(2);
        //DesenhaCuboComTextura(1);
	glPopMatrix();

	glPushMatrix();
		glTranslatef ( -4.0f, 0.0f, 2.0f );
		glRotatef(angulo,0,1,0);
		glColor3f(0.6156862745, 0.8980392157, 0.9803921569); // Azul claro
        glutSolidCube(2);
	glPopMatrix();
    
    glPushMatrix();
        glTranslatef ( PosicaoDoObjeto.x, PosicaoDoObjeto.y, PosicaoDoObjeto.z );
        //glRotatef(angulo,0,1,0);
        glBindTexture (GL_TEXTURE_2D, TEX1);//glColor3f(0.8f,0.8f, 0.0f); // AMARELO
        DesenhaCuboComTextura(1);//glutSolidCube(2);
        Ponto P;
        P = InstanciaPonto(Ponto(0,0,0), InvCameraMatrix);
        //P = InstanciaPonto(Ponto(0,0,0), OBS, ALVO);

        PosicaoDoObjeto.imprime("Posicao do Objeto:", "\n");
        P.imprime("Ponto Instanciado: ", "\n");
    glPopMatrix();

    glColor3f(0.8,0.8,0);
    //glutSolidTeapot(2);
    DesenhaParedao();
    

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
    case 'w': xObs += 1; break; // Move para cima
    case 's': xObs -= 1; break; // Move para baixo
    case 'a': yObs -= 1; break; // Move para esquerda
    case 'd': yObs += 1; break; // Move para direita
    case 'r': // Resetar a posição
        OBS = Ponto(0, 3, 10);
        ALVO = Ponto(0, 0, 0);
        break;    
    default:
            cout << key;
    break;
  }
}

// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
		case GLUT_KEY_UP:       // When Up Arrow Is Pressed...
            PosicaoDoObjeto.z--;
			break;
	    case GLUT_KEY_DOWN:     // When Down Arrow Is Pressed...
            PosicaoDoObjeto.z++; 
			break;
        case GLUT_KEY_RIGHT:
            PosicaoDoObjeto.x++;
            break;
        case GLUT_KEY_LEFT:
            PosicaoDoObjeto.x--;
            break;

		default:
			break;
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

	glutMainLoop ( );
	return 0;
}



