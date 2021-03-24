#include "SecretRoad.h"
#include "SecretPicking.h"
#include "SecretTerrain.h"
#include "SecretDecal.h"

//Road BoundingVolume

bool SRoadRectPlane::Intersect(Vector3 &p)
{
	Vector3 vDir, v ;

	v = (p - avLines[0].s).Normalize() ;
	vDir = avLines[0].v.cross(v) ;

	for(int i=1 ; i<4 ; i++)
	{
		v = (p - avLines[i].s).Normalize() ;
		v = avLines[i].v.cross(v) ;
		if(vDir.dot(v) < 0.0f)
			return false ;
	}
	return true ;
}
bool SRoadRectPlane::getDropPosition(Vector3 &p, Vector3 &vTargetPos, Vector3 *pvDropPos)
{
	Vector3 vDir, v ;

	v = (p - avLines[0].s).Normalize() ;
	vDir = avLines[0].v.cross(v) ;

	for(int i=1 ; i<4 ; i++)
	{
		v = (p - avLines[i].s).Normalize() ;
		v = avLines[i].v.cross(v) ;
		if(vDir.dot(v) < 0.0f)
		{
			//길에서 왼쪽 체크
			geo::SPlane plane(avLines[1].s, avLines[1].v) ;
			if(plane.Classify(vTargetPos) == geo::SPlane::SPLANE_BEHIND)
			{
				Vector3 dir = avLines[1].v ;
				dir.y = 0.0f ;
				(*pvDropPos) = plane.GetOnPos(p) - (dir.Normalize() * 0.5f) ;
			}
			else
			{
				//길에서 오른쪽 체크
				plane.set(avLines[3].s, avLines[3].v) ;
				if(plane.Classify(vTargetPos) == geo::SPlane::SPLANE_BEHIND)
				{
					Vector3 dir = avLines[3].v ;
					dir.y = 0.0f ;
					(*pvDropPos) = plane.GetOnPos(p) - (dir.Normalize() * 0.5f) ;
				}
			}
			return false ;
		}
	}
	return true ;
}
SRoadCirclePlane::SRoadCirclePlane()
{
	nNumRectPlane = 0 ;
	psRectPlanes = NULL ;
}
SRoadCirclePlane::~SRoadCirclePlane()
{
	SAFE_DELETEARRAY(psRectPlanes) ;
}
bool SRoadCirclePlane::Intersect(Vector3 &vPos)
{
	if( (sCircle.vPos - vPos).Magnitude() <= sCircle.fRadius )
		return true ;
	return false ;
}

SRoadBoundingPlane::SRoadBoundingPlane()
{
	nNumCirclePlane = 0 ;
	psCirclePlanes = NULL ;
}
SRoadBoundingPlane::~SRoadBoundingPlane()
{
	SAFE_DELETEARRAY(psCirclePlanes) ;
}
bool SRoadBoundingPlane::Intersect(Vector3 &vPos)
{
	SRoadCirclePlane *psCirclePlane = psCirclePlanes ;
	for(int i=0 ; i<nNumCirclePlane ; i++, psCirclePlane++)
	{
		if(psCirclePlane->Intersect(vPos))
		{
			SRoadRectPlane *psRectPlane = psCirclePlane->psRectPlanes ;
			for(int n=0 ; n<psCirclePlane->nNumRectPlane ; n++, psRectPlane++)
			{
				if(psRectPlane->Intersect(vPos))
					return true ;
			}
		}
	}
	return false ;
}
bool SRoadBoundingPlane::getDropPosition(Vector3 &vPos, Vector3 &vTargetPos, Vector3 *pvDropPos)
{
	SRoadCirclePlane *psCirclePlane = psCirclePlanes ;
	for(int i=0 ; i<nNumCirclePlane ; i++, psCirclePlane++)
	{
		if(psCirclePlane->Intersect(vPos))
		{
			SRoadRectPlane *psRectPlane = psCirclePlane->psRectPlanes ;
			for(int n=0 ; n<psCirclePlane->nNumRectPlane ; n++, psRectPlane++)
			{
				if(psRectPlane->getDropPosition(vPos, vTargetPos, pvDropPos))
					return true ;
			}
		}
	}
	return false ;
}

