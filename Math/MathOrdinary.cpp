#include "def.h"
#include "MathOrdinary.h"
#include "assert.h"

Matrix4 MatrixConvert(D3DXMATRIX &dxmat)
{
	Matrix4 mat ;

	mat.m11=dxmat._11; mat.m12=dxmat._12; mat.m13=dxmat._13; mat.m14=dxmat._14;
	mat.m21=dxmat._21; mat.m22=dxmat._22; mat.m23=dxmat._23; mat.m24=dxmat._24;
	mat.m31=dxmat._31; mat.m32=dxmat._32; mat.m33=dxmat._33; mat.m34=dxmat._34;
	mat.m41=dxmat._41; mat.m42=dxmat._42; mat.m43=dxmat._43; mat.m44=dxmat._44;

	return mat ;
}

D3DXMATRIX MatrixConvert(Matrix4 &mat)
{
	D3DXMATRIX dxmat ;

	dxmat._11=mat.m11; dxmat._12=mat.m12; dxmat._13=mat.m13; dxmat._14=mat.m14;
	dxmat._21=mat.m21; dxmat._22=mat.m22; dxmat._23=mat.m23; dxmat._24=mat.m24;
	dxmat._31=mat.m31; dxmat._32=mat.m32; dxmat._33=mat.m33; dxmat._34=mat.m34;
	dxmat._41=mat.m41; dxmat._42=mat.m42; dxmat._43=mat.m43; dxmat._44=mat.m44;

	return dxmat ;
}

Vector3 VectorConvert(D3DXVECTOR3 &v)
{
    return Vector3(v.x, v.y, v.z) ;
}

D3DXVECTOR3 VectorConvert(Vector3 &v)
{
	return D3DXVECTOR3(v.x, v.y, v.z) ;
}

Matrix4 MatrixConvert(D3DXMATRIX *pdxmat)
{
	Matrix4 mat ;

	mat.m11=pdxmat->_11; mat.m12=pdxmat->_12; mat.m13=pdxmat->_13; mat.m14=pdxmat->_14;
	mat.m21=pdxmat->_21; mat.m22=pdxmat->_22; mat.m23=pdxmat->_23; mat.m24=pdxmat->_24;
	mat.m31=pdxmat->_31; mat.m32=pdxmat->_32; mat.m33=pdxmat->_33; mat.m34=pdxmat->_34;
	mat.m41=pdxmat->_41; mat.m42=pdxmat->_42; mat.m43=pdxmat->_43; mat.m44=pdxmat->_44;

	return mat ;
}

D3DXMATRIX MatrixConvert(Matrix4 *pmat)
{
	D3DXMATRIX dxmat ;

	dxmat._11=pmat->m11; dxmat._12=pmat->m12; dxmat._13=pmat->m13; dxmat._14=pmat->m14;
	dxmat._21=pmat->m21; dxmat._22=pmat->m22; dxmat._23=pmat->m23; dxmat._24=pmat->m24;
	dxmat._31=pmat->m31; dxmat._32=pmat->m32; dxmat._33=pmat->m33; dxmat._34=pmat->m34;
	dxmat._41=pmat->m41; dxmat._42=pmat->m42; dxmat._43=pmat->m43; dxmat._44=pmat->m44;

	return dxmat ;
}

Vector3 VectorConvert(D3DXVECTOR3 *pv)
{
    return Vector3(pv->x, pv->y, pv->z) ;
}

D3DXVECTOR3 VectorConvert(Vector3 *pv)
{
	return D3DXVECTOR3(pv->x, pv->y, pv->z) ;
}

void SetMatrixTranslation(D3DXMATRIX &dxmat, Vector3 &p)
{
	dxmat._41 = p.x ;
	dxmat._42 = p.y ;
	dxmat._43 = p.z ;
}


void TRACE_Matrix(Matrix4 &mat)
{
	TRACE("\r\n") ;
	TRACE("|%+03.03f %+03.03f %+03.03f %+03.03f|\r\n", mat.m11, mat.m12, mat.m13, mat.m14) ;
	TRACE("|%+03.03f %+03.03f %+03.03f %+03.03f|\r\n", mat.m21, mat.m22, mat.m23, mat.m24) ;
	TRACE("|%+03.03f %+03.03f %+03.03f %+03.03f|\r\n", mat.m31, mat.m32, mat.m33, mat.m34) ;
	TRACE("|%+03.03f %+03.03f %+03.03f %+03.03f|\r\n", mat.m41, mat.m42, mat.m43, mat.m44) ;
	TRACE("\r\n") ;
}

void TRACE_Matrix(D3DXMATRIX &mat)
{
	TRACE("\r\n") ;
	TRACE("|%+03.03f %+03.03f %+03.03f %+03.03f|\r\n", mat._11, mat._12, mat._13, mat._14) ;
	TRACE("|%+03.03f %+03.03f %+03.03f %+03.03f|\r\n", mat._21, mat._22, mat._23, mat._24) ;
	TRACE("|%+03.03f %+03.03f %+03.03f %+03.03f|\r\n", mat._31, mat._32, mat._33, mat._34) ;
	TRACE("|%+03.03f %+03.03f %+03.03f %+03.03f|\r\n", mat._41, mat._42, mat._43, mat._44) ;
	TRACE("\r\n") ;
}

void TRACE_Vector(Vector3 &v)
{
	TRACE("[%+03.03f %+03.03f %+03.03f]\r\n", v.x, v.y, v.z) ;
}

void TRACE_Vector(D3DXVECTOR3 &v)
{
	TRACE("[%+03.03f %+03.03f %+03.03f]\r\n", v.x, v.y, v.z) ;
}