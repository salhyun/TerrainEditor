#pragma once

#include "def.h"
#include "D3Ddef.h"

struct Matrix4 ;

class CSecretCoordSystem
{
public :

private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	D3DXMATRIX m_matWorld ;

public :
	CSecretCoordSystem() ;
	CSecretCoordSystem(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX &mat) ;
	~CSecretCoordSystem() ;

	void Initialize(LPDIRECT3DDEVICE9 pd3dDevice) ;
	void SetMatrix(D3DXMATRIX &mat) ;
	void SetMatrix(Matrix4 &mat) ;
	D3DXMATRIX GetMatrix() {return m_matWorld;};
	void Render() ;
} ;