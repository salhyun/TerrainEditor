#pragma once

#include "D3DDef.h"

class CSecretEffect
{
public :

private :

public :
	D3DXMATRIX m_matWorld ;
	float m_fElapsedTime ;

public :
	CSecretEffect() ;
	virtual ~CSecretEffect() ;

	virtual void process(float time) ;
	virtual void render(float fElapsedTime, D3DXMATRIX *pmatWorld) ;
	virtual void release() ;
} ;
