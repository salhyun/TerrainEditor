#pragma once

#include "D3Ddef.h"
#include "ASEData.h"
#include "SecretMath.h"
#include "GeoLib.h"

//watching your including header files in order to
//namespace sal

#define D3DFVF_BSPTREEVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

struct SBSPTreeVertex
{
	D3DXVECTOR3 vPos ;
	D3DXVECTOR3 vNormal ;
	D3DXVECTOR2 vTex ;
} ;

struct SBSPTreeIndex
{
	unsigned short anIndex[3] ;
} ;

enum JUDGE
{
	JUDGE_COINCLIDING=0,
	JUDGE_INFRONT,
	JUDGE_BEHIND,
	JUDGE_SPANNING
} ;
/*
struct SPlane
{
	float a, b, c, d ;

	SPlane() { a=b=c=d=0.0f;}
} ;
*/
struct SPolygon
{
	D3DXVECTOR3 avPos[3] ;
	D3DXVECTOR2 avTex[3] ;
	geo::SPlane sPlane ;

	SPolygon()
	{
		avPos[0].x = avPos[0].y = avPos[0].z = 0.0f ;
		avPos[1].x = avPos[1].y = avPos[1].z = 0.0f ;
		avPos[2].x = avPos[2].y = avPos[2].z = 0.0f ;
		avTex[0].x = avTex[0].y = 0.0f ;
		avTex[1].x = avTex[1].y = 0.0f ;
		avTex[2].x = avTex[2].y = 0.0f ;
	}
	SPolygon(D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, D3DXVECTOR3 &v3)
	{
		avPos[0] = v1 ;
		avPos[1] = v2 ;
		avPos[2] = v3 ;

		MakePlane() ;
	}
	void MakePlane()
	{
		D3DXVECTOR3 vtemp, v1=(avPos[1] - avPos[0]), v2=(avPos[2] - avPos[0]) ;
		D3DXVec3Cross(&vtemp, &v1, &v2) ;
		D3DXVec3Normalize(&vtemp, &vtemp) ;

		sPlane.a = vtemp.x ;
		sPlane.b = vtemp.y ;
		sPlane.c = vtemp.z ;
		sPlane.d = -(D3DXVec3Dot(&avPos[0], &vtemp)) ;
	}
	void SetPos(D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, D3DXVECTOR3 &v3)
	{
        avPos[0] = v1 ;
		avPos[1] = v2 ;
		avPos[2] = v3 ;
	}
	void SetTex(D3DXVECTOR2 &vTex1, D3DXVECTOR2 &vTex2, D3DXVECTOR2 &vTex3)
	{
		avTex[0] = vTex1 ;
		avTex[1] = vTex2 ;
		avTex[2] = vTex3 ;
	}
} ;

struct SBSPNode
{
	SBSPNode *psFrontNode, *psBackNode ;
	int nNumPolygons ;
    SPolygon *psPolygons ;
	SPolygon sDivider ;

	SBSPNode()
	{
		psFrontNode = psBackNode = NULL ;
        psPolygons = NULL ;
	}
	~SBSPNode()
	{
		SAFE_DELETE(psFrontNode) ;
        SAFE_DELETE(psBackNode) ;
        SAFE_DELETEARRAY(psPolygons) ;
	}
} ;

class CSecretBSP
{
private :
	SBSPNode *m_psBSPRootNode ;

    int m_nNumPolygon ;
	SPolygon *m_psPolygonSet ;

	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pVI ;

private :

	bool _IsConvexSet(SPolygon *psPolygons, int nNumPolygon) ;

    JUDGE _ClassifyPoint(SPolygon *psPolygon, D3DXVECTOR3 *pvPos) ;
	JUDGE _ClassifyPolygon(SPolygon *psPolygon1, SPolygon *psPolygon2) ;
	SPolygon *_ChooseDividingPolygon(SPolygon *psPolygonSet, int nNumPolygon, int &nNumPositive, int &nNumNegative) ;
	void _GenerateBSPTree(SBSPNode *psBSPNode, SPolygon *psPolygonSet, int nNumPolygon) ;
	bool _SplitPolygon(SPolygon *psDivider, SPolygon *psPolygon, SPolygon *psInFront, SPolygon *psBehind) ;

	bool _IntersectRaytoPlane(D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, geo::SPlane &plane, D3DXVECTOR3 &vOut, float &t) ;
	bool _IntersectPolygontoPlane(SPolygon &polygon, geo::SPlane &plane, D3DXVECTOR3 *pvIntersect, D3DXVECTOR2 *pvTex, int &nIntersectPos) ;

	void _BestLength(D3DXVECTOR3 **ppvPos, int nNumPos, int *pnOder) ;

public :
	CSecretBSP() ;
	~CSecretBSP() ;

	void test(CASEData *pcASEData) ;//여기에서 각종함수를 테스트를 해볼것임.

	void Release() ;

} ;
