#include "def.h"
#include "SecretDecal.h"

//#define OUTPUT_RENDERDATA

CSecretDecal::CSecretDecal()
{
	m_pd3dDevice = NULL ;
	m_fWidth = m_fHeight = 0 ;
	m_pVertices = NULL ;
	m_pIndices = NULL ;
	m_nMaxNumVertex = m_nMaxNumIndex = 0 ;
	m_nKind = KIND::POLYGON ;
	//m_fNormalOffset = 0.005f;
	m_fNormalOffset = 0.05f;
	m_pTex = NULL ;
}

CSecretDecal::~CSecretDecal()
{
	Release() ;
}

void CSecretDecal::Release()
{
	SAFE_RELEASE(m_pTex) ;
	SAFE_DELETEARRAY(m_pVertices) ;
	SAFE_DELETEARRAY(m_pIndices) ;
}

void CSecretDecal::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, Vector3 &vNormal, Vector3 &vTangent, float width, float height, float depth)
{
	m_pd3dDevice = pd3dDevice ;
	m_vPos.set(-0.75f, 0, 0.25f) ;
	//m_vPos.set(1, 0, 1) ;
	m_vNormal = vNormal ;
	m_vTangent = vTangent ;
	m_vBinormal = m_vTangent.cross(m_vNormal) ;
	m_fWidth = width ;
	m_fHeight = height ;
	m_fRadius = sqrtf((width*width)*0.5f + (height*height)*0.5f) ;
	m_fDepth = depth ;
	m_pVertices = new SDecalVertex[MAXNUM_DECALVERTEX] ;
	m_pIndices = new SDecalIndex[MAXNUM_DECALINDEX] ;
	m_nMaxNumVertex = MAXNUM_DECALVERTEX ;
	m_nMaxNumIndex = MAXNUM_DECALINDEX ;
	m_nVertexCount = m_nIndexCount = 0 ;
	MakingBoundingPlanes() ;
	m_avTexFactor[0].set(0, 0) ;
	m_avTexFactor[1].set(1, 0) ;
	m_avTexFactor[2].set(1, 1) ;
	m_avTexFactor[3].set(0, 1) ;

	//if(FAILED(D3DXCreateTextureFromFile(m_pd3dDevice, "../../Media/love.tga", &m_pTex)))
	//{
	//	TRACE("Cannot load Texture path=../../Media/love.tga\r\n") ;
	//	assert(false && "Cannot load TextureFile") ;
	//}
}

void CSecretDecal::ReAllocateVB(int nNumVertex)
{
	SAFE_DELETEARRAY(m_pVertices) ;
	m_pVertices = new SDecalVertex[nNumVertex] ;
	m_nMaxNumVertex = nNumVertex ;
}
void CSecretDecal::ReAllocateIB(int nNumIndex)
{
	SAFE_DELETEARRAY(m_pIndices) ;
	m_pIndices = new SDecalIndex[nNumIndex] ;
	m_nMaxNumIndex = nNumIndex ;
}

