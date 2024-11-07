

#include <iostream>
#include <cmath>
#include <ctime>

using namespace std;
#include "Tools.h"

// **********************************************************************
//
// **********************************************************************
void ImprimeMatriz(GLfloat M[4][4])
{
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
            cout << M[i][j] << " ";
        cout << endl;
    }
}

// **********************************************************************
//
// **********************************************************************
void criaIdentidade(GLfloat M[4][4])
{
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++)
            if (i==j)
                M[i][j] = 1.0;
            else M[i][j] = 0.0;
    }
}

/****
 * FONTE : Wu, Kevin, Fast Matrix Inversion, Graphics Gems II, p. 342-350, code: p. 603-605,
 * affine_matrix4_inverse
 *
 * Computes the inverse of a 3D affine matrix; i.e. a matrix with a dimen-
 * sionality of 4 where the right column has the entries (0, 0, 0, 1).
 *
 * This procedure treats the 4 by 4 matrix as a block matrix and
 * calculates the inverse of one submatrix for a significant perform-
 * ance improvement over a general procedure that can invert any non-
 * singular matrix:
 *          --        --          --          --
 *          |          | -1       |    -1      |
 *          | A      0 |          |   A      0 |
 *    -1    |          |          |            |
 *   M   =  |          |     =    |     -1     |
 *          | C      1 |          | -C A     1 |
 *          |          |          |            |
 *          --        --          --          --
 *
 *  where     M is a 4 by 4 matrix,
 *            A is the 3 by 3 upper left submatrix of M,
 *            C is the 1 by 3 lower left submatrix of M.
 *
 * Input:
 *   a   - 3D affine matrix
 *
 * Output:
 *   b  - inverse of 3D affine matrix
 *
 ***/


void InverteMatriz(float a[4][4], float b[4][4])
{

	double    det_1;
    double    pos, neg, temp;

#define ACCUMULATE    \
    if (temp >= 0.0)  \
        pos += temp;  \
    else              \
        neg += temp;

#define PRECISION_LIMIT (1.0e-15)

    /*
     * Calculate the determinant of submatrix A and determine if the
     * the matrix is singular as limited by the double precision
     * floating-point data representation.
     */
    pos = neg = 0.0;
    temp =  a[0][0] * a[1][1] * a[2][2];
    ACCUMULATE
    temp =  a[0][1] * a[1][2] * a[2][0];
    ACCUMULATE
    temp =  a[0][2] * a[1][0] * a[2][1];
    ACCUMULATE
    temp = -a[0][2] * a[1][1] * a[2][0];
    ACCUMULATE
    temp = -a[0][1] * a[1][0] * a[2][2];
    ACCUMULATE
    temp = -a[0][0] * a[1][2] * a[2][1];
    ACCUMULATE
    det_1 = pos + neg;

    /* Is the submatrix A singular? */
    if ((det_1 == 0.0) || (fabs(det_1 / (pos - neg)) < PRECISION_LIMIT)) {

        /* Matrix M has no inverse */
        fprintf (stderr, "affine_matrix4_inverse: singular matrix\n");
    }

    else {

        /* Calculate inverse(A) = adj(A) / det(A) */
        det_1 = 1.0 / det_1;
        b[0][0] =   ( a[1][1] * a[2][2] -
                                 a[1][2] * a[2][1] )
                               * det_1;
        b[1][0] = - ( a[1][0] * a[2][2] -
                                 a[1][2] * a[2][0] )
                               * det_1;
        b[2][0] =   ( a[1][0] * a[2][1] -
                                 a[1][1] * a[2][0] )
                               * det_1;
        b[0][1] = - ( a[0][1] * a[2][2] -
                                 a[0][2] * a[2][1] )
                               * det_1;
        b[1][1] =   ( a[0][0] * a[2][2] -
                                 a[0][2] * a[2][0] )
                               * det_1;
        b[2][1] = - ( a[0][0] * a[2][1] -
                                 a[0][1] * a[2][0] )
                               * det_1;
        b[0][2] =   ( a[0][1] * a[1][2] -
                                 a[0][2] * a[1][1] )
                               * det_1;
        b[1][2] = - ( a[0][0] * a[1][2] -
                                 a[0][2] * a[1][0] )
                               * det_1;
        b[2][2] =   ( a[0][0] * a[1][1] -
                                 a[0][1] * a[1][0] )
                               * det_1;

        /* Calculate -C * inverse(A) */
        b[3][0] = - ( a[3][0] * b[0][0] +
                                 a[3][1] * b[1][0] +
                                 a[3][2] * b[2][0] );
        b[3][1] = - ( a[3][0] * b[0][1] +
                                 a[3][1] * b[1][1] +
                                 a[3][2] * b[2][1] );
        b[3][2] = - ( a[3][0] * b[0][2] +
                                 a[3][1] * b[1][2] +
                                 a[3][2] * b[2][2] );

        /* Fill in last column */
        b[0][3] = b[1][3] = b[2][3] = 0.0;
        b[3][3] = 1.0;

    }

}

