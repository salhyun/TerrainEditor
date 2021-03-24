#include "SecretGizmoSystem.h"
#include "MathOrdinary.h"

DWORD gizmo_colorX=0xfffe0000, gizmo_colorY=0xff00fe00, gizmo_colorZ=0xff0000fe ;
DWORD gizmo_arrowX=0xffff0000, gizmo_arrowY=0xff00ff00, gizmo_arrowZ=0xff0000ff ;
DWORD gizmo_yallow=0xffffff00, gizmo_transparent_yallow=0x80ffff00, gizmo_black=0xff040404 ;
SCVertex gizmo_move_vertex[] ;
unsigned short gizmo_move_index[] ;

D3DXVECTOR3 gizmo_axisx_positions[] ;
D3DXVECTOR3 gizmo_axisy_positions[] ;
D3DXVECTOR3 gizmo_axisz_positions[] ;
SCVertex gizmo_axisx_arrow_vertex[] ;
SCVertex gizmo_axisy_arrow_vertex[] ;
SCVertex gizmo_axisz_arrow_vertex[] ;
SCIndex gizmo_axis_arrow_index[] ;
D3DXVECTOR3 gizmo_rotate_by_axisy_positions[] ;

SGizmoObject::SGizmoObject()
{
	nKind = SGizmoObject::KIND_MESH ;
	nAttr = 0 ;
	nNumVertex = nNumIndex = nNumPosition = 0 ;
	nNumSubmitVertex = nNumSubmitIndex = 0 ;
	psOriginVertices = psVertices = NULL ;
	psOriginIndices = psIndices = NULL ;
	pvOriginPositions = pvPositions = NULL ;
	vCenter.x = vCenter.y = vCenter.z = 0.0f ;
	fLineWidth = 1 ;
}
SGizmoObject::~SGizmoObject()
{
	SAFE_DELETEARRAY(psVertices) ;
	SAFE_DELETEARRAY(psOriginVertices) ;
	SAFE_DELETEARRAY(psIndices) ;
	SAFE_DELETEARRAY(psOriginIndices) ;
	SAFE_DELETEARRAY(pvPositions) ;
	SAFE_DELETEARRAY(pvOriginPositions) ;
}
void SGizmoObject::SetVertex(int nNum, SCVertex *_psVertices)
{
	nNumVertex = nNum ;
	psVertices = new SCVertex[nNumVertex] ;
	psOriginVertices = new SCVertex[nNumVertex] ;

	if(_psVertices)
	{
		D3DXVECTOR3 vtemp(0, 0, 0) ;
		for(int i=0 ; i<nNumVertex ; i++)
		{
			psVertices[i] = _psVertices[i] ;
			psOriginVertices[i] = _psVertices[i] ;
			vtemp = _psVertices[i].pos ;
		}
		vCenter.x = vtemp.x/(float)nNumVertex ;
		vCenter.y = vtemp.y/(float)nNumVertex ;
		vCenter.z = vtemp.z/(float)nNumVertex ;
	}
}
void SGizmoObject::SetIndex(int nNum, SCIndex *_psIndices)
{
	nNumIndex = nNum ;
	psIndices = new SCIndex[nNumIndex] ;
	psOriginIndices = new SCIndex[nNumIndex] ;

	if(_psIndices)
	{
		for(int i=0 ; i<nNumIndex ; i++)
		{
			psIndices[i] = _psIndices[i] ;
			psOriginIndices[i] = _psIndices[i] ;
		}
	}
}
void SGizmoObject::SetPositions(int nNum, D3DXVECTOR3 *_pvPositions)
{
	nNumPosition = nNum ;
	D3DXVECTOR3 vtemp(0, 0, 0) ;
	pvPositions = new D3DXVECTOR3[nNumPosition] ;
	pvOriginPositions = new D3DXVECTOR3[nNumPosition] ;
	if(_pvPositions)
	{
		for(int i=0 ; i<nNumPosition ; i++)
		{
			pvPositions[i] = _pvPositions[i] ;
			pvOriginPositions[i] = _pvPositions[i] ;
			vtemp += _pvPositions[i] ;
		}

		vCenter.x = vtemp.x/(float)nNumPosition ;
		vCenter.y = vtemp.y/(float)nNumPosition ;
		vCenter.z = vtemp.z/(float)nNumPosition ;
	}
	nKind = SGizmoObject::KIND_LINE ;
}
void SGizmoObject::Process()
{
}
int compare_DistfromCamera_greater_gizmoObject(const void *pv1, const void *pv2)//decreasing order 내림차순 (적은숫자 -> 큰숫자)
{
	SGizmoTrueRenderingObject *pObject1 = (SGizmoTrueRenderingObject *)pv1 ;
	SGizmoTrueRenderingObject *pObject2 = (SGizmoTrueRenderingObject *)pv2 ;

	float dist = pObject1->fDistfromCamera - pObject2->fDistfromCamera ;
    if(float_less(dist, 0.0f))
		return 1 ;
	else if(float_greater(dist, 0.0f))
		return -1 ;
	else
		return 0 ;
}
void SGizmoTrueRenderingObject::Process(D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatVP, geo::SPlane *psViewPlane, int nCSecretGizmoSystem)
{
	int i ;
	D3DXVECTOR3 v ;
	D3DXVECTOR4 vret ;
	D3DXMATRIX matScale, mat ;

	D3DXMatrixIdentity(&matScale) ;
    
	float fScale, fLineWidth ;
	mat = (*pmatWorld)*(*pmatVP) ;
	D3DXVec3Transform(&vret, &psObject->vCenter, &mat) ;
	fLineWidth = vret.w*psObject->fLineWidth/1000.0f ;
	fScale = vret.w/10.0f ;
	if(fScale <= 0.1f)
		fScale = 0.1f ;

	if(fLineWidth <= 0.001f)
		fLineWidth = 0.001f ;

	matScale._11 = matScale._22 = matScale._33 = fScale ;
	//matWorld = matScale*(*pmatWorld) ;

	if(psObject->nKind == SGizmoObject::KIND_LINE)
	{
		for(i=0 ; i<psObject->nNumPosition ; i++)
		{
			D3DXVec3Transform(&vret, &psObject->pvOriginPositions[i], pmatWorld) ;
			psObject->pvPositions[i].x = vret.x ;
			psObject->pvPositions[i].y = vret.y ;
			psObject->pvPositions[i].z = vret.z ;
		}
		psObject->nNumSubmitVertex = psObject->nNumVertex ;
		psObject->nNumSubmitIndex = psObject->nNumIndex ;

		DWORD dwColor = psObject->psOriginVertices[0].color ;
		if(psObject->nAttr & SGizmoObject::ATTR_SELECTED)
		{
			dwColor = gizmo_yallow ;
            psObject->nAttr &= (~SGizmoObject::ATTR_SELECTED) ;
		}

		BuildQuadList(psObject->pvPositions, psObject->nNumPosition, pvCamera, pvUp, pmatVP, psObject->psVertices, psObject->psIndices, fLineWidth, dwColor) ;

		if(nCSecretGizmoSystem == CSecretGizmoSystem::ROTATE)
		{
			for(i=0 ; i<psObject->nNumSubmitVertex ; i++)
			{
				if(psViewPlane->Classify(Vector3(psObject->psVertices[i].pos.x, psObject->psVertices[i].pos.y, psObject->psVertices[i].pos.z)) == geo::SPlane::SPLANE_BEHIND)
					psObject->psVertices[i].color &= (~0xff000000) ;
			}
		}
	}
	else if(psObject->nKind == SGizmoObject::KIND_MESH)
	{
		for(i=0 ; i<psObject->nNumVertex ; i++)
		{
			D3DXVec3Transform(&vret, &psObject->psOriginVertices[i].pos, pmatWorld) ;
			psObject->psVertices[i].pos.x = vret.x ;
			psObject->psVertices[i].pos.y = vret.y ;
			psObject->psVertices[i].pos.z = vret.z ;
		}
		psObject->nNumSubmitVertex = psObject->nNumVertex ;
		psObject->nNumSubmitIndex = psObject->nNumIndex ;
	}

	D3DXVec3Transform(&vret, &psObject->vCenter, pmatWorld) ;
	v.x = vret.x-pvCamera->x ;
	v.y = vret.y-pvCamera->y ;
	v.z = vret.z-pvCamera->z ;
    fDistfromCamera = D3DXVec3Length(&v) ;
}

