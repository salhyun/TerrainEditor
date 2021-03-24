#pragma once

#include <windows.h>
#include "GeoLib.h"
#include <vector>

#define SIMPLEQUAD_MINSIZE 8.0f

struct TERRAINVERTEX ;

class CSimpleQuadTree
{
private :
	CSimpleQuadTree *m_pcChild[4], *m_pcParent ;
	float m_fRadius ;
	Vector3 m_vCorner[4], m_vCenter ;

	CSimpleQuadTree *_AddChild(Vector3 &vLeftUp, Vector3 &vRightUp, Vector3 &vLeftDown, Vector3 &vRightDown) ;
	bool _IsInQuad(geo::STriangle *psTriangle, CSimpleQuadTree *pcChild) ;
	bool _Divide() ;
	bool _Build(std::vector<geo::STriangle *> *psTriangles) ;
	void _Destroy() ;

public :
	std::vector<geo::STriangle *> *m_pTriangles ;
	bool m_bTriangles ;

public :
	CSimpleQuadTree(CSimpleQuadTree *pcParent=NULL) ;
	CSimpleQuadTree(float cx, float cz) ;
	~CSimpleQuadTree() ;

	bool Build(geo::STriangle *psTriangles, int nNum) ;


//Finding Functions
private :
    bool _GetIntersectedPos(Vector3 *pvPos, float fRadius) ;
	bool _GetIntersectedPos(TERRAINVERTEX *psVertex, float fRadius) ;

public :
	bool GetIntersectedPos(Vector3 *pvPos, float fRadius) ;
	bool GetIntersectedPos(TERRAINVERTEX *psVertex, float fRadius) ;
} ;