void CSecretDecal::MakingBoundingPlanes(int nWidth, int nHeight)
{
	if( (nWidth != 0) && (nHeight != 0) )
	{
		m_fWidth = (float)nWidth ;
		m_fHeight = (float)nHeight ;
		m_fRadius = sqrtf((m_fWidth*m_fWidth)*0.5f + (m_fHeight*m_fHeight)*0.5f) ;
	}

	Vector3 v ;

	m_asPlane[LEFT_PLANE].a = -m_vTangent.x ;
	m_asPlane[LEFT_PLANE].b = -m_vTangent.y ;
	m_asPlane[LEFT_PLANE].c = -m_vTangent.z ;

	v = (-m_vTangent)*(m_fWidth/2.0f) ;
	v += m_vPos ;
	m_asPlane[LEFT_PLANE].d = -(v.dot(-m_vTangent)) ;
	//m_asPlane[LEFT_PLANE].d = -fabs((m_vPos.dot(m_vTangent))-(m_fWidth/2.0f)) ;// (m_fWidth/2.0f)-(m_vPos.dot(m_vTangent)) ;

	m_asPlane[RIGHT_PLANE].a = m_vTangent.x ;
	m_asPlane[RIGHT_PLANE].b = m_vTangent.y ;
	m_asPlane[RIGHT_PLANE].c = m_vTangent.z ;

	v = (m_vTangent)*(m_fWidth/2.0f) ;
	v += m_vPos ;
	m_asPlane[RIGHT_PLANE].d = -(v.dot(m_vTangent)) ;
	//m_asPlane[RIGHT_PLANE].d = -fabs((m_vPos.dot(m_vTangent))+(m_fWidth/2.0f)) ;// (m_fWidth/2.0f)+(m_vPos.dot(m_vTangent)) ;

	m_asPlane[TOP_PLANE].a = m_vBinormal.x ;
	m_asPlane[TOP_PLANE].b = m_vBinormal.y ;
	m_asPlane[TOP_PLANE].c = m_vBinormal.z ;

	v = (m_vBinormal)*(m_fHeight/2.0f) ;
	v += m_vPos ;
	m_asPlane[TOP_PLANE].d = -(v.dot(m_vBinormal)) ;
	//m_asPlane[TOP_PLANE].d = -fabs((m_vPos.dot(m_vBinormal))+(m_fHeight/2.0f)) ;// (m_fHeight/2.0f)+(m_vPos.dot(m_vBinormal)) ;

	m_asPlane[BOTTOM_PLANE].a = -m_vBinormal.x ;
	m_asPlane[BOTTOM_PLANE].b = -m_vBinormal.y ;
	m_asPlane[BOTTOM_PLANE].c = -m_vBinormal.z ;

	v = (-m_vBinormal)*(m_fHeight/2.0f) ;
	v += m_vPos ;
	m_asPlane[BOTTOM_PLANE].d = -(v.dot(-m_vBinormal)) ;
	//m_asPlane[BOTTOM_PLANE].d = -fabs((m_vPos.dot(m_vBinormal))-(m_fHeight/2.0f)) ;// (m_fHeight/2.0f)-(m_vPos.dot(m_vBinormal)) ;

	Vector3 vStart ;

	//라인의 순서는 시계방향으로 돌아가는 순서로 되어있다.
	//나중에 버텍스카운팅에서 원하는 노말방향의 삼각형을 얻기위해서임.
	//LEFT-TOP
	vStart = m_vPos+(-m_vTangent*(m_fWidth/2))+(m_vBinormal*(m_fHeight/2))+(-m_vNormal*(m_fDepth/2)) ;
	m_asLine[0].set(vStart, m_vNormal, m_fDepth) ;

	//RIGHT-TOP
	vStart = m_vPos+(m_vTangent*(m_fWidth/2))+(m_vBinormal*(m_fHeight/2))+(-m_vNormal*(m_fDepth/2)) ;
	m_asLine[1].set(vStart, m_vNormal, m_fDepth) ;

    //RIGHT-BOTTOM
	vStart = m_vPos+(m_vTangent*(m_fWidth/2))+(-m_vBinormal*(m_fHeight/2))+(-m_vNormal*(m_fDepth/2)) ;
	m_asLine[2].set(vStart, m_vNormal, m_fDepth) ;	

	//LEFT-BOTTOM
	vStart = m_vPos+(-m_vTangent*(m_fWidth/2))+(-m_vBinormal*(m_fHeight/2))+(-m_vNormal*(m_fDepth/2)) ;
	m_asLine[3].set(vStart, m_vNormal, m_fDepth) ;
}
void CSecretDecal::MakingBoundingPlanes(Vector3 vCenter, Vector3 *pvPos, Vector3 vNormal, float depth)
{
	int i ;
	Vector3 v ;

	//   0-----1
	//   |     |
	//   |     |
    //   3-----2

	//pvPos는 시계방향순으로 들어와야 함.
    
    //make bounding planes
	v = (pvPos[1]-pvPos[0]).cross(vNormal) ;
	m_asPlane[TOP_PLANE].set(pvPos[0], v.Normalize()) ;//top plane

	v = (pvPos[2]-pvPos[1]).cross(vNormal) ;
	m_asPlane[RIGHT_PLANE].set(pvPos[1], v.Normalize()) ;//right plane

	v = (pvPos[3]-pvPos[2]).cross(vNormal) ;
	m_asPlane[BOTTOM_PLANE].set(pvPos[2], v.Normalize()) ;//bottom plane

	v = (pvPos[0]-pvPos[3]).cross(vNormal) ;
	m_asPlane[LEFT_PLANE].set(pvPos[3], v.Normalize()) ;//left plane

	//make edge point lines
	v = pvPos[0]+(vNormal*depth) ;
	m_asLine[0].set(v, -vNormal, depth*2.0f) ;

	v = pvPos[1]+(vNormal*depth) ;
	m_asLine[1].set(v, -vNormal, depth*2.0f) ;

	v = pvPos[2]+(vNormal*depth) ;
	m_asLine[2].set(v, -vNormal, depth*2.0f) ;

	v = pvPos[3]+(vNormal*depth) ;
	m_asLine[3].set(v, -vNormal, depth*2.0f) ;

    m_fRadius = 0.0f ;
	float r ;
	for(i=0 ; i<4 ; i++)
	{
		r = (vCenter-pvPos[i]).Magnitude() ;
		if(m_fRadius < r)
			m_fRadius = r ;
	}
    m_fDepth = depth ;
	m_nKind = KIND::POLYGON ;
	m_vPos = vCenter ;
}
HRESULT CSecretDecal::Render()
{
	m_pd3dDevice->SetFVF(D3DFVF_DECALVERTEX) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SDecalVertex)) ;
	m_pd3dDevice->SetIndices(0) ;
	//m_pd3dDevice->SetTexture(0, m_pTex) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	//m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE) ;
	m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 0x00000000) ;
	m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL) ;

	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME) ;

    /*
	m_pVertices[0].pos = D3DXVECTOR3(-0.5f, 1, 0.5f) ;
	m_pVertices[0].normal = D3DXVECTOR3(0, 1, 0) ;
	m_pVertices[0].tex = D3DXVECTOR2(0, 0) ;

	m_pVertices[1].pos = D3DXVECTOR3(0.5f, 1, 0.5f) ;
	m_pVertices[1].normal = D3DXVECTOR3(0, 1, 0) ;
	m_pVertices[1].tex = D3DXVECTOR2(1, 0) ;

	m_pVertices[2].pos = D3DXVECTOR3(-0.5f, 1, -0.5f) ;
	m_pVertices[2].normal = D3DXVECTOR3(0, 1, 0) ;
	m_pVertices[2].tex = D3DXVECTOR2(0, 1) ;

	m_pVertices[3].pos = D3DXVECTOR3(0.5f, 1, -0.5f) ;
	m_pVertices[3].normal = D3DXVECTOR3(0, 1, 0) ;
	m_pVertices[3].tex = D3DXVECTOR2(1, 1) ;

	m_pIndices[0].awIndex[0] = 0 ;
	m_pIndices[0].awIndex[1] = 1 ;
	m_pIndices[0].awIndex[2] = 2 ;

	m_pIndices[1].awIndex[0] = 2 ;
	m_pIndices[1].awIndex[1] = 1 ;
	m_pIndices[1].awIndex[2] = 3 ;

	m_nVertexCount = 4 ;
	m_nNumTriangle = m_nVertexCount-2 ;
	*/
    
	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_nVertexCount, m_nNumTriangle, m_pIndices, D3DFMT_INDEX16, m_pVertices, sizeof(m_pVertices[0])) ;
	//m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE) ;
	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID) ;
	return S_OK ;
}

