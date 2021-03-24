#include "SimpleQuadTree.h"
#include "def.h"

CSimpleQuadTree::CSimpleQuadTree(float cx, float cz)
{
	int i ;
	for(i=0 ; i<4 ; i++)
		m_pcChild[i] = NULL ;
	m_pcParent = NULL ;
	m_pTriangles = NULL ;
	m_fRadius = 0 ;
	m_bTriangles = false ;

	m_vCorner[0].set(-cx/2, 0, cz/2) ;//left-up
	m_vCorner[1].set(cx/2, 0, cz/2) ;//right-up
	m_vCorner[2].set(-cx/2, 0, -cz/2) ;//left-down
	m_vCorner[3].set(cx/2, 0, -cz/2) ;//right-down
}

CSimpleQuadTree::CSimpleQuadTree(CSimpleQuadTree *pcParent)
{
	int i ;
	for(i=0 ; i<4 ; i++)
		m_pcChild[i] = NULL ;
	m_pcParent = pcParent ;
	m_pTriangles = NULL ;
	m_fRadius = 0 ;
	m_bTriangles = false ;
}

CSimpleQuadTree::~CSimpleQuadTree()
{
	_Destroy() ;
}

void CSimpleQuadTree::_Destroy()
{
	for(int i=0 ; i<4 ; i++)
		SAFE_DELETE(m_pcChild[i]) ;

	SAFE_DELETE(m_pTriangles) ;
}

CSimpleQuadTree *CSimpleQuadTree::_AddChild(Vector3 &vLeftUp, Vector3 &vRightUp, Vector3 &vLeftDown, Vector3 &vRightDown)
{
    CSimpleQuadTree *pcChild = new CSimpleQuadTree(this) ;

    pcChild->m_vCorner[0] = vLeftUp ;
	pcChild->m_vCorner[1] = vRightUp ;
	pcChild->m_vCorner[2] = vLeftDown ;
	pcChild->m_vCorner[3] = vRightDown ;

	pcChild->m_vCenter.set((pcChild->m_vCorner[0].x+pcChild->m_vCorner[1].x)/2.0f, 0, (pcChild->m_vCorner[0].z+pcChild->m_vCorner[2].z)/2.0f) ;
	pcChild->m_fRadius = (pcChild->m_vCenter-pcChild->m_vCorner[0]).Magnitude() ;
	return pcChild ;
}

bool CSimpleQuadTree::_Divide()
{
	if(float_less_eq((m_vCorner[1]-m_vCorner[0]).Magnitude(), SIMPLEQUAD_MINSIZE))
		return false ;

	Vector3 vUpMid((m_vCorner[0].x+m_vCorner[1].x)/2.0f, 0, m_vCorner[0].z) ;
	Vector3 vLeftMid(m_vCorner[0].x, 0, (m_vCorner[0].z+m_vCorner[2].z)/2.0f) ;
	Vector3 vRightMid(m_vCorner[1].x, 0, (m_vCorner[1].z+m_vCorner[3].z)/2.0f) ;
	Vector3 vDownMid((m_vCorner[2].x+m_vCorner[3].x)/2.0f, 0, m_vCorner[2].z) ;
	Vector3 vMid((m_vCorner[0].x+m_vCorner[1].x)/2.0f, 0, (m_vCorner[0].z+m_vCorner[2].z)/2.0f) ;

	m_pcChild[0] = _AddChild(m_vCorner[0], vUpMid, vLeftMid, vMid) ;
	m_pcChild[1] = _AddChild(vUpMid, m_vCorner[1], vMid, vRightMid) ;
	m_pcChild[2] = _AddChild(vLeftMid, vMid, m_vCorner[2], vDownMid) ;
	m_pcChild[3] = _AddChild(vMid, vRightMid, vDownMid, m_vCorner[3]) ;

	return true ;
}

