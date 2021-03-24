#pragma once

#include "SecretMesh.h"

class CSecretDummyMesh : public CSecretMesh
{
protected :

public :
	CSecretDummyMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretDummyMesh() ;

	HRESULT Initialize(SMesh *pMesh) ;
} ;