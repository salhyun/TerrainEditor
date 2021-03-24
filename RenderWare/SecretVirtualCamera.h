#pragma once

#include "def.h"
#include "D3Ddef.h"

class CSecretVCP
{
private :
	LPDIRECT3DDEVICE m_pd3dDevice ;
	D3DXMATRIX m_matView, m_matProj ;

public :
	CSecretVCP() ;
	~CSecretVCP() ;


} ;