float IntersectPickingLinetoAxis(geo::SLine *psPickingLine, geo::SLine *psAxisLine, Matrix4 *pmatWorld)
{
	Vector3 pos ;
	Vector4 v4 ;
	geo::SLine line ;

	pos = psAxisLine->GetEndPos() ;
	line.s = psAxisLine->s*(*pmatWorld) ;
	pos *= (*pmatWorld) ;
	line.d = (line.s-pos).Magnitude() ;
	//line.d = 1.0f ;

	v4.set(psAxisLine->v.x, psAxisLine->v.y, psAxisLine->v.z, 0) ;//방향벡터라서 이동은 뺀다.
	v4 = v4*(*pmatWorld) ;
	line.v.set(v4.x, v4.y, v4.z) ;
	line.v = line.v.Normalize() ;
    
	return DistLinetoLine(psPickingLine, &line) ;
}
SGizmoMoveGeometry::SGizmoMoveGeometry()
{
	sAxisX.set(Vector3(0, 0, 0), Vector3(1, 0, 0), 1) ;
	sAxisY.set(Vector3(0, 0, 0), Vector3(0, 1, 0), 1) ;
	sAxisZ.set(Vector3(0, 0, 0), Vector3(0, 0, 1), 1) ;

	sPlaneXY.set(Vector3(1, 1, 0), Vector3(0, 0, 1)) ;
	sPlaneXZ.set(Vector3(1, 0, 1), Vector3(0, 1, 0)) ;
	sPlaneYZ.set(Vector3(0, 1, 1), Vector3(1, 0, 0)) ;
    dwSelectedSubset = 0 ;
}
DWORD SGizmoMoveGeometry::IntersectSubset(geo::SLine *psLine, Matrix4 *pmatWorld)
{
	float fDistX, fDistY, fDistZ ;
	Matrix4 mat ;
	mat.m41 = pmatWorld->m41 ;
	mat.m42 = pmatWorld->m42 ;
	mat.m43 = pmatWorld->m43 ;

	dwSelectedSubset = 0 ;

	fDistX = IntersectPickingLinetoAxis(psLine, &sAxisX, &mat) ;
	fDistY = IntersectPickingLinetoAxis(psLine, &sAxisY, &mat) ;
	fDistZ = IntersectPickingLinetoAxis(psLine, &sAxisZ, &mat) ;

	if(fDistX < fDistY)
	{
		if(fDistX < fDistZ)
		{
			if(fDistX < 0.1f)
				dwSelectedSubset = GIZMOSUBSET_AXISX ;
		}
		else
		{
			if(fDistZ < 0.1f)
				dwSelectedSubset = GIZMOSUBSET_AXISZ ;
		}
	}
	else
	{
		if(fDistY < fDistZ)
		{
			if(fDistY < 0.1f)
				dwSelectedSubset = GIZMOSUBSET_AXISY ;
		}
		else
		{
			if(fDistZ < 0.1f)
				dwSelectedSubset = GIZMOSUBSET_AXISZ ;
		}
	}

    //g_cDebugMsg.SetDebugMsg(11, "dist x=%.02f y=%.02f z=%.02f", fDistX, fDistY, fDistZ) ;
	//g_cDebugMsg.SetDebugMsg(12, "selected subset=%d", dwSelectedSubset) ;

    return dwSelectedSubset ;
}
int SGizmoMoveGeometry::GetIntersectedPoswithSubset(geo::SLine *psLine, Matrix4 *pmatWorld, Vector3 *pvPos)
{
	int nIntersect = geo::INTERSECT_NONE ;
	Matrix4 mat ;
	mat.m41 = pmatWorld->m41 ;
	mat.m42 = pmatWorld->m42 ;
	mat.m43 = pmatWorld->m43 ;

	if(dwSelectedSubset == GIZMOSUBSET_AXISX)
	{
		nIntersect = IntersectLinetoPlane(*psLine, sPlaneXY.GetPlanebyTransform(&mat), *pvPos, false, false) ;
		//nIntersect = IntersectLinetoPlane(*psLine, sPlaneXZ, *pvPos, false) ;
	}
	else if(dwSelectedSubset == GIZMOSUBSET_AXISY)
	{
		nIntersect = IntersectLinetoPlane(*psLine, sPlaneXY.GetPlanebyTransform(&mat), *pvPos, false, false) ;
		//nIntersect = IntersectLinetoPlane(*psLine, sPlaneYZ, *pvPos, false) ;
	}
	else if(dwSelectedSubset == GIZMOSUBSET_AXISZ)
	{
		nIntersect = IntersectLinetoPlane(*psLine, sPlaneXZ.GetPlanebyTransform(&mat), *pvPos, false, false) ;
		//nIntersect = IntersectLinetoPlane(*psLine, sPlaneXZ, *pvPos, false) ;
	}

	return nIntersect ;
}

