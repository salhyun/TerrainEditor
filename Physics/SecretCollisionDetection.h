#pragma once

//#ifndef DWORD
//#define DWORD unsigned long
//#endif

#ifndef NULL
#define NULL 0
#endif

#include "SecretPhysicsDef.h"
#include "GeoLib.h"
#include "coldet.h"
#include "D3DDef.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if(p){delete p; p=NULL;}}
#endif

#ifndef SAFE_DELETEARRAY
#define SAFE_DELETEARRAY(p) {if(p){delete []p; p=NULL;}}
#endif

#define MAXNUM_COLLIDEDTRIANGLE 512

class CSecretRigidbody ;
class CGolfBall ;
class CSecretTerrain ;
class CSecretRoadManager ;
class CSecretWaterManager ;
struct STrueRenderingObject ;
struct SHolecup ;

class CSecretCollisionDetection
{
public :
	enum RESPONSE_OPTION {COLLIDED_TRIANGLE=0x01, COLLIDED_CYLINDER=0x02, COLLIDED_SPHERE=0x04, DONTROLLING=0x08, PUREREFLECT=0x10} ;
	enum COLLISION_EVENT {COLLISIONEVENT_INTERSECTWATER=0x01} ;

	const int MAXNUM_RIGIDBODY ;
	const int MAXNUM_PLANE ;
	const int MAXNUM_COLLIDINGVERTEX ;
	const int MAXNUM_COLLIDEDCYLINDER ;
	const float COLDET_EPSILON ;
	const float ROLLINGSPACE ;
	const int MAXNUM_COLPRIMITIVE ;


private :
	DWORD m_dwCollisionEvent ;
	int m_nNumRigidbody ;
	CSecretRigidbody **m_ppcRigidbodies ;

	int m_nNumPlane ;
	geo::SPlane **m_ppsPlanes ;

	CSecretTerrain *m_pcTerrain ;
	CSecretRoadManager *m_pcRoadManager ;
	CSecretWaterManager *m_pcWaterManager ;

	float *m_pfCollidingVertices ;

	LPD3DXLINE m_pLine ;

	int m_nNumRangedTriangleIT ;
	geo::STriangle *m_psRangedTrianglesIT ;

	geo::STriangle *m_psIntersectedTriangleCT ;

	geo::SCylinder *m_psBoundingCylindersCT ;
	geo::SSphere *m_psBoundingSpheresCT ;

	geo::SCylinder *m_psBoundingCylindersRT ;
	geo::SSphere *m_psBoundingSpheresRT ;

	void _GetVelocityTrace(geo::SLine *psLine, CSecretRigidbody *pcRigidbody, Matrix4 *pmatWorld=NULL) ;

	//bool _IntersectBBox(CGolfBall *pcGolfBall, CSecretRigidbody *pcImmovable) ;
	//void _IntersectBodywithBody(CGolfBall *pcGolfBall) ;
	//void _FindAroundbody(CGolfBall *pcGolfBall) ;

	bool _CollisionResponse(CGolfBall *pcGolfBall, geo::SLine *psTrace, geo::SPlane *psPlane, Vector3 vIntersect, int nOption) ;
	Vector3 _CalculateReflection(CGolfBall *pcGolfBall, Vector3 vIncidence, Vector3 vnPlane, float fVelocity, float fRestitution=1.0f) ;
	Vector3 _CalculatePureReflection(CGolfBall *pcGolfBall, Vector3 vIncidence, Vector3 vnPlane, float fRestitution) ;

	bool _CollidingTriangles(CGolfBall *pcGolfBall, geo::SLine *psTrace, geo::STriangle *psRangedTriangles, int nNumRangedTriangle, CSecretTerrain *pcTerrain, STrueRenderingObject *psTRObject) ;
	bool _RollingTriangles(CGolfBall *pcGolfBall, geo::SLine *psTrace, geo::STriangle *psRangedTriangles, int nNumRangedTriangle, CSecretTerrain *pcTerrain, STrueRenderingObject *psTRObject) ;

	void _IntersectTerrain(CGolfBall *pcGolfBall) ;
	void _IntersectRigidbody(CGolfBall *pcGolfBall) ;
	bool _IntersectWater(CGolfBall *pcGolfBall) ;
	
	void _FindTriangleInRolling(Vector3 &vPos, float fRadius, geo::STriangle *psRangedTriangles, int nNumRangedTriangle, geo::STriangle **ppsClosestTriangle, float &fClosest, Vector3 &vOnPlane) ;

	void _GetCylinderMatrix(geo::SCylinder *psCylinder, Matrix4 *pmat) ;

	void _isSafe_colTri_to_rolTri(CGolfBall *pcGolfBall, geo::STriangle *psRolledTriangle) ;

public :
	int m_nNumCollidedCylinder ;
	Matrix4 *m_pmatCylinders ;
	Vector3 *m_pvCylinderIntersectPos ;

	int m_nNumCollidedTriangle ;
	geo::STriangle *m_psCollidedTriangles ;
	int m_motherfucker ;

	float m_fVelRange, m_fVelBase ;
	float m_fMaxIncidenceBase ;
	float m_fScaleRestitution ;
	Vector3 m_vOffsetGolfBallonRoad ;

	void (*m_pfuncPlaySound)(CGolfBall *) ;

	bool m_bEnableVertIntersectedTriangle ;

public :
	CSecretCollisionDetection() ;
	~CSecretCollisionDetection() ;

	bool Initialize() ;
	void Process() ;
	void Render(D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj) ;

	void AddRigidbody(CSecretRigidbody *pcRigidbody) ;
	void AddPlane(geo::SPlane *psPlane) ;

	void ClearRigidbody() ;
	void ClearPlane() ;

	void ClearCollidedTriangle() ;

	void SetTerrain(CSecretTerrain *pcTerrain) ;
	void SetHolecup(SHolecup *psHolecup) ;
	void SetRoadManager(CSecretRoadManager *pcRoadManager) ;
	void SetWaterManager(CSecretWaterManager *pcWaterManager) ;

	void ResetCollisionEvent() ;
	void AddCollisionEvent(DWORD dwEvent, bool bEnable=true) ;
	bool IsCollisionEvent(DWORD dwEvent) ;

	void Release() ;
} ;