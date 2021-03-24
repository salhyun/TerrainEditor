#include "SecretBSP.h"
#include "def.h"
#include <assert.h>

//using namespace sal ;

CSecretBSP::CSecretBSP()
{
	m_psBSPRootNode = NULL ;
	m_psPolygonSet = NULL ;
}

CSecretBSP::~CSecretBSP()
{
	SAFE_DELETEARRAY(m_psBSPRootNode) ;
    SAFE_DELETEARRAY(m_psPolygonSet) ;
}

void CSecretBSP::_BestLength(D3DXVECTOR3 **ppvPos, int nNumPos, int *pnOder)
{
	int i, n, t ;
	float fBestLength=0.0f, fLength ;

    for(i=0 ; i<nNumPos ; i++)
	{
		for(n=0 ; n<nNumPos ; n++)
		{
			if(i == n)
				continue ;

			fLength = D3DXVec3Length(&(*ppvPos[n] - *ppvPos[i])) ;
			if(fBestLength < fLength)
			{
				pnOder[0] = i ;
				pnOder[1] = n ;

				int nCount=2 ;
				for(t=0 ; t<nNumPos ; t++)
				{
					if((pnOder[0] != t) && (pnOder[1] != t))
						pnOder[nCount++] = t ;
				}

				fBestLength = fLength ;
			}
		}
	}
}

bool CSecretBSP::_IntersectRaytoPlane(D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, geo::SPlane &plane, D3DXVECTOR3 &vOut, float &t)
{
	//ax+by+cz+d=0
	//vStart + vNormal*t = vPositon
	//a*(vStart.x + vNormal.x*t) + b*(vStart.y + vNormal.y*t) + c*(vStart.z + vNormal.z*t) + d = 0
	//a*vStart.x + b*vStart.y + c*vStart.z + a*(vNormal.x*t) + b*(vNormal.y*t) + c*(vNormal.z*t) + d = 0
	//a*vStart.x + b*vStart.y + c*vStart.z + t*(a*vNormal.x + b*vNormal.y + c*vNormal.z) + d = 0
	//t = -((a*vStart.x + b*vStart.y + c*vStart.z + d)/((a*vNormal.x + b*vNormal.y + c*vNormal.z)))

    D3DXVECTOR3 v = v2-v1 ;
	t = -((plane.a*v1.x + plane.b*v1.y + plane.c*v1.z + plane.d)/(plane.a*v.x + plane.b*v.y + plane.c*v.z)) ;

	if(t >= 0.0f && t <= 1.0f)
	{
		vOut.x = v1.x+v.x*t ;
		vOut.y = v1.y+v.y*t ;
		vOut.z = v1.z+v.z*t ;
		return true ;
	}
	return false ;
}

bool CSecretBSP::_IntersectPolygontoPlane(SPolygon &polygon, geo::SPlane &plane, D3DXVECTOR3 *pvIntersect, D3DXVECTOR2 *pvTex, int &nIntersectPos)
{
	//polygon p1, p2, p3 to plane a, b, c, d
	D3DXVECTOR3 v ;
	int nIntersectCount=0, nNumPrevious ;
	float t ;

	//무한평면에 대해서 하나의 교차점만 있을수는 없다.

	if(_IntersectRaytoPlane(polygon.avPos[0], polygon.avPos[1], plane, pvIntersect[nIntersectCount], t))
	{
		// Calculate New Texture Coordinate by linear interpolation
		pvTex[nIntersectCount].x = polygon.avTex[0].x + (polygon.avTex[1]-polygon.avTex[0]).x*t ;
		pvTex[nIntersectCount].y = polygon.avTex[0].y + (polygon.avTex[1]-polygon.avTex[0]).y*t ;

		++nIntersectCount ;
		nNumPrevious = 0 ;
	}

	if(_IntersectRaytoPlane(polygon.avPos[1], polygon.avPos[2], plane, pvIntersect[nIntersectCount], t))
	{
		// Calculate New Texture Coordinate by linear interpolation
		pvTex[nIntersectCount].x = polygon.avTex[1].x + (polygon.avTex[2]-polygon.avTex[1]).x*t ;
		pvTex[nIntersectCount].y = polygon.avTex[1].y + (polygon.avTex[2]-polygon.avTex[1]).y*t ;

		nNumPrevious = 1 ;

		if(++nIntersectCount > 1)
		{
			nIntersectPos = 1 ;
			return true ;
		}
	}

	if(_IntersectRaytoPlane(polygon.avPos[2], polygon.avPos[0], plane, pvIntersect[nIntersectCount], t))
	{
		// Calculate New Texture Coordinate by linear interpolation
		pvTex[nIntersectCount].x = polygon.avTex[2].x + (polygon.avTex[0]-polygon.avTex[2]).x*t ;
		pvTex[nIntersectCount].y = polygon.avTex[2].y + (polygon.avTex[0]-polygon.avTex[2]).y*t ;

		if(++nIntersectCount > 1)
		{
			if(nNumPrevious == 1)
				nIntersectPos = 2 ;
			else if(nNumPrevious == 0)
				nIntersectPos = 0 ;

			return true ;
		}
	}

	if(nIntersectCount > 0)
		return true ;

	return false ;
}