SGizmoRotateGeometry::SGizmoRotateGeometry()
{
	sAxisX.set(Vector3(0, 0, 0), Vector3(1, 0, 0), 1) ;
	sAxisY.set(Vector3(0, 0, 0), Vector3(0, 1, 0), 1) ;
	sAxisZ.set(Vector3(0, 0, 0), Vector3(0, 0, 1), 1) ;

	sPlaneXY.set(Vector3(1, 1, 0), Vector3(0, 0, 1)) ;
	sPlaneXZ.set(Vector3(1, 0, 1), Vector3(0, 1, 0)) ;
	sPlaneYZ.set(Vector3(0, 1, 1), Vector3(1, 0, 0)) ;
	dwSelectedSubset = 0 ;
}
DWORD SGizmoRotateGeometry::IntersectSubset(geo::SLine *psLine, geo::SPlane *psViewPlane, Matrix4 *pmatWorld)
{
	int nNearest=-1 ;
	float afDist[3] ;
	Vector3 avPos[3], vDir, vPos ;
	geo::SLine *psAxis ;
	Matrix4 mat ;
	vPos.x = mat.m41 = pmatWorld->m41 ;
	vPos.y = mat.m42 = pmatWorld->m42 ;
	vPos.z = mat.m43 = pmatWorld->m43 ;

	dwSelectedSubset = 0 ;

	IntersectLinetoPlane(*psLine, sPlaneXY.GetPlanebyTransform(&mat),  avPos[0], false, false) ;
	IntersectLinetoPlane(*psLine, sPlaneXZ.GetPlanebyTransform(&mat),  avPos[1], false, false) ;
	IntersectLinetoPlane(*psLine, sPlaneYZ.GetPlanebyTransform(&mat),  avPos[2], false, false) ;

	afDist[0] = fabs((avPos[0]-vPos).Magnitude()-1.0f) ;
	afDist[1] = fabs((avPos[1]-vPos).Magnitude()-1.0f) ;
	afDist[2] = fabs((avPos[2]-vPos).Magnitude()-1.0f) ;

	if((afDist[0] > 0.1f) && (afDist[1] > 0.1f) && (afDist[2] > 0.1f))
		return dwSelectedSubset ;
	
	if(afDist[0] < afDist[1])
	{
		if(afDist[0] < afDist[2])
		{
			if((afDist[0] < 0.1f) && (psViewPlane->Classify(avPos[0]) == geo::SPlane::SPLANE_INFRONT))
			{
				nNearest = 0 ;
				psAxis = &sAxisZ ;
				dwSelectedSubset = GIZMOSUBSET_ROTATEBYAXISZ ;
			}
		}
		else
		{
			if((afDist[2] < 0.1f) && (psViewPlane->Classify(avPos[2]) == geo::SPlane::SPLANE_INFRONT))
			{
				nNearest = 2 ;
				psAxis = &sAxisX ;
				dwSelectedSubset = GIZMOSUBSET_ROTATEBYAXISX ;
			}
		}
	}
	else
	{
		if(afDist[1] < afDist[2])
		{
			if((afDist[1] < 0.1f) && (psViewPlane->Classify(avPos[1]) == geo::SPlane::SPLANE_INFRONT))
			{
				nNearest = 1 ;
				psAxis = &sAxisY ;
				dwSelectedSubset = GIZMOSUBSET_ROTATEBYAXISY ;
			}
		}
		else
		{
			if((afDist[2] < 0.1f) && (psViewPlane->Classify(avPos[2]) == geo::SPlane::SPLANE_INFRONT))
			{
				nNearest = 2 ;
				psAxis = &sAxisX ;
				dwSelectedSubset = GIZMOSUBSET_ROTATEBYAXISX ;
			}
		}
	}

	if(nNearest < 0)
		return dwSelectedSubset ;

	//IntersectLinetoPlane(*psLine, sPlaneXZ.GetPlanebyTransform(&mat),  avPos[1], false) ;
	//afDist[1] = fabs((avPos[1]-vPos).Magnitude()-1.0f) ;
	//if(afDist[1] > 0.1f)
	//	return dwSelectedSubset ;
	//nNearest = 1 ;
	//psAxis = &sAxisY ;
	//dwSelectedSubset = GIZMOSUBSET_ROTATEBYAXISY ;

	//현재 기즈모 위치에서 떨어진 방향. 결과적으로 평면상에서의 방향이 된다.
	vDir = (avPos[nNearest]-vPos).Normalize() ;

	sRotatePlane.set(avPos[nNearest], psAxis->v) ;

	sTangentLine.s = avPos[nNearest] ;
	sTangentLine.v = vDir.cross(psAxis->v) ;// psAxis->v.cross(vDir) ;
	sTangentLine.d = 1.0f ;

	return dwSelectedSubset ;
}
int SGizmoRotateGeometry::GetIntersectedPoswithSubset(geo::SLine *psLine, Matrix4 *pmatWorld, Vector3 *pvPos)
{
	int nIntersect = geo::INTERSECT_NONE ;
	Matrix4 mat ;
	mat.m41 = pmatWorld->m41 ;
	mat.m42 = pmatWorld->m42 ;
	mat.m43 = pmatWorld->m43 ;

	nIntersect = IntersectLinetoPlane(*psLine, sRotatePlane, *pvPos, false, false) ;

	return nIntersect ;
}
//###############################//
//        SGizmoScrape           //
//###############################//
SGizmoScrape::SGizmoScrape()
{
	nStatus = SGizmoScrape::SCRAPE_IDLE ;
	fSpeed = 1 ;
}
void SGizmoScrape::ProcessStatus(bool bPressed)
{
	if(bPressed)
	{
		if(nStatus == SCRAPE_IDLE)
			nStatus = SCRAPE_START ;
		else if(nStatus == SCRAPE_START)
			nStatus = SCRAPE_SCRAPING ;
	}
	else
	{
		if((nStatus == SCRAPE_START) || (nStatus == SCRAPE_SCRAPING))
			nStatus = SCRAPE_END ;
		else if(nStatus == SCRAPE_END)
			nStatus = SCRAPE_IDLE ;
	}
}
void SGizmoScrape::Process(Vector3 vPos, int nIntersect)
{
	if(nStatus == SCRAPE_IDLE)
		return ;

	if(nIntersect == geo::INTERSECT_POINT)
	{
		if(nStatus == SCRAPE_START)
			vPrePos = vCurPos = vPos ;
		else if(nStatus == SCRAPE_SCRAPING || nStatus == SCRAPE_END)
		{
			vPrePos = vCurPos ;
			vCurPos = vPos ;
		}
	}
}

