#pragma once

#include "SecretMesh.h"

#define MAXNUM_WEIGHT 4

#define D3DFVF_SKINNEDMESHVERTEX_VS (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX3|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE4(1)|D3DFVF_TEXCOORDSIZE4(2))

struct SSkinnedMeshVS_Vertex
{
	D3DXVECTOR3		pos;
	D3DXVECTOR3		normal;
	D3DXVECTOR2		t;
	D3DXVECTOR4 blend ;
	D3DXVECTOR4 indices ;
	

	SSkinnedMeshVS_Vertex()
	{
		pos.x = pos.y = pos.z = 0.0f ;
		normal.x = normal.y = normal.z = 0.0f ;
		t.x = t.y = 0.0f ;
		blend.x = blend.y = blend.z = blend.w = 0.0f ;
		indices.x = indices.y = indices.z = indices.w = 0.0f ;
	} ;
} ;


struct SSkinnedMeshVS_Index
{
	unsigned short anIndex[3] ;
} ;

//Shader를 사용하는 스키닝에니메이션
class CSecretSkinnedMeshVS : public CSecretMesh
{
protected :
	SSkinnedMeshVS_Vertex *m_psVertices ;
	SSkinnedMeshVS_Index *m_psIndices ;

public :
    CSecretSkinnedMeshVS(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretSkinnedMeshVS() ;

	HRESULT Initialize(SMesh *pMesh) ;

	virtual void Render() ;
} ;