// ************************************************
// void MultiplicaMatriz(SmVR_Matrix m1, SmVR_Matrix m2, SmVR_Matrix mr)
//		Function for matrix multiplcation.
//		This version uses the OpenGL matrix
//	functions to perform the miltiplication.
//		Tests has shown that this version spend
//	50% of the time spent by the "by hand version".
//    MR = M1 * M2
// ************************************************
void MultiplicaMatriz(float m1[4][4], float m2[4][4], float mr[4][4])
{
	GLint MatrizMode;

//	MatrizMode = GL_PROJECTION_MATRIX;
//	MatrizMode = GL_MODELVIEW_MATRIX;
//	MatrizMode = GL_MODELVIEW;
//	MatrizMode = GL_PROJECTION;

	glGetIntegerv(GL_MATRIX_MODE, &MatrizMode);

	// MR = M1 * M2
	glPushMatrix();
	{
		// set the OpenGL matrix with m1 matrix
		glLoadMatrixf(&m2[0][0]);
		// apply the rotation in the OpenGL matrix
		glMultMatrixf(&m1[0][0]);
		// set the object matrix with the new OpenGL matrix
		switch (MatrizMode)
		{
		case GL_MODELVIEW:
			glGetFloatv(GL_MODELVIEW_MATRIX, &mr[0][0]);
			break;
		case GL_PROJECTION:
			glGetFloatv(GL_PROJECTION_MATRIX, &mr[0][0]);
			break;
		case GL_TEXTURE:
			glGetFloatv(GL_TEXTURE_MATRIX, &mr[0][0]);
			break;
		default : printf("ERROR : Matrix multiplication method - OpenGL library Not Loaded !");
		}
	}
	glPopMatrix();


}

// ************************************************
// Calcula a distancia entre dois pontos
// ************************************************
float calculaDistancia(float P1[3], float P2[3])
{
    float soma =0;
    for (int i=0;i<3;i++)
    {
        soma = soma + (P1[i]-P2[i]) * (P1[i]-P2[i]);
    }
    return (float) sqrt(soma);
}

bool gluInvertMatrix(const double m[16], double invOut[16])
{
    double inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] -
             m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] -
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] +
              m[4]  * m[11] * m[14] +
              m[8]  * m[6]  * m[15] -
              m[8]  * m[7]  * m[14] -
              m[12] * m[6]  * m[11] +
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] -
             m[4]  * m[11] * m[13] -
             m[8]  * m[5] * m[15] +
             m[8]  * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] +
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] -
               m[8]  * m[6] * m[13] -
               m[12] * m[5] * m[10] +
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] +
              m[1]  * m[11] * m[14] +
              m[9]  * m[2] * m[15] -
              m[9]  * m[3] * m[14] -
              m[13] * m[2] * m[11] +
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] -
             m[0]  * m[11] * m[14] -
             m[8]  * m[2] * m[15] +
             m[8]  * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] +
              m[0]  * m[11] * m[13] +
              m[8]  * m[1] * m[15] -
              m[8]  * m[3] * m[13] -
              m[12] * m[1] * m[11] +
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] -
              m[0]  * m[10] * m[13] -
              m[8]  * m[1] * m[14] +
              m[8]  * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] -
             m[1]  * m[7] * m[14] -
             m[5]  * m[2] * m[15] +
             m[5]  * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] +
              m[0]  * m[7] * m[14] +
              m[4]  * m[2] * m[15] -
              m[4]  * m[3] * m[14] -
              m[12] * m[2] * m[7] +
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] -
              m[0]  * m[7] * m[13] -
              m[4]  * m[1] * m[15] +
              m[4]  * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] +
               m[0]  * m[6] * m[13] +
               m[4]  * m[1] * m[14] -
               m[4]  * m[2] * m[13] -
               m[12] * m[1] * m[6] +
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
              m[1] * m[7] * m[10] +
              m[5] * m[2] * m[11] -
              m[5] * m[3] * m[10] -
              m[9] * m[2] * m[7] +
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
               m[0] * m[7] * m[9] +
               m[4] * m[1] * m[11] -
               m[4] * m[3] * m[9] -
               m[8] * m[1] * m[7] +
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}