//###############################//
//        SGizmoSystem           //
//###############################//
CSecretGizmoSystem::CSecretGizmoSystem()
{
	m_pd3dDevice = NULL ;
	m_psOriginVertices = NULL ;
	m_psVertices = NULL ;
	m_pwLineIndices = NULL ;
	m_pwTriangleIndices = NULL ;
	nNumObject = 0 ;
	m_psGizmoObjects = NULL ;
	m_psGizmoTRObjects = NULL ;
}

CSecretGizmoSystem::~CSecretGizmoSystem()
{
	SAFE_DELETEARRAY(m_psOriginVertices) ;
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_pwLineIndices) ;
	SAFE_DELETEARRAY(m_pwTriangleIndices) ;
    SAFE_DELETEARRAY(m_psGizmoObjects) ;
	SAFE_DELETEARRAY(m_psGizmoTRObjects) ;
}

void CSecretGizmoSystem::Initialize(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice ;

	m_psOriginVertices = new SCVertex[MAXNUM_256] ;
	m_psVertices = new SCVertex[MAXNUM_256] ;
	m_pwLineIndices = new unsigned short[MAXNUM_256] ;
	m_pwTriangleIndices = new unsigned short[MAXNUM_256] ;

	m_nType = CSecretGizmoSystem::MOVE ;
    D3DXMatrixIdentity(&m_matWorld) ;
	D3DXMatrixIdentity(&m_matOrigin) ;

	int n ;
	nNumObject = 9 ;
	m_psGizmoObjects = new SGizmoObject[nNumObject] ;
	m_psGizmoTRObjects = new SGizmoTrueRenderingObject[nNumObject] ;

	n=0 ;//axis-x
    m_psGizmoObjects[n].SetVertex(4) ;
	m_psGizmoObjects[n].SetIndex(2) ;
	m_psGizmoObjects[n].SetPositions(2, gizmo_axisx_positions) ;
	m_psGizmoObjects[n].psOriginVertices[0].color = gizmo_colorX ;
	m_psGizmoObjects[n].nAttr = SGizmoObject::ATTR_BASEAXIS|SGizmoObject::ATTR_MOVE ;
	m_psGizmoTRObjects[n].psObject = &m_psGizmoObjects[n] ;
	m_psGizmoTRObjects[n].pmatWorld = &m_matWorld ;

	n=1 ;//axis-y
	m_psGizmoObjects[n].SetVertex(4) ;
	m_psGizmoObjects[n].SetIndex(2) ;
	m_psGizmoObjects[n].SetPositions(2, gizmo_axisy_positions) ;
	m_psGizmoObjects[n].psOriginVertices[0].color = gizmo_colorY ;
	m_psGizmoObjects[n].nAttr = SGizmoObject::ATTR_BASEAXIS|SGizmoObject::ATTR_MOVE ;
	m_psGizmoTRObjects[n].psObject = &m_psGizmoObjects[n] ;
	m_psGizmoTRObjects[n].pmatWorld = &m_matWorld ;

	n=2 ;//axis-z
	m_psGizmoObjects[n].SetVertex(4) ;
	m_psGizmoObjects[n].SetIndex(2) ;
	m_psGizmoObjects[n].SetPositions(2, gizmo_axisz_positions) ;
	m_psGizmoObjects[n].psOriginVertices[0].color = gizmo_colorZ ;
	m_psGizmoObjects[n].nAttr = SGizmoObject::ATTR_BASEAXIS|SGizmoObject::ATTR_MOVE ;
	m_psGizmoTRObjects[n].psObject = &m_psGizmoObjects[n] ;
	m_psGizmoTRObjects[n].pmatWorld = &m_matWorld ;

	n=3 ;//arrow axis-x
	m_psGizmoObjects[n].SetVertex(9, gizmo_axisx_arrow_vertex) ;
	m_psGizmoObjects[n].SetIndex(6, gizmo_axis_arrow_index) ;
	m_psGizmoObjects[n].nAttr = SGizmoObject::ATTR_MOVE ;
	m_psGizmoTRObjects[n].psObject = &m_psGizmoObjects[n] ;
	m_psGizmoTRObjects[n].pmatWorld = &m_matWorld ;

	n=4 ;//arrow axis-y
	m_psGizmoObjects[n].SetVertex(9, gizmo_axisy_arrow_vertex) ;
	m_psGizmoObjects[n].SetIndex(6, gizmo_axis_arrow_index) ;
	m_psGizmoObjects[n].nAttr = SGizmoObject::ATTR_MOVE ;
	m_psGizmoTRObjects[n].psObject = &m_psGizmoObjects[n] ;
	m_psGizmoTRObjects[n].pmatWorld = &m_matWorld ;

	n=5 ;//arrow axis-z
	m_psGizmoObjects[n].SetVertex(9, gizmo_axisz_arrow_vertex) ;
	m_psGizmoObjects[n].SetIndex(6, gizmo_axis_arrow_index) ;
	m_psGizmoObjects[n].nAttr = SGizmoObject::ATTR_MOVE ;
	m_psGizmoTRObjects[n].psObject = &m_psGizmoObjects[n] ;
	m_psGizmoTRObjects[n].pmatWorld = &m_matWorld ;

	int i, nNumPosition = 33 ;// two_pi/32.0f ;
	D3DXVECTOR3 pos[33] ;
	float add = two_pi/32.0f, t=0 ;

	for(i=0 ; i<nNumPosition ; i++, t+=add)
	{
		pos[i].x = 0 ;
		pos[i].y = sinf(t) ;
		pos[i].z = cosf(t) ;		
	}

	n=6 ;//rotate by axis-x
	m_psGizmoObjects[n].SetVertex((nNumPosition*4)-4) ;
	m_psGizmoObjects[n].SetIndex((nNumPosition-1)*2) ;

	m_psGizmoObjects[n].SetPositions(nNumPosition, pos) ;
	m_psGizmoObjects[n].psOriginVertices[0].color = gizmo_colorX ;
	m_psGizmoObjects[n].nAttr = SGizmoObject::ATTR_ROTATE ;

	m_psGizmoTRObjects[n].psObject = &m_psGizmoObjects[n] ;
	m_psGizmoTRObjects[n].pmatWorld = &m_matWorld ;

	for(i=0 ; i<nNumPosition ; i++, t+=add)
	{
		pos[i].x = cosf(t) ;
		pos[i].z = sinf(t) ;
		pos[i].y = 0 ;
	}

	n=7 ;//rotate by axis-y
	m_psGizmoObjects[n].SetVertex((nNumPosition*4)-4) ;
	m_psGizmoObjects[n].SetIndex((nNumPosition-1)*2) ;

	m_psGizmoObjects[n].SetPositions(nNumPosition, pos) ;
	m_psGizmoObjects[n].psOriginVertices[0].color = gizmo_colorY ;
	m_psGizmoObjects[n].nAttr = SGizmoObject::ATTR_ROTATE ;

	m_psGizmoTRObjects[n].psObject = &m_psGizmoObjects[n] ;
	m_psGizmoTRObjects[n].pmatWorld = &m_matWorld ;

	for(i=0 ; i<nNumPosition ; i++, t+=add)
	{
		pos[i].x = cosf(t) ;
		pos[i].y = sinf(t) ;
		pos[i].z = 0 ;
	}

	n=8 ;//rotate by axis-z
	m_psGizmoObjects[n].SetVertex((nNumPosition*4)-4) ;
	m_psGizmoObjects[n].SetIndex((nNumPosition-1)*2) ;

	m_psGizmoObjects[n].SetPositions(nNumPosition, pos) ;
	m_psGizmoObjects[n].psOriginVertices[0].color = gizmo_colorZ ;
	m_psGizmoObjects[n].nAttr = SGizmoObject::ATTR_ROTATE ;

	m_psGizmoTRObjects[n].psObject = &m_psGizmoObjects[n] ;
	m_psGizmoTRObjects[n].pmatWorld = &m_matWorld ;
}

