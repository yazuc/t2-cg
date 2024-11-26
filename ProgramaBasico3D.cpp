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


struct Vertex {
    float x, y, z;
};

struct Face {
    std::vector<int> vertexIndices; // Índices dos vértices
};

// Estrutura para armazenar o modelo
struct Model {
    std::vector<Vertex> vertices; // Lista de vértices
    std::vector<Face> faces;      // Lista de faces
};

enum ModoExibicao {
    Jogador,
    Superior,
    SuperiorCompleto,
    FreeCam,
    PlayerCam
    };

struct Projectile {
    float x, y, z;
    bool active;
};

struct BoundingBox {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

Projectile proj;
Projectile proj2;

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
bool estaQuebrado(float projX, float projY, float projZ, int altura, int largura);
Ponto CantoEsquerdo = Ponto(-20,0,-10);
Ponto OBS;
Ponto ALVO;
Ponto VetorAlvo;
GLfloat CameraMatrix[4][4];
GLfloat InvCameraMatrix[4][4];
GLuint TEX1, TEX2, TEX, TEX3;
GLfloat anguloCanhao = 5.0f;
GLfloat anguloCanhaod = 5.0f;
Ponto PosicaoDoObjeto(-29.5,0,10);
Ponto DirecaoDoObjeto(1,0,0);
Ponto DirecaoDoCanhao(1,0,0);
ModoExibicao tipoVista {PlayerCam};
float projX = 0.0f, projY = 0.0f, projZ = -10.0f; // Posição inicial do projétil
float projXd = 0.0f, projYd = 0.0f, projZd = -10.0f; // Posição inicial do projétil
Model model;
Model amigo;
Model Canhao;

int pontuacao = 0;
float velocidadeProj = 1.0f; // Velocidade do projétil
const float velocidadeObj = 0.5f; // Velocidade do objeto

float pontaX, pontaY, pontaZ, dirProjX, dirProjY, dirProjZ = 0.0;
float pontaXd, pontaYd, pontaZd, dirProjXd, dirProjYd, dirProjZd = 0.0;
const int largura = 40;  // Número de blocos na largura do paredão
const int altura = 15;   // Número de blocos na altura do paredão
bool paredao[altura][largura]; // Matriz de blocos do paredão (true = ativo)
const float LIMITE_MIN_X = -40; // Limite mínimo do mapa no eixo X
const float LIMITE_MAX_X = 40; // Limite máximo do mapa no eixo X
const float LIMITE_MIN_Z = -10; // Limite mínimo do mapa no eixo Z
const float LIMITE_MAX_Z = 30; // Limite máximo do mapa no eixo Z
float P0x, P0y, P0z; // Ponto inicial
float P1x, P1y, P1z; // Ponto de controle 1
float P2x, P2y, P2z; // Ponto de controle 2
float P3x, P3y, P3z; // Ponto final
float P0xd, P0yd, P0zd; // Ponto inicial
float P1xd, P1yd, P1zd; // Ponto de controle 1
float P2xd, P2yd, P2zd; // Ponto de controle 2
float P3xd, P3yd, P3zd; // Ponto final

float t = 0.0f;  // Parâmetro da curva (de 0 a 1)
float dt = 0.01f; // Incremento de t para o movimento

float td = 0.0f;  // Parâmetro da curva (de 0 a 1)
float dtd = 0.01f; // Incremento de t para o movimento

const int NUM_OBJETOS = 20;

struct Objeto {
    float x, y, z;       // Centro do objeto
    float r, g, b;
    float largura, altura, profundidade; // Dimensões
    int tipo;        // 1 para inimigo, 2 para amigo, 3 para canhão
    bool ativo;          // true se o objeto ainda não foi atingido
};

Objeto objetos[NUM_OBJETOS]; // Matriz estática para armazenar os objetos

const int larguraPiso = 40; // Número de blocos na largura do piso
const int profundidadePiso = 40; // Número de blocos na profundidade do piso
bool pisoEsquerda[larguraPiso][profundidadePiso]; // Matriz de blocos do piso da esquerda (true = ativo)
bool pisoDireita[larguraPiso][profundidadePiso]; // Matriz de blocos do piso da direita (true = ativo)


BoundingBox getModelBoundingBox(const Model& model){

  float minX = model.vertices[0].x;
  float minY = model.vertices[0].y;
  float minZ = model.vertices[0].z;
  float maxX = model.vertices[0].x;
  float maxY = model.vertices[0].y;
  float maxZ = model.vertices[0].z;

  for(size_t i = 1; i < model.vertices.size(); i++){
    minX = std::min(minX, model.vertices[i].x);
    minY = std::min(minY, model.vertices[i].y);
    minZ = std::min(minZ, model.vertices[i].z);

    maxX = std::max(maxX, model.vertices[i].x);
    maxY = std::max(maxY, model.vertices[i].y);
    maxZ = std::max(maxZ, model.vertices[i].z);
  }

   return {minX, minY, minZ, maxX, maxY, maxZ};
}
float clamp(float value, float min, float max) {
    return std::max(min, std::min(value, max));
}
bool checkObjectCollision(float px, float py, float pz, const Objeto& obj, const Model& m) {
    BoundingBox modelBox = getModelBoundingBox(m);
    modelBox.minX *= obj.largura;
    modelBox.minY *= obj.altura;
    modelBox.minZ *= obj.profundidade;
    modelBox.maxX *= obj.largura;
    modelBox.maxY *= obj.altura;
    modelBox.maxZ *= obj.profundidade;


    modelBox.minX += obj.x;
    modelBox.minY += obj.y;
    modelBox.minZ += obj.z;
    modelBox.maxX += obj.x;
    modelBox.maxY += obj.y;
    modelBox.maxZ += obj.z;

    float closestX = clamp(px, modelBox.minX, modelBox.maxX);
    float closestY = clamp(py, modelBox.minY, modelBox.maxY);
    float closestZ = clamp(pz, modelBox.minZ, modelBox.maxZ);

    float distanceSquared = (px - closestX) * (px - closestX) +
                           (py - closestY) * (py - closestY) +
                           (pz - closestZ) * (pz - closestZ);

    return distanceSquared <= 0.25f; 
}

void handleObjectCollision(int objectType){
    if (objectType == 1) {
        pontuacao += 10;
        std::cout << "Atingiu inimigo! Pontuação: " << pontuacao << std::endl;
    } else if (objectType == 2) {
        pontuacao -= 10;
        std::cout << "Atingiu amigo! Pontuação: " << pontuacao << std::endl;
    } else if (objectType == 3){
        std::cout << "Fim de jogo! Pontuação final: " << pontuacao << std::endl;
        exit(0);
    }
}

bool checkWallCollision(float px, float py, float pz) {
    const float paredaoMinX = -6.0f; 
    const float paredaoMaxX = -4.0f; 
    const float paredaoMinZ = -9.0f; 
    const float paredaoMaxZ = 30.0f; 
    const float paredaoMinY = -0.5f;  
    const float paredaoMaxY = 10.0f; 

    // Encontrar o ponto mais próximo do projétil no paredão
    float closestX = clamp(px, paredaoMinX, paredaoMaxX);
    float closestY = clamp(py, paredaoMinY, paredaoMaxY);
    float closestZ = clamp(pz, paredaoMinZ, paredaoMaxZ);

    // Calcular distância ao ponto mais próximo
    float distanceSquared = (px - closestX) * (px - closestX) +
                            (py - closestY) * (py - closestY) +
                            (pz - closestZ) * (pz - closestZ);
    // Verificar colisão (esfera com raio 0.5)
    if (distanceSquared <= 0.25f) {
        std::cout << "Colisão detectada!\n";
        return true;
    }

    std::cout << "Sem colisão.\n";
    return false;
}

bool LoadOBJ(const std::string& filePath, Model& model) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") { // Vértices
            Vertex vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            model.vertices.push_back(vertex);
        } else if (prefix == "f") { // Faces
            Face face;
            std::string vertexData;
            while (iss >> vertexData) {
                std::istringstream vss(vertexData);
                std::string vertexIndex;
                std::getline(vss, vertexIndex, '/'); // Pega apenas o índice do vértice
                face.vertexIndices.push_back(std::stoi(vertexIndex) - 1); // Índices começam em 1 no .obj
            }
            model.faces.push_back(face);
        }
    }

    file.close();
    return true;
}

