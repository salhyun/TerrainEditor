#pragma once

#include "SecretRigidMesh.h"

//#define D3DFVF_RIGIDMESHAMBOVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEXTUREFORMAT3|D3DFVF_TEX2)
#define D3DFVF_RIGIDMESHAMBOVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE1(1)|D3DFVF_TEX2)
struct SRigidMeshAmbO_Vertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR3 normal ;
	D3DXVECTOR2 t ;
	float ambient ;
	//D3DXVECTOR3 ambient ;
} ;

class CSecretRigidMeshAmbO : public CSecretRigidMesh
{
private :
	SRigidMeshAmbO_Vertex *m_psAmbOVertices ;

public :
	CSecretRigidMeshAmbO(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretRigidMeshAmbO() ;

	HRESULT Initialize(SMesh *pMesh) ;
	void PutAmbientDegree() ;

	virtual void Render() ;
} ;