CSecretRoad::CSecretRoad()
{
	m_pcTerrain = NULL ;
	m_pvPosts = NULL ;
	m_pvPositions = NULL ;
	m_psCVertices = NULL ;
	m_pTex = NULL ;
	m_nDivision = 10 ;
	m_dwVertexColor = 0xff00f0f0 ;
	m_fBoxSize = 0.25f ;
	m_fWidth = 1.0f ;
	m_fWeight = 0.45f ;
	m_fTexFactor = 1.0f ;
	m_nStatus = IDLING ;
	m_nAttr = 0 ;
	m_nTerrainRoadIndex = -1 ;
	m_pTerrainVertices = NULL ;
	m_pwTerrainIndex = NULL ;
	m_psBoundingPlane = NULL ;
}
CSecretRoad::~CSecretRoad()
{
}
void CSecretRoad::Release()
{
	SAFE_DELETEARRAY(m_pvPosts) ;
	SAFE_DELETEARRAY(m_pvPositions) ;
	SAFE_DELETEARRAY(m_psCVertices) ;

	SAFE_DELETEARRAY(m_pTerrainVertices) ;
	SAFE_DELETEARRAY(m_pwTerrainIndex) ;

    if(m_nTerrainRoadIndex != -1)
		m_pcTerrain->m_acRoad[m_nTerrainRoadIndex].Release() ;

	SAFE_DELETE(m_psBoundingPlane) ;
}
bool CSecretRoad::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretTerrain *pcTerrain, CSecretPicking *pcPicking, char *pszName, float width, int nDivision, LPDIRECT3DTEXTURE9 pTex, char *pszRoadKind)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcTerrain = pcTerrain ;
	m_pcPicking = pcPicking ;
	MAXNUM_POSITION = MAXNUM_SPLINEDATA*nDivision ;
	m_pvPosts = new Vector3[MAXNUM_POSITION] ;
	m_pvPositions = new D3DXVECTOR3[MAXNUM_POSITION] ;
	m_psCVertices = new SCVertex[MAXNUM_POSITION*2] ;
	sprintf(m_szName, "%s", pszName) ;
	m_nStatus = IDLING ;
	m_fWidth = width ;
	m_nDivision = nDivision ;
	sprintf(m_szRoadKind, "%s", pszRoadKind) ;
	m_pTex = pTex ;
	return true ;
}
void CSecretRoad::Process(bool bPressedLButton)
{
	if(m_nStatus == IDLING)
	{
	}
	else if(m_nStatus == BUILDING)
	{
		Vector3 vCurPos = m_pcPicking->GetCurVertex()->pos ;

		if(bPressedLButton)//찍을때 동시에 2개의 점을 찍고
		{
			if(m_cSpline.GetCurrentNode() < MAXNUM_SPLINEDATA)
			{				
				if(!m_cSpline.IsEnable())
				{
					_AddPosition(vCurPos) ;
					_AddPosition(vCurPos+Vector3(vCurPos.x, vCurPos.y, vCurPos.z+1.0f)) ;
				}
				else
				{
					if(m_cSpline.GetCurrentNode() > 0)
					{
						SPLINEDATA *psPrevData = m_cSpline.GetSplineData(m_cSpline.GetCurrentNode()-1) ;
						if((vCurPos-psPrevData->vPos).Magnitude() > 1.0f)//같은 위치에는 찍지않는다.
							_AddPosition(vCurPos+(vCurPos-psPrevData->vPos).Normalize()) ;
					}
				}
			}
		}
		else//누르지 않으면 마지막점만 계속 업데이트해준다.
		{
			if(!m_cSpline.IsEnable())
				return ;

			if(m_cSpline.GetCurrentNode() > 1)//2개이상
			{
				SPLINEDATA *psPPrevData = m_cSpline.GetSplineData(m_cSpline.GetCurrentNode()-2) ;
				SPLINEDATA *psPrevData = m_cSpline.GetSplineData(m_cSpline.GetCurrentNode()-1) ;
				SPLINEDATA *psCurData = m_cSpline.GetSplineData(m_cSpline.GetCurrentNode()) ;

				if((vCurPos-psPrevData->vPos).Magnitude() < 1.0f)
					return ;

				psCurData->vPos = vCurPos ;
				psCurData->vNor = (psCurData->vPos-psPrevData->vPos)*m_fWeight ;
				psPrevData->vNor = (psCurData->vPos-psPPrevData->vPos)*m_fWeight ;
			}
			else
			{
				SPLINEDATA *psPrevData = m_cSpline.GetSplineData(m_cSpline.GetCurrentNode()-1) ;
				SPLINEDATA *psCurData = m_cSpline.GetSplineData(m_cSpline.GetCurrentNode()) ;

				if((vCurPos-psPrevData->vPos).Magnitude() < 1.0f)
					return ;

				psCurData->vPos = vCurPos ;
				psCurData->vNor = (psCurData->vPos-psPrevData->vPos)*m_fWeight ;
				psPrevData->vNor = psCurData->vNor ;
			}
		}
	}
	BuildAssistant() ;
}
void CSecretRoad::BuildAssistant()
{
	m_nNumPost = m_nNumPosition = m_nNumCVertex = 0 ;
	Vector3 v ;
	SPLINEDATA *psSplineData ;
    int i, n, nNumNode = m_cSpline.GetCurrentNode() ;
	float t ;
	for(i=0 ; i<=nNumNode ; i++)//현재 노드를 가리킨다. 그러니까 루프를 돌리려면 '<=' 이렇게 해주어야 함.
	{
		psSplineData = m_cSpline.GetSplineData(i) ;
		m_pvPosts[m_nNumPost++] = psSplineData->vPos ;
		m_pvPositions[m_nNumPosition++] = D3DXVECTOR3(psSplineData->vPos.x, psSplineData->vPos.y, psSplineData->vPos.z) ;

		if(i < nNumNode)//여기는 끝까지 가면 안됨.
		{
			for(n=1 ; n<m_nDivision ; n++)
			{
				t = float_round((float)n/(float)m_nDivision) ;
				v = m_cSpline.GetPositionOnCubic(i, t) ;
				m_pvPositions[m_nNumPosition++] = D3DXVECTOR3(v.x, v.y, v.z) ;
			}
		}
	}
	m_nNumCVertex = BuildLineStrip(m_pvPositions, m_nNumPosition, m_fWidth/2.0f, m_psCVertices, m_dwVertexColor) ;
}
void CSecretRoad::_BuildBoundingPlane()
{
	SAFE_DELETE(m_psBoundingPlane) ;

	m_psBoundingPlane = new SRoadBoundingPlane() ;

	m_psBoundingPlane->nNumCirclePlane = m_nNumPost-1 ;
	m_psBoundingPlane->psCirclePlanes = new SRoadCirclePlane[m_psBoundingPlane->nNumCirclePlane] ;

	int i, n ;
	for(i=0 ; i<m_psBoundingPlane->nNumCirclePlane ; i++)
	{
		m_psBoundingPlane->psCirclePlanes[i].nNumRectPlane = m_nDivision ;
		m_psBoundingPlane->psCirclePlanes[i].psRectPlanes = new SRoadRectPlane[m_nDivision] ;
	}

	float fLength=0 ;
	D3DXVECTOR3 p ;
	SCVertex *psCVertices = m_psCVertices ;
	for(i=0 ; i<m_psBoundingPlane->nNumCirclePlane ; i++)
	{
		//TRACE("[%d] Circle Plane\r\n", i) ;

		if(m_nDivision%2)//홀수
		{
			int ad = (i*m_nDivision) + (m_nDivision/2) ;
			p = (m_pvPositions[ad] + m_pvPositions[ad+1])*0.5f ;
		}
		else//짝수
			p = m_pvPositions[(i*m_nDivision) + (m_nDivision/2)] ;

		m_psBoundingPlane->psCirclePlanes[i].sCircle.vPos.set(p.x, 0, p.z) ;

		p = m_pvPositions[(i*m_nDivision)+m_nDivision] - m_pvPositions[i*m_nDivision] ;
		m_psBoundingPlane->psCirclePlanes[i].sCircle.fRadius = D3DXVec3Length(&p)*0.75f ;
		//TRACE("Circle pos(%g %g %g) radius=%g\r\n", enumVector(m_psBoundingPlane->psCirclePlanes[i].sCircle.vPos), m_psBoundingPlane->psCirclePlanes[i].sCircle.fRadius) ;

		//   0-----1
		//   |     |
		//   |     |
		//   2-----3
		SRoadRectPlane *psRectPlane ;
		for(n=0 ; n<m_nDivision ; n++, psCVertices += 2)
		{
			psRectPlane = &m_psBoundingPlane->psCirclePlanes[i].psRectPlanes[n] ;

			//0 -> 2
			psRectPlane->avLines[0].s.set(psCVertices->pos.x, 0, psCVertices->pos.z) ;
			p = ((psCVertices+2)->pos - psCVertices->pos) ;
			p.y = 0.0f ;
			D3DXVec3Normalize(&p, &p) ;
			psRectPlane->avLines[0].v.set(p.x, p.y, p.z)  ;

			//2 -> 3
			psRectPlane->avLines[1].s.set((psCVertices+2)->pos.x, 0, (psCVertices+2)->pos.z) ;
			p = ((psCVertices+3)->pos - (psCVertices+2)->pos) ;
			p.y = 0.0f ;
			D3DXVec3Normalize(&p, &p) ;
			psRectPlane->avLines[1].v.set(p.x, p.y, p.z)  ;

			//3 -> 1
			psRectPlane->avLines[2].s.set((psCVertices+3)->pos.x, 0, (psCVertices+3)->pos.z) ;
			p = ((psCVertices+1)->pos - (psCVertices+3)->pos) ;
			p.y = 0.0f ;
			D3DXVec3Normalize(&p, &p) ;
			psRectPlane->avLines[2].v.set(p.x, p.y, p.z)  ;

			//1 -> 0
			psRectPlane->avLines[3].s.set((psCVertices+1)->pos.x, 0, (psCVertices+1)->pos.z) ;
			p = (psCVertices->pos - (psCVertices+1)->pos) ;
			p.y = 0.0f ;
			D3DXVec3Normalize(&p, &p) ;
			psRectPlane->avLines[3].v.set(p.x, p.y, p.z)  ;
		}
	}
}
bool CSecretRoad::_AddPosition(Vector3 vPos)
{
	if(!m_cSpline.IsEnable())//처음인경우
	{
		m_cSpline.Initialize(vPos, Vector3(0.0f, 0.0f, 1.0f)) ;
		m_cSpline.SetEnable() ;
	}
	else
	{
		Vector3 v ;
		if(m_cSpline.GetCurrentNode() > 1)//2개이상
		{
			SPLINEDATA *psSplineData1 = m_cSpline.GetSplineData(m_cSpline.GetCurrentNode()-1) ;
			SPLINEDATA *psSplineData2 = m_cSpline.GetSplineData(m_cSpline.GetCurrentNode()) ;

			v = (vPos - psSplineData2->vPos)*m_fWeight ;
			m_cSpline.AddNode(vPos, v) ;

			psSplineData2->vNor = (vPos - psSplineData1->vPos)*m_fWeight ;
		}
		else
		{
			SPLINEDATA *psSplineData = m_cSpline.GetSplineData(m_cSpline.GetCurrentNode()) ;
			v = (vPos - psSplineData->vPos)*m_fWeight ;
			m_cSpline.AddNode(vPos, v) ;
			psSplineData->vNor = v ;
		}
	}
	return true ;
}
void CSecretRoad::_WeldingVertex(TERRAINVERTEX **ppsVertices, int &nNumVertex, STerrainIndex1 *psIndices, int &nNumIndex)
{
	bool bRepeated, bEqual ;
	int i, n ;
	std::vector<TERRAINVERTEX> vertices ;
	std::vector<int> repeated ;
	std::vector<int>::iterator int_it ;
	TERRAINVERTEX *psVertices=*ppsVertices, *psVertices1, *psVertices2 ;

	for(i=0 ; i<nNumVertex ; i++)
	{
		bEqual = false ;
		bRepeated = false ;
		for(int_it=repeated.begin(); int_it!=repeated.end(); int_it++)
		{
			if((*int_it) == i)
				bRepeated = true ;
		}
		if(bRepeated)
			continue ;

		psVertices1 = &psVertices[i] ;

		for(n=0 ; n<nNumVertex ; n++)
		{
			if(i==n)
				continue ;

			psVertices2 = &psVertices[n] ;

			if(vector_eq(psVertices1->pos, psVertices2->pos))
				//&& float_eq(psVertices1->t.x, psVertices2->t.x) && float_eq(psVertices1->t.y, psVertices2->t.y))
			{
				TRACE("same vector\r\n") ;
				TRACE("p(%10.07f %10.07f %10.07f) n(%10.07f %10.07f %10.07f) t(%10.07f %10.07f)\r\n", enumVector(psVertices1->pos), enumVector(psVertices1->normal), psVertices1->t.x, psVertices1->t.y) ;
				TRACE("p(%10.07f %10.07f %10.07f) n(%10.07f %10.07f %10.07f) t(%10.07f %10.07f)\r\n", enumVector(psVertices2->pos), enumVector(psVertices2->normal), psVertices2->t.x, psVertices2->t.y) ;
				TRACE("\r\n") ;

				repeated.push_back(n) ;
				bEqual = true ;
			}
		}
		vertices.push_back(*psVertices1) ;
	}

	TRACE("new vertex num=%d\r\n", (int)vertices.size()) ;

	if((int)vertices.size() != nNumVertex)
	{
		nNumVertex = (int)vertices.size() ;
		for(i=0 ; i<nNumVertex ; i++)
			psVertices[i] = vertices[i] ;
	}
}
void CSecretRoad::AlignOnTerrain()
{
	if(!m_cSpline.IsEnable())
		return ;

	if(m_nTerrainRoadIndex == -1)
	{
		m_nTerrainRoadIndex = m_pcTerrain->GetEmptyRoad() ;
		if(m_nTerrainRoadIndex == -1)
			return ;
	}

	int i, n, t, temp, nNumTriangle ;
	float radius, r, fTexYOffset1=0, fTexYOffset2=0 ;
	Vector3 avPos[4], vNormal(0, 1, 0), vCenter ;
	Vector2 avTexFactor[4] ;
	int nTerrainVertexCount=0, nTerrainIndexCount=0 ;

	//geo::STriangle *psTriangles = new geo::STriangle[1024] ;
	geo::STriangle *psTriangles ;

	//static TERRAINVERTEX asTerrainVertices[1024] ;
	//static STerrainIndex1 asTerrainIndices[1024] ;

    int nMaxNumVertex=65536, nMaxNumIndex=52428 ;
	TERRAINVERTEX *psTerrainVertices = new TERRAINVERTEX[nMaxNumVertex] ;
	STerrainIndex1 *psTerrainIndices = new STerrainIndex1[nMaxNumIndex] ;

	//m_pTerrainVertices = new TERRAINVERTEX[1024] ;
	//m_pwTerrainIndex = new WORD[2048] ;

	m_nNumTerrainIndex = 0 ;
	m_nNumTerrainVertex = 0 ;

    //   0-----1
	//   |     |
	//   |     |
    //   3-----2
	for(i=m_nNumCVertex-1 ; i>=2 ; i -= 2)
	//for(i=0 ; i<m_nNumCVertex ; i += 2)
	{
		CSecretDecal decal ;
		decal.Initialize(m_pd3dDevice, vNormal, Vector3(1, 0, 0), 2, 2, 2) ;

		temp = i ;
		avPos[0].set(m_psCVertices[temp].pos.x, m_psCVertices[temp].pos.y, m_psCVertices[temp].pos.z) ;
		temp = i-1 ;
		avPos[1].set(m_psCVertices[temp].pos.x, m_psCVertices[temp].pos.y, m_psCVertices[temp].pos.z) ;
		temp = i-3 ;
		avPos[2].set(m_psCVertices[temp].pos.x, m_psCVertices[temp].pos.y, m_psCVertices[temp].pos.z) ;
		temp = i-2 ;
		avPos[3].set(m_psCVertices[temp].pos.x, m_psCVertices[temp].pos.y, m_psCVertices[temp].pos.z) ;

		//TRACE("avPos[0].set(%10.07ff, %10.07ff, %10.07ff);\r\n", enumVector(avPos[0])) ;
		//TRACE("avPos[1].set(%10.07ff, %10.07ff, %10.07ff);\r\n", enumVector(avPos[1])) ;
		//TRACE("avPos[2].set(%10.07ff, %10.07ff, %10.07ff);\r\n", enumVector(avPos[2])) ;
		//TRACE("avPos[3].set(%10.07ff, %10.07ff, %10.07ff);\r\n", enumVector(avPos[3])) ;

		vCenter = (avPos[0]+avPos[1]+avPos[2]+avPos[3])*0.25f ;
		radius = (vCenter-avPos[0]).Magnitude() ;
		for(n=1 ; n<4 ; n++)
		{
			r = (vCenter-avPos[n]).Magnitude() ;
			if(radius < r)
				radius = r ;
		}
		radius *= 2.0f ;

        //fTexYOffset1 왼쪽엣지 avPos[2]-avPos[0]
		//fTexYOffset2 오른쪽엣지 avPos[3]-avPos[1]
  //      avTexFactor[0].set(0, fTexYOffset1) ;
		//avTexFactor[1].set(m_fWidth*m_fTexFactor, fTexYOffset2) ;

  //      fTexYOffset2 += (avPos[3]-avPos[0]).Magnitude()*m_fTexFactor ;
		////fTexYOffset2 = float_round(fTexYOffset2) ;        
		//avTexFactor[2].set(m_fWidth*m_fTexFactor, fTexYOffset2) ;

		//fTexYOffset1 += (avPos[2]-avPos[1]).Magnitude()*m_fTexFactor ;
		////fTexYOffset1 = float_round(fTexYOffset1) ;
		//avTexFactor[3].set(0, fTexYOffset1) ;

		avTexFactor[0].set(0, 0) ;
		avTexFactor[1].set(m_fTexFactor, 0) ;
		avTexFactor[2].set(m_fTexFactor, (avPos[2]-avPos[1]).Magnitude()*m_fTexFactor) ;
		avTexFactor[3].set(0, (avPos[3]-avPos[0]).Magnitude()*m_fTexFactor) ;

		//TRACE("\r\n") ;
		//TRACE("texOffset[0] (%10.06f %10.06f)\r\n", avTexFactor[0].x, avTexFactor[0].y) ;
		//TRACE("texOffset[1] (%10.06f %10.06f)\r\n", avTexFactor[1].x, avTexFactor[1].y) ;
		//TRACE("texOffset[2] (%10.06f %10.06f)\r\n", avTexFactor[2].x, avTexFactor[2].y) ;
		//TRACE("texOffset[3] (%10.06f %10.06f)\r\n", avTexFactor[3].x, avTexFactor[3].y) ;
		//TRACE("\r\n") ;

		decal.SetTexFactor(avTexFactor) ;

		int a ;
        float fBestWidth, fBestHeight, fMinX, fMinZ, fMaxX, fMaxZ ;
		fMinX = fMaxX = avPos[0].x ;
		fMinZ = fMaxZ = avPos[0].z ;
		for(a=1 ; a<4 ; a++)
		{
			if(fMinX > avPos[a].x)
				fMinX = avPos[a].x ;
			if(fMinZ > avPos[a].z)
				fMinZ = avPos[a].z ;
			if(fMaxX < avPos[a].x)
				fMaxX = avPos[a].x ;
			if(fMaxZ < avPos[a].z)
				fMaxZ = avPos[a].z ;
		}
		fBestWidth = (fMaxX-fMinX)*2.0f ;
		fBestHeight = (fMaxZ-fMinZ)*2.0f ;
        if(fBestWidth < 4.0f)
			fBestWidth = 4.0f ;
		if(fBestHeight < 4.0f)
			fBestHeight = 4.0f ;

		decal.MakingBoundingPlanes(vCenter, avPos, vNormal, 10.0f) ;

		int nMaxTriangle = m_pcTerrain->m_sHeightMap.GetNumTriangle(vCenter, fBestWidth, fBestHeight) ;
		psTriangles = new geo::STriangle[nMaxTriangle] ;

		nNumTriangle = m_pcTerrain->m_sHeightMap.GetTriangles(vCenter, fBestWidth, fBestHeight, psTriangles, false, nMaxTriangle) ;
		//overflow number of triangles
		if(nNumTriangle == 0)
			break ;

		decal.ReAllocateVB(nNumTriangle*3) ;
		decal.ReAllocateIB(nNumTriangle) ;

		decal.AddInBoundTriangles(psTriangles, nNumTriangle) ;

		//overflow number of vertices
		if(((nTerrainVertexCount+decal.GetVertexCount()) >= nMaxNumVertex) || (nTerrainIndexCount+decal.GetIndexCount()) >= nMaxNumIndex)
			break ;

        SDecalVertex *psDecalVertices = decal.GetVertices() ;
		for(t=0 ; t<decal.GetVertexCount() ; t++, nTerrainVertexCount++)
		{
			assert(nTerrainVertexCount < nMaxNumVertex) ;

			psTerrainVertices[nTerrainVertexCount].pos = psDecalVertices[t].pos ;
			psTerrainVertices[nTerrainVertexCount].normal = psDecalVertices[t].normal ;
			psTerrainVertices[nTerrainVertexCount].t = psDecalVertices[t].tex ;

			//m_pTerrainVertices[m_nNumTerrainVertex++] = asTerrainVertices[nTerrainVertexCount] ;
		}

		int nIndexOffset = nTerrainVertexCount-decal.GetVertexCount() ;

		SDecalIndex *psDecalIndices = decal.GetIndices() ;
		for(t=0 ; t<decal.GetIndexCount() ; t++, nTerrainIndexCount++)
		{
			assert(nTerrainIndexCount < nMaxNumIndex) ;

			psTerrainIndices[nTerrainIndexCount].i1 = psDecalIndices[t].awIndex[0]+nIndexOffset ;
			psTerrainIndices[nTerrainIndexCount].i2 = psDecalIndices[t].awIndex[1]+nIndexOffset ;
			psTerrainIndices[nTerrainIndexCount].i3 = psDecalIndices[t].awIndex[2]+nIndexOffset ;

			//m_pwTerrainIndex[m_nNumTerrainIndex++] = psDecalIndices[t].awIndex[0]+nIndexOffset ;
			//m_pwTerrainIndex[m_nNumTerrainIndex++] = psDecalIndices[t].awIndex[1]+nIndexOffset ;
			//m_pwTerrainIndex[m_nNumTerrainIndex++] = psDecalIndices[t].awIndex[2]+nIndexOffset ;
		}

		SAFE_DELETEARRAY(psTriangles) ;
	}

	TRACE("vertext=%d index=%d\r\n", nTerrainVertexCount, nTerrainIndexCount) ;
	//_WeldingVertex(&psTerrainVertices, nTerrainVertexCount, psTerrainIndices, nTerrainIndexCount) ;
	m_pcTerrain->m_acRoad[m_nTerrainRoadIndex].Initialize(m_pd3dDevice, psTerrainVertices, nTerrainVertexCount, psTerrainIndices, nTerrainIndexCount, m_pTex) ;

	m_nAttr |= ATTR_ONTERRAIN ;

    SAFE_DELETEARRAY(psTriangles) ;

	SAFE_DELETEARRAY(psTerrainVertices) ;
	SAFE_DELETEARRAY(psTerrainIndices) ;

	_BuildBoundingPlane() ;
}
void CSecretRoad::SetStatus(int nStatus)
{
	m_nStatus = nStatus ;
}
void CSecretRoad::Reset(float fWidth, int nDivision, LPDIRECT3DTEXTURE9 pTex, char *pszRoadKind)
{
	m_fWidth = fWidth ;
	m_nDivision = nDivision ;
	sprintf(m_szRoadKind, "%s", pszRoadKind) ;
	m_pTex = pTex ;
}
bool CSecretRoad::Intersect(Vector3 &vPos)
{
	if(m_psBoundingPlane)
		return m_psBoundingPlane->Intersect(vPos) ;
	
	return false ;
}
bool CSecretRoad::getDropPosition(Vector3 &vPos, Vector3 &vTargetPos, Vector3 *pvDropPos)
{
	if(m_psBoundingPlane)
		return m_psBoundingPlane->getDropPosition(vPos, vTargetPos, pvDropPos) ;
	
	return false ;
}
void CSecretRoad::Render()
{
}
void CSecretRoad::_BoxRender(geo::SAACube *psCube)
{
	Vector3 p ;
	SCVertex vertex[8] ;

    for(int i=0; i<8; i++)
	{
		p = psCube->GetPos(i) ;
		vertex[i].pos.x = p.x ;
		vertex[i].pos.y = p.y ;
		vertex[i].pos.z = p.z ;
		vertex[i].color = 0xffffff00 ;
	}

	unsigned short index[] =
	{
		0, 1,
		1, 2,
		2, 3,
        3, 0,

		4, 5,
		5, 6,
		6, 7,
		7, 4,

		0, 4,
		1, 5,
		2, 6,
		3, 7,
	} ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 8, 12, index, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;
}

