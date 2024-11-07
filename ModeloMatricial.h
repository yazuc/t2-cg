//
//  ModeloMatricial.hpp
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 02/05/21.
//  Copyright © 2021 Márcio Sarroglia Pinho. All rights reserved.
//

#ifndef ModeloMatricial_hpp
#define ModeloMatricial_hpp

#include <iostream>
#include <iomanip>
using namespace std;

class RGB{
    public:
    int r,g,b;
    void imprime()
    {
        cout << "Cor: " << setw(4) << g << setw(4) << r << setw(4) << b;
    }
};


class ModeloMatricial{
    int Matriz[50][50];
public:
    void leModelo(const char *nome);   
    int getColor(int i, int j);
    
};

#endif /* ModeloMatricial_hpp */
