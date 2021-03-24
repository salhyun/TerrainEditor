#pragma once

#include <windows.h>
#include "GeoLib.h"
#include "DataStructLib.h"
#include <vector>
#include <list>

struct TERRAINMAP ;
struct TERRAINVERTEX ;
struct TERRAININDEX ;
class CSecretFrustum ;

#define QUADTREE_MINSIZE 32
#define QUADTREE_LODRATIO 0.025f

class CSecretTile ;
struct STrueRenderingObject ;

extern float g_fQuadTreeLODRatio ;

class CSecretQuadTree
{
public :
	enum CORNER {CORNER_TL=0, CORNER_TR, CORNER_BL, CORNER_BR} ;
	enum NEIGHBOR {NEIGHBOR_UP=0, NEIGHBOR_DOWN, NEIGHBOR_LEFT, NEIGHBOR_RIGHT} ;
	enum FRUSTUMCULL {FRUSTUM_OUT=0, FRUSTUM_PARTIALLY_IN, FRUSTUM_COMPLETELY_IN, FRUSTUM_UNKNOWN=-1} ;

private :
	CSecretQuadTree *m_pcChild[4], *m_pcParent ;
	CSecretQuadTree *m_pcNeighbor[4] ;

	int m_nCorner[4] ;
	int m_nCenter ;
	float m_fRadius, m_fBestHeight ;

	Vector3 m_vCenter ;
	int m_nDetailLevel ;
	int m_nTreeLevel ;
	bool m_bCulled , m_bIntersectLine, m_bIntersectPos, m_bLastDepthLevel ;

public :
	geo::SAACube m_sCube ;
	std::list<STrueRenderingObject *> m_sTRObjects ;

private :
	bool _Build(TERRAINVERTEX *pVertices, float fMaxHeight, float fMinHeight) ;
	CSecretQuadTree *_AddChild(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR) ;
	bool _SetCorners( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR ) ;
	bool _SubDivide(TERRAINVERTEX *pVertices) ;
	int _IsInFrustum(TERRAINVERTEX *pVertices, CSecretFrustum *pcFrustum) ;
	int _GetLODLevel(Vector3 *pvCamera, TERRAINVERTEX *pVertices) ;
	bool _IsVisible(int nLevel, Vector3 *pvCamera, TERRAINVERTEX *pVertices) ;
	//int _GetTriangles(Vector3 *pvCamera, TERRAINVERTEX *pVertices, TERRAININDEX *pIndices) ;

	int _GetNeighborNodeIndex(int nEdge, int nPitch, int nHeight, int *pnEdge) ;
	CSecretQuadTree *_FindNode(TERRAINVERTEX *pVertices, int nTL, int nTR, int nBL, int nBR) ;
	void _BuildNeighborNode(CSecretQuadTree *pRoot, TERRAINVERTEX *pVertices, int nPitch, int nHeight) ;
	void _SetCube(TERRAINVERTEX *pVertices, float height, float fMidHeight) ;
	//void _PlaceObjects(int nNum, STrueRenderingObject *psTRObjects) ;// use SubTrees
	void _PlaceObjects() ;// use SubTrees

	void _Destroy() ;
    
public :
	CSecretQuadTree(CSecretQuadTree *pcParent=NULL) ;
	CSecretQuadTree(int cx, int cy) ;
	~CSecretQuadTree() ;

	bool Build(TERRAINVERTEX *pVertices, float fMaxHeight, float fMinHeight, int nTileSize) ;
	//int FrustumCull(TERRAINVERTEX *pVertices, CSecretFrustum *pcSecretFrustum, Vector3 *pvCamera, TERRAININDEX *pIndices, int nNumIndex, std::vector<STrueRenderingObject *> *psTRObjects=NULL) ;
	int FrustumCull(TERRAINVERTEX *pVertices, CSecretFrustum *pcFrustum, Vector3 *pvCamera, TERRAININDEX *pIndices, int nNumIndex, data::SKeepingArray<TERRAININDEX> *psExceptionalIndices) ;
	int GetTreeCubes(std::vector<geo::SAACube *> *pCubeSet, int nNumCube) ;
	void DeleteTiles() ;
	std::list<STrueRenderingObject *> *GetTRObjects() ;

	//int GetTriangles(Vector3 *pvCamera, TERRAINVERTEX *pVertices, TERRAININDEX *pIndices) ;

	void PlaceObjects(int nNum, STrueRenderingObject *psTRObjects) ;// use RootTree

	CSecretQuadTree *GetNeighbor(int nNeighbor) ;
} ;


struct STileBit
{
	int tl, tr, bl, br, c ;

	STileBit() {tl=tr=bl=br=0;};
	void set(int _tl, int _tr, int _bl, int _br)
	{
		tl=_tl; tr=_tr; bl=_bl; br=_br; c = (tl+tr+bl+br)/4;
	} ;    
} ;

struct SDivideTile
{
	Vector3 avEdge[4], vCenter ;
	int anTriangleCount[16] ;
	geo::STriangle *aapsTriangles[16][512] ;

	void set(int tl, int tr, int bl, int br, TERRAINVERTEX *psVertices) ;
} ;


#define MAXNUM_TILEPOS 9

