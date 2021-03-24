#include "Spline.h"

//bool IdentitySplineData(void *pSpline1, void *pSpline2)
//{
//	if(((SPLINEDATA *)pSpline1)->nIndex == ((SPLINEDATA *)pSpline2)->nIndex)
//		return true ;
//	return false ;
//}

CSpline::CSpline()
{
	memset(m_SplineData, 0, sizeof(SPLINEDATA)*MAXNUM_SPLINEDATA) ;
	m_fMaxDist=0 ;
	m_nNodeCount=0 ;
	m_nNumNode=0 ;

	m_matHermite.Set(
						2.f, -2.f,  1.f,  1.f,
				       -3.f,  3.f, -2.f, -1.f,
					    0.f,  0.f,  1.f,  0.f,
	                    1.f,  0.f,  0.f,  0.f
					) ;

	m_nIndexCount = 0 ;
	m_SplineData[0].nIndex = m_nIndexCount++ ;
	m_SplineData[1].nIndex = m_nIndexCount++ ;
	m_bEnable = false ;
}

CSpline::~CSpline()
{

}

void CSpline::Initialize(Vector3 vStart, Vector3 vNorStart, Vector3 vEnd, Vector3 vNorEnd)
{
	m_nNodeCount=0 ;

    SetSpline(&m_SplineData[m_nNodeCount++], vStart, vNorStart) ;
	SetSpline(&m_SplineData[m_nNodeCount], vEnd, vNorEnd) ;
}

void CSpline::Initialize(Vector3 vStart, Vector3 vNorStart)
{
    m_nNodeCount=0 ;
	SetSpline(&m_SplineData[m_nNodeCount], vStart, vNorStart) ;
}

void CSpline::SetSpline(SPLINEDATA *pSpline, Vector3 vPos, Vector3 vNor)
{
	pSpline->vPos = vPos ;
	pSpline->vNor = vNor ;
	pSpline->nIndex = m_nIndexCount++ ;
	m_nNumNode++ ;
}

void CSpline::AddNode(Vector3 vPos, Vector3 vNor)
{
	if(m_nNodeCount >= MAXNUM_SPLINEDATA)
		return ;

    SetSpline(&m_SplineData[++m_nNodeCount], vPos, vNor) ;
}

SPLINEDATA *CSpline::GetSplineData(int nNode)
{
	//SPLINEDATA SplineData ;
	//memcpy(&SplineData, &m_SplineData[nNode], sizeof(SPLINEDATA)) ;
	//return SplineData ;
	return &m_SplineData[nNode] ;
}

int CSpline::GetCurrentNode()
{
	return m_nNodeCount ;
}
void CSpline::ImportSplineData(SPLINEDATA *psSplineData, int nNum)
{
    for(int i=0 ; i<nNum ; i++)
		m_SplineData[i] = psSplineData[i] ;
	m_nNodeCount = nNum-1 ;
	m_nNumNode = nNum ;
}
Vector3 CSpline::GetPositionOnCubic(int nNode, float t)
{
	SPLINEDATA *pSpline1, *pSpline2 ;

	pSpline1 = &m_SplineData[nNode] ;
	pSpline2 = &m_SplineData[nNode+1] ;

	Vector3 vRet ;
	float T[4] ;

	T[0] = t*t*t ;
	T[1] = t*t ;
	T[2] = t ;
	T[3] = 1.f ;

	vRet.x = ((T[0]*m_matHermite.m11) + (T[1]*m_matHermite.m21) + (T[2]*m_matHermite.m31) + (T[3]*m_matHermite.m41))*pSpline1->vPos.x +
		     ((T[0]*m_matHermite.m12) + (T[1]*m_matHermite.m22) + (T[2]*m_matHermite.m32) + (T[3]*m_matHermite.m42))*pSpline2->vPos.x +
		     ((T[0]*m_matHermite.m13) + (T[1]*m_matHermite.m23) + (T[2]*m_matHermite.m33) + (T[3]*m_matHermite.m43))*pSpline1->vNor.x +
		     ((T[0]*m_matHermite.m14) + (T[1]*m_matHermite.m24) + (T[2]*m_matHermite.m34) + (T[3]*m_matHermite.m44))*pSpline2->vNor.x ;

	vRet.y = ((T[0]*m_matHermite.m11) + (T[1]*m_matHermite.m21) + (T[2]*m_matHermite.m31) + (T[3]*m_matHermite.m41))*pSpline1->vPos.y +
		     ((T[0]*m_matHermite.m12) + (T[1]*m_matHermite.m22) + (T[2]*m_matHermite.m32) + (T[3]*m_matHermite.m42))*pSpline2->vPos.y +
		     ((T[0]*m_matHermite.m13) + (T[1]*m_matHermite.m23) + (T[2]*m_matHermite.m33) + (T[3]*m_matHermite.m43))*pSpline1->vNor.y +
		     ((T[0]*m_matHermite.m14) + (T[1]*m_matHermite.m24) + (T[2]*m_matHermite.m34) + (T[3]*m_matHermite.m44))*pSpline2->vNor.y ;

	vRet.z = ((T[0]*m_matHermite.m11) + (T[1]*m_matHermite.m21) + (T[2]*m_matHermite.m31) + (T[3]*m_matHermite.m41))*pSpline1->vPos.z +
		     ((T[0]*m_matHermite.m12) + (T[1]*m_matHermite.m22) + (T[2]*m_matHermite.m32) + (T[3]*m_matHermite.m42))*pSpline2->vPos.z +
		     ((T[0]*m_matHermite.m13) + (T[1]*m_matHermite.m23) + (T[2]*m_matHermite.m33) + (T[3]*m_matHermite.m43))*pSpline1->vNor.z +
		     ((T[0]*m_matHermite.m14) + (T[1]*m_matHermite.m24) + (T[2]*m_matHermite.m34) + (T[3]*m_matHermite.m44))*pSpline2->vNor.z ;

	return vRet ;
}