void CSecretGizmoSystem::SetType(int nType)
{
	if(m_nType == CSecretGizmoSystem::MOVE)
	{
		m_sGizmoMoveGeometry.dwSelectedSubset = 0 ;
	}
	else if(m_nType == CSecretGizmoSystem::ROTATE)
	{
		m_sGizmoRotateGeometry.dwSelectedSubset = 0 ;
	}
	else if(m_nType == CSecretGizmoSystem::SCALE)
	{
	}
	m_sScrape.nStatus = SGizmoScrape::SCRAPE_IDLE ;

	m_nType = nType ;
	if(m_nType == CSecretGizmoSystem::MOVE)
	{
		int i ;
		m_nNumVertex = 42 ;

		memcpy(m_psOriginVertices, gizmo_move_vertex, sizeof(SCVertex)*m_nNumVertex) ;
		memcpy(m_pwLineIndices, gizmo_move_index, sizeof(unsigned short)*18) ;
		memcpy(m_pwTriangleIndices, &gizmo_move_index[18], sizeof(unsigned short)*54) ;

		for(i=0 ; i<54 ; i++)
			m_pwTriangleIndices[i] += 9 ;

		m_nLineMinVertex = 0 ;
        m_nLineNumVertex = 15 ;
		m_nLineCount = 9 ;

		m_nTriangleMinVertex = 0 ;
		m_nTriangleNumVertex = 42 ;
		m_nTriangleCount = 18 ;

		m_fPreScale = m_fCurScale = 1.0f ;

		for(i=0 ; i<m_nNumVertex ; i++)
			m_psVertices[i] = m_psOriginVertices[i] ;
	}
	else if(m_nType == CSecretGizmoSystem::ROTATE)
	{
	}
	else if(m_nType == CSecretGizmoSystem::SCALE)
	{
	}
}

int CSecretGizmoSystem::GetType()
{
	return m_nType ;
}

void CSecretGizmoSystem::SetmatWorld(D3DXMATRIX *pmatWorld)
{
    m_matWorld = (*pmatWorld) ;
	m_matOrigin = (*pmatWorld) ;
}