bool CSimpleQuadTree::_Build(std::vector<geo::STriangle *> *psTriangles)
{
	if(_Divide())
	{
		int i ;
		int nNumIncludeChild[4] ;
		std::vector<geo::STriangle *>::iterator it ;
		for(it=m_pTriangles->begin() ; it!=m_pTriangles->end() ; it++)
		{
			nNumIncludeChild[0] = nNumIncludeChild[1] = nNumIncludeChild[2] = nNumIncludeChild[3] = 0 ;

			//TRACE("Triangle 0(%07.03f %07.03f %07.03f) 1(%07.03f %07.03f %07.03f) 2(%07.03f %07.03f %07.03f)\r\n",
			//		enumVector((*it)->avVertex[0].vPos), enumVector((*it)->avVertex[1].vPos), enumVector((*it)->avVertex[2].vPos)) ;

			//Triangle 0(-02.000 003.100 002.000) 1(000.000 003.100 002.000) 2(-02.000 002.900 000.000)

			if( float_eq((*it)->avVertex[0].vPos.x, -2.0f) && float_eq((*it)->avVertex[0].vPos.y, 3.1f) && float_eq((*it)->avVertex[0].vPos.z, 2.0f)
				&& float_eq((*it)->avVertex[1].vPos.x, 0) && float_eq((*it)->avVertex[1].vPos.y, 3.1f) && float_eq((*it)->avVertex[1].vPos.z, 2.0f)
				&& float_eq((*it)->avVertex[2].vPos.x, -2.0f) && float_eq((*it)->avVertex[2].vPos.y, 2.9f) && float_eq((*it)->avVertex[1].vPos.z, 0))
			{
				if(float_eq(m_vCenter.x, -2) && float_eq(m_vCenter.z, 2))
					int aaaa=0 ;
			}

			for(i=0 ; i<4 ; i++)
			{
				if(m_pcChild[i])
				{
					if(_IsInQuad((*it), m_pcChild[i]))
					{
						m_pcChild[i]->m_pTriangles->push_back((*it)) ;
						nNumIncludeChild[i] = 1 ;
					}
				}
			}

			if((!nNumIncludeChild[0] && !nNumIncludeChild[1] && !nNumIncludeChild[2] && !nNumIncludeChild[3]))
				int eee = 0 ;

			//if(float_less_eq((m_pcChild[0]->m_vCorner[1]-m_pcChild[0]->m_vCorner[0]).Magnitude(), SIMPLEQUAD_MINSIZE))
			//{
			//	int www = 0 ;

			//	TRACE("Triangle 0(%07.03f %07.03f %07.03f) 1(%07.03f %07.03f %07.03f) 2(%07.03f %07.03f %07.03f)\r\n",
			//		enumVector((*it)->avVertex[0].vPos), enumVector((*it)->avVertex[1].vPos), enumVector((*it)->avVertex[2].vPos)) ;

			//	TRACE("is included in child[0]=%d child[1]=%d child[2]=%d child[3]=%d\r\n",
			//		nNumIncludeChild[0], nNumIncludeChild[1], nNumIncludeChild[2], nNumIncludeChild[3]) ;
			//}
		}

		//TRACE("\r\n") ;

		if(float_less_eq((m_pcChild[0]->m_vCorner[1]-m_pcChild[0]->m_vCorner[0]).Magnitude(), SIMPLEQUAD_MINSIZE))
			int www = 0 ;

		for(i=0 ; i<4 ; i++)
		{
			if(m_pcChild[i])
			{
				if(m_pcChild[i]->m_bTriangles)
					m_pcChild[i]->_Build(psTriangles) ;
			}
		}
	}
	return true ;
}

bool CSimpleQuadTree::Build(geo::STriangle *psTriangles, int nNum)
{
	m_pTriangles = new std::vector<geo::STriangle *> ;
	m_pTriangles->resize(0) ;
	for(int i=0 ; i<nNum ; i++, psTriangles++)
		m_pTriangles->push_back(psTriangles) ;

	m_bTriangles = true ;
	m_fRadius = (m_vCenter-m_vCorner[0]).Magnitude() ;

    return _Build(m_pTriangles) ;
}

