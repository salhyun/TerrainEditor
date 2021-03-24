#include "SecretOctTree.h"
#include "SecretRigidMesh.h"
#include "SecretTerrain.h"
#include "ArrangedDNode.h"

using namespace std ;

STrnPolygon::STrnPolygon()
{
	nCurTreeFloor = 0 ;
    pIndex = NULL ;	
}

STrnPolygon::STrnPolygon(UINT lIndex0, UINT lIndex1, UINT lIndex2)
{
	nCurTreeFloor = 0 ;
    pIndex = NULL ;	
}

static int nAllocCount=0 ;

CSecretOctTree::CSecretOctTree(Vector3 &vStart, Vector3 &vEnd)
{
    int i ;

	for(i=0 ; i<8 ; i++)
		m_pcChild[i] = NULL ;
	for(i=0 ; i<6 ; i++)
		m_pcNeighbor[i] = NULL ;

	m_PolygonSet.resize(0) ;
    
	m_sCube.set(vStart, vEnd) ;
	m_nIndex = nAllocCount++ ;
	m_bPolygons = false ;

	//TRACE("[%02d] ROOT construction OctTree\r\n", m_nIndex) ;
}

CSecretOctTree::CSecretOctTree()
{
    int i ;

	for(i=0 ; i<8 ; i++)
		m_pcChild[i] = NULL ;
	for(i=0 ; i<6 ; i++)
		m_pcNeighbor[i] = NULL ;

	m_PolygonSet.resize(0) ;

	m_nIndex = nAllocCount++ ;
	m_bPolygons = false ;
	//TRACE("[%02d] child construction OctTree\r\n", m_nIndex) ;
}

CSecretOctTree::~CSecretOctTree()
{
	int i ;

	for(i=0 ; i<8 ; i++)
		SAFE_DELETE(m_pcChild[i]) ;
	for(i=0 ; i<6 ; i++)
		SAFE_DELETE(m_pcNeighbor[i]) ;

	//TRACE("[%02d]destruction OctTree\r\n", m_nIndex) ;
}

bool CSecretOctTree::IsPolygons()
{
	return m_bPolygons ;
}
void CSecretOctTree::EnablePolygons(bool bEnable)
{
	m_bPolygons = bEnable ;
}
bool CSecretOctTree::IsVisible()
{
	return m_bVisible ;
}

CSecretOctTree *CSecretOctTree::_AppendChild(Vector3 vPos, Vector3 vLength)
{
	CSecretOctTree *pChild = new CSecretOctTree() ;
	pChild->m_sCube.set(vPos, vPos+vLength) ;
	pChild->m_nTreeFloor = m_nTreeFloor+1 ;

	//TRACE("floor=%02d Min(%+02.02f, %+02.02f, %+02.02f) Max(%+02.02f, %+02.02f, %+02.02f)\r\n",
	//	pChild->m_nTreeFloor,
	//	pChild->m_sCube.vMin.x, pChild->m_sCube.vMin.y, pChild->m_sCube.vMin.z,
	//	pChild->m_sCube.vMax.x, pChild->m_sCube.vMax.y, pChild->m_sCube.vMax.z) ;


	return pChild ;
}

CSecretOctTree *CSecretOctTree::_AppendChild(geo::SAACube *pCube)
{
	CSecretOctTree *pChild = new CSecretOctTree() ;
	memcpy(pChild->GetCube(), pCube, sizeof(geo::SAACube)) ;

	pChild->m_nTreeFloor = m_nTreeFloor+1 ;

	return pChild ;
}

bool CSecretOctTree::_Divide()
{
	if(m_sCube.fWidth <= MINIMUMCUBESIZE)
		return false ;

	geo::SAACube sDividedCube ;

	for(int i=0 ; i<MAXDIVISION ; i++)
	{
		m_pfSubDivide(&sDividedCube, &this->m_sCube, i) ;
		m_pcChild[i] = _AppendChild(&sDividedCube) ;
	}

	return true ;
}

