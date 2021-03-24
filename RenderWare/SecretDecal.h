#pragma once

#include "D3Ddef.h"
#include "GeoLib.h"

#define D3DFVF_DECALVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

#define MAXNUM_DECALVERTEX 512
#define MAXNUM_DECALINDEX 512

struct SDecalVertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR3 normal ;
	D3DXVECTOR2 tex ;
} ;

struct SDecalIndex
{
	WORD awIndex[3] ;
} ;

class CSecretDecal
{
public :
	enum BOUNDING_PLANE {LEFT_PLANE=0, RIGHT_PLANE, TOP_PLANE, BOTTOM_PLANE, NEAR_PLANE, FAR_PLANE} ;
	enum BOUNDING_VERTEX {VERTEX_ULT=0, VERTEX_URT, VERTEX_ULB, VERTEX_URB, VERTEX_LLT, VERTEX_LRT, VERTEX_LLB, VERTEX_LRB} ;
	enum KIND {SQUARE=0, POLYGON} ;

private :
	int m_nKind ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	Vector3 m_vPos ;//가운데
	Vector3 m_vNormal, m_vTangent, m_vBinormal ;
	float m_fWidth, m_fHeight, m_fDepth ;
	float m_fRadius ;//탄젠트, 바이노말 기저로 한 평면의 평면적인 반지름(쿼드트리에서 사용하기 위한것임.)
	float m_fNormalOffset ;
	geo::SPlane m_asPlane[6] ;//Bounding Planes
	geo::SLine m_asLine[4] ;//Bounding Lines
	Vector3 m_vBoundingVertex[8] ;

	SDecalVertex *m_pVertices ;
	SDecalIndex *m_pIndices ;
	int m_nMaxNumVertex, m_nMaxNumIndex ;

	LPDIRECT3DTEXTURE9 m_pTex ;

	Vector2 m_avTexFactor[4] ;

	int m_nVertexCount, m_nIndexCount, m_nNumTriangle ;

private :
	void _GetAppropriatedTriangleInBoundingBox(geo::STriangle *psTriangle) ;
	void _DivideTrianglebyBoundingBox(geo::STriangle *psTriangle) ;
	D3DXVECTOR2 _GetTexCoord(D3DXVECTOR3 *pvPos) ;
	bool _IsSameVectorInList(Vector3 *pvValidPos, int nCount, Vector3 &vPos) ;
	bool _PutPosInClockWise(Vector3 *pvValidPos, int &nCount, geo::STriangleWeight *pvWeights) ;

public :
	CSecretDecal() ;
	~CSecretDecal() ;

	void Initialize(LPDIRECT3DDEVICE9 pd3dDevice, Vector3 &vNormal, Vector3 &vTangent, float width, float height, float depth) ;
	void MakingBoundingPlanes(int nWidth=0, int nHeight=0) ;
	void MakingBoundingPlanes(Vector3 vCenter, Vector3 *pvPos, Vector3 vNormal, float depth) ;//4개의 점으로 바운딩평면을 만듬, pvPos는 시계방향순으로 들어와야 함.
	//Vector3 GetBoundingVertex() ;
	void AddInBoundTriangles(geo::STriangle **ppsTriangle, int nNumTriangle) ;
	void AddInBoundTriangles(geo::STriangle *psTriangle, int nNumTriangle) ;
	bool IsInBoundingBox(Vector3 &vPos) ;

    void SetPos(Vector3 &vPos) ;
	float SetSize(int nWidth, int nHeight) ;
	Vector3 *GetPos() {return &m_vPos;} ;
	float GetRadius() {return m_fRadius;} ;
	SDecalVertex *GetVertices() {return m_pVertices;}
	SDecalIndex *GetIndices() {return m_pIndices;}
	int GetVertexCount() {return m_nVertexCount;}
	int GetIndexCount() {return m_nIndexCount;}
	int GetTriangleCount() {return m_nNumTriangle;}
	LPDIRECT3DTEXTURE9 GetTexture() {return m_pTex;}
    void SetTexFactor(Vector2 *pvTexFactor) ;
	void SetNormalOffset(float fOffset) ;

	void ReAllocateVB(int nNumVertex) ;
	void ReAllocateIB(int nNumIndex) ;

	HRESULT Render() ;
	HRESULT Render(SD3DEffect *psd3dEffect) ;
	void Release() ;
} ;