# Makefile para Windows

PROG = BasicoOpenGL.exe
#FONTES = Linha.cpp Ponto.cpp Temporizador.cpp InterseccaoEntreTodasAsLinhas.cpp 
#FONTES = Ponto.cpp Poligono.cpp Temporizador.cpp ListaDeCoresRGB.cpp ProgramaBasicoOpenGL.cpp
#FONTES = ExemploDeManipulacaoDeImagens.cpp ImageClass.cpp Temporizador.cpp SOIL/SOIL.cpp SOIL/image_DXT.cpp SOIL/image_helper.cpp SOIL/stb_image_aug.cpp
FONTES = Ponto.cpp Poligono.cpp Temporizador.cpp ListaDeCoresRGB.cpp Instancia.cpp Tools.cpp ImageClass.cpp ProgramaBasico3D.cpp

FONTES_SOIL = SOIL/SOIL.cpp SOIL/image_DXT.cpp SOIL/image_helper.cpp SOIL/stb_image_aug.cpp


OBJETOS = $(FONTES:.cpp=.cpp)
OBJETOS_SOIL = $(FONTES_SOIL:.c=.o)

CFLAGS = -g -Iinclude\GL -Iinclude/SOIL -Wno-write-strings -Wno-narrowing # -Wall -g  # Todas as warnings, infos de debug
#LDFLAGS = -Llib\GL -lfreeglut -lopengl32 -lglu32 -lm
LDFLAGS = -Llib  -lfreeglut -lopengl32 -lglu32 -lm 
CC = g++

$(PROG): $(OBJETOS) $(OBJETOS_SOIL)
	g++ $(CFLAGS) $(OBJETOS) $(OBJETOS_SOIL) -o $@ $(LDFLAGS)

clean:
	rm $(OBJETOS) $(OBJETOS_SOIL) $(PROG)