//Divide and Append Child
//bool CSecretOctTree::_Divide()
//{
//	if(m_sCube.fWidth <= MINIMUMCUBESIZE)
//		return false ;
//
//	Vector3 vHalfLength = m_sCube.vCenter - m_sCube.vMin ;
//
//	//Upper-Left-Top Cube
//	m_pcChild[0] = _AppendChild(Vector3(m_sCube.vCenter.x-m_sCube.fWidth/2.0f, m_sCube.vCenter.y, m_sCube.vCenter.z), vHalfLength) ;
//	//Upper-Right-Top Cube
//	m_pcChild[1] = _AppendChild(Vector3(m_sCube.vCenter.x, m_sCube.vCenter.y, m_sCube.vCenter.z), vHalfLength) ;
//	//Upper-Left-Bottom Cube
//	m_pcChild[2] = _AppendChild(Vector3(m_sCube.vCenter.x-m_sCube.fWidth/2.0f, m_sCube.vCenter.y, m_sCube.vCenter.z-m_sCube.fDepth/2.0f), vHalfLength) ;
//	//Upper-Right-Bottom Cube
//	m_pcChild[3] = _AppendChild(Vector3(m_sCube.vCenter.x, m_sCube.vCenter.y, m_sCube.vCenter.z-m_sCube.fDepth/2.0f), vHalfLength) ;
//
//	//Lower-Left-Top Cube
//	m_pcChild[4] = _AppendChild(Vector3(m_sCube.vCenter.x-m_sCube.fWidth/2.0f, m_sCube.vCenter.y-m_sCube.fHeight/2.0f, m_sCube.vCenter.z), vHalfLength) ;
//	//Lower-Right-Top Cube
//	m_pcChild[5] = _AppendChild(Vector3(m_sCube.vCenter.x, m_sCube.vCenter.y-m_sCube.fHeight/2.0f, m_sCube.vCenter.z), vHalfLength) ;
//	//Lower-Left-Bottom Cube
//	m_pcChild[6] = _AppendChild(Vector3(m_sCube.vCenter.x-m_sCube.fWidth/2.0f, m_sCube.vCenter.y-m_sCube.fHeight/2.0f, m_sCube.vCenter.z-m_sCube.fDepth/2.0f), vHalfLength) ;
//	//Lower-Right-Bottom Cube
//	m_pcChild[7] = _AppendChild(Vector3(m_sCube.vCenter.x, m_sCube.vCenter.y-m_sCube.fHeight/2.0f, m_sCube.vCenter.z-m_sCube.fDepth/2.0f), vHalfLength) ;
//
//	return true ;
//}

bool CSecretOctTree::_IsInCubeQuad(Vector3 &v0, Vector3 &v1, Vector3 &v2)//test for Triangle Center in Cube
{
	geo::SPlane sPlane ;
	Vector3 vCenter ;

	vCenter.x = (v0.x+v1.x+v2.x)/3 ;
	vCenter.y = 0 ;// (v0.y+v1.y+v2.y)/3 ;
	vCenter.z = (v0.z+v1.z+v2.z)/3 ;

	sPlane = m_sCube.GetFace(geo::SAACube::SAACUBE_LEFTFACE) ;
	if(sPlane.Classify(vCenter) == geo::SPlane::SPLANE_INFRONT)
		return false ;

	sPlane = m_sCube.GetFace(geo::SAACube::SAACUBE_RIGHTFACE) ;
	if(sPlane.Classify(vCenter) == geo::SPlane::SPLANE_INFRONT)
		return false ;

	sPlane = m_sCube.GetFace(geo::SAACube::SAACUBE_FRONTFACE) ;
	if(sPlane.Classify(vCenter) == geo::SPlane::SPLANE_INFRONT)
		return false ;

	sPlane = m_sCube.GetFace(geo::SAACube::SAACUBE_BACKFACE) ;
	if(sPlane.Classify(vCenter) == geo::SPlane::SPLANE_INFRONT)
		return false ;

	return true ;
}

