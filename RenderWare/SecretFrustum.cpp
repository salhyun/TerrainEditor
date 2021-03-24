#include "SecretFrustum.h"
#include "SecretTerrain.h"
#include "SecretToolBrush.h"
#include "SecretTerrainModifier.h"
#include "MathOrdinary.h"
#include "SecretCoordSystem.h"

CSecretFrustum::CSecretFrustum()
{
	m_nPatchType = 0 ;
	m_pcToolBrush = NULL ;
	m_pcTerrainModifier = NULL ;
	m_pcLocalSystem = new CSecretCoordSystem ;

	m_nNumBoundingPosition = m_nMaxNumBoundingPosition = 0 ;
	m_pvBoundingPositions = NULL ;
}

CSecretFrustum::~CSecretFrustum()
{
    SAFE_DELETE(m_pcLocalSystem) ;
	SAFE_DELETEARRAY(m_pvBoundingPositions) ;
}

void CSecretFrustum::BuildFrustum(Matrix4 &matV, Matrix4 &matP)
{
	int i ;
	Matrix4 mat ;
	m_matVP = matV*matP ;
	m_matView = matV ;

	//투영행렬을 거치면 모든 좌표가 정규화가 된다. (-1,-1, 0) - (1, 1, 1) 사이의 값이 된다.
	//미리 그 정규화좌표를 얻어둔다.
	m_avVertex[0].set(-1.0f, -1.0f,  0.0f) ;
	m_avVertex[1].set( 1.0f, -1.0f,  0.0f) ;
	m_avVertex[2].set( 1.0f, -1.0f,  1.0f) ;
	m_avVertex[3].set(-1.0f, -1.0f,  1.0f) ;

	m_avVertex[4].set(-1.0f,  1.0f,  0.0f) ;
	m_avVertex[5].set( 1.0f,  1.0f,  0.0f) ;
	m_avVertex[6].set( 1.0f,  1.0f,  1.0f) ;
	m_avVertex[7].set(-1.0f,  1.0f,  1.0f) ;

	mat = m_matVP.Inverse() ;

	//D3DXMATRIX invVP, VP ;
	//VP = MatrixConvert(matVP) ;
	//D3DXMatrixInverse(&invVP, NULL, &VP) ;
	//mat = MatrixConvert(invVP) ;

    for(i=0 ; i<8 ; i++)
		m_avVertex[i] *= mat ;

	//CameraPosition 왜 이런 생각을 하게 되었지?
	m_vCameraPosition.x = -matV.m41 ;// mat.m41 ;
	m_vCameraPosition.y = -matV.m42 ;// mat.m42 ;
	m_vCameraPosition.z = -matV.m43 ;// mat.m43 ;

	//CameraPosition
	m_vCameraPosition = (m_avVertex[0]+m_avVertex[5])*(1.0f/2.0f) ;

	if(VECTOR_EQ(m_avVertex[0], m_avVertex[1]))
	{
		TRACE("\r\n") ;

		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", m_matVP.m11, m_matVP.m12, m_matVP.m13, m_matVP.m14) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", m_matVP.m21, m_matVP.m22, m_matVP.m23, m_matVP.m24) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", m_matVP.m31, m_matVP.m32, m_matVP.m33, m_matVP.m34) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", m_matVP.m41, m_matVP.m42, m_matVP.m43, m_matVP.m44) ;
		TRACE("\r\n") ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", mat.m11, mat.m12, mat.m13, mat.m14) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", mat.m21, mat.m22, mat.m23, mat.m24) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", mat.m31, mat.m32, mat.m33, mat.m34) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", mat.m41, mat.m42, mat.m43, mat.m44) ;
		TRACE("\r\n") ;

		TRACE("%matV %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f;\r\n",
			matV.m11, matV.m12, matV.m13, matV.m14, matV.m21, matV.m22, matV.m23, matV.m24,
			matV.m31, matV.m32, matV.m33, matV.m34, matV.m41, matV.m42, matV.m43, matV.m44) ;

		TRACE("\r\n") ;

		TRACE("%m_matVP %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f;\r\n",
			matP.m11, matP.m12, matP.m13, matP.m14, matP.m21, matP.m22, matP.m23, matP.m24,
			matP.m31, matP.m32, matP.m33, matP.m34, matP.m41, matP.m42, matP.m43, matP.m44) ;

		TRACE("\r\n") ;
		TRACE("%m_matVP %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f;\r\n",
			m_matVP.m11, m_matVP.m12, m_matVP.m13, m_matVP.m14, m_matVP.m21, m_matVP.m22, m_matVP.m23, m_matVP.m24,
			m_matVP.m31, m_matVP.m32, m_matVP.m33, m_matVP.m34, m_matVP.m41, m_matVP.m42, m_matVP.m43, m_matVP.m44) ;

		TRACE("\r\n") ;

		TRACE("%Invm_matVP %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f; %07.03f %07.03f %07.03f %07.03f;\r\n",
			mat.m11, mat.m12, mat.m13, mat.m14, mat.m21, mat.m22, mat.m23, mat.m24,
			mat.m31, mat.m32, mat.m33, mat.m34, mat.m41, mat.m42, mat.m43, mat.m44) ;

		TRACE("\r\n") ;

		D3DXMATRIX ivp, vp, v, p ;

        v=MatrixConvert(matV) ;
		p=MatrixConvert(matP) ;

        D3DXMatrixMultiply(&vp, &v, &p) ;


		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", vp._11, vp._12, vp._13, vp._14) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", vp._21, vp._22, vp._23, vp._24) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", vp._31, vp._32, vp._33, vp._34) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", vp._41, vp._42, vp._43, vp._44) ;
		TRACE("\r\n") ;

        D3DXMatrixInverse(&ivp, NULL, &vp) ;

        TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", ivp._11, ivp._12, ivp._13, ivp._14) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", ivp._21, ivp._22, ivp._23, ivp._24) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", ivp._31, ivp._32, ivp._33, ivp._34) ;
		TRACE("|%+07.03f %+07.03f %+07.03f %+07.03f|\r\n", ivp._41, ivp._42, ivp._43, ivp._44) ;
		TRACE("\r\n") ;
        

		TRACE("[0]Vertex(%07.03f %07.03f %07.03f)\r\n", enumVector(m_avVertex[0])) ;
		TRACE("[1]Vertex(%07.03f %07.03f %07.03f)\r\n", enumVector(m_avVertex[1])) ;
	}

	//법선벡터계산을 CrossProduct로 계산하기 때문에 아래와 같은 순서로 하게되면
	//D3D에서 사용하고 있는 왼손법칙에 반대가 되는 결과가 나오게 될것이다.
    GetPlaneFromVertex(m_asPlane[0], m_avVertex[4], m_avVertex[7], m_avVertex[5]) ;//top    Normal( 0, -1,  0) 위쪽을 바라보는 평면
	GetPlaneFromVertex(m_asPlane[1], m_avVertex[0], m_avVertex[1], m_avVertex[3]) ;//bottom Normal( 0,  1,  0) 아래쪽을 바라보는 평면

	GetPlaneFromVertex(m_asPlane[2], m_avVertex[0], m_avVertex[4], m_avVertex[1]) ;//near   Normal( 0,  0, -1) 뒤쪽을 바라보는 평면
	GetPlaneFromVertex(m_asPlane[3], m_avVertex[3], m_avVertex[2], m_avVertex[6]) ;//far    Normal( 0,  0,  1) 앞쪽을 바라보는 평면

	GetPlaneFromVertex(m_asPlane[4], m_avVertex[0], m_avVertex[3], m_avVertex[4]) ;//left   Normal( 1,  0,  0) 왼쪽을 바라보는 평면
	GetPlaneFromVertex(m_asPlane[5], m_avVertex[1], m_avVertex[5], m_avVertex[2]) ;//right  Normal(-1,  0,  0) 오른쪽을 바라보는 평면

	//for(i=0 ; i<6; i++)
	//	TRACE("Plane[%d] normal(%+07.03f %+07.03f %+07.03f)\r\n", i, enumVector(m_asPlane[i].GetNormal())) ;

	m_bDrawPlane = false ;
}