//폴리곤(삼각형) 분할순서
//[약속] 삼각형 p1, p2, p3, divider평면 plane
//1. (p1-p2), (p1-p3), (p2-p3), 3개의 라인과 Divider의 plane과 교차점 2개를 찾는다.( 교차점은 n1, n2)
//2. n1, n2가 (p1,p2,p3)셋중하나와 일치한다면 ex) if (n1=p1) then (n1, n2, p2), (n1, n2, p3)
//3. 그게 아니고 (n1-p2), (n2-p2) 이것처럼 공유하는 하나의 점을 찾을수 있다면 (n1, n2, p2)삼각형을 하나  만들수 있다.
//4. 그리고 남는점 n1, n2, p1, p3 네개의 점은 사각형일 것이다. 그러면 2개씩 짝지어서 선분의 길이가 가장 긴것이 대각선2개중 하나일 것이다.
//5. 그것이 만약 p1, n2 라면 (p1, n2, n1), (p1, n2, p3) 이렇게 2개의 삼각형이 나온다

bool CSecretBSP::_SplitPolygon(SPolygon *psDivider, SPolygon *psPolygon, SPolygon *psInFront, SPolygon *psBehind)
{
	int i, n ;
	int p1, p2, p3 ;
	D3DXVECTOR3 avIntersects[2] ;
	D3DXVECTOR2 avTexIntersects[2] ;
	int nIntersectPos ;

//1. (p1-p2), (p1-p3), (p2-p3), 3개의 라인과 Divider의 plane과 교차점 2개를 찾는다.( 교차점은 n1, n2)
	if(!_IntersectPolygontoPlane(*psPolygon, psDivider->sPlane, avIntersects, avTexIntersects, nIntersectPos))//여기서 얻어진 avIntersects는 ClockWise(시계방향) 순서대로 되어있다
		return false ;

	for(n=0 ; n<2 ; n++)
	{
		for(i=0 ; i<3 ; i++)
		{
//2. n1, n2가 (p1,p2,p3)셋중하나와 일치한다면 ex) if (n1=p1) then (n1, n2, p2), (n1, n2, p3)
			if(VECTOR_EQ(avIntersects[n], psPolygon->avPos[i]))
			{
                //D3D 왼손좌표계, 시계방향 삼각형이 앞면, 반시계방향 삼각형이 뒷면
				p1 = (i+1 >= 3) ? 0 : i+1 ;
				p2 = (n+1 >= 2) ? 0 : n+1 ;
				p3 = (i-1 < 0) ? 2 : i-1 ;

				SPolygon polygon1(psPolygon->avPos[i], psPolygon->avPos[p1], avIntersects[p2]) ;
				SPolygon polygon2(psPolygon->avPos[i], avIntersects[p2], psPolygon->avPos[p3]) ;
//				SPolygon polygon1(psPolygon->avPos[i], psPolygon->avPos[((i+1) >= 3) ? 0 : (i+1)], avIntersects[((n+1) >= 2) ? 0 : (n+1)]) ;
//				SPolygon polygon2(psPolygon->avPos[i], avIntersects[((n+1) >= 2) ? 0 : (n+1)], psPolygon->avPos[((i-1) < 0) ? 2 : (i-1)]) ;

				if(_ClassifyPolygon(psDivider, &polygon1) == JUDGE_INFRONT)
				{
					memcpy(psInFront, &polygon1, sizeof(SPolygon)) ;
					memcpy(psBehind, &polygon2, sizeof(SPolygon)) ;
				}
				else 
				{
					memcpy(psInFront, &polygon2, sizeof(SPolygon)) ;
					memcpy(psBehind, &polygon1, sizeof(SPolygon)) ;
				}
                return true ;
			}
		}
	}

//3. 그게 아니고 (n1-p2), (n2-p2) 이것처럼 공유하는 하나의 점을 찾을수 있다면 (n1, n2, p2)삼각형을 하나  만들수 있다.

	SPolygon *psPolygonTemp1, *psPolygonTemp2 ;

	if((_ClassifyPoint(psDivider, &psPolygon->avPos[nIntersectPos]) <= JUDGE_INFRONT)
		&& (_ClassifyPoint(psDivider, &avIntersects[0]) <= JUDGE_INFRONT)
		&& (_ClassifyPoint(psDivider, &avIntersects[1]) <= JUDGE_INFRONT))
	{
		psPolygonTemp1 = psInFront ;
		psPolygonTemp2 = psBehind ;
	}
	else
	{
		psPolygonTemp1 = psBehind ;
		psPolygonTemp2 = psInFront ;
	}

	psPolygonTemp1->SetPos(psPolygon->avPos[nIntersectPos], avIntersects[0], avIntersects[1]) ;
	psPolygonTemp1->MakePlane() ;

	if(FLOAT_EQ(psPolygon->sPlane.a, psPolygonTemp1->sPlane.a)
		&& FLOAT_EQ(psPolygon->sPlane.b, psPolygonTemp1->sPlane.b)
		&& FLOAT_EQ(psPolygon->sPlane.c, psPolygonTemp1->sPlane.c))
	{
		psPolygonTemp1->SetTex(psPolygon->avTex[nIntersectPos], avTexIntersects[0], avTexIntersects[1]) ;
	}
	else
	{
		psPolygonTemp1->SetPos(psPolygon->avPos[nIntersectPos], avIntersects[1], avIntersects[0]) ;
		psPolygonTemp1->MakePlane() ;
		psPolygonTemp1->SetTex(psPolygon->avTex[nIntersectPos], avTexIntersects[1], avTexIntersects[0]) ;
	}

	D3DXVECTOR3 *apvPos[4] ;
	D3DXVECTOR2 *apvTex[4] ;
	int anOder[4] ;

    if(nIntersectPos == 0)
	{
		apvPos[0] = &psPolygon->avPos[1] ;
		apvPos[1] = &psPolygon->avPos[2] ;

		apvTex[0] = &psPolygon->avTex[1] ;
		apvTex[1] = &psPolygon->avTex[2] ;
	}
	else if(nIntersectPos == 1)
	{
		apvPos[0] = &psPolygon->avPos[0] ;
		apvPos[1] = &psPolygon->avPos[2] ;

		apvTex[0] = &psPolygon->avTex[1] ;
		apvTex[1] = &psPolygon->avTex[2] ;
	}
	else if(nIntersectPos == 2)
	{
		apvPos[0] = &psPolygon->avPos[0] ;
		apvPos[1] = &psPolygon->avPos[1] ;

		apvTex[0] = &psPolygon->avTex[1] ;
		apvTex[1] = &psPolygon->avTex[2] ;
	}

	apvPos[2] = &avIntersects[0] ;
	apvPos[3] = &avIntersects[1] ;

	apvTex[2] = &avTexIntersects[0] ;
	apvTex[3] = &avTexIntersects[1] ;

	_BestLength(apvPos, 4, anOder) ;

	psPolygonTemp2[0].SetPos(*apvPos[anOder[0]], *apvPos[anOder[1]], *apvPos[anOder[2]]) ;
	psPolygonTemp2[0].MakePlane() ;
	if(FLOAT_EQ(psPolygon->sPlane.a, psPolygonTemp2[0].sPlane.a)
		&& FLOAT_EQ(psPolygon->sPlane.b, psPolygonTemp2[0].sPlane.b)
		&& FLOAT_EQ(psPolygon->sPlane.c, psPolygonTemp2[0].sPlane.c))
	{
		psPolygonTemp2[0].SetTex(*apvTex[anOder[0]], *apvTex[anOder[1]], *apvTex[anOder[2]]) ;
	}
	else
	{
		psPolygonTemp2[0].SetPos(*apvPos[anOder[0]], *apvPos[anOder[2]], *apvPos[anOder[1]]) ;
		psPolygonTemp2[0].MakePlane() ;
		psPolygonTemp2[0].SetTex(*apvTex[anOder[0]], *apvTex[anOder[2]], *apvTex[anOder[1]]) ;
	}

	psPolygonTemp2[1].SetPos(*apvPos[anOder[0]], *apvPos[anOder[1]], *apvPos[anOder[3]]) ;
	psPolygonTemp2[1].MakePlane() ;
	if(FLOAT_EQ(psPolygon->sPlane.a, psPolygonTemp2[1].sPlane.a)
		&& FLOAT_EQ(psPolygon->sPlane.b, psPolygonTemp2[1].sPlane.b)
		&& FLOAT_EQ(psPolygon->sPlane.c, psPolygonTemp2[1].sPlane.c))
	{
		psPolygonTemp2[1].SetTex(*apvTex[anOder[0]], *apvTex[anOder[1]], *apvTex[anOder[3]]) ;
	}
	else
	{
		psPolygonTemp2[1].SetPos(*apvPos[anOder[0]], *apvPos[anOder[3]], *apvPos[anOder[1]]) ;
		psPolygonTemp2[1].MakePlane() ;
		psPolygonTemp2[1].SetTex(*apvTex[anOder[0]], *apvTex[anOder[3]], *apvTex[anOder[1]]) ;
	}

    return true ;
}