class CSecretTile
{
public :
	enum TILE_NEIGHBOR {NEIGHBOR_UP=0x01, NEIGHBOR_DOWN=0x02, NEIGHBOR_LEFT=0x04, NEIGHBOR_RIGHT=0x08} ;
	enum TILE_POS {TILEPOS_LEFTTOP=0, TILEPOS_RIGHTTOP, TILEPOS_LEFTBOTTOM, TILEPOS_RIGHTBOTTOM, TILEPOS_TOP, TILEPOS_BOTTOM, TILEPOS_LEFT, TILEPOS_RIGHT, TILEPOS_INSIDE} ;

private :
	int m_nLevel ;
    STileBit *m_psTileBit ;//타일의 위치에 따라 분류를 한것. TILE_POS

public :
	TERRAININDEX *m_apsIndices[16] ;
	int m_anNumIndex[16] ;
	DWORD m_dwClassSize ;
	int m_nMaxNumBitInLine, m_nMaxNumLine ;

    void _Make2Triangles(int tl, int tr, int bl, int br, TERRAININDEX *pIndices) ;//현재 타일에서 온전한 2개의 삼각형을 만들어낸다.
	void _MakePatch(int nNeighbor, int tl, int tr, int bl, int br, TERRAININDEX *pIndices) ;//패치를 만들어낸다.
	void _MakePatch(int nNeighbor, int tl, int tr, int bl, int br, std::vector<TERRAININDEX> *pIndexSet) ;//패치를 만들어낸다.
	void _ClassifyBitTile(int nBitCount, int nLineCount, std::vector<STileBit *> *pTileBitSets, STileBit *psTileBit) ;
	void _GenerateIndices(int nNeighbor, std::vector<STileBit *> *pTileBitSets, std::vector<TERRAININDEX> *pIndexSet) ;

public :
	CSecretTile() ;
	~CSecretTile() ;

	void CreateTriangles(int tl, int tr, int bl, int br, TERRAINVERTEX *pVertices) ;

	int GetPatchTypeFromNeighbor(int nNeighbor) ;
	int GetIndices(int nNeighbor, TERRAININDEX *pIndices, int tl, data::SKeepingArray<TERRAININDEX> *psExceptionalIndices) ;
	int GetLevel() {return m_nLevel;};
	void DivideTile(int tl, int tr, int bl, int br, int nLevel) ;
    void GenerateIndexBuffer(int tl, int tr, int bl, int br, TERRAININDEX *pIndices, int nLevel) ;//현재 타일의 레벨에서 더 세분화된 레벨로 분할하고 인덱스버퍼를 생성시킨다.
	void GernerateZeroLevelIndices(int tl, int tr, int bl, int br) ;//가장 상위레벨
	void GernerateOneLevelIndices(int tl, int tr, int bl, int br) ;//두번째 레벨
	void Release() ;
} ;


#define TILEDIVISION_MINSIZE 2
//#define TILEDIVISION_MAXNUMINDEX (TILEDIVISION_MINSIZE*TILEDIVISION_MINSIZE*2*4)+10
#define TILEDIVISION_MAXNUMINDEX (4*4*2*4)

//타일을 축정렬 큐브로 경계를 만든다.
class CSecretTileSubTree
{
public :
	enum CORNER {CORNER_TL=0, CORNER_TR, CORNER_BL, CORNER_BR} ;
private :
	CSecretTileSubTree *m_pcChild[4], *m_pcParent ;
	int m_nCorner[4], m_nCenter ;
	float m_fRadius ;//2d 평면적인 입장에서 반지름

	TERRAININDEX ****m_ppppsIndices ;
	int **m_ppnNumIndex ;
	int m_nNumLevel ;

	void _SetCorners( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR ) ;
	CSecretTileSubTree *_AddChild(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR) ;
	bool _Division(TERRAINVERTEX *pVertices, float fBestHeight, CSecretTile *pcTile, int nNumLevel) ;
	bool _Build(TERRAINVERTEX *pVertices, float fBestHeight, CSecretTile *pcTile, int nNumLevel) ;
	void _Release() ;
	int _GetIntersectPos(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle *psTriangles, int nTriangles, int nNumLevel, int nTilePatch, int tl) ;
	int _GetIntersectLine(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle *psTriangles, int nTriangles, int nNumLevel, int nTilePatch, int tl, float fBestHeight) ;
	void _SetCube(TERRAINVERTEX *pVertices, float height, int tl, geo::SAACube *psCube) ;

public :
	CSecretTileSubTree(CSecretTileSubTree *pcParent=NULL) ;
	CSecretTileSubTree(int tl, int tr, int bl, int br) ;
	~CSecretTileSubTree() ;

	//fBestHeight 현재 사용하고 있지 않음.
	bool Build(TERRAINVERTEX *pVertices, float fBestHeight, CSecretTile *pcTile, int nNumLevel) ;
	int GetIntersectPos(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle *psTriangles, int nTriangles, int nNumLevel, int nTilePatch, int tl) ;
	int GetIntersectLine(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle *psTriangles, int nTriangles, int nNumLevel, int nTilePatch, int tl, float fBestHeight) ;
} ;

extern bool bIntersectTri_TileSubTree ;
extern Vector3 vIntersectTriPos_TileSubTree ;