void CSecretFrustum::SetCameraPos(Vector3 &vPos)
{
	m_vCameraPosition = vPos ;
}

bool CSecretFrustum::IsIn(Vector3 &v)
{
	float fDist ;

	//top
	//fDist = GetPlaneDotCoord(m_asPlane[0], v) ;
	//if(fDist > fPlaneEpsilon) return false ;
    //bottom
	//fDist = GetPlaneDotCoord(m_asPlane[1], v) ;
	//if(fDist > fPlaneEpsilon) return false ;
    //near
	fDist = GetPlaneDotCoord(m_asPlane[2], v) ;
	if(fDist > fPlaneEpsilon) return false ;
    //far
	fDist = GetPlaneDotCoord(m_asPlane[3], v) ;
	if(fDist > fPlaneEpsilon) return false ;
    //left
	fDist = GetPlaneDotCoord(m_asPlane[4], v) ;
	if(fDist > fPlaneEpsilon) return false ;
    //right
	fDist = GetPlaneDotCoord(m_asPlane[5], v) ;
	if(fDist > fPlaneEpsilon) return false ;

	return true ;
}

bool CSecretFrustum::IsIn(D3DXVECTOR3 &v)
{
	float fDist ;

	//top
	//fDist = GetPlaneDotCoord(m_asPlane[0], v) ;
	//if(fDist > fPlaneEpsilon) return false ;
    //bottom
	//fDist = GetPlaneDotCoord(m_asPlane[1], v) ;
	//if(fDist > fPlaneEpsilon) return false ;
    //near
	fDist = GetPlaneDotCoord(m_asPlane[2], v) ;
	if(fDist > fPlaneEpsilon) return false ;
    //far
	fDist = GetPlaneDotCoord(m_asPlane[3], v) ;
	if(fDist > fPlaneEpsilon) return false ;
    //left
	fDist = GetPlaneDotCoord(m_asPlane[4], v) ;
	if(fDist > fPlaneEpsilon) return false ;
    //right
	fDist = GetPlaneDotCoord(m_asPlane[5], v) ;
	if(fDist > fPlaneEpsilon) return false ;

	return true ;
}