JUDGE CSecretBSP::_ClassifyPoint(SPolygon *psPolygon, D3DXVECTOR3 *pvPos)
{
	D3DXVECTOR3 vNormal(psPolygon->sPlane.a, psPolygon->sPlane.b, psPolygon->sPlane.c) ;
	float fSideValue = D3DXVec3Dot(&vNormal, pvPos) ;

	if(fSideValue == (-psPolygon->sPlane.d))
		return JUDGE_COINCLIDING ;
	if(fSideValue < (-psPolygon->sPlane.d))
		return JUDGE_BEHIND ;

	return JUDGE_INFRONT ;
}

JUDGE CSecretBSP::_ClassifyPolygon(SPolygon *psPolygon1, SPolygon *psPolygon2)
{
	int i ;
	int nNumPositive=0, nNumNegative=0 ;
	JUDGE judge ;

	for(i=0 ; i<3 ; i++)
	{
		if(_ClassifyPoint(psPolygon1, &psPolygon2->avPos[i]) == JUDGE_INFRONT)
			nNumPositive++ ;
		else if(_ClassifyPoint(psPolygon1, &psPolygon2->avPos[i]) == JUDGE_BEHIND)
			nNumNegative++ ;
	}

	if(nNumPositive == 0 && nNumNegative == 0)
		judge = JUDGE_COINCLIDING ;
	else if(nNumPositive > 0 && nNumNegative == 0)
        judge = JUDGE_INFRONT ;
	else if(nNumPositive == 0 && nNumNegative > 0)
		judge = JUDGE_BEHIND ;
	else
		judge = JUDGE_SPANNING ;

    return judge ;
}