bool CSimpleQuadTree::_IsInQuad(geo::STriangle *psTriangle, CSimpleQuadTree *pcChild)
{
	Vector3 v, vMid = psTriangle->GetMidPos() ;
	vMid.y = 0 ;

	float l1, l2, l3, length ;
	v = psTriangle->avVertex[0].vPos ;
	v.y = 0 ;
	l1 = (vMid-v).Magnitude() ;

	v = psTriangle->avVertex[1].vPos ;
	v.y = 0 ;
	l2 = (vMid-v).Magnitude() ;

	v = psTriangle->avVertex[2].vPos ;
	v.y = 0 ;
	l3 = (vMid-v).Magnitude() ;

	if(l1 > l2)
	{
		if(l1 > l3)
            length = l1 ;
		else
            length = l3 ;
	}
	else
	{
		if(l2 > l3)
			length = l2 ;
		else
			length = l3 ;
	}

	//if((vMid-(pcChild->m_vCenter)).Magnitude() <= (length+pcChild->m_fRadius))
	if(float_less_eq((vMid-(pcChild->m_vCenter)).Magnitude(), (length+pcChild->m_fRadius)))
	{
		if(!pcChild->m_bTriangles)
		{
			pcChild->m_bTriangles = true ;
			pcChild->m_pTriangles = new std::vector<geo::STriangle *> ;
			pcChild->m_pTriangles->resize(0) ;

		}
		return true ;
	}
	return false ;
}

bool bIntersectedPos ;

bool CSimpleQuadTree::_GetIntersectedPos(Vector3 *pvPos, float fRadius)
{
	if(bIntersectedPos)
		return true ;

	if(!m_bTriangles)
		return false ;

	//평면적으로 검사를 하기때문에 y값을 개입시키면 안됨!
	if((pvPos->x >= m_vCorner[0].x) && (pvPos->x <= m_vCorner[1].x) && (pvPos->z <= m_vCorner[0].z) && (pvPos->z >= m_vCorner[2].z))
	{
		if(float_less_eq((m_vCorner[1]-m_vCorner[0]).Magnitude(), SIMPLEQUAD_MINSIZE))
		{
			geo::SLine line(Vector3(pvPos->x, 1000.0f, pvPos->z), Vector3(0, -1, 0), 2000.0f) ;

			Vector3 v ;
			geo::STriangle *psTriangle ;
			std::vector<geo::STriangle *>::iterator it ;
			for(it=m_pTriangles->begin() ; it!=m_pTriangles->end(); it++)
			{
				psTriangle = (*it) ;

				//TRACE("Triangle 0(%07.03f %07.03f %07.03f) 1(%07.03f %07.03f %07.03f) 2(%07.03f %07.03f %07.03f)\r\n",
				//	enumVector((*it)->avVertex[0].vPos), enumVector((*it)->avVertex[1].vPos), enumVector((*it)->avVertex[2].vPos)) ;

				if(IntersectLinetoTriangle(line, *psTriangle, v, true, true, 0.001f) == geo::INTERSECT_POINT)
				{
					pvPos->y = v.y ;
					//TRACE("Intersect pos(%07.03f %07.03f %07.03f)\r\n", enumVector((*pvPos))) ;
					bIntersectedPos = true ;
					return true ;
				}
			}
		}
		else
		{
			for(int i=0 ; i<4 ; i++)
			{
				if(m_pcChild[i])
					m_pcChild[i]->_GetIntersectedPos(pvPos, fRadius) ;
			}
		}
	}

	return bIntersectedPos ;
}

bool CSimpleQuadTree::GetIntersectedPos(Vector3 *pvPos, float fRadius)
{
	bIntersectedPos = false ;
    return _GetIntersectedPos(pvPos, fRadius) ;
}

#include "SecretTerrain.h"

bool bIntersectedVertex ;

