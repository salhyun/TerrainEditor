#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#include "def.h"
#include "Matrix.h"
#include "Vector3.h"
#include "Vector4.h"
#include "GeoLib.h"

const float fPlaneEpsilon=2.0f ;
class CSecretToolBrush ;
struct STrueRenderingObject ;
class CSecretTerrainModifier ;
class CSecretCoordSystem ;
class CSecretSodManager ;


class CSecretFrustum
{
public :
	enum {FRUSTUM_TOP=0, FRUSTUM_BOTTOM, FRUSTUM_NEAR, FRUSTUM_FAR, FRUSTUM_LEFT, FRUSTUM_RIGHT} ;
	enum {COLLISION_INFRUSTUM=0, COLLISION_INTERSECT, COLLISION_OUT} ;

private :
	bool m_bDrawPlane ;//Enable : render with plane, Disable : render with line
	Vector3 m_avVertex[8] ;
	Vector3 m_vCameraPosition ;
	geo::SPlane m_asPlane[6] ;
	Matrix4 m_matView, m_matVP ;

public :
	int m_nPatchType ;
	int m_nIntersectCount ;

    CSecretToolBrush *m_pcToolBrush ;//지형트리를 검색하며 프러스텀컬링을 할때 툴브러쉬도 같이 따라다니며 충돌을 검색하고 삼각형정보를 얻는다.
	int m_nNumTRObject ;
	STrueRenderingObject *m_apsTRObjects[2048] ;

    CSecretTerrainModifier *m_pcTerrainModifier ;

	int m_nMaxNumBoundingPosition, m_nNumBoundingPosition ;
	Vector3 *m_pvBoundingPositions ;
	CSecretCoordSystem *m_pcLocalSystem ;

	CSecretFrustum() ;
	~CSecretFrustum() ;

	void BuildFrustum(Matrix4 &matV, Matrix4 &matP) ;
	void SetCameraPos(Vector3 &vPos) ;

	bool IsIn(Vector3 &v) ;
	bool IsInSphere(Vector3 &v, float fRadius) ;

	bool IsIn(D3DXVECTOR3 &v) ;
	bool IsInSphere(D3DXVECTOR3 &v, float fRadius) ;

	bool IsIn(Vector3 &vStart, Vector3 &vEnd) ;

	bool DrawFrustum(LPDIRECT3DDEVICE9 pd3dDevice) ;

	Vector3 GetCameraPosition() { return m_vCameraPosition ;} ;
	Matrix4 *GetmatView() { return &m_matView ;} ;

	geo::SPlane *GetPlane(int nNum) ;
	float GetPlaneDotCoord(geo::SPlane &sPlane, Vector3 &v) ;
	float GetPlaneDotCoord(geo::SPlane &sPlane, D3DXVECTOR3 &v) ;
	float GetPlaneDotCoord(int nNum, Vector3 &v) ;
	float GetPlaneDotCoord(int nNum, D3DXVECTOR3 &v) ;
	void GetPlaneFromVertex(geo::SPlane &sPlane, Vector3 &v1, Vector3 &v2, Vector3 &v3) ;

	void AllocateBoundingPositions(int nMaxNum) ;
	void InsertBoundPosition(Vector3 *pv) ;

	void ResetTRObjects() ;
} ;