bool CSecretBSP::_IsConvexSet(SPolygon *psPolygons, int nNumPolygon)//닫혀진 공간을 찾아낸다.
{
	int i, n ;
	SPolygon *psPolygon1, *psPolygon2 ;

	psPolygon1 = psPolygons ;
	for(i=0 ; i<nNumPolygon ; i++, psPolygon1++)
	{
		psPolygon2 = psPolygons ;
		for(n=0 ; n<nNumPolygon ; n++, psPolygon2++)
		{
			if(i == n)
				continue ;
            
			if(_ClassifyPolygon(psPolygon1, psPolygon2) == JUDGE_BEHIND)
				return false ;
		}
	}
	return true ;
}

SPolygon *CSecretBSP::_ChooseDividingPolygon(SPolygon *psPolygonSet, int nNumPolygon, int &nNumPositive, int &nNumNegative)
{
	int i, n ;
	SPolygon *psBestPolygon=NULL, *psPolygons1=NULL, *psPolygons2=NULL ;
	float fMinRelation = 0.0f, fBestRelation = 0.0f, fRelation ;
	int nCountPositive=0, nCountNegative=0, nCountSpanning=0, nCountCoinclide=0, nLeastSplits=0 ;
	JUDGE judge ;

	while(psBestPolygon == NULL)
	{
		psPolygons1 = psPolygonSet ;
		for(i=0 ; i<nNumPolygon ; i++, psPolygons1++)
		{
			nCountPositive = nCountNegative = nCountCoinclide = 0 ;
			psPolygons2 = psPolygonSet ;
			for(n=0 ; n<nNumPolygon ; n++, psPolygons2++)
			{
				if(n == i)//except same polygon
					continue ;

				judge = _ClassifyPolygon(psPolygons1, psPolygons2) ;
				if(judge == JUDGE_INFRONT)
					nCountPositive++ ;
				else if(judge == JUDGE_BEHIND)
					nCountNegative++ ;
				else if(judge == JUDGE_COINCLIDING)
					nCountCoinclide++ ;
				else if(judge == JUDGE_SPANNING)
					nCountSpanning++ ;
			}

			//앞에 있는 폴리곤, 뒤에 있는 폴리곤 갯수의 비율 0-1사이의 값으로 만들어 둔다
			if(nCountPositive < nCountNegative)
				fRelation = (float)nCountPositive/(float)nCountNegative ;
			else
				fRelation = (float)nCountNegative/(float)nCountPositive ;

			//분할갯수와 앞뒤폴리곤의 관계를 가지고 가장 적절한 폴리곤을 선정한다.
			if((fRelation > fMinRelation)
				&& ((nCountSpanning < nLeastSplits)
				|| ((nCountSpanning == nLeastSplits) && (fRelation > fBestRelation))))
			{
				psBestPolygon = psPolygons1 ;
				nLeastSplits = nCountSpanning ;
				fBestRelation = fRelation ;

				nNumPositive = nCountPositive + nCountSpanning + nCountCoinclide ;
				nNumNegative = nCountNegative + nCountSpanning ;
			}
		}
	}

    return psBestPolygon ;
}

