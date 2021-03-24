#include "SecretDummyMesh.h"

CSecretDummyMesh::CSecretDummyMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) : CSecretMesh(lEigenIndex, pd3dDevice)
{
	//TRACE("CSecretDummyMesh construction EigenIndex=%d\r\n", lEigenIndex) ;
}

CSecretDummyMesh::~CSecretDummyMesh()
{
	//TRACE("CSecretDummyMesh destruction\r\n") ;
}

HRESULT CSecretDummyMesh::Initialize(SMesh *pMesh)
{
	CSecretMesh::Initialize(pMesh) ;

	return S_OK ;
}