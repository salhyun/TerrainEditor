#pragma once

#include "SecretMesh.h"

#define D3DFVF_BILLBOARDMESHVERTEX (D3DFVF_XYZ|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1))

struct SBillboardMesh_Vertex
{
    D3DXVECTOR3		pos ;
    D3DXVECTOR2 t ;
	float rightFactor, upFactor ;
} ;

struct SBillboardMesh_Index
{
	unsigned short anIndex[3] ;

	SBillboardMesh_Index()
	{
		anIndex[0] = anIndex[1] = anIndex[2] = 0 ;
	}

	SBillboardMesh_Index(unsigned short *pnIndex)
	{
		anIndex[0] = pnIndex[0] ;
		anIndex[1] = pnIndex[1] ;
		anIndex[2] = pnIndex[2] ;
	}
} ;

class CSecretBillboardMesh : public CSecretMesh
{
public :
	enum BILLBOARDTYPE {AXIAL=1, VIEWORIENTED} ;

protected :
	SBillboardMesh_Vertex *m_psVertices ;
	SBillboardMesh_Index *m_psIndices ;

	int nBillboardType ;

public :
	Vector3 m_vBillboardOffset ;
	float m_fWidth, m_fHeight ;
	Vector2 m_vTexMin, m_vTexMax ; 	

public :
	CSecretBillboardMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretBillboardMesh() ;

	HRESULT Initialize(SMesh *pMesh) ;
	HRESULT Initialize_soo(SMesh *pMesh) ;

	void SetWireFrame(bool bEnable) {m_bWireFrame=bEnable;} ;

	virtual void Render() ;

	void SetBillboardDiffuse(char *pszObjectName) ;
} ;