void CSecretBSP::_GenerateBSPTree(SBSPNode *psBSPNode, SPolygon *psPolygonSet, int nNumPolygon)
{
	int i ;
	int nNumPositive=0, nNumNegative=0, nCountPositive=0, nCountNegative=0 ;
	SPolygon *psDivider=NULL, *psPolygons, *psInFront, *psBehind ;
	SPolygon *psPositiveSet, *psNegativeSet ;
	JUDGE judge ;

	//더이상 나눌수 없는 PolygonSet 일경우
	if(_IsConvexSet(psPolygonSet, nNumPolygon))
	{
		psBSPNode->psPolygons = new SPolygon[nNumPolygon] ;
		memcpy(psBSPNode->psPolygons, psPolygonSet, sizeof(SPolygon)*nNumPolygon) ;
		psBSPNode->nNumPolygons = nNumPolygon ;
		return ;
	}

	psDivider = _ChooseDividingPolygon(psPolygonSet, nNumPolygon, nNumPositive, nNumNegative) ;

	TRACE("Divid NumPositive=%d NumNegative=%d\r\n", nNumPositive, nNumNegative) ;

	memcpy(&psBSPNode->sDivider, psDivider, sizeof(SPolygon)) ;

    psPositiveSet = new SPolygon[nNumPositive + 1] ;// nNumPositive + Divider
	psNegativeSet = new SPolygon[nNumNegative] ;
	nCountPositive = nCountNegative = 0 ;

	psPolygons = m_psPolygonSet ;
	for(i=0 ; i<nNumPolygon ; i++, psPolygons++)
	{
		judge = _ClassifyPolygon(psDivider, psPolygons) ;

		if(judge == JUDGE_INFRONT || judge == JUDGE_COINCLIDING)
		{
			psPositiveSet[nCountPositive++] = *psPolygons ;
		}
		else if(judge == JUDGE_BEHIND)
		{
			psNegativeSet[nCountNegative++] = *psPolygons ;
		}
		else if(judge == JUDGE_SPANNING)
		{
			psInFront = new SPolygon ;
			psBehind = new SPolygon ;

			//_SplitPolygon(psDivider, psPolygons, psInFront, psBehind) ;

			psPositiveSet[nCountPositive++] = *psInFront ;
			psNegativeSet[nCountNegative++] = *psBehind ;
		}
	}

	assert((nNumPositive+1) == nCountPositive && nNumNegative == nCountNegative) ;

	psBSPNode->psFrontNode = new SBSPNode ;
	psBSPNode->psBackNode = new SBSPNode ;

	_GenerateBSPTree(psBSPNode->psFrontNode, psPositiveSet, nNumPositive+1) ;
	_GenerateBSPTree(psBSPNode->psBackNode, psNegativeSet, nNumNegative) ;

	SAFE_DELETEARRAY(psPositiveSet) ;
	SAFE_DELETEARRAY(psNegativeSet) ;
}