void CSecretGizmoSystem::Process(Vector3 *pvCamera, geo::SLine *psPickingLine, D3DXMATRIX *pmatV, D3DXMATRIX *pmatP, bool bPressed)
{
	if(!m_bEnable)
		return ;

	D3DXVECTOR3 vCamera(pvCamera->x, pvCamera->y, pvCamera->z) ;
	D3DXVECTOR3 vCenter ;
	vCenter.x = m_matWorld._41 ;
	vCenter.y = m_matWorld._42 ;
	vCenter.z = m_matWorld._43 ;
    vCenter = vCamera-vCenter ;
	D3DXVec3Normalize(&vCenter, &vCenter) ;
	m_sViewPlane.set(Vector3(m_matWorld._41, m_matWorld._42, m_matWorld._43), Vector3(vCenter.x, vCenter.y, vCenter.z)) ;

	D3DXVECTOR3 vUp = D3DXVECTOR3(pmatV->_21, pmatV->_22, pmatV->_23) ;
	D3DXVec3Normalize(&vUp, &vUp) ;
	D3DXMATRIX matVP = (*pmatV)*(*pmatP) ;

	//D3DXVECTOR4 vProj ;
	//D3DXVECTOR3 vPos(m_matWorld._41, m_matWorld._42, m_matWorld._43) ;
	//D3DXVec3Transform(&vProj, &vPos, &matVP) ;
	//m_fCurScale = vProj.w/10.0f ;

	//D3DXMATRIX matScale ;
	//D3DXMatrixIdentity(&matScale) ;
	//matScale._11 = matScale._22 = matScale._33 = 1.0f+(m_fCurScale-m_fPreScale) ;
	//m_fPreScale = m_fCurScale ;

	//m_matWorld = matScale*m_matWorld ;

	Matrix4 mat = MatrixConvert(m_matWorld) ;

	if(m_nType == CSecretGizmoSystem::MOVE)
	{
		if(m_sScrape.nStatus == SGizmoScrape::SCRAPE_IDLE)//마우스 왼쪽버튼을 누르지 않았을 경우 체크를 한다.
			m_sGizmoMoveGeometry.IntersectSubset(psPickingLine, &mat) ;

		if(m_sGizmoMoveGeometry.dwSelectedSubset)
		{
			if(m_sGizmoMoveGeometry.dwSelectedSubset & GIZMOSUBSET_AXISX)
			{
				m_psGizmoObjects[0].nAttr |= SGizmoObject::ATTR_SELECTED ;
			}
			else if(m_sGizmoMoveGeometry.dwSelectedSubset & GIZMOSUBSET_AXISY)
			{
				m_psGizmoObjects[1].nAttr |= SGizmoObject::ATTR_SELECTED ;
			}
			else if(m_sGizmoMoveGeometry.dwSelectedSubset & GIZMOSUBSET_AXISZ)
			{
				m_psGizmoObjects[2].nAttr |= SGizmoObject::ATTR_SELECTED ;
			}

			m_sScrape.ProcessStatus(bPressed) ;

			if((m_sScrape.nStatus > SGizmoScrape::SCRAPE_IDLE))
			{
				int nIntersect = geo::INTERSECT_NONE ;
				Vector3 vPos ;

				Matrix4 matOrigin = MatrixConvert(m_matOrigin) ;
				nIntersect = m_sGizmoMoveGeometry.GetIntersectedPoswithSubset(psPickingLine, &matOrigin, &vPos) ;
				g_cDebugMsg.SetDebugMsg(5, "picking pos(%07.03f %07.03f %07.03f)", enumVector(vPos)) ;

				if(m_sScrape.nStatus == SGizmoScrape::SCRAPE_START)
					m_sGizmoMoveGeometry.vFirstPos = vPos ;

				m_sScrape.Process(vPos, nIntersect) ;
				m_sScrape.vAmount = m_sScrape.vCurPos-m_sScrape.vPrePos ;
				if(m_sGizmoMoveGeometry.dwSelectedSubset & GIZMOSUBSET_AXISX)
				{
					//m_matWorld._41 += m_sScrape.vAmount.x ;
					m_matWorld._41 = m_matOrigin._41+vPos.x-m_sGizmoMoveGeometry.vFirstPos.x ;
				}
				else if(m_sGizmoMoveGeometry.dwSelectedSubset & GIZMOSUBSET_AXISY)
				{
					//m_matWorld._42 += m_sScrape.vAmount.y ;
					m_matWorld._42 = m_matOrigin._42+vPos.y-m_sGizmoMoveGeometry.vFirstPos.y ;
				}
				else if(m_sGizmoMoveGeometry.dwSelectedSubset & GIZMOSUBSET_AXISZ)
				{
					//m_matWorld._43 += m_sScrape.vAmount.z ;
					m_matWorld._43 = m_matOrigin._43+vPos.z-m_sGizmoMoveGeometry.vFirstPos.z ;
				}

				if(m_sScrape.nStatus == SGizmoScrape::SCRAPE_END)//마지막에는 오리진행렬에도 적용해준다.
					m_matOrigin = m_matWorld ;
			}
		}
	}
	else if(m_nType == CSecretGizmoSystem::ROTATE)
	{
		if(m_sScrape.nStatus == SGizmoScrape::SCRAPE_IDLE)//마우스 왼쪽버튼을 누르지 않았을 경우 체크를 한다.
			m_sGizmoRotateGeometry.IntersectSubset(psPickingLine, &m_sViewPlane, &mat) ;
		if(m_sGizmoRotateGeometry.dwSelectedSubset)
		{
			if(m_sGizmoRotateGeometry.dwSelectedSubset & GIZMOSUBSET_ROTATEBYAXISX)
				m_psGizmoObjects[6].nAttr |= SGizmoObject::ATTR_SELECTED ;
			else if(m_sGizmoRotateGeometry.dwSelectedSubset & GIZMOSUBSET_ROTATEBYAXISY)
				m_psGizmoObjects[7].nAttr |= SGizmoObject::ATTR_SELECTED ;
			else if(m_sGizmoRotateGeometry.dwSelectedSubset & GIZMOSUBSET_ROTATEBYAXISZ)
				m_psGizmoObjects[8].nAttr |= SGizmoObject::ATTR_SELECTED ;

			m_sScrape.ProcessStatus(bPressed) ;

			if((m_sScrape.nStatus > SGizmoScrape::SCRAPE_START))
			{
				int nIntersect = geo::INTERSECT_NONE ;
				float fAngle ;
				Vector3 vPos, vv ;
				Matrix4 matOrigin = MatrixConvert(m_matOrigin) ;
				nIntersect = m_sGizmoRotateGeometry.GetIntersectedPoswithSubset(psPickingLine, &mat, &vPos) ;
				//vPos = ProjectionVector((vPos-m_sGizmoRotateGeometry.sTangentLine.s), m_sGizmoRotateGeometry.sTangentLine.v, false) ;
				//fAngle = vPos.GreatestElement() ;
				vv = (vPos-m_sGizmoRotateGeometry.sTangentLine.s) ;

				fAngle = -(vPos-m_sGizmoRotateGeometry.sTangentLine.s).dot(m_sGizmoRotateGeometry.sTangentLine.v) ;

				g_cDebugMsg.SetDebugMsg(4, "tangent(%07.03f %07.03f %07.03f)", enumVector(m_sGizmoRotateGeometry.sTangentLine.v)) ;
				g_cDebugMsg.SetDebugMsg(5, "vPos(%07.03f %07.03f %07.03f)", enumVector(vv)) ;
				g_cDebugMsg.SetDebugMsg(6, "angle=%g", fAngle) ;

				if(m_sGizmoRotateGeometry.dwSelectedSubset & GIZMOSUBSET_ROTATEBYAXISX)
				{
					D3DXMatrixRotationX(&m_sGizmoRotateGeometry.matRotate, fAngle) ;
				}
				else if(m_sGizmoRotateGeometry.dwSelectedSubset & GIZMOSUBSET_ROTATEBYAXISY)
				{
					D3DXMatrixRotationY(&m_sGizmoRotateGeometry.matRotate, fAngle) ;
				}
				else if(m_sGizmoRotateGeometry.dwSelectedSubset & GIZMOSUBSET_ROTATEBYAXISZ)
				{
					D3DXMatrixRotationZ(&m_sGizmoRotateGeometry.matRotate, fAngle) ;
				}
			}
		}
	}

	//g_cDebugMsg.SetDebugMsg(6, "gizmo pos(%07.03f %07.03f %07.03f)", m_matWorld._41, m_matWorld._42, m_matWorld._43) ;

	int i ;
	//D3DXVECTOR4 pos ;
	//DWORD color ;
	//for(i=0 ; i<m_nNumVertex ; i++)
	//{
	//	D3DXVec3Transform(&pos, &m_psOriginVertices[i].pos, &m_matWorld) ;
	//	m_psVertices[i].pos = D3DXVECTOR3(pos.x, pos.y, pos.z) ;

	//	color = m_psOriginVertices[i].color ;
	//	if(m_sGizmoMoveGeometry.dwSelectedSubset & GIZMOSUBSET_AXISX)
	//	{
	//		if(m_psOriginVertices[i].color == gizmo_colorX)
	//			color = gizmo_yallow ;
	//	}
	//	if(m_sGizmoMoveGeometry.dwSelectedSubset & GIZMOSUBSET_AXISY)
	//	{
	//		if(m_psOriginVertices[i].color == gizmo_colorY)
	//			color = gizmo_yallow ;
	//	}
	//	if(m_sGizmoMoveGeometry.dwSelectedSubset & GIZMOSUBSET_AXISZ)
	//	{
	//		if(m_psOriginVertices[i].color == gizmo_colorZ)
	//			color = gizmo_yallow ;
	//	}

	//	m_psVertices[i].color = color ;// m_psOriginVertices[i].color ;
	//}

	SGizmoObject *psObject ;
	for(i=0 ; i<nNumObject ; i++)
	{
		psObject = m_psGizmoTRObjects[i].psObject ;
		if(m_nType == CSecretGizmoSystem::MOVE)
		{
			if(psObject->nAttr & SGizmoObject::ATTR_MOVE)
				m_psGizmoTRObjects[i].Process(&vCamera, &vUp, &matVP, &m_sViewPlane, m_nType) ;
		}
		else if(m_nType == CSecretGizmoSystem::ROTATE)
		{
			if(psObject->nAttr & SGizmoObject::ATTR_ROTATE)
				m_psGizmoTRObjects[i].Process(&vCamera, &vUp, &matVP, &m_sViewPlane, m_nType) ;
		}
	}

	qsort(m_psGizmoTRObjects, nNumObject, sizeof(SGizmoTrueRenderingObject), compare_DistfromCamera_greater_gizmoObject) ;
}
bool CSecretGizmoSystem::IsSelectedSubset()
{
	if(m_nType == CSecretGizmoSystem::MOVE)
	{
		if(m_sGizmoMoveGeometry.dwSelectedSubset)
			return true ;
	}
	else if(m_nType == CSecretGizmoSystem::ROTATE)
	{
		if(m_sGizmoRotateGeometry.dwSelectedSubset)
			return true ;
	}
	return false ;
}
void CSecretGizmoSystem::Render()
{
	if(!m_bEnable)
		return ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	DWORD dwZEnable, dwAlphaTest ;
	m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;

	m_pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTest) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00) ;
    m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	SGizmoObject *psObject ;
	for(int i=0 ; i<nNumObject ; i++)
	{
		psObject = m_psGizmoTRObjects[i].psObject ;

		if(psObject->nNumSubmitIndex == 0 || psObject->nNumSubmitVertex == 0)
			continue ;

		if(m_nType == CSecretGizmoSystem::MOVE)
		{
			if(psObject->nAttr & SGizmoObject::ATTR_MOVE)
				m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, psObject->nNumSubmitVertex, psObject->nNumSubmitIndex, psObject->psIndices, D3DFMT_INDEX16, psObject->psVertices, sizeof(psObject->psVertices[0])) ;
		}
		else if(m_nType == CSecretGizmoSystem::ROTATE)
		{
			if(psObject->nAttr & SGizmoObject::ATTR_ROTATE)
				m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, psObject->nNumSubmitVertex, psObject->nNumSubmitIndex, psObject->psIndices, D3DFMT_INDEX16, psObject->psVertices, sizeof(psObject->psVertices[0])) ;
		}
	}

	//m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, m_nLineMinVertex, m_nLineNumVertex, m_nLineCount, m_pwLineIndices, D3DFMT_INDEX16, m_psVertices, sizeof(m_psVertices[0])) ;
	//m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, m_nTriangleMinVertex, m_nTriangleNumVertex, m_nTriangleCount, m_pwTriangleIndices, D3DFMT_INDEX16, m_psVertices, sizeof(m_psVertices[0])) ;

	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTest) ;
	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;
}