bool CSecretOctTree::_IsInCube(Vector3 &v0, Vector3 &v1, Vector3 &v2)//test for Triangle Center in Cube
{
	geo::SPlane sPlane ;
	Vector3 vCenter ;

	vCenter.x = (v0.x+v1.x+v2.x)/3 ;
	vCenter.y = (v0.y+v1.y+v2.y)/3 ;
	vCenter.z = (v0.z+v1.z+v2.z)/3 ;

	for(int i=0 ; i<6 ; i++)
	{
		sPlane = m_sCube.GetFace(i) ;
		if(sPlane.Classify(vCenter) == geo::SPlane::SPLANE_INFRONT)
			return false ;
	}
    return true ;
}

//CubeEdge[0] : Lower-Left-Bottom
//CubeEdge[1] : Upper-Right-Top
bool CSecretOctTree::_BuildTree(TERRAINVERTEX *psVertices, void (*pfSubDivide)(geo::SAACube *, geo::SAACube *, int))
{
	//float fMin, fMax ;
	Vector3 avPos[3], *pvMax, vMax ;

	m_pfSubDivide = pfSubDivide ;

	if(_Divide())
	{
		for(int n=0 ; n<(int)m_PolygonSet.size() ; n++)
		{
			avPos[0].set(psVertices[m_PolygonSet[n]->pIndex->i1].pos.x,
				psVertices[m_PolygonSet[n]->pIndex->i1].pos.y,
				psVertices[m_PolygonSet[n]->pIndex->i1].pos.z) ;

			avPos[1].set(psVertices[m_PolygonSet[n]->pIndex->i2].pos.x,
				psVertices[m_PolygonSet[n]->pIndex->i2].pos.y,
				psVertices[m_PolygonSet[n]->pIndex->i2].pos.z) ;

			avPos[2].set(psVertices[m_PolygonSet[n]->pIndex->i2].pos.x,
				psVertices[m_PolygonSet[n]->pIndex->i2].pos.y,
				psVertices[m_PolygonSet[n]->pIndex->i2].pos.z) ;

			for(int i=0 ; i<MAXDIVISION ; i++)
			{
#ifdef _USING_QUADTREE_
				if(m_pcChild[i]->_IsInCubeQuad(avPos[0], avPos[1], avPos[2]))
#else
				if(m_pcChild[i]->_IsInCube(avPos[0], avPos[1], avPos[2]))
#endif
				{
					m_PolygonSet[n]->nCurTreeFloor = m_pcChild[i]->m_nTreeFloor ;
					m_pcChild[i]->m_PolygonSet.push_back(m_PolygonSet[n]) ;
					m_pcChild[i]->EnablePolygons(true) ;

					if(avPos[0].y >= avPos[1].y)
						pvMax = &avPos[0] ;
					else
						pvMax = &avPos[1] ;

					if(pvMax->y < avPos[2].y)
						pvMax = &avPos[2] ;


					if(m_pcChild[i]->m_PolygonSet.size() == 0)//first
					{
						vMax = *pvMax ;
					}
					else
					{
						if(vMax.y < pvMax->y)
							vMax = *pvMax ;
					}

					m_vMaxY = vMax ;

					//TRACE("Num[%02d]Triangle [0](%+02.02f, %+02.02f, %+02.02f) [1](%+02.02f, %+02.02f, %+02.02f) [2](%+02.02f, %+02.02f, %+02.02f) --> ",
					//	n,
					//	avPos[0].x, avPos[0].y, avPos[0].z,
					//	avPos[1].x, avPos[1].y, avPos[1].z,
					//	avPos[2].x, avPos[2].y, avPos[2].z) ;

					//TRACE("Num[%02d]Cube Min(%+02.02f, %+02.02f, %+02.02f) Max(%+02.02f, %+02.02f, %+02.02f)\r\n",
					//	i,
					//	m_pcChild[i]->m_sCube.vMin.x, m_pcChild[i]->m_sCube.vMin.y, m_pcChild[i]->m_sCube.vMin.z,
					//	m_pcChild[i]->m_sCube.vMax.x, m_pcChild[i]->m_sCube.vMax.y, m_pcChild[i]->m_sCube.vMax.z) ;
				}
			}
			if(m_PolygonSet[n]->nCurTreeFloor == m_nTreeFloor)
			{
				TRACE("############## NOT INCLUDED ##############\r\n") ;
				TRACE("Num[%02d]Triangle [0](%+02.02f, %+02.02f, %+02.02f) [1](%+02.02f, %+02.02f, %+02.02f) [2](%+02.02f, %+02.02f, %+02.02f)\r\n ",
					n,
					avPos[0].x, avPos[0].y, avPos[0].z,
					avPos[1].x, avPos[1].y, avPos[1].z,
					avPos[2].x, avPos[2].y, avPos[2].z) ;
				TRACE("############################################\r\n") ;

				assert(false && "Polygon is not included") ;
			}
		}

		for(int i=0 ; i<MAXDIVISION ; i++)
		{
			m_pcChild[i]->_BuildTree(psVertices, pfSubDivide) ;
		}
	}

    return true ;
}

