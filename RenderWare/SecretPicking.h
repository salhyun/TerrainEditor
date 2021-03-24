#pragma once

#include "D3Ddef.h"
#include "GeoLib.h"

class CD3DEnvironment ;
class CSecretTerrain ;
class CSecretQuadTree ;
class CSecretTileSubTree ;
struct TERRAINVERTEX ;
struct TERRAININDEX ;

struct SPickingVertex
{
	Vector3 pos ;
	Vector3 normal ;
	Vector2 tex ;
} ;

struct STerrainTileDataMustbeTested
{
	Vector3 vCenter ;
	float fDistfromCamera ;

    TERRAININDEX *psIndices ;
	int nNumIndex ;

    int nNumLevel, nPatchType, tl ;
	float fBestHeight ;
	CSecretQuadTree *pcQuadTree ;
	CSecretTileSubTree *pcTileSubTree ;
} ;

class CSecretPicking
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	CD3DEnvironment *m_pEnvironment ;
	CSecretTerrain *m_pcTerrain ;
	geo::STriangle *m_psIntersectedTriangles, *m_psTestedTriangle ;

	SPickingVertex m_sCurVertex, m_sPrevVertex ;

	STerrainTileDataMustbeTested m_asNearestTiles[256], m_asTilesMustbeTested[256] ;

public :
	bool m_bIntersect ;
	geo::SLine m_sLine ;
	int m_nNumTiles, m_nNumTestedTriangle ;
	const int MAXNUM_INTERSECTEDTRIANGLE ;
	Vector2 m_vScreenPos ;

public :
	CSecretPicking() ;
	~CSecretPicking() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CD3DEnvironment *pEnvironment) ;

	void SetLine(Vector2 *pvPos, Matrix4 &matV, Matrix4 &matP) ;
	//bool IsPickingPoint(geo::STriangle &psTriangle) ;
	bool IntersectedQuadTreeTile(geo::SAACube *psCube, TERRAINVERTEX *pVertices, TERRAININDEX *pIndices, int nTriangles) ;//사용하고있지않음
	bool IntersectedQuadTreeTile(geo::SAACube *psCube, int nNumLevel, int nPatchType, int tl, float fBestHeight, CSecretTileSubTree *pcSubTree) ;//사용하고있지않음
	void Render() ;
	void Release() ;

	void ResetTilesMustbeTested() ;
	void InsertTilesMustbeTested(TERRAININDEX *psIndices, int nNum, Vector3 *pvCenter) ;//지형 LOD Level 1일때 사용
	void InsertTilesMustbeTested(CSecretQuadTree *pcQuadTree, CSecretTileSubTree *pcTileSubTree, int nNumLevel, int nPatchType, int tl, float fBestHeight, Vector3 *pvCenter) ;//지형 LOD Level 1이상일때 사용
	bool IntersectTiles(TERRAINVERTEX *pVertices) ;

	SPickingVertex *GetCurVertex() {return &m_sCurVertex;}
	SPickingVertex *GetPrevVertex() {return &m_sPrevVertex;}
	void SetTerrain(CSecretTerrain *pcTerrain) {m_pcTerrain=pcTerrain;}
} ;

class CSecretTerrain ;

class CSecretBrushTool
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	CSecretPicking *m_pcPicking ;
	SPickingVertex m_vtCurPicking, m_vtPrevPicking ;

	Vector3 *m_pvBoundaryPos1 ;
	D3DXVECTOR3 *m_pvBoundaryPos2 ;
	Matrix4 *m_pmatView ;
	D3DXMATRIX *m_pmatProj ;
	LPD3DXLINE m_pLine ;
	float m_fRadius ;
	int m_nNumBoundary ;
	Vector3 m_vCenter ;

public :
	CSecretTerrain *m_pcTerrain ;

public :
	CSecretBrushTool() ;
	~CSecretBrushTool() ;

	void Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretPicking *pcPicking) ;
	void Process(Matrix4 *pmatView, D3DXMATRIX *pmatProj) ;
	void Render() ;
	void Release() ;

	void SetRadius(float fRadius) {m_fRadius=fRadius;}
	float GetRadius() {return m_fRadius;}
	Vector3 GetCenterPos() {return m_vCenter;}
	void SetTerrain(CSecretTerrain *pcTerrain) {m_pcTerrain=pcTerrain;}
} ;