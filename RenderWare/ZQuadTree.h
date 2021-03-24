#pragma once

/**
 * @brief QuadTree의 베이스 클래스
 *
 */

#include <windows.h>
#include "GeoLib.h"

#define _USE_INDEX16

struct TERRAINMAP ;
struct TERRAINVERTEX ;
struct TERRAININDEX ;
class CSecretFrustum ;

struct Vector3 ;

class ZQuadTree
{
	/// 쿼드트리에 보관되는 4개의 코너값에 대한 상수값
	enum			CornerType { CORNER_TL, CORNER_TR, CORNER_BL, CORNER_BR };
	enum			NeighborType { NEIGHBORNODE_UP, NEIGHBORNODE_DOWN, NEIGHBORNODE_LEFT, NEIGHBORNODE_RIGHT };

	/// 쿼드트리와 프러스텀간의 관계
	enum			QuadLocation { FRUSTUM_OUT = 0,				/// 프러스텀에서 완전벗어남
								   FRUSTUM_PARTIALLY_IN = 1,	/// 프러스텀에 부분포함
								   FRUSTUM_COMPLETELY_IN = 2,	/// 프러스텀에 완전포함
								   FRUSTUM_UNKNOWN = -1 };		/// 모르겠음(^^;)

private:
	ZQuadTree*	m_pChild[4];		/// QuadTree의 4개의 자식노드

	ZQuadTree *m_pParent ;
	ZQuadTree *m_pNeighbor[4] ;

	int			m_nCenter;			/// QuadTree에 보관할 첫번째 값
	int			m_nCorner[4];		/// QuadTree에 보관할 두번째 값
									///    TopLeft(TL)      TopRight(TR)
									///              0------1
									///              |      |
									///              |      |
									///              2------3
									/// BottomLeft(BL)      BottomRight(BR)
	BOOL		m_bCulled;			/// 프러스텀에서 컬링된 노드인가?
	float		m_fRadius;			/// 노드를 감싸는 경계구(bounding sphere)의 반지름

	geo::STriangle *m_apTriangles[8] ;
	int m_nNumTriangle ;

private:
	/// 자식 노드를 추가한다.
	ZQuadTree*	_AddChild( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR );

	/// 4개의 코너값을 셋팅한다.
	BOOL		_SetCorners( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR );

	/// Quadtree를 4개의 하위 트리로 부분분할(subdivide)한다.
	BOOL		_SubDivide();		// Quadtree를 subdivide한다.

	/// 현재 노드가 출력이 가능한 노드인가?
	//BOOL		_IsVisible() { return ( m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL] <= 1 ); }

	/// 출력할 폴리곤의 인덱스를 생성한다.
	int			_GenTriIndex( int nTris, LPVOID pIndex, TERRAINVERTEX *pHeightMap, CSecretFrustum* pcSecretFrustum );

	/// 메모리에서 쿼드트리를 삭제한다.
	void		_Destroy();

	/// 현재노드가 프러스텀에 포함되는가?
	int			_IsInFrustum( TERRAINVERTEX* pHeightMap, CSecretFrustum* pcSecretFrustum );

	int _GetLODLevel(TERRAINVERTEX* pHeightMap, Vector3 &vCamera, float fLODRatio) ;
	bool _IsVisible(TERRAINVERTEX* pHeightMap, Vector3 &vCamera, float fLODRatio) ;

	ZQuadTree *_FindNode(TERRAINVERTEX *pHeightMap, int nTL, int nTR, int nBL, int nBR) ;
	int _GetNeighborNodeIndex(int nEdge, int nPitch, int nHeight, int *pnEdge) ;
	void _BuildNeighborNode(ZQuadTree *pRoot, TERRAINVERTEX *pHeightMap, int nPitch, int nHeight) ;

	/// QuadTree를 구축한다.
	BOOL		_BuildQuadTree( TERRAINVERTEX* pHeightMap );

	void _SetTriangle(geo::STriangle *psTriangle, TERRAINVERTEX *pVertex0, TERRAINVERTEX *pVertex1, TERRAINVERTEX *pVertex2) ;

	int _GenerateTriangles(TERRAINVERTEX *pVertices, TERRAININDEX *pIndices, geo::STriangle *pTriangles, Vector3 *pvCameraPos, int nTris) ;

	bool _IsIntersected(geo::SCube *psCube, TERRAINVERTEX *pVertices) ;
	int _GetIntersectedTriangle(geo::SCube *psCube, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles) ;
	int _GetIntersectedTriangle(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles) ;
	int _GetIntersectedTriangle(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles) ;

public:
				/// 최초 루트노드 생성자
				ZQuadTree( int cx, int cy );

				/// 하위 자식노드 생성자
				ZQuadTree( /*ZQuadTree* pParent*/ );

				/// 소멸자
				~ZQuadTree();

                bool Build(TERRAINVERTEX *pHeightMap) ;

	/// _IsInFrustum()함수의 결과에 따라 프러스텀 컬링 수행
	void		_FrustumCull( TERRAINVERTEX* pHeightMap, CSecretFrustum* pcSecretFrustum );

	///	삼각형의 인덱스를 만들고, 출력할 삼각형의 개수를 반환한다.
	int			GenerateIndex( LPVOID pIndex, TERRAINVERTEX *pHeightMap, CSecretFrustum* pcSecretFrustum );

	//쿼드트리 렌더링
	//버텍스데이타는 이미 비디오메모리에 올라가있는상태고
	//인덱스데이타만 매번 갱신해서 인덱스 Lock 걸어놓고 채우고 렌더링하는 방법이다.

	int GenerateTriangles(TERRAINMAP *pTerrainMap, Vector3 *pvCameraPos) ;//인덱스데이타도 갱신하고 cpu연산에서 사용될 삼각형데이타들도 갱신한다.
    
	void GetIntersectedTriangle(Vector3 *pvPos, TERRAINVERTEX *pVertices, Vector3 *pvIntersectedPos, geo::STriangle **ppsIntersectedTriangle, bool *pbIntersected) ;

	int GetIntersectedTriangle(geo::SCube *psCube, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles) ;//큐브형태의 경계박스안의 삼각형 골라내기
	int GetIntersectedTriangle(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles) ;
	int GetIntersectedTriangle(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles) ;

};
