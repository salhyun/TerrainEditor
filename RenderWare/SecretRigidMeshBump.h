#pragma once

#include "SecretRigidMesh.h"

#define D3DFVF_RIGIDMESHBUMPVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEXTUREFORMAT3|D3DFVF_TEX2)
struct SRigidMeshBump_Vertex
{
    D3DXVECTOR3 pos ;
	D3DXVECTOR3 normal ;
	D3DXVECTOR3 tangent ;
	D3DXVECTOR2 t ;
} ;

//Normalmap을 사용하는 메쉬
class CSecretRigidMeshBump : public CSecretRigidMesh
{
private :
	SRigidMeshBump_Vertex *m_psBumpVertices ;

public :
	CSecretRigidMeshBump(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretRigidMeshBump() ;

	HRESULT Initialize(SMesh *pMesh) ;
	virtual void Render() ;

	virtual HRESULT InitShadowData() ;
} ;