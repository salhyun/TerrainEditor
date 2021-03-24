//#define D3D_OVERLOADS

#include "D3DMathLib.h"

void D3DXMatrixDecomposeTranslation(D3DXMATRIX *pmatRet, D3DXMATRIX *pmat)
{
	D3DXMatrixIdentity(pmatRet) ;
	pmatRet->_41 = pmat->_41 ;
	pmatRet->_42 = pmat->_42 ;
	pmatRet->_43 = pmat->_43 ;
}
void D3DXMatrixDecomposeRotation(D3DXMATRIX *pmatRet, D3DXMATRIX *pmat)
{
	D3DXMatrixIdentity(pmatRet) ;
	float fScaleX, fScaleY, fScaleZ ;
	D3DXVECTOR3 vAxisX(pmat->_11, pmat->_21, pmat->_31), vAxisY(pmat->_12, pmat->_22, pmat->_32), vAxisZ(pmat->_13, pmat->_23, pmat->_33) ;
	fScaleX = 1.0f/D3DXVec3Length(&vAxisX) ;
	fScaleY = 1.0f/D3DXVec3Length(&vAxisY) ;
	fScaleZ = 1.0f/D3DXVec3Length(&vAxisZ) ;

	(*pmatRet) = (*pmat) ;
	pmatRet->_41 = pmatRet->_42 = pmatRet->_43 = 0.0f ;

	pmatRet->_11 *= fScaleX ;
	pmatRet->_21 *= fScaleX ;
	pmatRet->_31 *= fScaleX ;

	pmatRet->_12 *= fScaleY ;
	pmatRet->_22 *= fScaleY ;
	pmatRet->_32 *= fScaleY ;

	pmatRet->_13 *= fScaleZ ;
	pmatRet->_23 *= fScaleZ ;
	pmatRet->_33 *= fScaleZ ;	
}
void D3DXMatrixDecomposeScaling(D3DXMATRIX *pmatRet, D3DXMATRIX *pmat)
{
	D3DXMatrixIdentity(pmatRet) ;
	D3DXVECTOR3 vAxisX(pmat->_11, pmat->_21, pmat->_31), vAxisY(pmat->_12, pmat->_22, pmat->_32), vAxisZ(pmat->_13, pmat->_23, pmat->_33) ;
	pmatRet->_11 = D3DXVec3Length(&vAxisX) ;
	pmatRet->_22 = D3DXVec3Length(&vAxisY) ;
	pmatRet->_33 = D3DXVec3Length(&vAxisZ) ;
}