D3DXVECTOR3 gizmo_axisx_positions[] = 
{
	D3DXVECTOR3(0, 0, 0),
	D3DXVECTOR3(1, 0, 0),
} ;
D3DXVECTOR3 gizmo_axisy_positions[] = 
{
	D3DXVECTOR3(0, 0, 0),
	D3DXVECTOR3(0, 1, 0),
} ;
D3DXVECTOR3 gizmo_axisz_positions[] = 
{
	D3DXVECTOR3(0, 0, 0),
	D3DXVECTOR3(0, 0, 1),
} ;
SCVertex gizmo_axisx_arrow_vertex[] = 
{
	D3DXVECTOR3(1.3f, 0, 0), gizmo_arrowX,//15
	D3DXVECTOR3(1, 0.1f, 0), gizmo_arrowX,
	D3DXVECTOR3(1, 0, -0.1f), gizmo_arrowX,
	D3DXVECTOR3(1, -0.1f, 0), gizmo_arrowX,
	D3DXVECTOR3(1, 0, 0.1f), gizmo_arrowX,

	D3DXVECTOR3(1, 0.1f, 0), gizmo_black,//30
	D3DXVECTOR3(1, 0, -0.1f), gizmo_black,
	D3DXVECTOR3(1, -0.1f, 0), gizmo_black,
	D3DXVECTOR3(1, 0, 0.1f), gizmo_black,
} ;
SCVertex gizmo_axisy_arrow_vertex[] = 
{
	D3DXVECTOR3(0, 1.3f, 0), gizmo_arrowY,//20
	D3DXVECTOR3(-0.1f, 1, 0), gizmo_arrowY,
	D3DXVECTOR3(0, 1, -0.1f), gizmo_arrowY,
	D3DXVECTOR3(0.1f, 1, 0), gizmo_arrowY,
	D3DXVECTOR3(0, 1, 0.1f), gizmo_arrowY,

	D3DXVECTOR3(-0.1f, 1, 0), gizmo_black,//34
	D3DXVECTOR3(0, 1, -0.1f), gizmo_black,
	D3DXVECTOR3(0.1f, 1, 0), gizmo_black,
	D3DXVECTOR3(0, 1, 0.1f), gizmo_black,
} ;
SCVertex gizmo_axisz_arrow_vertex[] = 
{
	D3DXVECTOR3(0, 0, 1.3f), gizmo_arrowZ,//25
	D3DXVECTOR3(0, 0.1f, 1), gizmo_arrowZ,
	D3DXVECTOR3(0.1f, 0, 1), gizmo_arrowZ,
	D3DXVECTOR3(0, -0.1f, 1), gizmo_arrowZ,
	D3DXVECTOR3(-0.1f, 0, 1), gizmo_arrowZ,

	D3DXVECTOR3(0, 0.1f, 1), gizmo_black,//38
	D3DXVECTOR3(0.1f, 0, 1), gizmo_black,
	D3DXVECTOR3(0, -0.1f, 1), gizmo_black,
	D3DXVECTOR3(-0.1f, 0, 1), gizmo_black,
} ;
SCIndex gizmo_axis_arrow_index[] =
{
	SCIndex(0, 2, 1),
	SCIndex(0, 3, 2),
	SCIndex(0, 4, 3),
	SCIndex(0, 1, 4),

    SCIndex(5, 6, 7),
	SCIndex(5, 7, 8),
} ;