bool CSecretFrustum::IsInSphere(Vector3 &v, float fRadius)
{
	float fDist ;
    //top
	fDist = GetPlaneDotCoord(m_asPlane[0], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //bottom
	fDist = GetPlaneDotCoord(m_asPlane[1], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //near
	fDist = GetPlaneDotCoord(m_asPlane[2], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //far
	fDist = GetPlaneDotCoord(m_asPlane[3], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //left
	fDist = GetPlaneDotCoord(m_asPlane[4], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //right
	fDist = GetPlaneDotCoord(m_asPlane[5], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;

	return true ;
}

bool CSecretFrustum::IsInSphere(D3DXVECTOR3 &v, float fRadius)
{
	float fDist ;
    //top
	fDist = GetPlaneDotCoord(m_asPlane[0], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //bottom
	fDist = GetPlaneDotCoord(m_asPlane[1], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //near
	fDist = GetPlaneDotCoord(m_asPlane[2], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //far
	fDist = GetPlaneDotCoord(m_asPlane[3], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //left
	fDist = GetPlaneDotCoord(m_asPlane[4], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;
    //right
	fDist = GetPlaneDotCoord(m_asPlane[5], v) ;
    if(fDist > (fPlaneEpsilon+fRadius)) return false ;

	return true ;
}

//bool CSecretFrustum::IsIn(Vector3 &vStart, Vector3 &vEnd)
//{
//	geo::SAACube sFrustumCube(Vector3(-1,-1, 0), Vector3(1, 1, 1)) ;
//	Vector3 v1 = vStart * m_matVP ;
//	Vector3 v2 = (vEnd * m_matVP) - v1 ;
//	geo::SLine line(v1, v2.Normalize(), v2.Magnitude()) ;
//
//	if(IntersectLineToCube(&line, &sFrustumCube))
//		return true ;
//	return false ;
//}

//bool CSecretFrustum::IsIn(geo::SLine *psLine)
//{
//	int nDecision1=0, nDecision2=0 ;
//	Vector3 vEnd = psLine->GetEndPos(), vIntersect ;
//	for(int i=0 ; i<6 ; i++)
//	{
//		if(m_asPlane[i].Classify(psLine->s) == geo::SPlane::SPLANE_BEHIND)
//			nDecision1++ ;
//		if(m_asPlane[i].Classify(vEnd) == geo::SPlane::SPLANE_BEHIND)
//			nDecision2++ ;
//	}
//	if( (nDecision1 < 6) && (nDecision2 < 6) )
//		return false ;
//	return true ;
//}

bool CSecretFrustum::DrawFrustum(LPDIRECT3DDEVICE9 pd3dDevice)
{
	//삼각형을 그릴때는 시계방향으로 그린다.
	unsigned short index[] =
	{
		//bottom
		0, 1, 2,
		0, 2, 3,
        //top
		4, 7, 6,
		4, 6, 5,
        //right
		1, 5, 6,
		1, 6, 2,
        //left
		0, 3, 7,
		0, 7, 4,
        //near
		0, 4, 5,
		0, 5, 1,
        //far
		//3, 7, 6,
		//3, 6, 2
		3, 2, 6,
		3, 6, 7
	} ;

	unsigned short index_edge[] = 
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
		3, 7,

		2, 6,
		1, 5
	} ;

	typedef struct
	{
		D3DXVECTOR3 pos ;
		unsigned long color ;
	} VERTEX ;

	int i ;
	VERTEX vertex[8] ;

    for(i=0 ; i<8 ; i++)
	{
		vertex[i].pos = D3DXVECTOR3(m_avVertex[i].x, m_avVertex[i].y, m_avVertex[i].z) ;
		vertex[i].color = 0x80ffffff ;
	}

    DWORD dwAlphaBlendEnable ;
	pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlendEnable) ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;


	pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(VERTEX)) ;
	pd3dDevice->SetTexture(0, NULL) ;
	pd3dDevice->SetIndices(0) ;
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	//pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
	//pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) ;
	//pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA) ;
	//pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE) ;

	unsigned short *pIndex=index ;

	if(m_bDrawPlane)
	{
		for(i=0 ; i<8 ; i++)
			vertex[i].color = 0x8000ff00 ;

		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 4, pIndex, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;

		for(i=0 ; i<8 ; i++)
			vertex[i].color = 0xfff0f0f ;

		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 4, pIndex+12, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;

		for(i=0 ; i<8 ; i++)
			vertex[i].color = 0x80ff0000 ;

		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 4, pIndex+24, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;
	}
	else
	{
		for(i=0 ; i<8 ; i++)
			vertex[i].color = 0xff00ff00 ;

		pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 8, 12, index_edge, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;
	}

    if(m_pcLocalSystem)
	{
		m_pcLocalSystem->Initialize(pd3dDevice) ;
		m_pcLocalSystem->SetMatrix(m_matView.Inverse()) ;
		m_pcLocalSystem->Render() ;
	}

	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlendEnable) ;

    return true ;
}

geo::SPlane *CSecretFrustum::GetPlane(int nNum)
{
	return &m_asPlane[nNum] ;
}

float CSecretFrustum::GetPlaneDotCoord(geo::SPlane &sPlane, Vector3 &v)
{
	return (sPlane.a*v.x)+(sPlane.b*v.y)+(sPlane.c*v.z)+sPlane.d ;
}

float CSecretFrustum::GetPlaneDotCoord(geo::SPlane &sPlane, D3DXVECTOR3 &v)
{
	return (sPlane.a*v.x)+(sPlane.b*v.y)+(sPlane.c*v.z)+sPlane.d ;
}

float CSecretFrustum::GetPlaneDotCoord(int nNum, Vector3 &v)
{
	return (m_asPlane[nNum].a*v.x)+(m_asPlane[nNum].b*v.y)+(m_asPlane[nNum].c*v.z)+m_asPlane[nNum].d ;
}

float CSecretFrustum::GetPlaneDotCoord(int nNum, D3DXVECTOR3 &v)
{
	return (m_asPlane[nNum].a*v.x)+(m_asPlane[nNum].b*v.y)+(m_asPlane[nNum].c*v.z)+m_asPlane[nNum].d ;
}

void CSecretFrustum::GetPlaneFromVertex(geo::SPlane &sPlane, Vector3 &v1, Vector3 &v2, Vector3 &v3)
{
	//Vector3 vNormal = ((v2-v1).cross((v3-v1))).Normalize() ;
	Vector3 a = (v2-v1).Normalize() ;
	Vector3 b = (v3-v1).Normalize() ;
	Vector3 vNormal = a.cross(b) ;

	sPlane.a = vNormal.x ;
	sPlane.b = vNormal.y ;
	sPlane.c = vNormal.z ;

	sPlane.d = -(v1.dot(vNormal)) ;
}
void CSecretFrustum::AllocateBoundingPositions(int nMaxNum)
{
	m_nMaxNumBoundingPosition = nMaxNum ;
    m_pvBoundingPositions = new Vector3[m_nMaxNumBoundingPosition] ;
}
void CSecretFrustum::InsertBoundPosition(Vector3 *pv)
{
	if(m_nNumBoundingPosition >= m_nMaxNumBoundingPosition)
		return ;

	m_pvBoundingPositions[m_nNumBoundingPosition++] = *pv ;
}

void CSecretFrustum::ResetTRObjects()
{
	m_nNumTRObject=0 ;
}
