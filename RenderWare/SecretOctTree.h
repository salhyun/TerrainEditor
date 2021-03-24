#pragma once

#include "def.h"
#include "Vector3.h"
#include "Matrix.h"
#include "GeoLib.h"
#include "DataStructLib.h"
#include <vector>

struct TERRAINVERTEX ;
struct TERRAININDEX ;
class CSecretFrustum ;
struct SAACube ;

#define _USING_QUADTREE_
#define MAXDIVISION 4

#define MINIMUMCUBESIZE 4.0f
const float CubeEpsilon=1.0f ;

struct STrnPolygon : public geo::STriangle
{
	TERRAININDEX *pIndex ;//���� �ε�������Ÿ�� ����Ű�� ����
    int nCurTreeFloor ;
	int nTexID ;

	STrnPolygon() ;
	STrnPolygon(UINT lIndex0, UINT lIndex1, UINT lIndex2) ;
} ;

class CSecretOctTree
{
public :
	enum {UPPER_LEFT_TOP, UPPER_RIGHT_TOP, UPPER_LEFT_BOTTOM, UPPER_RIGHT_BOTTOM, LOWER_LEFT_TOP, LOWER_RIGHT_TOP, LOWER_LEFT_BOTTOM, LOWER_RIGHT_BOTTOM} ;
	enum {LEFT_TOP, RIGHT_TOP, LEFT_BOTTOM, RIGHT_BOTTOM} ;
	enum {OUT_OF_FRUSTUM, IN_FRUSTUM, INTERSECT_FRUSTUM} ;

protected :
	CSecretOctTree *m_pcChild[8] ;
	CSecretOctTree *m_pcNeighbor[6] ;
	int m_nTreeFloor ;
	bool m_bPolygons, m_bVisible, m_bUntilthis ;

    Vector3 m_vMaxY ;//for using Occlusion Culling
	geo::SAACube m_sCube ;//Bounding Cube of node
	std::vector<STrnPolygon *> m_PolygonSet ;

	void (*m_pfSubDivide)(geo::SAACube *, geo::SAACube *, int) ;

	int m_nIndex ;
	CSecretOctTree *_AppendChild(Vector3 vPos, Vector3 vLength) ;
	CSecretOctTree *_AppendChild(geo::SAACube *pCube) ;
	bool _Divide() ;
	bool _BuildTree(TERRAINVERTEX *psVertices, void (*pfSubDivide)(geo::SAACube *, geo::SAACube *, int)) ;
	bool _IsInCube(Vector3 &v0, Vector3 &v1, Vector3 &v2) ;
	bool _IsInCubeQuad(Vector3 &v0, Vector3 &v1, Vector3 &v2) ;
	int _IsInFrustum(CSecretFrustum *pcFrustum, int *pnClassify) ;//ť��� Ǫ������������ ������
	int _IsInFrustum(CSecretFrustum *pcFrustum) ;//ť������� Ǫ������������ ������
	int _IsInFrustumQuad(CSecretFrustum *pcFrustum) ;//4������ ��� ����ϴ� Ǫ����������� 4��������� ������

public :

	CSecretOctTree() ;
	CSecretOctTree(Vector3 &vStart, Vector3 &vEnd) ;
	virtual ~CSecretOctTree() ;
   
	bool IsPolygons() ;
	void EnablePolygons(bool bEnable) ;
	bool IsVisible() ;
	bool BuildTree(TERRAINVERTEX *psVertices, STrnPolygon *psPolygons, int nNumPolygon, void (*pfSubDivide)(geo::SAACube *, geo::SAACube *, int)) ;
	void GetTriangleToRender(CSecretFrustum *pcFrustum, std::vector<TERRAININDEX *> *pIndexSet, UINT *plCount) ;
    void GetTriangleToRender(std::vector<TERRAININDEX *> *pIndexSet, UINT *plCount) ;
	void GetTriangleToRender(CSecretFrustum *pcFrustum, data::SArray<TERRAININDEX> *pIndexSet, UINT *plCount) ;

	//��Ŭ���� �ø�(Occlusion Culling)���� �ﰢ���� �����Ѵ�. stl���.
	void GetNodeToRender(CSecretFrustum *pcFrustum, std::vector<CSecretOctTree *> *pNode, UINT *plCount) ;

	void GetAllTheCubes(std::vector<geo::SAACube *> *pCubeSet) ;
	geo::SAACube *GetCube() {return &m_sCube;} ;
	Vector3 GetMaxY() {return m_vMaxY;} ;
} ;