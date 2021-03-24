#pragma once

/**
 * @brief QuadTree�� ���̽� Ŭ����
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
	/// ����Ʈ���� �����Ǵ� 4���� �ڳʰ��� ���� �����
	enum			CornerType { CORNER_TL, CORNER_TR, CORNER_BL, CORNER_BR };
	enum			NeighborType { NEIGHBORNODE_UP, NEIGHBORNODE_DOWN, NEIGHBORNODE_LEFT, NEIGHBORNODE_RIGHT };

	/// ����Ʈ���� �������Ұ��� ����
	enum			QuadLocation { FRUSTUM_OUT = 0,				/// �������ҿ��� �������
								   FRUSTUM_PARTIALLY_IN = 1,	/// �������ҿ� �κ�����
								   FRUSTUM_COMPLETELY_IN = 2,	/// �������ҿ� ��������
								   FRUSTUM_UNKNOWN = -1 };		/// �𸣰���(^^;)

private:
	ZQuadTree*	m_pChild[4];		/// QuadTree�� 4���� �ڽĳ��

	ZQuadTree *m_pParent ;
	ZQuadTree *m_pNeighbor[4] ;

	int			m_nCenter;			/// QuadTree�� ������ ù��° ��
	int			m_nCorner[4];		/// QuadTree�� ������ �ι�° ��
									///    TopLeft(TL)      TopRight(TR)
									///              0------1
									///              |      |
									///              |      |
									///              2------3
									/// BottomLeft(BL)      BottomRight(BR)
	BOOL		m_bCulled;			/// �������ҿ��� �ø��� ����ΰ�?
	float		m_fRadius;			/// ��带 ���δ� ��豸(bounding sphere)�� ������

	geo::STriangle *m_apTriangles[8] ;
	int m_nNumTriangle ;

private:
	/// �ڽ� ��带 �߰��Ѵ�.
	ZQuadTree*	_AddChild( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR );

	/// 4���� �ڳʰ��� �����Ѵ�.
	BOOL		_SetCorners( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR );

	/// Quadtree�� 4���� ���� Ʈ���� �κк���(subdivide)�Ѵ�.
	BOOL		_SubDivide();		// Quadtree�� subdivide�Ѵ�.

	/// ���� ��尡 ����� ������ ����ΰ�?
	//BOOL		_IsVisible() { return ( m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL] <= 1 ); }

	/// ����� �������� �ε����� �����Ѵ�.
	int			_GenTriIndex( int nTris, LPVOID pIndex, TERRAINVERTEX *pHeightMap, CSecretFrustum* pcSecretFrustum );

	/// �޸𸮿��� ����Ʈ���� �����Ѵ�.
	void		_Destroy();

	/// �����尡 �������ҿ� ���ԵǴ°�?
	int			_IsInFrustum( TERRAINVERTEX* pHeightMap, CSecretFrustum* pcSecretFrustum );

	int _GetLODLevel(TERRAINVERTEX* pHeightMap, Vector3 &vCamera, float fLODRatio) ;
	bool _IsVisible(TERRAINVERTEX* pHeightMap, Vector3 &vCamera, float fLODRatio) ;

	ZQuadTree *_FindNode(TERRAINVERTEX *pHeightMap, int nTL, int nTR, int nBL, int nBR) ;
	int _GetNeighborNodeIndex(int nEdge, int nPitch, int nHeight, int *pnEdge) ;
	void _BuildNeighborNode(ZQuadTree *pRoot, TERRAINVERTEX *pHeightMap, int nPitch, int nHeight) ;

	/// QuadTree�� �����Ѵ�.
	BOOL		_BuildQuadTree( TERRAINVERTEX* pHeightMap );

	void _SetTriangle(geo::STriangle *psTriangle, TERRAINVERTEX *pVertex0, TERRAINVERTEX *pVertex1, TERRAINVERTEX *pVertex2) ;

	int _GenerateTriangles(TERRAINVERTEX *pVertices, TERRAININDEX *pIndices, geo::STriangle *pTriangles, Vector3 *pvCameraPos, int nTris) ;

	bool _IsIntersected(geo::SCube *psCube, TERRAINVERTEX *pVertices) ;
	int _GetIntersectedTriangle(geo::SCube *psCube, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles) ;
	int _GetIntersectedTriangle(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles) ;
	int _GetIntersectedTriangle(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles) ;

public:
				/// ���� ��Ʈ��� ������
				ZQuadTree( int cx, int cy );

				/// ���� �ڽĳ�� ������
				ZQuadTree( /*ZQuadTree* pParent*/ );

				/// �Ҹ���
				~ZQuadTree();

                bool Build(TERRAINVERTEX *pHeightMap) ;

	/// _IsInFrustum()�Լ��� ����� ���� �������� �ø� ����
	void		_FrustumCull( TERRAINVERTEX* pHeightMap, CSecretFrustum* pcSecretFrustum );

	///	�ﰢ���� �ε����� �����, ����� �ﰢ���� ������ ��ȯ�Ѵ�.
	int			GenerateIndex( LPVOID pIndex, TERRAINVERTEX *pHeightMap, CSecretFrustum* pcSecretFrustum );

	//����Ʈ�� ������
	//���ؽ�����Ÿ�� �̹� �����޸𸮿� �ö��ִ»��°�
	//�ε�������Ÿ�� �Ź� �����ؼ� �ε��� Lock �ɾ���� ä��� �������ϴ� ����̴�.

	int GenerateTriangles(TERRAINMAP *pTerrainMap, Vector3 *pvCameraPos) ;//�ε�������Ÿ�� �����ϰ� cpu���꿡�� ���� �ﰢ������Ÿ�鵵 �����Ѵ�.
    
	void GetIntersectedTriangle(Vector3 *pvPos, TERRAINVERTEX *pVertices, Vector3 *pvIntersectedPos, geo::STriangle **ppsIntersectedTriangle, bool *pbIntersected) ;

	int GetIntersectedTriangle(geo::SCube *psCube, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles) ;//ť�������� ���ڽ����� �ﰢ�� ��󳻱�
	int GetIntersectedTriangle(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles) ;
	int GetIntersectedTriangle(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles) ;

};
