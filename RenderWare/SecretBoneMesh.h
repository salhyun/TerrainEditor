#pragma once

#include "SecretMesh.h"

#define D3DFVF_BONEMESHVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)

struct SBoneMesh_Vertex
{
    D3DXVECTOR3		pos ;
	D3DXVECTOR3     normal ;
} ;

struct SBoneMesh_Index
{
	unsigned short anIndex[3] ;
} ;

class CSecretBoneMesh : public CSecretMesh
{
protected :
	SBoneMesh_Vertex *m_psVertices ;
	SBoneMesh_Index *m_psIndices ;

public :
	CSecretBoneMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretBoneMesh() ;

	HRESULT Initialize(SMesh *pMesh) ;

	void SetWireFrame(bool bEnable) {m_bWireFrame=bEnable;} ;

	virtual void Render() ;

} ;