void CSecretDecal::AddInBoundTriangles(geo::STriangle *psTriangle, int nNumTriangle)
{
	int i ;

	m_nNumTriangle = 0 ;
	m_nVertexCount = m_nIndexCount = 0 ;
	for(i=0 ; i<nNumTriangle ; i++)
	{
		if(m_vNormal.dot(psTriangle[i].sPlane.GetNormal()) < 0)//바운딩박스의 노말과 반대노말의 삼각형은 제외
			continue ;

        _GetAppropriatedTriangleInBoundingBox(&psTriangle[i]) ;
	}
}
void CSecretDecal::AddInBoundTriangles(geo::STriangle **ppsTriangle, int nNumTriangle)
{
	int i ;

	m_nNumTriangle = 0 ;
	m_nVertexCount = m_nIndexCount = 0 ;
	for(i=0 ; i<nNumTriangle ; i++)
	{
		if(m_vNormal.dot(ppsTriangle[i]->sPlane.GetNormal()) < 0)//바운딩박스의 노말과 반대노말의 삼각형은 제외
			continue ;

        _GetAppropriatedTriangleInBoundingBox(ppsTriangle[i]) ;
	}
}

void CSecretDecal::_GetAppropriatedTriangleInBoundingBox(geo::STriangle *psTriangle)
{
	bool bInAll ;
	int i, n ;
	geo::STriangle asNewTriangles[10] ;

	//3개의 점이 모두 경계박스안에 포함되어 있는가?
	for(i=0 ; i<3 ; i++)
	{
		bInAll = true ;
		for(n=0 ; n<4 ; n++)
		{
			psTriangle->avVertex[i].nPosDecision = m_asPlane[n].Classify(psTriangle->avVertex[i].vPos) ;
			//TRACE("plane[%02d] tri[%02d] decision=%02d\r\n", n, i, psTriangle->avVertex[i].nPosDecision) ;
			if(psTriangle->avVertex[i].nPosDecision == geo::SPlane::SPLANE_INFRONT)
			{
				bInAll = false ;
			}
		}
		if(!bInAll)
			psTriangle->avVertex[i].nPosDecision = geo::SPlane::SPLANE_INFRONT ;
	}

	if( (psTriangle->avVertex[0].nPosDecision == geo::SPlane::SPLANE_INFRONT)
		|| (psTriangle->avVertex[1].nPosDecision == geo::SPlane::SPLANE_INFRONT)
		|| (psTriangle->avVertex[2].nPosDecision == geo::SPlane::SPLANE_INFRONT) )
	{
		_DivideTrianglebyBoundingBox(psTriangle) ;
	}
	else//3개의 점이 모두 포함되어 있는경우
	{
#ifdef OUTPUT_RENDERDATA
		TRACE("Triangle v0(%+07.03f %+07.03f %+07.03f) v1(%+07.03f %+07.03f %+07.03f) v2(%+07.03f %+07.03f %+07.03f)\r\n",
		enumVector(psTriangle->avVertex[0].vPos), enumVector(psTriangle->avVertex[1].vPos), enumVector(psTriangle->avVertex[2].vPos)) ;
#endif //#ifdef OUTPUT_RENDERDATA

		int nFirstVertexCount = m_nVertexCount ;
		Vector3 v ;
		for(n=0 ; n<3 ; n++, m_nVertexCount++)
		{
			//assert(m_nVertexCount < MAXNUM_DECALVERTEX) ;
			assert(m_nVertexCount < m_nMaxNumVertex) ;

			v = m_vNormal*m_fNormalOffset ;
			v = psTriangle->avVertex[n].vPos+v ;

			//m_pVertices[m_nVertexCount].pos = D3DXVECTOR3(psTriangle->avVertex[n].vPos.x, psTriangle->avVertex[n].vPos.y, psTriangle->avVertex[n].vPos.z) ;
			m_pVertices[m_nVertexCount].pos = D3DXVECTOR3(v.x, v.y, v.z) ;
			m_pVertices[m_nVertexCount].normal = D3DXVECTOR3(psTriangle->avVertex[n].vNormal.x, psTriangle->avVertex[n].vNormal.y, psTriangle->avVertex[n].vNormal.z) ;
			m_pVertices[m_nVertexCount].tex = _GetTexCoord(&m_pVertices[m_nVertexCount].pos) ;

#ifdef OUTPUT_RENDERDATA
			TRACE("add Vertex[%02d] pos(%+07.03f %+07.03f %+07.03f) nor(%+07.03f %+07.03f %+07.03f) tex(%+07.03f %+07.03f)\r\n",
			m_nVertexCount,
			enumVector(m_pVertices[m_nVertexCount].pos),
			enumVector(m_pVertices[m_nVertexCount].normal),
			m_pVertices[m_nVertexCount].tex.x, m_pVertices[m_nVertexCount].tex.y) ;
#endif //#ifdef OUTPUT_RENDERDATA
		}

		//assert(m_nIndexCount < MAXNUM_DECALINDEX) ;
		assert(m_nIndexCount < m_nMaxNumIndex) ;

		m_pIndices[m_nIndexCount].awIndex[0] = nFirstVertexCount ;
		m_pIndices[m_nIndexCount].awIndex[1] = nFirstVertexCount+1 ;
		m_pIndices[m_nIndexCount].awIndex[2] = nFirstVertexCount+2 ;

#ifdef OUTPUT_RENDERDATA
		TRACE("add Index[%02d] (%02d %02d %02d)\r\n",
			m_nIndexCount,
			m_pIndices[m_nIndexCount].awIndex[0],
			m_pIndices[m_nIndexCount].awIndex[1],
			m_pIndices[m_nIndexCount].awIndex[2]) ;
#endif //#ifdef OUTPUT_RENDERDATA

		m_nIndexCount++ ;

		m_nNumTriangle++ ;

        asNewTriangles[0] = *psTriangle ;
	}
}