bool CSecretOctTree::BuildTree(TERRAINVERTEX *psVertices, STrnPolygon *psPolygons, int nNumPolygon, void (*pfSubDivide)(geo::SAACube *, geo::SAACube *, int))
{
	m_pfSubDivide = pfSubDivide ;

	for(int i=0 ; i<nNumPolygon ; i++)
	{
		m_PolygonSet.push_back(&psPolygons[i]) ;
	}

	m_nTreeFloor = 0 ;
	EnablePolygons(true) ;

	return _BuildTree(psVertices, pfSubDivide) ;
}

int CSecretOctTree::_IsInFrustumQuad(CSecretFrustum *pcFrustum)//y=0 으로 보고 판정한다.
{
	int i ;
	float fDist ;
	Vector2 vPos, vCenter ;
	geo::SPlane *psPlane ;
	int nResult=IN_FRUSTUM ;

	//카메라의 위치가 현재 큐브안에 있을경우
	vPos.set(pcFrustum->GetCameraPosition().x, pcFrustum->GetCameraPosition().z) ;
	vCenter.set(m_sCube.vCenter.x, m_sCube.vCenter.z) ;
	if((vPos-vCenter).Magnitude() <= m_sCube.fDiagonal)
		return INTERSECT_FRUSTUM ;

	for(i=0 ; i<6; i++)
	{
		psPlane = pcFrustum->GetPlane(i) ;
		fDist = (psPlane->a*m_sCube.vCenter.x)+(psPlane->b*0)+(psPlane->c*m_sCube.vCenter.z)+psPlane->d ;
		if(fDist > 0)//현재평면의 앞면에 있는경우(푸러스텀 바깥에 있다는 소리다)
		{
			if((m_sCube.fDiagonal-fDist) <= 0)//큐브경계원반지름보다 더 멀리떨어진경우
				return OUT_OF_FRUSTUM ;
			else
				nResult = INTERSECT_FRUSTUM ;
		}
		else//fDist<0 (푸러스텀 안쪽에 있다는 소리다)
		{
			if((m_sCube.fDiagonal+fDist) > 0)//그 거리가 큐브경계원반지름보다 큰경우
				nResult = INTERSECT_FRUSTUM ;
		}
	}
    return nResult ;
}