Vector3 CSpline::GetTangentAtPosition(int nNode, float t, bool bNormalize)
{
	SPLINEDATA *pSpline1, *pSpline2 ;

	pSpline1 = &m_SplineData[nNode] ;
	pSpline2 = &m_SplineData[nNode+1] ;

	Vector3 vRet ;
	float T[4] ;

	T[0] = 3.f*t*t ;
	T[1] = 2.f*t ;
	T[2] = 1.f ;
	T[3] = 0.f ;

	vRet.x = ((T[0]*m_matHermite.m11) + (T[1]*m_matHermite.m21) + (T[2]*m_matHermite.m31))*pSpline1->vPos.x +
		     ((T[0]*m_matHermite.m12) + (T[1]*m_matHermite.m22) + (T[2]*m_matHermite.m32))*pSpline2->vPos.x +
		     ((T[0]*m_matHermite.m13) + (T[1]*m_matHermite.m23) + (T[2]*m_matHermite.m33))*pSpline1->vNor.x +
		     ((T[0]*m_matHermite.m14) + (T[1]*m_matHermite.m24) + (T[2]*m_matHermite.m34))*pSpline2->vNor.x ;

	vRet.y = ((T[0]*m_matHermite.m11) + (T[1]*m_matHermite.m21) + (T[2]*m_matHermite.m31))*pSpline1->vPos.y +
		     ((T[0]*m_matHermite.m12) + (T[1]*m_matHermite.m22) + (T[2]*m_matHermite.m32))*pSpline2->vPos.y +
		     ((T[0]*m_matHermite.m13) + (T[1]*m_matHermite.m23) + (T[2]*m_matHermite.m33))*pSpline1->vNor.y +
		     ((T[0]*m_matHermite.m14) + (T[1]*m_matHermite.m24) + (T[2]*m_matHermite.m34))*pSpline2->vNor.y ;

	vRet.z = ((T[0]*m_matHermite.m11) + (T[1]*m_matHermite.m21) + (T[2]*m_matHermite.m31))*pSpline1->vPos.z +
		     ((T[0]*m_matHermite.m12) + (T[1]*m_matHermite.m22) + (T[2]*m_matHermite.m32))*pSpline2->vPos.z +
		     ((T[0]*m_matHermite.m13) + (T[1]*m_matHermite.m23) + (T[2]*m_matHermite.m33))*pSpline1->vNor.z +
		     ((T[0]*m_matHermite.m14) + (T[1]*m_matHermite.m24) + (T[2]*m_matHermite.m34))*pSpline2->vNor.z ;

	if(float_eq(vRet.Magnitude(), 0.f))
		return vRet ;

	if(bNormalize)
		vRet = vRet.Normalize() ;

	return vRet ;
}

Vector3 CSpline::GetAccelVectorAtPosition(int nNode, float t, bool bNormalize)//Acceleration Vector °¡¼Óµµ º¤ÅÍ r"(t)
{
	SPLINEDATA *pSpline1, *pSpline2 ;

	pSpline1 = &m_SplineData[nNode] ;
	pSpline2 = &m_SplineData[nNode+1] ;

	Vector3 vRet ;
	float T[4] ;

	T[0] = 6.f*t ;
	T[1] = 2.f ;
	T[2] = 0.f ;
	T[3] = 0.f ;

	vRet.x = ((T[0]*m_matHermite.m11) + (T[1]*m_matHermite.m21))*pSpline1->vPos.x +
		     ((T[0]*m_matHermite.m12) + (T[1]*m_matHermite.m22))*pSpline2->vPos.x +
		     ((T[0]*m_matHermite.m13) + (T[1]*m_matHermite.m23))*pSpline1->vNor.x +
		     ((T[0]*m_matHermite.m14) + (T[1]*m_matHermite.m24))*pSpline2->vNor.x ;

	vRet.y = ((T[0]*m_matHermite.m11) + (T[1]*m_matHermite.m21))*pSpline1->vPos.y +
		     ((T[0]*m_matHermite.m12) + (T[1]*m_matHermite.m22))*pSpline2->vPos.y +
		     ((T[0]*m_matHermite.m13) + (T[1]*m_matHermite.m23))*pSpline1->vNor.y +
		     ((T[0]*m_matHermite.m14) + (T[1]*m_matHermite.m24))*pSpline2->vNor.y ;

	vRet.z = ((T[0]*m_matHermite.m11) + (T[1]*m_matHermite.m21))*pSpline1->vPos.z +
		     ((T[0]*m_matHermite.m12) + (T[1]*m_matHermite.m22))*pSpline2->vPos.z +
		     ((T[0]*m_matHermite.m13) + (T[1]*m_matHermite.m23))*pSpline1->vNor.z +
		     ((T[0]*m_matHermite.m14) + (T[1]*m_matHermite.m24))*pSpline2->vNor.z ;

	if(float_eq(vRet.Magnitude(), 0.f))
		return vRet ;

	if(bNormalize)
		vRet = vRet.Normalize() ;

	return vRet ;
}
/*
Vector3 CSpline::GetBinormalVectorAtPosition(int nNode, float t, bool bNormalize)//Binormal Vector Á¾¹ý¼±º¤ÅÍ
{
	Vector3 vRet, vTangent, vUnitNor ;

	vTangent = GetTangentAtPosition(nNode, t) ;
	vUnitNor = GetUnitNorVectorAtPosition(nNode, t) ;

	vRet = vTangent.cross(vUnitNor) ;

    return vRet ;
}
*/