void CSecretRoad::AssistantRender()
{
	if(!m_cSpline.IsEnable())
		return ;

	//CSecretDecal decal ;
	//decal.Initialize(m_pd3dDevice, Vector3(0, 1, 0), Vector3(1, 0, 0), 2, 2, 2) ;

    int i ;
	geo::SAACube sCube ;
	for(i=0 ; i<m_nNumPost ; i++)
	{
		sCube.set(m_pvPosts[i], m_fBoxSize, m_fBoxSize, m_fBoxSize) ;
		_BoxRender(&sCube) ;
	}

	DWORD dwFillMode ;
	m_pd3dDevice->GetRenderState(D3DRS_FILLMODE, &dwFillMode) ;

	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME) ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_nNumCVertex-2, m_psCVertices, sizeof(SCVertex)) ;
	//m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, nStripCount-1, asVertices, sizeof(SCVertex)) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;

    //decal
	//if(m_nAttr & ATTR_ONTERRAIN)
	//{
	//	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID) ;
	//	m_pd3dDevice->SetFVF(D3DFVF_TERRAINVERTEX) ;
	//	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(TERRAINVERTEX)) ;
	//	m_pd3dDevice->SetTexture(0, decal.GetTexture()) ;
	//	m_pd3dDevice->SetIndices(0) ;

	//	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_nNumTerrainVertex, m_nNumTerrainIndex, m_pwTerrainIndex, D3DFMT_INDEX16, m_pTerrainVertices, sizeof(TERRAINVERTEX)) ;	
	//}

	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, dwFillMode) ;
}
/*
#include "MathOrdinary.h"
void CSecretRoad::AssistantRender()
{
    int i ;
	geo::SAACube sCube ;
	for(i=0 ; i<m_nNumPost ; i++)
	{
		sCube.set(m_pvPosts[i], m_fBoxSize, m_fBoxSize, m_fBoxSize) ;
		_BoxRender(&sCube) ;
	}

	SCVertex vtRoad[6] ;

	float h = m_pcTerrain->m_sHeightMap.GetHeightMap(0.0f, 0.0f) ;

	//   0-----1
	//   |     |
	//   |     |
    //   2-----3
	Vector3 avPos[4], vNormal(0, 1, 0), vCenter ;
	//avPos[0].set(0.5f, h, 2.5f) ;
	//avPos[1].set(1.2f, h, -0.4f) ;
	//avPos[2].set(0.75f, h, -0.85f) ;
	//avPos[3].set(-0.95f, h, 0.25f) ;

	avPos[0].set(-0.2054380f, 59.9805756f, -96.8088989f);
	avPos[1].set( 1.5654103f, 59.9805756f, -97.7375183f);
	avPos[2].set( 0.0139733f, 60.0508842f, -100.6960754f);
	avPos[3].set(-1.7568750f, 60.0508842f, -99.7674561f);

	vtRoad[0].pos = VectorConvert(avPos[0]) ;
	vtRoad[1].pos = VectorConvert(avPos[1]) ;
	vtRoad[2].pos = VectorConvert(avPos[3]) ;
	vtRoad[3].pos = VectorConvert(avPos[3]) ;
	vtRoad[4].pos = VectorConvert(avPos[1]) ;
	vtRoad[5].pos = VectorConvert(avPos[2]) ;

	vCenter = (avPos[0]+avPos[1]+avPos[2]+avPos[3])*0.25f ;
	float radius = (vCenter-avPos[0]).Magnitude() ;
	for(i=1 ; i<4 ; i++)
	{
		float r = (vCenter-avPos[i]).Magnitude() ;
        if(radius < r)
			radius = r ;
	}
	radius *= 2.0f ;

	CSecretDecal decal ;
	decal.Initialize(m_pd3dDevice, vNormal, Vector3(1, 0, 0), 2, 2, 2) ;
	decal.MakingBoundingPlanes(vCenter, avPos, vNormal, radius) ;

	geo::STriangle asTri[1024] ;
	int nTris = m_pcTerrain->m_sHeightMap.GetTrianglesInRadius(vCenter, radius, asTri) ;

	//asTri[0].set(Vector3(-1, h, 1), Vector3(0, h, 1), Vector3(-1, h, 0)) ;
	//asTri[0].avVertex[0].vNormal = m_pcTerrain->m_sHeightMap.GetNormal(asTri[0].avVertex[0].vPos.x, asTri[0].avVertex[0].vPos.z) ;
	//asTri[0].avVertex[1].vNormal = m_pcTerrain->m_sHeightMap.GetNormal(asTri[0].avVertex[1].vPos.x, asTri[0].avVertex[1].vPos.z) ;
	//asTri[0].avVertex[2].vNormal = m_pcTerrain->m_sHeightMap.GetNormal(asTri[0].avVertex[2].vPos.x, asTri[0].avVertex[2].vPos.z) ;
	//int nTris = 1 ;

	decal.AddInBoundTriangles(asTri, nTris) ;
	SDecalVertex *pVertices = decal.GetVertices() ;
	SDecalIndex *pIndices = decal.GetIndices() ;
	int nVertexCount = decal.GetVertexCount() ;
	int nIndexCount = decal.GetIndexCount() ;

	static SCVertex vertex[1024] ;
	for(i=0 ; i<nVertexCount ; i++)
	{
		vertex[i].pos = pVertices[i].pos ;
		vertex[i].color = m_dwVertexColor ;
	}

	static unsigned short index[1024] ;
	int count=0 ;
	for(i=0 ; i<nIndexCount ; i++)
	{
		index[count++] = pIndices[i].awIndex[0] ;
		index[count++] = pIndices[i].awIndex[1] ;
		index[count++] = pIndices[i].awIndex[2] ;
	}

	DWORD dwFillMode ;
	m_pd3dDevice->GetRenderState(D3DRS_FILLMODE, &dwFillMode) ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	m_pd3dDevice->SetFVF(D3DFVF_DECALVERTEX) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SDecalVertex)) ;
	m_pd3dDevice->SetTexture(0, decal.GetTexture()) ;
	m_pd3dDevice->SetIndices(0) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, nVertexCount, decal.GetTriangleCount(), pIndices, D3DFMT_INDEX16, pVertices, sizeof(SDecalVertex)) ;	

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME) ;

	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, vtRoad, sizeof(SCVertex)) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, nVertexCount, nIndexCount, pIndices, D3DFMT_INDEX16, vertex, sizeof(SCVertex)) ;

	//m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_nNumCVertex-2, m_psCVertices, sizeof(SCVertex)) ;
	//m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, nStripCount-1, asVertices, sizeof(SCVertex)) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;

	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, dwFillMode) ;
}
*/
//#########################################################
//CSecretRoadManager
//#########################################################

