#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#include "Matrix.h"
#include "Vector3.h"
#include "Vector4.h"

Matrix4 MatrixConvert(D3DXMATRIX &dxmat) ;
D3DXMATRIX MatrixConvert(Matrix4 &mat) ;
Vector3 VectorConvert(D3DXVECTOR3 &v) ;
D3DXVECTOR3 VectorConvert(Vector3 &v) ;

Matrix4 MatrixConvert(D3DXMATRIX *pdxmat) ;
D3DXMATRIX MatrixConvert(Matrix4 *pmat) ;
Vector3 VectorConvert(D3DXVECTOR3 *pv) ;
D3DXVECTOR3 VectorConvert(Vector3 *pv) ;

void SetMatrixTranslation(D3DXMATRIX &dxmat, Vector3 &p) ;

void TRACE_Matrix(Matrix4 &mat) ;
void TRACE_Matrix(D3DXMATRIX &mat) ;
void TRACE_Vector(Vector3 &v) ;
void TRACE_Vector(D3DXVECTOR3 &v) ;