double cross_normal_test(Vector3 &v0, Vector3 &v1, Vector3 &v2, Vector3 &vNormal)
{
    double x1, y1, z1, x2, y2, z2 ;
	double x, y, z ;

	x1 = (double)v1.x - (double)v0.x ;
	y1 = (double)v1.y - (double)v0.y ;
	z1 = (double)v1.z - (double)v0.z ;

	x2 = (double)v2.x - (double)v0.x ;
	y2 = (double)v2.y - (double)v0.y ;
	z2 = (double)v2.z - (double)v0.z ;

    x = y1*z2 - z1*y2 ;
	y = z1*x2 - x1*z2 ;
	z = x1*y2 - y1*x2 ;

	double m = sqrt(x*x + y*y + z*z) ;
	x = x/m ;
	y = y/m ;
	z = z/m ;

    double dot ;
	dot = x*vNormal.x + y*vNormal.y + z*vNormal.z ;
	return dot ;
}

bool CSecretDecal::_PutPosInClockWise(Vector3 *pvValidPos, int &nCount, geo::STriangleWeight *pvWeights)
{
	if(nCount < 3)//can't make triangles less than 3
		return false ;

	int i, nIndicater ;
	Vector3 v1, v2, v3, v4 ;
	geo::STriangleWeight sWeight ;

    nIndicater=1 ;
	for(i=0 ; i<(nCount-2) ; i++, nIndicater++)//'(nCount-2)' is the number of triangles from vertices
	{
		//v1 = (pvValidPos[nIndicater] - pvValidPos[0]).Normalize() ;
		//v2 = (pvValidPos[nIndicater+1] - pvValidPos[0]).Normalize() ;
		//v3 = v1.cross(v2) ;

		//if(m_vNormal.dot(v3) < 0)//서로 다른 방향을 보면은
		if(cross_normal_test(pvValidPos[0], pvValidPos[nIndicater], pvValidPos[nIndicater+1], m_vNormal) < 0)
		{
			v4 = pvValidPos[nIndicater] ;
			pvValidPos[nIndicater] = pvValidPos[nIndicater+1] ;
			pvValidPos[nIndicater+1] = v4 ;

			sWeight = pvWeights[nIndicater] ;
			pvWeights[nIndicater] = pvWeights[nIndicater+1] ;
			pvWeights[nIndicater+1] = sWeight ;

			if(_PutPosInClockWise(pvValidPos, nCount, pvWeights))
				return true ;
		}
	}
	return true ;
}

bool CSecretDecal::_IsSameVectorInList(Vector3 *pvValidPos, int nCount, Vector3 &vPos)
{
	for(int i=0 ; i<nCount ; i++)
	{
		if( vector_eq(pvValidPos[i], vPos) )
			return true ;
	}
    return false ;
}