CSecretRoadManager::CSecretRoadManager() : MAXNUM_ROAD(6)
{
	m_pd3dDevice = NULL ;
	m_pcTerrain = NULL ;
	m_pcPicking = NULL ;
	m_pcCurrentRoad = NULL ;
	m_pcTexContainer = NULL ;
	m_psRoadKinds = NULL ;
	m_nNumRoadKind = 0 ;
	m_bRender = false ;
}
CSecretRoadManager::~CSecretRoadManager()
{
	Release() ;
}
void CSecretRoadManager::Release()
{
	for(int i=0 ; i<m_cRoads.nCurPos ; i++)
		m_cRoads.GetAt(i)->Release() ;
	m_cRoads.Reset() ;
	SAFE_DELETE(m_pcTexContainer) ;
	SAFE_DELETEARRAY(m_psRoadKinds) ;
}
bool CSecretRoadManager::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretPicking *pcPicking)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcPicking = pcPicking ;

	m_cRoads.Initialize(MAXNUM_ROAD) ;

	m_pcTexContainer = new CSecretTextureContainer() ;
	m_pcTexContainer->Initialize("../../Media/map/Road/Roads.txr", pd3dDevice) ;

	m_nNumRoadKind = m_pcTexContainer->m_nNumTex ;
	m_psRoadKinds = new SRoadKind[m_nNumRoadKind] ;
	for(int i=0 ; i<m_nNumRoadKind ; i++)
		sprintf(m_psRoadKinds[i].szName, "%s", m_pcTexContainer->GetTextureName(i)) ;

	return true ;
}
void CSecretRoadManager::ResetCurrentRoad(float fWidth, int nDivision, char *pszRoadKind)
{
    if(m_pcCurrentRoad)
		m_pcCurrentRoad->Reset(fWidth, nDivision, m_pcTexContainer->FindTexture(pszRoadKind), pszRoadKind) ;
}
void CSecretRoadManager::Process(bool bPressedLButton)
{
	if(m_pcCurrentRoad)
		m_pcCurrentRoad->Process(bPressedLButton) ;
}
void CSecretRoadManager::Render()
{
	if(!m_bRender)
		return ;

	for(int i=0 ; i<m_cRoads.nCurPos ; i++)
		m_cRoads.GetAt(i)->AssistantRender() ;
}
void CSecretRoadManager::CreateRoad(char *pszName, float width, int nDivision, char *pszRoadKind)
{
	CSecretRoad cRoad ;
	cRoad.Initialize(m_pd3dDevice, m_pcTerrain, m_pcPicking, pszName, width, nDivision, m_pcTexContainer->FindTexture(pszRoadKind), pszRoadKind) ;
	m_cRoads.Insert(&cRoad) ;
}
CSecretRoad *CSecretRoadManager::SelectRoad(int nNum)
{
	m_pcCurrentRoad = m_cRoads.GetAt(nNum) ;
	return m_pcCurrentRoad ;
}
void CSecretRoadManager::DeleteRoad(int nNum)
{
	CSecretRoad *pcRoad = m_cRoads.Delete(nNum) ;
	if(pcRoad)
	{
		pcRoad->Release() ;
		if(pcRoad == m_pcCurrentRoad)
			m_pcCurrentRoad = NULL ;
	}
}
void CSecretRoadManager::SetTerrain(CSecretTerrain *pcTerrain)
{
	m_pcTerrain = pcTerrain ;
}
void CSecretRoadManager::AlignOnTerrain()
{
	if(m_pcCurrentRoad)
		m_pcCurrentRoad->AlignOnTerrain() ;
}
bool CSecretRoadManager::Intersect(Vector3 vPos)
{
	vPos.y = 0 ;
	for(int i=0 ; i<m_cRoads.nCurPos ; i++)
	{
		if(m_cRoads.GetAt(i)->Intersect(vPos))
			return true ;
	}
	return false ;
}
bool CSecretRoadManager::getDropPosition(Vector3 vPos, Vector3 vTargetPos, Vector3 *pvDropPos)
{
	vPos.y = 0 ;
	for(int i=0 ; i<m_cRoads.nCurPos ; i++)
	{
		if(m_cRoads.GetAt(i)->getDropPosition(vPos, vTargetPos, pvDropPos))
			return true ;
	}
	return false ;
}

void CSecretRoadManager::ImportRoad(char *pszName, float width, int nDivision, char *pszRoadKind, SPLINEDATA *psSplineData, int nNumSplineData)
{
	CSecretRoad cRoad ;
	cRoad.Initialize(m_pd3dDevice, m_pcTerrain, m_pcPicking, pszName, width, nDivision, m_pcTexContainer->FindTexture(pszRoadKind), pszRoadKind) ;
	cRoad.GetSpline()->ImportSplineData(psSplineData, nNumSplineData) ;
	cRoad.GetSpline()->SetEnable() ;
	cRoad.BuildAssistant() ;
	cRoad.AlignOnTerrain() ;
	m_cRoads.Insert(&cRoad) ;
}
void CSecretRoadManager::ResetRoads()
{
	for(int i=0 ; i<m_cRoads.nCurPos ; i++)
		m_cRoads.GetAt(i)->Release() ;
	m_cRoads.Reset() ;
	m_pcCurrentRoad = NULL ;
}