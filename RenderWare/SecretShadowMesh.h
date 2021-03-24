#pragma once

#include "D3Ddef.h"

#define D3DFVF_SHADOWMESHVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)
struct SShadowMesh_Vertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR3 normal ;
} ;

struct SShadowMesh_Index
{
	unsigned short anIndex[3] ;

	SShadowMesh_Index()
	{
		anIndex[0] = anIndex[1] = anIndex[2] = 0 ;
	}

	SShadowMesh_Index(unsigned short *pnIndex)
	{
		anIndex[0] = pnIndex[0] ;
		anIndex[1] = pnIndex[1] ;
		anIndex[2] = pnIndex[2] ;
	}
} ;

struct SShadowMesh_IndexSubSet
{
	int nMaterialID ;
	int nNumTriangle ;
	SShadowMesh_Index *psIndices ;//Set in same Texture

	SShadowMesh_IndexSubSet()
	{
		nMaterialID = nNumTriangle = 0 ;
		psIndices = NULL ;
	}
	void set(int nID, int nNum)
	{
		nMaterialID = nID ;
		nNumTriangle = nNum ;
		psIndices = new SShadowMesh_Index[nNum] ;
	}

	~SShadowMesh_IndexSubSet()
	{
		nMaterialID = nNumTriangle = 0 ;
		SAFE_DELETEARRAY(psIndices) ;
	}
} ;

//그림자렌더링에 사용되는 메쉬
//CSecretMesh에서 맴버로 정의되고 라이트뷰에서 렌더링할때 이 클래스로 렌더링 된다.
class CSecretShadowMesh
{
public :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	UINT m_lNumVertex, m_lNumIndex ;
	SShadowMesh_Vertex *m_psVertices ;
	SShadowMesh_Index *m_psIndices ;

	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pVI ;

	//using SubMaterial
	int m_nNumSubset ;
    SShadowMesh_IndexSubSet *m_psIndexSubset ;
	LPDIRECT3DINDEXBUFFER9 *m_ppSubsetIB ;

public :
	CSecretShadowMesh(LPDIRECT3DDEVICE9 pd3dDevice) ;
	~CSecretShadowMesh() ;

	HRESULT SetVertexData(SShadowMesh_Vertex *psSrcVertices, UINT lNumVertex) ;
	HRESULT SetIndexData(SShadowMesh_Index *psSrcIndices, UINT lNumIndex) ;
	void SetSubsetIndexData(SShadowMesh_IndexSubSet *psIndexSubset, int nNumSubset) ;

	void Render() ;

} ;