void CSecretDecal::_DivideTrianglebyBoundingBox(geo::STriangle *psTriangle)
{
	geo::SVertex avVertex[8] ;
	Vector3 avValidPos[8] ;
	geo::STriangleWeight asWeight[8] ;
	geo::SLine sLine ;
	Vector3 vIntersected, vBaryCenter, avIntersectedPlane[2] ;
	int i, n, nResult, nValidCount, nIntersectedPlaneCount ;
	float t, afTRatio[2] ;
	bool bOutSideLine ;
	int nFrom, nTo ;
	float ratio1[2], ratio2[2] ;
	static int s_nTriangleCount=0 ;

	float fOldEpsilon = g_fEpsilon ;
	g_fEpsilon = 0.0001f ;

	nValidCount = 0 ;

	//출발점이 삼각형의 꼭지점이 경계상자 안이나 접하여 있을경우
	if((psTriangle->avVertex[0].nPosDecision == geo::SPlane::SPLANE_BEHIND)
			|| psTriangle->avVertex[0].nPosDecision == geo::SPlane::SPLANE_COINCLIDING)
	{
		if(!_IsSameVectorInList(avValidPos, nValidCount, avVertex[0].vPos))
		{
			asWeight[nValidCount].set(1, 0, 0) ;
			avValidPos[nValidCount++] = psTriangle->avVertex[0].vPos ;
		}
		//TRACE("ValidPos(%+07.03f %+07.03f %+07.03f) TriangleWeight(%+07.03f %+07.03f %+07.03f)\r\n",
		//	enumVector(avValidPos[nValidCount-1]),
		//			asWeight[nValidCount-1].afweight[0], asWeight[nValidCount-1].afweight[1], asWeight[nValidCount-1].afweight[2]) ;
	}

	for(i=0 ; i<3 ; i++)
	{
        //두개의 점이 모두 바깥에 있는지 없는지 체크해야 된다.
		bOutSideLine = false ;
		if(i==0)
		{
			nFrom = 0 ;
			nTo = 1 ;
		}
		else if(i==1)
		{
			nFrom = 1 ;
			nTo = 2 ;
		}
		else if(i==2)
		{
			nFrom = 2 ;
			nTo = 0 ;
		}

		if( (psTriangle->avVertex[nFrom].nPosDecision == geo::SPlane::SPLANE_INFRONT) && (psTriangle->avVertex[nTo].nPosDecision == geo::SPlane::SPLANE_INFRONT) )
				bOutSideLine = true ;

		psTriangle->GetLine(i, &sLine) ;
		nIntersectedPlaneCount = 0 ;
		for(n=0 ; n<4 ; n++)
		{
			nResult = IntersectLinetoPlane(sLine, m_asPlane[n], vIntersected, t) ;//삼각형 각 꼭지점라인들과 경계평면이 교차하는경우

			if(nResult == geo::INTERSECT_POINT && IsInBoundingBox(vIntersected))
			{
				//두점이 모두 바깥에 있을경우에 유효한 점을 얻은 경우는 반드시 2개의 교차점이 있어야 된다.
				//만약 없다면 유효한 점이 아니다.

				assert(nIntersectedPlaneCount < 2) ;//하나의 라인이 평면2개이상과 충돌할수 없다.

                //avValidPos[nValidCount++] = vIntersected ;
				afTRatio[nIntersectedPlaneCount] = t ;
				avIntersectedPlane[nIntersectedPlaneCount++] = vIntersected ;
			}
		}

		if(nIntersectedPlaneCount == 2)//2개의 평면과 충돌했을경우 출발점과 가까운 쪽을 가려낸다.
		{
			ratio1[1] = afTRatio[0]/(sLine.d) ;
			ratio1[0] = 1-ratio1[1] ;

            //ratio2[1] = afTRatio[0]/(sLine.d) ;
			ratio2[1] = afTRatio[1]/(sLine.d) ;
			ratio2[0] = 1-ratio2[1] ;

			if(float_less(afTRatio[0], afTRatio[1]))
			{
				if(!_IsSameVectorInList(avValidPos, nValidCount, avIntersectedPlane[0]))
				{
					asWeight[nValidCount].afweight[nFrom] = ratio1[0] ;
					asWeight[nValidCount].afweight[nTo] = ratio1[1] ;
					avValidPos[nValidCount++] = avIntersectedPlane[0] ;
				}

				if(!_IsSameVectorInList(avValidPos, nValidCount, avIntersectedPlane[1]))
				{
					asWeight[nValidCount].afweight[nFrom] = ratio2[0] ;
					asWeight[nValidCount].afweight[nTo] = ratio2[1] ;
					avValidPos[nValidCount++] = avIntersectedPlane[1] ;
				}
			}
			else if(float_greater(afTRatio[0], afTRatio[1]))
			{
				if(!_IsSameVectorInList(avValidPos, nValidCount, avIntersectedPlane[1]))
				{
					asWeight[nValidCount].afweight[nFrom] = ratio1[0] ;
					asWeight[nValidCount].afweight[nTo] = ratio1[1] ;
					avValidPos[nValidCount++] = avIntersectedPlane[1] ;
				}

				if(!_IsSameVectorInList(avValidPos, nValidCount, avIntersectedPlane[0]))
				{
					asWeight[nValidCount].afweight[nFrom] = ratio2[0] ;
					asWeight[nValidCount].afweight[nTo] = ratio2[1] ;
					avValidPos[nValidCount++] = avIntersectedPlane[0] ;
				}
			}
			else if(float_eq(afTRatio[0], afTRatio[1]))//두개가 같다면 모서리 끝에서 만난것이다. 그래서 하나만 인정한다.
			{
				if(!_IsSameVectorInList(avValidPos, nValidCount, avIntersectedPlane[0]))
				{
					asWeight[nValidCount].afweight[nFrom] = ratio1[0] ;
					asWeight[nValidCount].afweight[nTo] = ratio1[1] ;
					avValidPos[nValidCount++] = avIntersectedPlane[0] ;
				}
				//TRACE("ValidPos(%+07.03f %+07.03f %+07.03f) TriangleWeight(%+07.03f %+07.03f %+07.03f)\r\n",
				//	enumVector(avValidPos[nValidCount-1]),
				//	asWeight[nValidCount-1].afweight[0], asWeight[nValidCount-1].afweight[1], asWeight[nValidCount-1].afweight[2]) ;
			}
			else
				assert(false && "Neither Greater than and less than") ;
		}
		else if(nIntersectedPlaneCount == 1)
		{
			//if(!bOutSideLine)
			{
				ratio1[1] = afTRatio[0]/(sLine.d) ;
				ratio1[0] = 1-ratio1[1] ;

				if(!_IsSameVectorInList(avValidPos, nValidCount, avIntersectedPlane[0]))
				{
					asWeight[nValidCount].afweight[nFrom] = ratio1[0] ;
					asWeight[nValidCount].afweight[nTo] = ratio1[1] ;
					avValidPos[nValidCount++] = avIntersectedPlane[0] ;
				}
			}
		}
		else
		{
		}

		if( (i==0) || (i==1) )
		{
			int nPos = i+1 ;
			//삼각형의 꼭지점이 경계상자 안이나 접하여 있을경우. 끝점을 체크하는 것이기 때문에 맨 마지막에 한다.
			if((psTriangle->avVertex[nPos].nPosDecision == geo::SPlane::SPLANE_BEHIND)
				|| psTriangle->avVertex[nPos].nPosDecision == geo::SPlane::SPLANE_COINCLIDING)
			{
				if(!_IsSameVectorInList(avValidPos, nValidCount, psTriangle->avVertex[nPos].vPos))
				{
					asWeight[nValidCount].afweight[nPos] = 1 ;
					avValidPos[nValidCount++] = psTriangle->avVertex[nPos].vPos ;
				}
				//TRACE("ValidPos(%+07.03f %+07.03f %+07.03f) TriangleWeight(%+07.03f %+07.03f %+07.03f)\r\n",
				//	enumVector(avValidPos[nValidCount-1]),
				//	asWeight[nValidCount-1].afweight[0], asWeight[nValidCount-1].afweight[1], asWeight[nValidCount-1].afweight[2]) ;
			}
		}
 	}

	for(i=0 ; i<4 ; i++)
	{
		//데칼의 바운드영역의 꼭지점이 삼각형과 충돌했는지 검사한다.
		if(IntersectLinetoTriangle(m_asLine[i], *psTriangle, vBaryCenter) == geo::INTERSECT_POINT)
		{
			vIntersected = psTriangle->avVertex[0].vPos*vBaryCenter.x + psTriangle->avVertex[1].vPos*vBaryCenter.y + psTriangle->avVertex[2].vPos*vBaryCenter.z ;
			if(!_IsSameVectorInList(avValidPos, nValidCount, vIntersected))
			{
				asWeight[nValidCount].set(vBaryCenter.x, vBaryCenter.y, vBaryCenter.z) ;
				avValidPos[nValidCount++] = vIntersected ;
				_PutPosInClockWise(avValidPos, nValidCount, asWeight) ;
			}
			//TRACE("ValidPos(%+07.03f %+07.03f %+07.03f) TriangleWeight(%+07.03f %+07.03f %+07.03f)\r\n",
			//	enumVector(avValidPos[nValidCount-1]),
			//	asWeight[nValidCount-1].afweight[0], asWeight[nValidCount-1].afweight[1], asWeight[nValidCount-1].afweight[2]) ;
		}
	}

	if(!nValidCount)
		return ;
#ifdef OUTPUT_RENDERDATA
	TRACE("[%02d] Triangle v0(%+07.03f %+07.03f %+07.03f) v1(%+07.03f %+07.03f %+07.03f) v2(%+07.03f %+07.03f %+07.03f)\r\n",
		s_nTriangleCount, enumVector(psTriangle->avVertex[0].vPos), enumVector(psTriangle->avVertex[1].vPos), enumVector(psTriangle->avVertex[2].vPos)) ;
#endif //#ifdef OUTPUT_RENDERDATA

	s_nTriangleCount++ ;

    //앞으로 각 정점당 텍스쳐 좌표 얻는 것과 노말값을 보간해서 얻는것
	//그래서 버텍스와 인덱스를 모두 조합하여 렌더링할 준비를 한다.
	int nTriangleCount, nFirstVertexCount ;
	Vector3 vNormal, v ;
	Vector2 vTex ;

	//같은 버텍스들이 있으면 삭제해야 된다.(하나의 삼각형에서만 시행하고 뒤쪽에 중복된 버텍스를 삭제한다.)

	nFirstVertexCount = m_nVertexCount ;
    for(i=0 ; i<nValidCount ; i++, m_nVertexCount++)
	{
		//assert(m_nVertexCount < MAXNUM_DECALVERTEX) ;
		assert(m_nVertexCount < m_nMaxNumVertex) ;

		v = m_vNormal *m_fNormalOffset ;
		v = avValidPos[i]+v ;

		//m_pVertices[m_nVertexCount].pos = D3DXVECTOR3(avValidPos[i].x, avValidPos[i].y, avValidPos[i].z) ;
		m_pVertices[m_nVertexCount].pos = D3DXVECTOR3(v.x, v.y, v.z) ;

		vNormal = psTriangle->GetNormalbyWeight(&asWeight[i]) ;
		m_pVertices[m_nVertexCount].normal = D3DXVECTOR3(vNormal.x, vNormal.y, vNormal.z) ;
		m_pVertices[m_nVertexCount].tex = _GetTexCoord(&m_pVertices[m_nVertexCount].pos) ;

#ifdef OUTPUT_RENDERDATA
		TRACE("add Vertex[%02d] pos(%+07.03f %+07.03f %+07.03f) nor(%+07.03f %+07.03f %+07.03f) tex(%+07.03f %+07.03f)\r\n",
			m_nVertexCount,
			enumVector(m_pVertices[m_nVertexCount].pos),
			enumVector(m_pVertices[m_nVertexCount].normal),
			m_pVertices[m_nVertexCount].tex.x, m_pVertices[m_nVertexCount].tex.y) ;
#endif //#ifdef OUTPUT_RENDERDATA
	}

	D3DXVECTOR3 vOut, vPos1, vPos2, vNor ;
	vNor = D3DXVECTOR3(m_vNormal.x, vNormal.y, vNormal.z) ;
	float fDot ;
	nTriangleCount = nValidCount-2 ;
	int nSecondVertexCount = nFirstVertexCount+1 ;
	for(i=0 ; i<nTriangleCount ; i++, m_nIndexCount++, nSecondVertexCount++)
	{
		//assert(m_nIndexCount < MAXNUM_DECALINDEX) ;
		assert(m_nIndexCount < m_nMaxNumIndex) ;

		vPos1 = m_pVertices[nSecondVertexCount].pos-m_pVertices[nFirstVertexCount].pos ;
		vPos2 = m_pVertices[nSecondVertexCount+1].pos-m_pVertices[nFirstVertexCount].pos ;
		D3DXVec3Cross(&vOut, &vPos1, &vPos2) ;
		fDot = D3DXVec3Dot(&vNor, &vOut) ;
		if(fDot < 0)//음수라면 현재 데칼바운딩박스의 노말과 현재 삼각형의 노말의 반대이기 때문에 순서를 바꿔준다.
		{
			m_pIndices[m_nIndexCount].awIndex[0] = nFirstVertexCount ;
			m_pIndices[m_nIndexCount].awIndex[1] = nSecondVertexCount+1 ;
			m_pIndices[m_nIndexCount].awIndex[2] = nSecondVertexCount ;
		}
		else
		{
			m_pIndices[m_nIndexCount].awIndex[0] = nFirstVertexCount ;
			m_pIndices[m_nIndexCount].awIndex[1] = nSecondVertexCount ;
			m_pIndices[m_nIndexCount].awIndex[2] = nSecondVertexCount+1 ;
		}

#ifdef OUTPUT_RENDERDATA
		TRACE("add Index[%02d] (%02d %02d %02d)\r\n",
			m_nIndexCount,
			m_pIndices[m_nIndexCount].awIndex[0],
			m_pIndices[m_nIndexCount].awIndex[1],
			m_pIndices[m_nIndexCount].awIndex[2]) ;
#endif //#ifdef OUTPUT_RENDERDATA
	}

	if(nTriangleCount > 0)
		m_nNumTriangle += nTriangleCount ;

	g_fEpsilon = fOldEpsilon ;
}