void CSecretBSP::test(CASEData *pcASEData)
{
	int i, n, t ;
	int nCount=0, nIndex ;
	SMesh *psMesh ;

	D3DXVECTOR3 vIntersect[2] ;
	SPolygon polygon1, polygon2, polygonResult[2] ;

	polygon1.avPos[0] = D3DXVECTOR3(5.0f, 5.0f, 0.0f) ;
	polygon1.avPos[1] = D3DXVECTOR3(5.0f, 0.0f, 0.0f) ;
	polygon1.avPos[2] = D3DXVECTOR3(-5.0f, 0.0f, 0.0f) ;
	polygon1.MakePlane() ;

	polygon2.avPos[0] = D3DXVECTOR3(0.0f, 5.0f, 0.0f) ;
	polygon2.avPos[1] = D3DXVECTOR3(0.0f, 0.0f, 5.0f) ;
	polygon2.avPos[2] = D3DXVECTOR3(0.0f, 0.0f, -5.0f) ;
	polygon2.MakePlane() ;

	D3DXVec3Cross(&vIntersect[0], &polygon2.avPos[0], &polygon2.avPos[1]) ;
	D3DXVec3Cross(&vIntersect[1], &polygon2.avPos[1], &polygon2.avPos[0]) ;



    _SplitPolygon(&polygon2, &polygon1, &polygonResult[0], &polygonResult[1]) ;

	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
	{
		nCount += pcASEData->m_apMesh[i]->nNumTriangle ;
	}

	m_psPolygonSet = new SPolygon[nCount] ;
	nCount=0 ;

	//Plane Oder 3->1->2->4->5->6->7->8

	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
	{
		psMesh = pcASEData->m_apMesh[i] ;
		for(n=0 ; n<psMesh->nNumTriangle ; n++)
		{
			for(t=0 ; t<3 ; t++)
			{
				nIndex = psMesh->psTriangle[n].anVertexIndex[t] ;

				m_psPolygonSet[nCount].avPos[t].x = psMesh->psVertex[nIndex].pos.x * psMesh->smatLocal.m11
					                               +psMesh->psVertex[nIndex].pos.y * psMesh->smatLocal.m21
					                               +psMesh->psVertex[nIndex].pos.z * psMesh->smatLocal.m31
												   +psMesh->smatLocal.m41 ;

				m_psPolygonSet[nCount].avPos[t].y = psMesh->psVertex[nIndex].pos.x * psMesh->smatLocal.m12
					                               +psMesh->psVertex[nIndex].pos.y * psMesh->smatLocal.m22
					                               +psMesh->psVertex[nIndex].pos.z * psMesh->smatLocal.m32
												   +psMesh->smatLocal.m42 ;

				m_psPolygonSet[nCount].avPos[t].z = psMesh->psVertex[nIndex].pos.x * psMesh->smatLocal.m13
					                               +psMesh->psVertex[nIndex].pos.y * psMesh->smatLocal.m23
					                               +psMesh->psVertex[nIndex].pos.z * psMesh->smatLocal.m33
												   +psMesh->smatLocal.m43 ;

				m_psPolygonSet[nCount].avTex[t] = D3DXVECTOR2(psMesh->psVertex[nIndex].tex.u, psMesh->psVertex[nIndex].tex.v) ;
			}
			m_psPolygonSet[nCount++].MakePlane() ;
		}
	}

	m_psBSPRootNode = new SBSPNode ;
	_GenerateBSPTree(m_psBSPRootNode, m_psPolygonSet, nCount) ;

	int test ;
	test=1 ;
}

void CSecretBSP::Release()
{

}