int CSecretOctTree::_IsInFrustum(CSecretFrustum *pcFrustum)
{
	float fDist ;
	geo::SPlane *psPlane ;
	int nResult=IN_FRUSTUM ;
    
	//카메라의 위치가 현재 큐브안에 있을경우
	if( (pcFrustum->GetCameraPosition()-m_sCube.vCenter).Magnitude() <= m_sCube.fRadius )
		return INTERSECT_FRUSTUM ;

	//큐브경계원이 푸러스텀평면의 바깥에 있고 평면과 큐브경계원 중심과의 거리가 큐브경계원의 반지름보다 클경우 보이지 않는다.
	//푸러스텀평면은 바깥쪽이 앞면이다.
	for(int i=0 ; i<6 ; i++)
	{
		psPlane = pcFrustum->GetPlane(i) ;
		fDist = (psPlane->a*m_sCube.vCenter.x)+(psPlane->b*m_sCube.vCenter.y)+(psPlane->c*m_sCube.vCenter.z)+psPlane->d ;
		if(fDist > 0)//현재평면의 앞면에 있는경우(푸러스텀 바깥에 있다는 소리다)
		{
			if((m_sCube.fRadius-fDist) <= 0)//큐브경계원반지름보다 더 멀리떨어진경우
				return OUT_OF_FRUSTUM ;
			else
				nResult = INTERSECT_FRUSTUM ;
		}
		else//fDist<0 (푸러스텀 안쪽에 있다는 소리다)
		{
			if((m_sCube.fRadius+fDist) > 0)//그 거리가 큐브경계원반지름보다 큰경우
				nResult = INTERSECT_FRUSTUM ;
		}
	}
	return nResult ;
}

int CSecretOctTree::_IsInFrustum(CSecretFrustum *pcFrustum, int *pnClassify)
{
	geo::SPlane *psPlane ;
	Vector3 vPos ;
	int nResult=IN_FRUSTUM, nCount=0 ;//, nClassify ;

	//카메라의 위치가 현재 큐브안에 있을경우
	if( (pcFrustum->GetCameraPosition()-m_sCube.vCenter).Magnitude() <= m_sCube.fRadius )
		return INTERSECT_FRUSTUM ;

	for(nCount=0 ; nCount<6 ; nCount++)
	{
		psPlane = pcFrustum->GetPlane(nCount) ;
		pnClassify[nCount] = IntersectPlaneToCube(psPlane, &m_sCube) ;
		if(pnClassify[nCount] == geo::SPlane::SPLANE_INFRONT)//컬링푸러스텀의 평면들이 전부 바깥쪽에 법선(바깥에서 보인다는말)이 있다.
		{
			if(nCount == 2)//nearPlane
				return OUT_OF_FRUSTUM ;

			if(fabs(psPlane->ToPosition(m_sCube.vCenter)) > MINIMUMCUBESIZE*1.5f)
				return OUT_OF_FRUSTUM ;
			else
				return INTERSECT_FRUSTUM ;

		}
		else if(pnClassify[nCount] == geo::SPlane::SPLANE_INTERSECT)
			nResult = INTERSECT_FRUSTUM ;
	}

	return nResult ;
}

void CSecretOctTree::GetTriangleToRender(std::vector<TERRAININDEX *> *pIndexSet, UINT *plCount)
{
	for(int i=0 ; i<(int)m_PolygonSet.size() ; i++)
		pIndexSet[m_PolygonSet[i]->nTexID].push_back(m_PolygonSet[i]->pIndex) ;

	return ;
}

void CSecretOctTree::GetTriangleToRender(CSecretFrustum *pcFrustum, std::vector<TERRAININDEX *> *pIndexSet, UINT *plCount)
{
	static int nCount=0 ;
	int anClassify[6] ;
	//TERRAININDEX sIndex ;

	m_bVisible = true ;
	m_bUntilthis = false ;

	(*plCount)++ ;

	if(!IsPolygons())
	{
		m_bVisible = false ;
		m_bUntilthis = true ;
		return ;
	}

    int nResult = _IsInFrustum(pcFrustum, anClassify) ;
	if(nResult == OUT_OF_FRUSTUM)
	{
		m_bVisible = false ;
		return ;
	}
	else if(nResult == INTERSECT_FRUSTUM)
	{
		m_bVisible = true ;
		if((m_sCube.fWidth <= MINIMUMCUBESIZE))//It is a least node
		{
			for(int i=0 ; i<(int)m_PolygonSet.size() ; i++)
				pIndexSet[m_PolygonSet[i]->nTexID].push_back(m_PolygonSet[i]->pIndex) ;
            
			m_bUntilthis = true ;
			return ;
		}

		for(int i=0 ; i<MAXDIVISION ; i++)
			m_pcChild[i]->GetTriangleToRender(pcFrustum, pIndexSet, plCount) ;
	}
	else if(nResult == IN_FRUSTUM)
	{
		m_bVisible = true ;
		for(int i=0 ; i<(int)m_PolygonSet.size() ; i++)
			pIndexSet[m_PolygonSet[i]->nTexID].push_back(m_PolygonSet[i]->pIndex) ;

		m_bUntilthis = true ;
	}

	return ;
}