bool CSimpleQuadTree::_GetIntersectedPos(TERRAINVERTEX *psVertex, float fRadius)
{
	if(bIntersectedVertex)
		return true ;

	if(!m_bTriangles)
		return false ;

	Vector3 vPos(psVertex->pos.x, psVertex->pos.y, psVertex->pos.z) ;

	//평면적으로 검사를 하기때문에 y값을 개입시키면 안됨!
	if((vPos.x >= m_vCorner[0].x) && (vPos.x <= m_vCorner[1].x) && (vPos.z <= m_vCorner[0].z) && (vPos.z >= m_vCorner[2].z))
	{
		if(float_less_eq((m_vCorner[1]-m_vCorner[0]).Magnitude(), SIMPLEQUAD_MINSIZE))
		{
			geo::SLine line(Vector3(vPos.x, 1000.0f, vPos.z), Vector3(0, -1, 0), 2000.0f) ;

			Vector3 v ;
			geo::STriangle *psTriangle ;
			std::vector<geo::STriangle *>::iterator it ;
			for(it=m_pTriangles->begin() ; it!=m_pTriangles->end(); it++)
			{
				psTriangle = (*it) ;

				//TRACE("Triangle 0(%07.03f %07.03f %07.03f) 1(%07.03f %07.03f %07.03f) 2(%07.03f %07.03f %07.03f)\r\n",
				//	enumVector((*it)->avVertex[0].vPos), enumVector((*it)->avVertex[1].vPos), enumVector((*it)->avVertex[2].vPos)) ;

				if(IntersectLinetoTriangle(line, *psTriangle, v, true, true, 0.001f) == geo::INTERSECT_POINT)
				{
					//psVertex->pos.x = (psTriangle->avVertex[0].vPos.x*v.x)+(psTriangle->avVertex[1].vPos.x*v.y)+(psTriangle->avVertex[2].vPos.x*v.z) ;
					psVertex->pos.y = (psTriangle->avVertex[0].vPos.y*v.x)+(psTriangle->avVertex[1].vPos.y*v.y)+(psTriangle->avVertex[2].vPos.y*v.z) ;
					//psVertex->pos.z = (psTriangle->avVertex[0].vPos.z*v.x)+(psTriangle->avVertex[1].vPos.z*v.y)+(psTriangle->avVertex[2].vPos.z*v.z) ;

					psVertex->normal.x = (psTriangle->avVertex[0].vNormal.x*v.x)+(psTriangle->avVertex[1].vNormal.x*v.y)+(psTriangle->avVertex[2].vNormal.x*v.z) ;
					psVertex->normal.y = (psTriangle->avVertex[0].vNormal.y*v.x)+(psTriangle->avVertex[1].vNormal.y*v.y)+(psTriangle->avVertex[2].vNormal.y*v.z) ;
					psVertex->normal.z = (psTriangle->avVertex[0].vNormal.z*v.x)+(psTriangle->avVertex[1].vNormal.z*v.y)+(psTriangle->avVertex[2].vNormal.z*v.z) ;

					psVertex->t.x = (psTriangle->avVertex[0].vTex.x*v.x)+(psTriangle->avVertex[1].vTex.x*v.y)+(psTriangle->avVertex[2].vTex.x*v.z) ;
					psVertex->t.y = (psTriangle->avVertex[0].vTex.y*v.x)+(psTriangle->avVertex[1].vTex.y*v.y)+(psTriangle->avVertex[2].vTex.y*v.z) ;
					
					//TRACE("Intersect pos(%07.03f %07.03f %07.03f)\r\n", enumVector((*pvPos))) ;
					bIntersectedVertex = true ;
					return true ;
				}
			}
		}
		else
		{
			for(int i=0 ; i<4 ; i++)
			{
				if(m_pcChild[i])
					m_pcChild[i]->_GetIntersectedPos(psVertex, fRadius) ;
			}
		}
	}

	return false ;
}

bool CSimpleQuadTree::GetIntersectedPos(TERRAINVERTEX *psVertex, float fRadius)
{
	bIntersectedVertex = false ;
    return _GetIntersectedPos(psVertex, fRadius) ;
}