D3DXVECTOR3 gizmo_rotate_by_axisy_positions[] =
{
	D3DXVECTOR3(      0.3f, 0.0f,       0.0f),
	D3DXVECTOR3(0.2942356f, 0.0f, 0.0585271f),
	D3DXVECTOR3(0.2771639f, 0.0f, 0.1148050f),
	D3DXVECTOR3(0.2494409f, 0.0f, 0.1666710f),
	D3DXVECTOR3(0.2121321f, 0.0f, 0.2121320f),
	D3DXVECTOR3(0.1666711f, 0.0f, 0.2494408f),
	D3DXVECTOR3(0.1148051f, 0.0f, 0.2771638f),
	D3DXVECTOR3(0.0585272f, 0.0f, 0.2942356f),
	D3DXVECTOR3(      0.0f, 0.0f,       0.3f)
} ;

SCVertex gizmo_move_vertex[] = 
{
		//line
		D3DXVECTOR3(0, 0, 0), gizmo_colorX,
		D3DXVECTOR3(0, 0, 0), gizmo_colorY,
		D3DXVECTOR3(0, 0, 0), gizmo_colorZ,
		D3DXVECTOR3(1, 0, 0), gizmo_colorX,
		D3DXVECTOR3(0, 1, 0), gizmo_colorY,
		D3DXVECTOR3(0, 0, 1), gizmo_colorZ,

		D3DXVECTOR3(0.3f, 0, 0), gizmo_colorX,//6
		D3DXVECTOR3(0.3f, 0.3f, 0), gizmo_colorX,
		D3DXVECTOR3(0.3f, 0, 0.3f), gizmo_colorX,

		D3DXVECTOR3(0, 0.3f, 0), gizmo_colorY,//9
		D3DXVECTOR3(0.3f, 0.3f, 0), gizmo_colorY,
		D3DXVECTOR3(0, 0.3f, 0.3f), gizmo_colorY,

		D3DXVECTOR3(0, 0, 0.3f), gizmo_colorZ,//12
		D3DXVECTOR3(0.3f, 0, 0.3f), gizmo_colorZ,
		D3DXVECTOR3(0, 0.3f, 0.3f), gizmo_colorZ,

        //triangle
		D3DXVECTOR3(1.3f, 0, 0), gizmo_arrowX,//15
		D3DXVECTOR3(1, 0.1f, 0), gizmo_arrowX,
		D3DXVECTOR3(1, 0, -0.1f), gizmo_arrowX,
		D3DXVECTOR3(1, -0.1f, 0), gizmo_arrowX,
		D3DXVECTOR3(1, 0, 0.1f), gizmo_arrowX,

		D3DXVECTOR3(0, 1.3f, 0), gizmo_arrowY,//20
		D3DXVECTOR3(-0.1f, 1, 0), gizmo_arrowY,
		D3DXVECTOR3(0, 1, -0.1f), gizmo_arrowY,
		D3DXVECTOR3(0.1f, 1, 0), gizmo_arrowY,
		D3DXVECTOR3(0, 1, 0.1f), gizmo_arrowY,

		D3DXVECTOR3(0, 0, 1.3f), gizmo_arrowZ,//25
		D3DXVECTOR3(0, 0.1f, 1), gizmo_arrowZ,
		D3DXVECTOR3(0.1f, 0, 1), gizmo_arrowZ,
		D3DXVECTOR3(0, -0.1f, 1), gizmo_arrowZ,
		D3DXVECTOR3(-0.1f, 0, 1), gizmo_arrowZ,

		D3DXVECTOR3(1, 0.1f, 0), gizmo_black,//30
		D3DXVECTOR3(1, 0, -0.1f), gizmo_black,
		D3DXVECTOR3(1, -0.1f, 0), gizmo_black,
		D3DXVECTOR3(1, 0, 0.1f), gizmo_black,

		D3DXVECTOR3(-0.1f, 1, 0), gizmo_black,//34
		D3DXVECTOR3(0, 1, -0.1f), gizmo_black,
		D3DXVECTOR3(0.1f, 1, 0), gizmo_black,
		D3DXVECTOR3(0, 1, 0.1f), gizmo_black,

		D3DXVECTOR3(0, 0.1f, 1), gizmo_black,//38
		D3DXVECTOR3(0.1f, 0, 1), gizmo_black,
		D3DXVECTOR3(0, -0.1f, 1), gizmo_black,
		D3DXVECTOR3(-0.1f, 0, 1), gizmo_black,
} ;//number=42

unsigned short gizmo_move_index[] =
{
		0, 3,
		1, 4,
		2, 5,

        6, 7,
		6, 8,

		9, 10,
		9, 11,

		12, 13,
        12, 14,

		6, 8, 7,//18
		6, 7, 10,
		6, 10, 9,
		6, 9, 8,
		23, 24, 21,
		21, 22, 23,

		11, 13, 12,//36
		11, 12, 15,
		11, 15, 14,
		11, 14, 13,
		27, 28, 25,
		25, 26, 27,

		16, 18, 17,//54
		16, 17, 20,
		16, 20, 19,
		16, 19, 18,
		31, 32, 29,
		29, 30, 31
} ;//number:72