void CSecretOctTree::GetTriangleToRender(CSecretFrustum *pcFrustum, data::SArray<TERRAININDEX> *pIndexSet, UINT *plCount)
{
	static int nCount=0 ;
	//int anClassify[6] ;
	int nResult ;

	m_bVisible = true ;
	m_bUntilthis = false ;

	(*plCount)++ ;

	if(!IsPolygons())
	{
		m_bVisible = false ;
		m_bUntilthis = true ;
		return ;
	}

#ifdef _USING_QUADTREE_
	nResult = _IsInFrustumQuad(pcFrustum) ;
#else
	nResult = _IsInFrustum(pcFrustum) ;
#endif

	if(nResult == OUT_OF_FRUSTUM)
	{
		m_bVisible = false ;
		return ;
	}
	else if(nResult == INTERSECT_FRUSTUM)
	{
		m_bVisible = true ;
		if((m_sCube.fWidth <= MINIMUMCUBESIZE))//It is a least node
		{
			for(int i=0 ; i<(int)m_PolygonSet.size() ; i++)
				pIndexSet[m_PolygonSet[i]->nTexID].push_back(*m_PolygonSet[i]->pIndex) ;
            
			m_bUntilthis = true ;
			return ;
		}

		for(int i=0 ; i<MAXDIVISION ; i++)
			m_pcChild[i]->GetTriangleToRender(pcFrustum, pIndexSet, plCount) ;
	}
	else if(nResult == IN_FRUSTUM)
	{
		m_bVisible = true ;
		for(int i=0 ; i<(int)m_PolygonSet.size() ; i++)
			pIndexSet[m_PolygonSet[i]->nTexID].push_back(*m_PolygonSet[i]->pIndex) ;

		m_bUntilthis = true ;
	}

	return ;
}

void CSecretOctTree::GetNodeToRender(CSecretFrustum *pcFrustum, std::vector<CSecretOctTree *> *pNode, UINT *plCount)
{
	static int nCount=0 ;
	int anClassify[6] ;
	//TERRAININDEX sIndex ;

	m_bVisible = true ;
	m_bUntilthis = false ;

	(*plCount)++ ;

	if(!IsPolygons())
	{
		m_bVisible = false ;
		m_bUntilthis = true ;
		return ;
	}

	int nResult = _IsInFrustum(pcFrustum, anClassify) ;
	if(nResult == OUT_OF_FRUSTUM)
	{
		m_bVisible = false ;
		return ;
	}
	else if(nResult == INTERSECT_FRUSTUM || nResult == IN_FRUSTUM)
	{
		m_bVisible = true ;

		if((m_sCube.fWidth <= MINIMUMCUBESIZE))//It is a least node
		{
			pNode->push_back(this) ;
			m_bUntilthis = true ;
			return ;
		}

		for(int i=0 ; i<MAXDIVISION ; i++)
			m_pcChild[i]->GetNodeToRender(pcFrustum, pNode, plCount) ;
	}
	return ;
}

void CSecretOctTree::GetAllTheCubes(vector<geo::SAACube *> *pCubeSet)
{
	if(!m_bVisible)
		return ;

	if(m_bUntilthis)
	{
		pCubeSet->push_back(GetCube()) ;
		return ;
	}

	//if(m_sCube.fWidth <= MINIMUMCUBESIZE)
	//{
	//	pCubeSet->push_back(GetCube()) ;
	//	return ;
	//}

	for(int i=0 ; i<MAXDIVISION ; i++)
		m_pcChild[i]->GetAllTheCubes(pCubeSet) ;
}