bool CSecretDecal::IsInBoundingBox(Vector3 &vPos)
{
	if(m_nKind == KIND::POLYGON)
	{
		for(int i=0 ; i<4 ; i++)
		{
			if(m_asPlane[i].Classify(vPos) == geo::SPlane::SPLANE_INFRONT)
				return false ;
		}
		return true ;
	}

	Vector3 vDist = vPos-m_vPos ;

	float fwidth = ProjectionVector(vDist, m_vTangent, false).Magnitude() ;// (vDist.Projection(m_vTangent)).Magnitude() ;
	float fheight = ProjectionVector(vDist, m_vBinormal, false).Magnitude() ;// (vDist.Projection(m_vBinormal)).Magnitude() ;

	if( float_greater(fwidth, m_fWidth/2.0f) || float_greater(fheight, m_fHeight/2.0f) )
		return false ;

	return true ;
}

//Vector2 CSecretDecal::_GetTexCoord(Vector3 &vPos)
//{
//	Vector2 vTex ;
//	vTex.x = m_vTangent.dot(vPos-m_vPos)/m_fWidth + 0.5f ;
//	vTex.y = 1.0f - (m_vBinormal.dot(vPos-m_vPos)/m_fHeight + 0.5f) ;
//	return vTex ;
//}

D3DXVECTOR2 CSecretDecal::_GetTexCoord(D3DXVECTOR3 *pvPos)
{
	D3DXVECTOR2 vTex(0, 0) ;
	if(m_nKind == KIND::POLYGON)
	{
		Vector3 v(pvPos->x, m_vPos.y, pvPos->z) ;
		geo::SLine line(Vector3(pvPos->x, 1000.0f, pvPos->z), -m_vNormal, 2000.0f) ;

		//if(float_eq(line.s.x, 0.831453f) && float_eq(line.s.z, -57.0f))
		//	TRACE("dont intersected pos(%10.06f %10.06f %10.06f)\r\n", line.s.x, line.s.y, line.s.z) ;

		geo::STriangle tri(Vector3(m_asLine[0].s.x, m_vPos.y, m_asLine[0].s.z),
			Vector3(m_asLine[1].s.x, m_vPos.y, m_asLine[1].s.z),
			Vector3(m_asLine[3].s.x, m_vPos.y, m_asLine[3].s.z)) ;

		//tri.avVertex[0].vTex.set(0, 0) ;
		//tri.avVertex[1].vTex.set(1, 0) ;
		//tri.avVertex[2].vTex.set(0, 1) ;
		tri.avVertex[0].vTex = m_avTexFactor[0] ;
		tri.avVertex[1].vTex = m_avTexFactor[1] ;
		tri.avVertex[2].vTex = m_avTexFactor[3] ;

		if(IntersectLinetoTriangle(line, tri, v, false, true, 0.001f) == geo::INTERSECT_POINT)
		{
			Vector2 t = tri.avVertex[0].vTex*v.x + tri.avVertex[1].vTex*v.y + tri.avVertex[2].vTex*v.z ;
			vTex.x = t.x ;
			vTex.y = t.y ;
			return vTex ;
		}

		tri.set(Vector3(m_asLine[3].s.x, m_vPos.y, m_asLine[3].s.z),
			Vector3(m_asLine[1].s.x, m_vPos.y, m_asLine[1].s.z),
			Vector3(m_asLine[2].s.x, m_vPos.y, m_asLine[2].s.z)) ;

		//tri.avVertex[0].vTex.set(0, 1) ;
		//tri.avVertex[1].vTex.set(1, 0) ;
		//tri.avVertex[2].vTex.set(1, 1) ;
		tri.avVertex[0].vTex = m_avTexFactor[3] ;
		tri.avVertex[1].vTex = m_avTexFactor[1] ;
		tri.avVertex[2].vTex = m_avTexFactor[2] ;

		if(IntersectLinetoTriangle(line, tri, v, false, true, 0.001f) == geo::INTERSECT_POINT)
		{
			Vector2 t = tri.avVertex[0].vTex*v.x + tri.avVertex[1].vTex*v.y + tri.avVertex[2].vTex*v.z ;
			vTex.x = t.x ;
			vTex.y = t.y ;
			return vTex ;
		}
		TRACE("dont intersected pos(%10.06f %10.06f %10.06f)\r\n", line.s.x, line.s.y, line.s.z) ;
	}

	Vector3 vPos(pvPos->x, pvPos->y, pvPos->z) ;
	vTex.x = m_vTangent.dot(vPos-m_vPos)/m_fWidth + 0.5f ;
	vTex.y = 1.0f - (m_vBinormal.dot(vPos-m_vPos)/m_fHeight + 0.5f) ;
	return vTex ;
}
void CSecretDecal::SetTexFactor(Vector2 *pvTexFactor)
{
	m_avTexFactor[0] = pvTexFactor[0] ;
	m_avTexFactor[1] = pvTexFactor[1] ;
	m_avTexFactor[2] = pvTexFactor[2] ;
	m_avTexFactor[3] = pvTexFactor[3] ;
}
void CSecretDecal::SetNormalOffset(float fOffset)
{
	m_fNormalOffset = fOffset ;
}
void CSecretDecal::SetPos(Vector3 &vPos)
{
    m_vPos = vPos ;
}

float CSecretDecal::SetSize(int nWidth, int nHeight)
{
	m_fWidth = (float)nWidth ;
	m_fHeight = (float)nHeight ;
	m_fRadius = sqrtf((m_fWidth*m_fWidth)*0.5f + (m_fHeight*m_fHeight)*0.5f) ;
	return m_fRadius ;
}