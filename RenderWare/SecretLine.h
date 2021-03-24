#pragma once

#include "def.h"
#include "D3Ddef.h"
#include "Vector3.h"

class CSecretLine
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	LPD3DXLINE m_pLine ;

    Matrix4 *m_pmatView ;
	D3DXMATRIX *m_pmatProj ;
	Vector3 *m_pvPositions ;
	D3DXVECTOR3 *m_pvd3dPositions ;
	int m_nNumPosition ;

public :
	CSecretLine() ;
	~CSecretLine() ;
	void Release() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice) ;
	void SetPositions(Vector3 *pvPositions, int nNumPosition) ;
	void UpdateMatrices(Matrix4 *pmatView, D3DXMATRIX *pmatProj) ;
	void Render() ;
} ;