void DrawModel(const Model& model) {
    glBegin(GL_TRIANGLES);
    for (const auto& face : model.faces) {
        for (const auto& index : face.vertexIndices) {
            const Vertex& vertex = model.vertices[index];
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
    }
    glEnd();
}


void inicializarPisos() {
    for (int i = 0; i < larguraPiso; i++) {
        for (int j = 0; j < profundidadePiso; j++) {
            pisoEsquerda[i][j] = true; // Todos os ladrilhos estão presentes inicialmente
            pisoDireita[i][j] = true; // Todos os ladrilhos estão presentes inicialmente
        }
    }
}

void inicializarObjetos() {
    srand(time(0)); // Semente para posições aleatórias
    int a, b, c, d = 0;
    a= 30;
    b = 20;
    c = 10; 
    d = 2;
    float coordenadas[20][2] = {
        {a, 24},
        {a, 16},
        {a, 8},
        {a, 0},
        {a, -8},
        {b, 24},
        {b, 16},
        {b, 8},
        {b, 0},
        {b, -8},
        {c, 24},
        {c, 16},
        {c, 8},
        {c, 0},
        {c, -8},
        {d, 24},
        {d, 16},
        {d, 8},
        {d, 0},
        {d, -8},
    };
    for (int i = 0; i < NUM_OBJETOS; i++) {
        objetos[i].x = coordenadas[i][0];
        objetos[i].y = 0;
        objetos[i].z = coordenadas[i][1];
        objetos[i].r = static_cast<float>(rand()) / RAND_MAX;
        objetos[i].g = static_cast<float>(rand()) / RAND_MAX;
        objetos[i].b = static_cast<float>(rand()) / RAND_MAX;
        objetos[i].largura = 2.0f;  // Tamanho fixo para todos
        objetos[i].altura = 2.0f;   // Tamanho fixo para todos
        objetos[i].profundidade = 2.0f; // Tamanho fixo para todos
        objetos[i].tipo = (i == 3) ? 3 : (i < 10) ? 1 : 2;
        objetos[i].ativo = true;     // Todos começam ativos
    }
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
    TEX1 = LoadTexture ("./paredao.png");
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

    glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    if (ModoDeExibicao) // Faces Preenchidas??
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    ALVO = Ponto(0, 0, 0);
    OBS = Ponto(0,3,10);
    VetorAlvo = ALVO - OBS;        
    initTexture();
    inicializarPisos();
    inicializarParedao();
    inicializarObjetos();
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

bool verificarColisoesObjetosPiso(float x, float y, float z) {   
     for (int i = 0; i < NUM_OBJETOS; ++i) {
        if (!objetos[i].ativo) continue;

        if (checkObjectCollision(x, y, z, objetos[i], (objetos[i].tipo == 1 || objetos[i].tipo == 3) ? model : amigo)) {
            objetos[i].ativo = false; 
            handleObjectCollision(objetos[i].tipo); 
            return true;  
        }
    }
     return false; 
}

bool verificarColisaoCarro(float novoX, float novoZ, int altura, int largura) {
    const float paredaoXMin = -7.0f;
    const float paredaoXMax = -5.0f;
    const float paredaoZMin = -9.0f;
    const float paredaoZMax = 33.0f;

    //Simulate projectile position
    float projX = novoX;
    float projY = 0; //Adjust if necessary
    float projZ = novoZ;

    if (!estaQuebrado(projX, projY, projZ, altura, largura)) { 
        bool colisao = (novoX >= paredaoXMin && novoX <= paredaoXMax &&
                        novoZ >= paredaoZMin && novoZ <= paredaoZMax);
        if (colisao) {
            std::cout << "Atingiu paredão com o carro!" << std::endl;
            return true;
        } else {
            std::cout << "Pode atravessar o paredão!" << std::endl;
            return false;
        }
    } else {
        std::cout << "Pode atravessar o paredão!" << std::endl;
        return false;
    }
}


void quebrarBlocoPiso(float x, float z) {
    const int alcance = 1; // Alcance de destruição (1 bloco ao redor)

    bool areaEsquerda = (x < 0); // Determina se está na área esquerda ou direita
    bool (&pisoAtual)[larguraPiso][profundidadePiso] = areaEsquerda ? pisoEsquerda : pisoDireita;

    // Ajusta o deslocamento para lidar com as coordenadas locais de cada piso
    float xOffset = areaEsquerda ? LIMITE_MIN_X : 0.0f; 
    float zOffset = LIMITE_MIN_Z;

    // Converte a posição do projétil para índices da matriz
    int blocoX = int((x - xOffset) / ((LIMITE_MAX_X - LIMITE_MIN_X) / 2.0f) * larguraPiso);
    int blocoZ = int((z - zOffset) / (LIMITE_MAX_Z - LIMITE_MIN_Z) * profundidadePiso);

    // Garante que os índices estão dentro dos limites da matriz
    for (int dx = -alcance; dx <= alcance; dx++) {
        for (int dz = -alcance; dz <= alcance; dz++) {
            int vizinhoX = blocoX + dx;
            int vizinhoZ = blocoZ + dz;

            if (vizinhoX >= 0 && vizinhoX < larguraPiso &&
                vizinhoZ >= 0 && vizinhoZ < profundidadePiso) {
                pisoAtual[vizinhoX][vizinhoZ] = false; // Marca o ladrilho como destruído
            }
        }
    }
}


bool quebrarBloco(float projX, float projY, float projZ)
{
    const float transX = -18.0f; 
    const float transY = -1.0f;  
    const float transZ = 7.0f;   
    const float rotAngulo = 90.0f; 

    projX -= transX;
    projY -= transY + 7;
    projZ -= transZ + 2;

    float radAngulo = rotAngulo * M_PI / 180.0f; 
    float cosAngulo = cos(-radAngulo);  
    float sinAngulo = sin(-radAngulo);

    float rotatedX = cosAngulo * projX + sinAngulo * projZ;
    float rotatedZ = sinAngulo * projX - cosAngulo * projZ;

    projX = rotatedX;
    projZ = rotatedZ;

    int blocoY = int(projY + altura / 2.0f);   
    int blocoX = int((projX + largura / 2.0f));  

    if (blocoX >= 0 && blocoX < largura && blocoY >= 0 && blocoY < altura)
    {
        if(paredao[blocoY][blocoX]){
            pontuacao += 5;
            return paredao[blocoY][blocoX] = false; 
        }
    }
    return true;
}

bool estaQuebrado(float projX, float projY, float projZ, int altura, int largura) {
    const float transX = -18.0f;
    const float transY = -1.0f;
    const float transZ = 7.0f;
    const float rotAngulo = 90.0f; // Rotation around Y-axis

    projX -= transX;
    projY -= transY + 7;
    projZ -= transZ + 2;

    float radAngulo = rotAngulo * M_PI / 180.0f; 
    float cosAngulo = cos(-radAngulo);  
    float sinAngulo = sin(-radAngulo);

    float rotatedX = cosAngulo * projX + sinAngulo * projZ;
    float rotatedZ = sinAngulo * projX - cosAngulo * projZ;

    projX = rotatedX;
    projZ = rotatedZ;

    int blocoY = int(projY + altura / 2.0f);   
    int blocoX = int((projX + largura / 2.0f));  
  
    if (blocoX < 0 || blocoX >= largura || blocoY < 0 || blocoY >= altura) {
        return false; 
    }
    return !paredao[blocoY][blocoX]; 
}


void DesenhaObjetos() {
    for (int i = 0; i < NUM_OBJETOS; i++) {
        if (!objetos[i].ativo) continue; // Ignorar objetos inativos

        const Objeto& obj = objetos[i];

        glPushMatrix();
        glTranslatef(obj.x, obj.y, obj.z);
        glColor3f(obj.r, obj.g, obj.b);
        glScalef(obj.largura, obj.altura, obj.profundidade);
        if(objetos[i].tipo == 1){
            DrawModel(model);
        }
        if(objetos[i].tipo == 2){
            DrawModel(amigo);
        }
        if(objetos[i].tipo == 3){            
            DrawModel(model);
        }
        glPopMatrix();
    }
}

Ponto caculaBezier(float t, float P0x, float P0y, float P0z,
                           float P1x, float P1y, float P1z,
                           float P2x, float P2y, float P2z,
                           float P3x, float P3y, float P3z) {
    float u = 1.0f - t;
    float x = P0x * u * u * u + 3 * P1x * u * u * t + 3 * P2x * u * t * t + P3x * t * t * t;
    float y = P0y * u * u * u + 3 * P1y * u * u * t + 3 * P2y * u * t * t + P3y * t * t * t;
    float z = P0z * u * u * u + 3 * P1z * u * u * t + 3 * P2z * u * t * t + P3z * t * t * t;
    return Ponto(x,y,z);
}

void calculaParamProj() {
    float radX = anguloPrincipal * M_PI / 180.0f; 
    float radY = anguloCanhao * M_PI / 180.0f;   

    float initialSpeed = 15.0f; 
    float dirX = cos(radY) * sin(radX);
    float dirY = sin(radY);
    float dirZ = cos(radY) * cos(radX);

    float scale = 10.0f; 
    P0x = pontaX;
    P0y = pontaY;
    P0z = pontaZ;

    P1x = P0x + dirX * scale;
    P1y = P0y + dirY * scale + 5; 
    P1z = P0z + dirZ * scale;

    P2x = P1x + dirX * scale;
    P2y = P1y + dirY * scale - 5; 
    P2z = P1z + dirZ * scale;

    P3x = P2x + dirX * scale * 2; 
    P3y = P2y;
    P3z = P2z;
}

void calculaParamProjd() {
    float radX = anguloPrincipal * M_PI / 180.0f; 
    float radY = anguloCanhaod * M_PI / 180.0f;   

    float initialSpeed = 15.0f; 
    float dirX = cos(radY) * sin(radX);
    float dirY = sin(radY);
    float dirZ = cos(radY) * cos(radX);

    float scale = 10.0f; 
    P0xd = pontaXd;
    P0yd = pontaYd;
    P0zd = pontaZd;

    P1xd = P0xd + dirX * scale;
    P1yd = P0yd + dirY * scale + 5; 
    P1zd = P0zd + dirZ * scale;

    P2xd = P1xd + dirX * scale;
    P2yd = P1yd + dirY * scale - 5; 
    P2zd = P1zd + dirZ * scale;

    P3xd = P2xd + dirX * scale * 2; 
    P3yd = P2yd;
    P3zd = P2zd;
}

void DesenhaProjetil()
{
    bool continua;
    bool continua1;
    if (proj.active) {
        Ponto p = caculaBezier(t, P0x, P0y, P0z, P1x, P1y, P1z, P2x, P2y, P2z, P3x, P3y, P3z);
        proj.x = p.x;
        proj.y = p.y;
        proj.z = p.z;

        // Draw the projectile
        glPushMatrix();
        glTranslatef(proj.x, proj.y, proj.z);
        glutSolidSphere(0.5f, 20, 20);
        glPopMatrix();

        t += dt * velocidadeProj;
        if(!estaQuebrado(proj.x, proj.y, proj.z, altura, largura)){
            if (checkWallCollision(proj.x, proj.y, proj.z)) 
            {
                p.imprime("Posicao do Projetil");
                const int alcance = 1; 
                for (int dx = -alcance; dx <= alcance; ++dx)
                {
                    for (int dy = -alcance; dy <= alcance; ++dy)
                    {
                        for (int dz = -alcance; dz <= alcance; ++dz)
                        {
                            int blocoX = static_cast<int>(proj.x) + dx;
                            int blocoY = static_cast<int>(proj.y) + dy;
                            int blocoZ = static_cast<int>(proj.z) + dz;

                            quebrarBloco(blocoX, blocoY, blocoZ);
                        }
                    }
                }    
                proj.active = false;
            }
        }else{                 
            printf("esta quebrado");
             proj.active = !verificarColisoesObjetosPiso(proj.x, proj.y, proj.z);                  
        }
        // Colisão com o piso
        if (proj.z >= LIMITE_MIN_Z && proj.z <= LIMITE_MAX_Z && proj.x >= LIMITE_MIN_X && proj.x <= LIMITE_MAX_X && proj.y <= -0.10f) {
            pontuacao -= 5;
            quebrarBlocoPiso(proj.x, proj.z); 
            proj.active = false;
        }            
        if (t >= 1.0f) {     
            verificarColisoesObjetosPiso(proj.x, proj.y, proj.z);       
            proj.active = false;
        }
        if(verificarColisoesObjetosPiso(proj.x, proj.y, proj.z) && t <= 1.0f){
            proj.active = false;
        }
    }
    if (proj2.active) {
        Ponto p = caculaBezier(td, P0xd, P0yd, P0zd, P1xd, P1yd, P1zd, P2xd, P2yd, P2zd, P3xd, P3yd, P3zd);
        proj2.x = p.x;
        proj2.y = p.y;
        proj2.z = p.z;

        // Draw the projectile
        glColor3f(1.0f, 1.0f, 0.0f);
        glPushMatrix();
        glTranslatef(proj2.x, proj2.y, proj2.z);
        glutSolidSphere(0.5f, 20, 20);
        glPopMatrix();

        td += dtd * velocidadeProj;
        if(!estaQuebrado(proj2.x, proj2.y, proj2.z, altura, largura)){
            if (checkWallCollision(proj2.x, proj2.y, proj2.z)) 
            {
                const int alcance = 1; 
                for (int dx = -alcance; dx <= alcance; ++dx)
                {
                    for (int dy = -alcance; dy <= alcance; ++dy)
                    {
                        for (int dz = -alcance; dz <= alcance; ++dz)
                        {
                            int blocoX = static_cast<int>(proj.x) + dx;
                            int blocoY = static_cast<int>(proj2.y) + dy;
                            int blocoZ = static_cast<int>(proj2.z) + dz;

                            quebrarBloco(blocoX, blocoY, blocoZ);
                        }
                    }
                }    
                proj2.active = false;
            }
        }else{           
             proj2.active = !verificarColisoesObjetosPiso(proj2.x, proj2.y, proj2.z);                                       
        }
        // Colisão com o piso
        if (proj2.z >= LIMITE_MIN_Z && proj2.z <= LIMITE_MAX_Z && proj2.x >= LIMITE_MIN_X && proj2.x <= LIMITE_MAX_X && proj2.y <= 0.0f) {
            pontuacao -= 5;
            quebrarBlocoPiso(proj2.x, proj2.z); 
            proj2.active = false;
        }            
        if (td >= 1.0f) {     
            verificarColisoesObjetosPiso(proj2.x, proj2.y, proj2.z);       
            proj2.active = false;
        }
        if(verificarColisoesObjetosPiso(proj2.x, proj2.y, proj2.z) && t <= 1.0f){
            proj2.active = false;
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
void DesenhaPiso(bool pisoAtual[larguraPiso][profundidadePiso]) {
    srand(100); // Usa uma semente fixa para gerar sempre as mesmas cores no piso
    glPushMatrix();
    glTranslated(CantoEsquerdo.x, CantoEsquerdo.y, CantoEsquerdo.z);
    for (int x = 0; x < larguraPiso; x++) {
        glPushMatrix();
        for (int z = 0; z < profundidadePiso; z++) {
            if (pisoAtual[x][z]) { // Desenha apenas os ladrilhos ativos
                DesenhaLadrilho(MediumGoldenrod, rand() % 40);
            }
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
{    
    glPushMatrix();
    glRotatef(90, 0, 1, 0); 
    glTranslatef(-29.5, -1, -5);

    // Desenha os quadrados de 1m x 1m
    for (int i = 0; i < altura; i++) 
    {
        for (int j = 0; j < largura; j++) 
        {
            if (paredao[i][j]) 
            {
                glPushMatrix(); 
                    glTranslatef(j, i, 0); 
                    glBindTexture(GL_TEXTURE_2D, TEX1);
                    glBegin(GL_QUADS);
                        glColor3f(1.0f, 1.0f, 1.0f); 
                        glTexCoord2f(j / float(largura), i / float(altura)); 
                        glVertex3f(0, 0, 0);
                        glTexCoord2f((j + 1) / float(largura), i / float(altura)); // (1, 0)
                        glVertex3f(1, 0, 0);
                        glTexCoord2f((j + 1) / float(largura), (i + 1) / float(altura)); // (1, 1)
                        glVertex3f(1, 1, 0);
                        glTexCoord2f(j / float(largura), (i + 1) / float(altura)); // (0, 1)
                        glVertex3f(0, 1, 0);
                    glEnd(); 
                glPopMatrix(); 
            }
        }
    }
    glPopMatrix();
}

void DesenhaChao()
{
    glPushMatrix();
        glTranslated(-20, 0, 0);
        DesenhaPiso(pisoEsquerda);
    glPopMatrix();
    glPushMatrix();
        glTranslated(20, 0, 0);
        DesenhaPiso(pisoDireita);
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
  glMateriali(GL_FRONT,GL_SHININESS,128);

}
// **********************************************************************

void MygluPerspective(float fieldOfView, float aspect, float zNear, float zFar )
{
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
    else MygluPerspective(90, AspectRatio, 0.1, 1000.0f); // Projecao perspectiva

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

}

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


void exibirTexto(int x, int y, const char *texto) {
    glRasterPos2i(x, y);  // Define a posição do texto na tela (canto superior esquerdo)
    
    // Itera sobre os caracteres da string e desenha cada um
    for (const char *c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);  // Desenha o caractere atual
    }
}

void DesenhaPontuacao(){
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();    
    glLoadIdentity();
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    gluOrtho2D(0, width, height, 0); 

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    char scoreText[50];
    sprintf(scoreText, "Pontos: %d", pontuacao);
    exibirTexto(10, 10, scoreText); 

    glMatrixMode(GL_PROJECTION);
    glPopMatrix(); 
    glMatrixMode(GL_MODELVIEW);

}

void DesenhaEAplicaTexturaProtagonista(){
    glPushMatrix();
        glTranslatef ( PosicaoDoObjeto.x, PosicaoDoObjeto.y, PosicaoDoObjeto.z );
        glRotatef(anguloPrincipal,0,1,0);
        glBindTexture (GL_TEXTURE_2D, TEX);//glColor3f(0.8f,0.8f, 0.0f); // AMARELO
        DesenhaParalelepipedoComTextura();//glutSolidCube(2);
        glRotatef(270.0f, 0, 1, 0);
        DesenhaCanhao();    
        float comprimentoCanhao = 0.0f;

        // Converte o ângulo para radianos
        float radAngulo = anguloPrincipal * M_PI / 180.0f;
        float radAnguloCanhao = (anguloCanhao ) * M_PI / 180.0f;
        float radAnguloCanhaod = (anguloCanhaod ) * M_PI / 180.0f;

        float direcaoX = cos(radAngulo) * comprimentoCanhao; // Direção no eixo X
        float direcaoY = sin(radAnguloCanhao);                             // Direção no eixo Y 
        float direcaoZ = sin(radAngulo) * comprimentoCanhao; // Direção no eixo Z

        float direcaoXd = cos(radAngulo) * comprimentoCanhao; // Direção no eixo X
        float direcaoYd = sin(radAnguloCanhaod);                             // Direção no eixo Y 
        float direcaoZd = sin(radAngulo) * comprimentoCanhao; // Direção no eixo Z

        float deslocamentoLateral = 2.0f;

        pontaX = PosicaoDoObjeto.x + direcaoX ;
        pontaY = PosicaoDoObjeto.y + direcaoY + 1;
        pontaZ = PosicaoDoObjeto.z + direcaoZ - deslocamentoLateral;

        pontaXd = PosicaoDoObjeto.x + direcaoXd;
        pontaYd = PosicaoDoObjeto.y + direcaoYd + 1;
        pontaZd = PosicaoDoObjeto.z + direcaoZd + deslocamentoLateral; 

        Ponto P;
        P = InstanciaPonto(Ponto(0,0,0), InvCameraMatrix);
        PosicaoDoObjeto.imprime("Posicao do Objeto:", "\n");        
    glPopMatrix();
}
// **********************************************************************
//  void display( void )
// **********************************************************************
float PosicaoZ = -30;
void display( void )
{

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    DesenhaPontuacao();
	DefineLuz();
	PosicUser();

	glMatrixMode(GL_MODELVIEW);    
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(0, -1, 0);
    DesenhaChao();
    glPopMatrix();
    
    DesenhaEAplicaTexturaProtagonista();
    DesenhaParedao();
    DesenhaObjetos();
    glColor3f(0.8,0.8,0);    
    
    DesenhaProjetil();
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
    case 'c': 
        anguloCanhao -= 5.0f;
        if(anguloCanhao > 360)
            anguloCanhao = 0;
        DirecaoDoCanhao.rotacionaY(anguloCanhao);
        break; // Move para direita
    case 'v':
        anguloCanhaod -= 5.0f;
        if(anguloCanhaod > 360)
            anguloCanhaod = 0; 
        DirecaoDoCanhao.rotacionaY(anguloCanhao);
        break; // Move para direita
    case 'z':  // Aumenta a velocidade
        velocidadeProj += 0.05f;  // Aumenta a velocidade do projétil
        if (velocidadeProj > 5.0f) {  // Limita a velocidade máxima
            velocidadeProj = 0.05f;
        }
        std::cout << "Velocidade aumentada: " << velocidadeProj << std::endl;
        break;
        
    case 'x':  // Diminui a velocidade
        velocidadeProj -= 0.05f;  // Diminui a velocidade do projétil
        if (velocidadeProj < 0.05f) {  // Limita a velocidade mínima
            velocidadeProj = 0.05f;
        }
        std::cout << "Velocidade diminuída: " << velocidadeProj << std::endl;
        break; 
    case ' ': // Atirar canhão
        if (!proj.active) {
            proj.active = true;
            t = 0.0f; 
            calculaParamProj();
        }
        if (!proj2.active) {
            proj2.active = true;
            td = 0.0f; 
            calculaParamProjd();
        }
        break;     
    default:
            cout << key;
    break;
  }
}

void atualizaCamera()
{
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
    //glMatrixMode(GL_MODELVIEW);
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
        if(!verificarColisaoCarro(novoX, novoZ, altura, largura)){
            PosicaoDoObjeto.x = novoX;
            PosicaoDoObjeto.z = novoZ;
        }    
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

    if (!LoadOBJ("Ape.obj", model)) {
    return -1;
    }
    if (!LoadOBJ("eyeball.obj", amigo)) {
    return -1;
    }

    glutMainLoop ( );
	return 0;
}



