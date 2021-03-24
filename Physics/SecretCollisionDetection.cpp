#include "SecretCollisionDetection.h"
#include "GolfBall.h"
#include "SecretTerrain.h"
#include "SecretSceneManager.h"
#include "SecretMeshObject.h"
#include "MathOrdinary.h"
#include "SecretTrail.h"
#include "SecretRoad.h"
#include "SecretWater.h"
#include "time.h"

int closest_triangle(const void *pv1, const void *pv2)
{
	geo::STriangle *pstri1 = (geo::STriangle *)pv1 ;
	geo::STriangle *pstri2 = (geo::STriangle *)pv2 ;

	float dist = pstri1->dist - pstri2->dist ;
	if(float_less(dist, 0.0f))
		return 1 ;
	else if(float_greater(dist, 0.0f))
		return -1 ;
	else
		return 0 ;
}

bool isWrongSleepingHolecup(geo::STriangle *psTriangle, CGolfBall *pcGolfBall, CSecretTerrain *pcTerrain)
{
	if(!pcTerrain)
		return false ;

	Vector3 vGolfBallPos = pcGolfBall->GetPos() ;
	Vector3 vHolecupPos = pcTerrain->m_sHolecup.GetHolecupPos() ;
	vGolfBallPos.y = vHolecupPos.y = 0.0f ;
	float distance = (vGolfBallPos-vHolecupPos).Magnitude() ;

	if(distance > 1.0f)
	{
		char str[256] ;
		time_t t ;
		tm *pt ;
		time(&t) ;
		pt = localtime(&t) ;
		sprintf_s(str, 256, "wrongSleeping_%04d%02d%02d%02d%02d%02d.txt", pt->tm_year+1900, pt->tm_mon+1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec) ;
		FILE *pf = fopen(str, "w+") ;
		if(pf)
		{
			sprintf_s(str, 256, "holecup_pos(%g %g %g)\r\n", enumVector(vGolfBallPos)) ;
			fwrite(str, strlen(str), 1, pf) ;
			sprintf_s(str, 256, "triangle_midpos(%g %g %g)\r\n", enumVector(psTriangle->GetMidPos())) ;
			fwrite(str, strlen(str), 1, pf) ;
			sprintf_s(str, 256, "distance=%g\r\n", distance) ;
			fwrite(str, strlen(str), 1, pf) ;
			sprintf_s(str, 256, "ball_pos(%g %g %g), move_kind=%d move_status=%d\r\n", enumVector(pcGolfBall->GetPos()), pcGolfBall->GetMoveKind(), pcGolfBall->GetMoveStatus()) ;
			fwrite(str, strlen(str), 1, pf) ;
			sprintf_s(str, 256, "attr=%d hiword=%d loword=%d\r\n", psTriangle->dwAttr, (psTriangle->dwAttr>>16)&0xffff, psTriangle->dwAttr&0xffff) ;
			fwrite(str, strlen(str), 1, pf) ;
			fclose(pf) ;
		}
		return false ;
	}
	return true ;
}


CSecretCollisionDetection::CSecretCollisionDetection() :
MAXNUM_RIGIDBODY(12),
MAXNUM_PLANE(24),
MAXNUM_COLLIDINGVERTEX(256*3*3),
MAXNUM_COLLIDEDCYLINDER(16),
MAXNUM_COLPRIMITIVE(1024),
COLDET_EPSILON(FLT_EPSILON),
ROLLINGSPACE(0.0001f)
{
	m_nNumRigidbody = 0 ;
	m_ppcRigidbodies = NULL ;

	m_nNumPlane = 0 ;
	m_ppsPlanes = NULL ;

	m_pfCollidingVertices = NULL ;
	m_psCollidedTriangles = NULL ;
	m_pmatCylinders = NULL ;
	m_pvCylinderIntersectPos = NULL ;
	m_pLine = NULL ;

	m_fVelRange = 0.5f ;
	m_fVelBase = 12.0f ;
	m_fMaxIncidenceBase = 1.0f ;
	m_fScaleRestitution = 0.86f ;

	m_vOffsetGolfBallonRoad.set(0.0f, 0.05f, 0.0f) ;

	m_pfuncPlaySound = NULL ;
	m_bEnableVertIntersectedTriangle = false ;
}
CSecretCollisionDetection::~CSecretCollisionDetection()
{
	Release() ;
}

void CSecretCollisionDetection::Release()
{
	SAFE_DELETEARRAY(m_ppcRigidbodies) ;
	SAFE_DELETEARRAY(m_ppsPlanes) ;
	SAFE_DELETEARRAY(m_pfCollidingVertices) ;
	SAFE_DELETEARRAY(m_psCollidedTriangles) ;
	SAFE_DELETEARRAY(m_pmatCylinders) ;
	SAFE_DELETEARRAY(m_pvCylinderIntersectPos) ;
	SAFE_RELEASE(m_pLine) ;

	SAFE_DELETEARRAY(m_psRangedTrianglesIT) ;
	SAFE_DELETEARRAY(m_psIntersectedTriangleCT) ;

	SAFE_DELETEARRAY(m_psBoundingCylindersCT) ;
	SAFE_DELETEARRAY(m_psBoundingSpheresCT) ;
	SAFE_DELETEARRAY(m_psBoundingCylindersRT) ;
	SAFE_DELETEARRAY(m_psBoundingSpheresRT) ;
}

bool CSecretCollisionDetection::Initialize()
{
	m_ppcRigidbodies = new CSecretRigidbody*[MAXNUM_RIGIDBODY] ;
	m_ppsPlanes = new geo::SPlane*[MAXNUM_PLANE] ;

	m_pfCollidingVertices = new float[MAXNUM_COLLIDINGVERTEX] ;
	m_psCollidedTriangles = new geo::STriangle[MAXNUM_COLLIDEDTRIANGLE] ;
	m_nNumCollidedTriangle = 0 ;

	m_pmatCylinders = new Matrix4[MAXNUM_COLLIDEDCYLINDER] ;
	m_pvCylinderIntersectPos = new Vector3[MAXNUM_COLLIDEDCYLINDER] ;
	m_nNumCollidedCylinder = 0 ;

	m_psRangedTrianglesIT = new geo::STriangle[MAXNUM_COLPRIMITIVE] ;
	ZeroMemory(m_psRangedTrianglesIT, sizeof(geo::STriangle)*MAXNUM_COLPRIMITIVE) ;
	m_nNumRangedTriangleIT = 0 ;

	m_psIntersectedTriangleCT = new geo::STriangle[MAXNUM_COLPRIMITIVE] ;
	ZeroMemory(m_psIntersectedTriangleCT, sizeof(geo::STriangle)*MAXNUM_COLPRIMITIVE) ;

	m_psBoundingCylindersCT = new geo::SCylinder[MAXNUM_COLPRIMITIVE] ;
	ZeroMemory(m_psBoundingCylindersCT, sizeof(geo::SCylinder)*MAXNUM_COLPRIMITIVE) ;
	m_psBoundingSpheresCT = new geo::SSphere[MAXNUM_COLPRIMITIVE] ;
	ZeroMemory(m_psBoundingSpheresCT, sizeof(geo::SSphere)*MAXNUM_COLPRIMITIVE) ;

	m_psBoundingCylindersRT = new geo::SCylinder[MAXNUM_COLPRIMITIVE] ;
	ZeroMemory(m_psBoundingCylindersRT, sizeof(geo::SCylinder)*MAXNUM_COLPRIMITIVE) ;
	m_psBoundingSpheresRT = new geo::SSphere[MAXNUM_COLPRIMITIVE] ;
	ZeroMemory(m_psBoundingSpheresRT, sizeof(geo::SSphere)*MAXNUM_COLPRIMITIVE) ;

	return true ;
}
Vector3 CSecretCollisionDetection::_CalculatePureReflection(CGolfBall *pcGolfBall, Vector3 vIncidence, Vector3 vnPlane, float fRestitution)
{
	Vector3 a = ProjectionVector(vIncidence, vnPlane, false) ;
	Vector3 b = (vIncidence-a) ;
	a = -a * fRestitution ;
	return (a+b) ;
}
Vector3 CSecretCollisionDetection::_CalculateReflection(CGolfBall *pcGolfBall, Vector3 vIncidence, Vector3 vnPlane, float fVelocity, float fRestitution)
{
	Vector3 vResult ;

	int nDeformCount = 0 ;
	if(pcGolfBall->m_sContactSurfaceInfo.nKind == SContactSurfaceInfo::OB)
		nDeformCount = 1 ;
	else if(pcGolfBall->m_sContactSurfaceInfo.nKind == SContactSurfaceInfo::ROUGH)
		nDeformCount = 2 ;
	else if(pcGolfBall->m_sContactSurfaceInfo.nKind == SContactSurfaceInfo::GREEN)
		nDeformCount = 5 ;// 4 ;
	else if(pcGolfBall->m_sContactSurfaceInfo.nKind == SContactSurfaceInfo::GREENSIDE)
		nDeformCount = 3 ;
	else if(pcGolfBall->m_sContactSurfaceInfo.nKind == SContactSurfaceInfo::FAIRWAY)
		nDeformCount = 5 ;// 4 ;
	else if(pcGolfBall->m_sContactSurfaceInfo.nKind == SContactSurfaceInfo::TEE)
		nDeformCount = 3 ;

	if(pcGolfBall->m_nCollisionCount < nDeformCount)
	{
		m_fVelRange = pcGolfBall->m_sContactSurfaceInfo.fVelRange ;
		m_fVelBase = pcGolfBall->m_sContactSurfaceInfo.fVelBase ;
		m_fMaxIncidenceBase = pcGolfBall->m_sContactSurfaceInfo.fMaxIncidenceBase ;
		m_fScaleRestitution = pcGolfBall->m_sContactSurfaceInfo.fScaleRestitution ;

		//속도에 따라서 충돌평면을 변형 시켜준다. 속도를 [0.0, 0.5] 범위로 만든다.
		float scale = fVelocity*m_fVelRange/m_fVelBase ;
		if(scale > m_fVelRange)
			scale = m_fVelRange ;

		//입사각에 따라서 또 각도를 조정한다.
		float coef1 = 1.0f - vIncidence.Normalize().dot(-vnPlane) ;
		if(coef1 > m_fMaxIncidenceBase)
			coef1 = m_fMaxIncidenceBase ;

		coef1 *= m_fVelRange/m_fMaxIncidenceBase ;// [0, 85] -> [0, 0.5]
		scale -= coef1 ;
		if(scale < 0.0f)
			scale = 0.0f ;

		Vector3 vnIncidence = vIncidence.Normalize() ;
		float theta = IncludedAngle(vnIncidence, -vnPlane) ;

		Vector3 vnNewPlane ;
		if(float_greater(theta, 0.0f))
			vnNewPlane = GetNewVectorfromTwoVector(-vnPlane, vnIncidence, theta*(scale)) ;
		else
			vnNewPlane = -vnPlane ;

		Vector3 a = ProjectionVector(vIncidence, vnNewPlane, false) ;
		Vector3 b = (vIncidence-a) ;
		a = -a * (fRestitution*m_fScaleRestitution) ;
		vResult = a+b ;
	}
	else
	{
		m_fScaleRestitution = pcGolfBall->m_sContactSurfaceInfo.fScaleRestitution ;

		Vector3 a = ProjectionVector(vIncidence, vnPlane, false) ;
		Vector3 b = (vIncidence-a) ;
		a = -a * (fRestitution*m_fScaleRestitution) ;
		vResult = a+b ;
	}
	return vResult ;

	/*
	vnPlane = -vnPlane ;
	Vector3 a, b ;

	a = ProjectionVector(vIncidence, vnPlane, false)*fRestitution ;
	b = (vIncidence-a) ;
	a = -a ;
	b = a+b ;
	return b ;
	*/

	//Vector3 vn1=a.Normalize(), vn2=b.Normalize(), vn3 ;
	//float theta = IncludedAngle(vn1, vn2) ;
	//vn3 = GetNewVectorfromTwoVector(vn1, vn2, theta*0.75f) ;
	//vn3 *= b.Magnitude() ;
	//return vn3 ;
}
bool CSecretCollisionDetection::_CollisionResponse(CGolfBall *pcGolfBall, geo::SLine *psTrace, geo::SPlane *psPlane, Vector3 vIntersect, int nOption)
{
	float fOldEpsilon = g_fEpsilon ;
	//g_fEpsilon = FLT_EPSILON ;

	if(pcGolfBall->GetMoveKind() == CSecretRigidbody::FLYING)
	{
		Vector3 vIncidenceForce = ProjectionVector(pcGolfBall->GetCurState()->vLinearVel, psPlane->GetNormal(), false) ;
		pcGolfBall->m_fBoundForce = vIncidenceForce.Magnitude() ;

		//vIntersect -= (psTrace->v*ROLLINGSPACE) ;//충돌위치를 약간 뒤로 밀어낸다.

		//홀컵 들어갈때 실린더에 충돌하고 잠시 멈칫하는문제
		//공의 이동거리가 너무 짧아져서 반사되는거 어떡해?

		float dist = psTrace->d - (vIntersect - psTrace->s).Magnitude() ;

		if(float_abs(dist) <= COLDET_EPSILON)
			int iqi=0 ;

		//충돌한 위치까지 간 뒤 그곳에서 (velocity-dist)만큼 이동
		Vector3 vnPlane = psPlane->GetNormal() ;

		//dist = (psTrace->d-dist) ;

		Vector3 vReflection, vIncidence = psTrace->v*dist ;

		vIncidence /= dist ;

		if(nOption & PUREREFLECT)
			vReflection = _CalculatePureReflection(pcGolfBall, vIncidence, vnPlane, pcGolfBall->m_sContactSurfaceInfo.fRestitution) ;
		else
			vReflection = _CalculateReflection(pcGolfBall, vIncidence, vnPlane, pcGolfBall->GetCurState()->vLinearVel.Magnitude()) ;

		vReflection *= dist ;

		TRACE("###Collision Response###\r\n") ;

		//TRACE("Plane Normal(%g %g %g)\r\n", enumVector(vnPlane)) ;

		float fOldLengthOfVert = psPlane->ToPosition(pcGolfBall->GetCurState()->vPos) ;

		//TRACE("old vPos(%g %g %g)\r\n", enumVector(pcGolfBall->GetCurState()->vPos)) ;
		//TRACE("old momentum(%g %g %g)\r\n", enumVector(pcGolfBall->GetCurState()->vLinearMomentum)) ;

		pcGolfBall->GetPrevState()->vPos = vIntersect ;

		if(dist <= COLDET_EPSILON)
			pcGolfBall->GetCurState()->vPos = vIntersect + (psPlane->GetNormal()*ROLLINGSPACE) ;
		else
		{
			if(vReflection.Magnitude() < ROLLINGSPACE)//반사되는 크기가 너무 작을경우는 ROLLINGSPACE 값으로 밀어준다.
			{
				vReflection += vReflection.Normalize()*ROLLINGSPACE ;
				TRACE("too small Reflection(%g %g %g)\r\n", enumVector(vReflection)) ;
			}

			pcGolfBall->GetCurState()->vPos = vIntersect + vReflection ;
		}

		dist = psPlane->ToPosition(pcGolfBall->GetCurState()->vPos) ;//표면보다 공의 들어가있는지 체크
		if(dist<COLDET_EPSILON)
		{
			Vector3 vNewPos = psPlane->GetOnPos(pcGolfBall->GetCurState()->vPos) ;
			vNewPos += psPlane->GetNormal()*ROLLINGSPACE ;
			float xxx = psPlane->ToPosition(vNewPos) ;
			pcGolfBall->GetCurState()->vPos = vNewPos ;
			TRACE("push up golfball newPos(%g %g %g) d=%g\r\n", enumVector(vNewPos), xxx) ;
		}

		//dist = psPlane->ToPosition(pcGolfBall->GetCurState()->vPos) ;//표면보다 공의 들어가있는지 체크
		//if( !(float_abs(dist-pcGolfBall->GetRadius()) <= COLDET_EPSILON) && (dist<pcGolfBall->GetRadius()) )
		//{
		//	pcGolfBall->GetCurState()->vPos += (vnPlane*(pcGolfBall->GetRadius()-dist)) ;
		//	TRACE("push up golfball(%g %g %g)\r\n", enumVector(pcGolfBall->GetCurState()->vPos)) ;
		//}

		Vector3 vnLinearMomentum = pcGolfBall->GetCurState()->vLinearMomentum ;
		float fMomentumDist = vnLinearMomentum.Magnitude() ;
		vnLinearMomentum /= fMomentumDist ;

		if(nOption & PUREREFLECT)
			pcGolfBall->GetCurState()->vLinearMomentum = _CalculatePureReflection(pcGolfBall, vnLinearMomentum, vnPlane, pcGolfBall->m_sContactSurfaceInfo.fRestitution) ;
		else
			pcGolfBall->GetCurState()->vLinearMomentum = _CalculateReflection(pcGolfBall, vnLinearMomentum, vnPlane, pcGolfBall->GetCurState()->vLinearVel.Magnitude(), pcGolfBall->m_sContactSurfaceInfo.fRestitution) ;

		pcGolfBall->GetCurState()->vLinearMomentum *= fMomentumDist ;

		float fCurLengthOfVert = psPlane->ToPosition(pcGolfBall->GetCurState()->vPos) ;

		//TRACE("length of vert old=%g\r\n", fOldLengthOfVert) ;
		//TRACE("length of vert cur=%g\r\n", fCurLengthOfVert) ;
		//TRACE("differ=%g\r\n", fCurLengthOfVert-fOldLengthOfVert) ;

		Vector3 vVert = ProjectionVector(psTrace->v*psTrace->d, psPlane->GetNormal(), false) ;

		//if(pcGolfBall->m_sContactSurfaceInfo.nKind != SContactSurfaceInfo::TREE)
		{
			//if(float_abs(fCurLengthOfVert-fOldLengthOfVert) <= pcGolfBall->GetRadius()*0.075f)//이전 공위치 튕기고난뒤에 공위치 차이가 이정도 난다면
			if((vVert.Magnitude() <= 0.015f) && (nOption & COLLIDED_TRIANGLE) && !(nOption & DONTROLLING))
			{
				//여기에서 공이 제대로 된 위치로 미끌려야 된다.
				Vector3 vNewPos ;
				if(pcGolfBall->m_sContactSurfaceInfo.fRestitution < 0.1f)
				{
					Vector3 vBelowTerrain = (vIntersect - pcGolfBall->GetCurState()->vPos)*0.1f ;
					vNewPos = psPlane->GetOnPos(pcGolfBall->GetCurState()->vPos + vBelowTerrain) ;
				}
				else
					vNewPos = psPlane->GetOnPos(pcGolfBall->GetCurState()->vPos) ;

				vNewPos += (psPlane->GetNormal()*pcGolfBall->GetRadius()) ;
				pcGolfBall->GetCurState()->vPos = vNewPos ;

				Vector3 a = ProjectionVector(pcGolfBall->GetCurState()->vLinearMomentum, -psPlane->GetNormal(), false) ;
				Vector3 vNewMomentum = pcGolfBall->GetCurState()->vLinearMomentum - a ;
				if(!vNewMomentum.IsZero())
					vNewMomentum = vNewMomentum.Normalize() * pcGolfBall->GetCurState()->vLinearMomentum.Magnitude() ;

				pcGolfBall->GetCurState()->vLinearMomentum = vNewMomentum ;

				a = ProjectionVector(pcGolfBall->GetCurState()->vLinearVel, -psPlane->GetNormal(), false) ;
				Vector3 vNewVelocity = pcGolfBall->GetCurState()->vLinearVel - a ;
				if(!vNewVelocity.IsZero())
					vNewVelocity = vNewVelocity.Normalize() * pcGolfBall->GetCurState()->vLinearVel.Magnitude() ;

				pcGolfBall->GetCurState()->vLinearVel = vNewVelocity ;
				pcGolfBall->SetMoveKind(CSecretRigidbody::ROLLING) ;

				//if(pcGolfBall->IsGolfBallAttr(CGolfBall::TRAILING))
				//	pcGolfBall->AddGolfBallAttr(CGolfBall::TRAILING, false) ;
				TRACE("ROLLING\r\n") ;
			}
		}

		if(pcGolfBall->IsGolfBallAttr(CGolfBall::FORCEDGASTATION))
			pcGolfBall->AddGolfBallAttr(CGolfBall::FORCEDGASTATION, false) ;

		pcGolfBall->m_nCollisionCount++ ;
		if(pcGolfBall->m_nCollisionCount == 1)//first hit
			pcGolfBall->m_fTimeAfterFirstHit = 0.0f ;

		//TRACE("dist=%g vVert(%g %g %g) Magnitude=%g\r\n", dist, enumVector(vVert), vVert.Magnitude()) ;
		//TRACE("vIntersect(%g %g %g)\r\n", enumVector(vIntersect)) ;
		//TRACE("vReflect(%g %g %g)\r\n", enumVector(vReflection)) ;
		//TRACE("vPos(%g %g %g)\r\n", enumVector(pcGolfBall->GetCurState()->vPos)) ;
		//TRACE("momentum(%g %g %g)\r\n", enumVector(pcGolfBall->GetCurState()->vLinearMomentum)) ;
		TRACE("\r\n") ;

		_GetVelocityTrace(psTrace, pcGolfBall) ;

		pcGolfBall->m_bBoundEvent = true ;
		if(m_pfuncPlaySound)
			m_pfuncPlaySound(pcGolfBall) ;

		g_fEpsilon = fOldEpsilon ;
		return true ;
	}
	else if(pcGolfBall->GetMoveKind() == CSecretRigidbody::ROLLING)
	{
		Vector3 vNewPos ;
		float dist = psPlane->ToPosition(pcGolfBall->GetCurState()->vPos) ;

		Vector3 vOnPos = psPlane->GetOnPos(pcGolfBall->GetCurState()->vPos) ;
		vOnPos += (psPlane->GetNormal()*pcGolfBall->GetRadius()) ;

		pcGolfBall->GetCurState()->vPos = vOnPos ;

		//if(dist < 0.0f)//오르막길
		//{
		//	vNewPos = pcGolfBall->GetCurState()->vPos + (psPlane->GetNormal()*(-dist+pcGolfBall->GetRadius())) ;
		//}
		//else if(dist > 0.0f)//내리막길
		//{
		//	vNewPos = pcGolfBall->GetCurState()->vPos + (-psPlane->GetNormal()*(dist-pcGolfBall->GetRadius())) ;
		//}
		//else
		//	vNewPos = pcGolfBall->GetCurState()->vPos ;

		//pcGolfBall->GetCurState()->vPos = vNewPos + (psPlane->GetNormal()*ROLLINGSPACE) ;

		float scalar_normalforce = pcGolfBall->m_vNormalForce.Magnitude() ;
		float scalar_momentum = pcGolfBall->GetCurState()->vLinearMomentum.Magnitude() ;

		//마찰은 속도와 관계없으나 구르는 속도가 빠르면 시간당 마찰한 면적이 많기때문에 이것을 계산에 넣어줘야 함.
		float scalar_friction = pcGolfBall->m_sContactSurfaceInfo.fFriction * scalar_normalforce ;
		//scalar_friction = scalar_friction > pcGolfBall->m_sContactSurfaceInfo.fFriction ? scalar_friction : pcGolfBall->m_sContactSurfaceInfo.fFriction ;

		//if(dist < 0.0f)
		//	//TRACE("오르막길\r\n") ;
		//else if(dist > 0.0f)
		//	//TRACE("내리막길\r\n") ;

		//TRACE("[ROLLING MOTION] normalforce=%g friction=%g momentum=%g golfballpos(%g %g %g)\r\n",
		//	scalar_normalforce, scalar_friction, scalar_momentum, enumVector(pcGolfBall->GetCurState()->vPos)) ;

		if(scalar_friction >= scalar_momentum)
		{
			pcGolfBall->GetCurState()->vLinearMomentum.set(0, 0, 0) ;
			pcGolfBall->SetMoveStatus(CSecretRigidbody::SLEEP) ;
			pcGolfBall->m_fSleepCount = 0.0f ;
			TRACE("SLEEP\r\n") ;
			return true ;
			//scalar_friction = scalar_momentum * 0.9f ;
		}

		Vector3 a = ProjectionVector(pcGolfBall->GetCurState()->vLinearMomentum, -psPlane->GetNormal(), false) ;
		Vector3 vNewMomentum = pcGolfBall->GetCurState()->vLinearMomentum - a ;
		vNewMomentum = vNewMomentum.Normalize() * scalar_momentum ;

		float fScalingFriction = (psTrace->d*pcGolfBall->m_sContactSurfaceInfo.fRollingCoef) ;
		scalar_friction = (fScalingFriction < 1.0f) ? scalar_friction : scalar_friction * (psTrace->d*pcGolfBall->m_sContactSurfaceInfo.fRollingCoef) ;

		Vector3 vFriction = -vNewMomentum.Normalize() * scalar_friction ;

		pcGolfBall->GetCurState()->vLinearMomentum = vNewMomentum + vFriction ;

		_GetVelocityTrace(psTrace, pcGolfBall) ;

		g_fEpsilon = fOldEpsilon ;
		//static int count=0 ;
		////TRACE("friction(%07.03f %07.03f %07.03f) momentum(%07.03f %07.03f %07.03f) count=%d\r\n",
		//	enumVector(vFriction), enumVector(pcGolfBall->GetCurState()->vLinearMomentum), count++) ;

		return true ;
	}
	//static int failcount=0 ;
	////TRACE("response fail %d\r\n", failcount++) ;
	//IntersectLinetoPlane(*psTrace, sIntersectPlane, vIntersect) ;
	g_fEpsilon = fOldEpsilon ;
	return false ;
}

void CSecretCollisionDetection::_FindTriangleInRolling(Vector3 &vPos, float fRadius, geo::STriangle *psRangedTriangles, int nNumRangedTriangle, geo::STriangle **ppsClosestTriangle, float &fClosest, Vector3 &vOnPlane)
{
	int i ;
	float d, closest=fRadius ;
	Vector3 vtemp, vBaryCenter ;
	//geo::STriangle *psClosestTriangle = *ppsClosestTriangle ;
	
	for(i=0 ; i<nNumRangedTriangle ; i++)
	{
		d = psRangedTriangles[i].sPlane.ToPosition(vPos) ;
		if(float_less(float_abs(d), closest))
		{
			vtemp = vPos-(psRangedTriangles[i].sPlane.GetNormal()*d) ;//평면위로 점을 이동시킴.
			if(IsPointOnTriangle(vtemp, psRangedTriangles[i], vBaryCenter))
			{
				vOnPlane = vtemp ;
				//psClosestTriangle = &psRangedTriangles[i] ;
				*ppsClosestTriangle = &psRangedTriangles[i] ;
				fClosest = d ;
			}
		}
	}
}

#define MAXNUM_STATICBOUNDINGBOLUME 512

bool CSecretCollisionDetection::_CollidingTriangles(CGolfBall *pcGolfBall, geo::SLine *psTrace, geo::STriangle *psRangedTriangles, int nNumRangedTriangle, CSecretTerrain *pcTerrain, STrueRenderingObject *psTRObject)
{
	//if(pcGolfBall->m_bSleepInHolecup)
	//	return false ;

	bool bCollision=false ;
	//static geo::STriangle sIntersectedTriangle[MAXNUM_STATICBOUNDINGBOLUME] ;
	//static geo::SCylinder sBoundingCylinders[MAXNUM_STATICBOUNDINGBOLUME] ;
	//static geo::SSphere sBoundingSpheres[MAXNUM_STATICBOUNDINGBOLUME] ;

	////TRACE("#### _CollidingTriangles numTri=%d //TRACE start(%g %g %g) end(%g %g %g)\r\n",
	//	nNumRangedTriangle, enumVector(psTrace->s), enumVector(psTrace->GetEndPos())) ;

	Vector3 vProj ;
	geo::STriangle sProjTriangle ;
	int nResponseOption ;
	int i, col_count, nNumIntersectedTriangle=0, nNumCylinder=0, nNumSphere=0, nTotalCylinderCount=0, nTotalSphereCount=0 ;
	Vector3 vIntersect, vClosestIntersect ;
	geo::SCylinder sCylinder ;

	for(i=0 ; i<nNumRangedTriangle ; i++)
	{
		////TRACE("[%d] collidedTriangle y=%g box_center(%g %g %g) box_corner(%g %g %g) boxsize(%g %g %g)\r\n",
		//	i, psRangedTriangles[i].avVertex[0].vPos.y, enumVector(psRangedTriangles[i].avVertex[0].vPos), enumVector(psRangedTriangles[i].avVertex[1].vPos), enumVector(psRangedTriangles[i].avVertex[2].vPos)) ;

		//일단 실린더와 스피어를 만들어둠.
		for(int n=0 ; n<3 ; n++)//Create Bounding-Cylinders
		{
			bool bExistCylinder=false, bExistSphere=false ;
			int nStart = n ;
			int nEnd = ((n+1)<3) ? n+1 : 0 ;
			//sCylinder.set(psRangedTriangles[i].avVertex[nStart].vPos, psRangedTriangles[i].avVertex[nEnd].vPos, pcGolfBall->GetRadius()) ;
			nTotalCylinderCount++ ;

			for(int cylinder_count=0 ; cylinder_count<nNumCylinder ; cylinder_count++)
			{
				if( (vector_eq(psRangedTriangles[i].avVertex[nStart].vPos, m_psBoundingCylindersCT[cylinder_count].vStart) && vector_eq(psRangedTriangles[i].avVertex[nEnd].vPos, m_psBoundingCylindersCT[cylinder_count].vEnd))
					|| (vector_eq(psRangedTriangles[i].avVertex[nEnd].vPos, m_psBoundingCylindersCT[cylinder_count].vStart) && vector_eq(psRangedTriangles[i].avVertex[nStart].vPos, m_psBoundingCylindersCT[cylinder_count].vEnd)) )
				{
					bExistCylinder=true ;
					break ;
				}
			}
			if(!bExistCylinder)
			{
				m_psBoundingCylindersCT[nNumCylinder].set(psRangedTriangles[i].avVertex[nStart].vPos, psRangedTriangles[i].avVertex[nEnd].vPos, pcGolfBall->GetRadius()) ;
				m_psBoundingCylindersCT[nNumCylinder++].dwAttr = psRangedTriangles[i].dwAttr ;
				assert(nNumCylinder < MAXNUM_STATICBOUNDINGBOLUME) ;
			}

			nTotalSphereCount++ ;
			for(int sphere_count=0 ; sphere_count<nNumSphere ; sphere_count++)
			{
				if(vector_eq(psRangedTriangles[i].avVertex[n].vPos, m_psBoundingSpheresCT[sphere_count].vPos))
				{
					bExistSphere = true ;
					break ;
				}
			}
			if(!bExistSphere)
			{
				m_psBoundingSpheresCT[nNumSphere].set(psRangedTriangles[i].avVertex[n].vPos, pcGolfBall->GetRadius()) ;
				m_psBoundingSpheresCT[nNumSphere++].dwAttr = psRangedTriangles[i].dwAttr ;
				assert(nNumSphere < MAXNUM_STATICBOUNDINGBOLUME) ;
			}
		}

		//먼저 삼각형의 바운딩박스와 골프공충돌테스트
		Vector3 vDist = psTrace->GetPos(0.5f) - psRangedTriangles[i].sBoundingSphere.vPos ;
		if(vDist.Magnitude() <= (psRangedTriangles[i].sBoundingSphere.fRadius+psTrace->d))
		{
			m_psIntersectedTriangleCT[nNumIntersectedTriangle++] = psRangedTriangles[i] ;
			assert(nNumIntersectedTriangle < MAXNUM_STATICBOUNDINGBOLUME) ;
		}
	}
	if(!nNumIntersectedTriangle)
		return bCollision ;

	//TRACE("bitch_line(Vector3(%g, %g, %g), Vector3(%g, %g, %g), %g) ;\r\n", enumVector(psTrace->s), enumVector(psTrace->v), psTrace->d) ;

	float fClosest ;
	geo::STriangle sClosestIntersectedTriangle ;

	//2번 충돌검출을 하겠음.
	for(col_count=0 ; col_count<2 ; col_count++)
	{
		fClosest=FLT_MAX ;
		sClosestIntersectedTriangle.set(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0)) ;
		sClosestIntersectedTriangle.bSelected = false ;
		//충돌검색
		for(i=0 ; i<nNumIntersectedTriangle ; i++)
		{
			//공의 반지름만큼 튀어나온 삼각형을 만듬.
			vProj = m_psIntersectedTriangleCT[i].sPlane.GetNormal()*pcGolfBall->GetRadius() ;
			sProjTriangle.set(m_psIntersectedTriangleCT[i].avVertex[0].vPos+vProj, m_psIntersectedTriangleCT[i].avVertex[1].vPos+vProj, m_psIntersectedTriangleCT[i].avVertex[2].vPos+vProj) ;

			//TRACE("bitch_tri.set(Vector3(%g, %g, %g), Vector3(%g, %g, %g), Vector3(%g, %g, %g)) ;\r\n",
			//	enumVector(sIntersectedTriangle[i].avVertex[0].vPos), enumVector(sIntersectedTriangle[i].avVertex[1].vPos), enumVector(sIntersectedTriangle[i].avVertex[2].vPos)) ;

			//TRACE("cGeoRenderer.AddTriangle(&bitch_tri, 0xffff0000) ;\r\n") ;

			//TRACE("bitch_tri.set(Vector3(%g, %g, %g), Vector3(%g, %g, %g), Vector3(%g, %g, %g)) ;\r\n",
			//	enumVector(sProjTriangle.avVertex[0].vPos), enumVector(sProjTriangle.avVertex[1].vPos), enumVector(sProjTriangle.avVertex[2].vPos)) ;

			//TRACE("cGeoRenderer.AddTriangle(&bitch_tri, 0xffff0000) ;\r\n") ;

			if(float_abs(m_psIntersectedTriangleCT[i].sPlane.ToPosition(pcGolfBall->GetCurState()->vPos)) > (psTrace->d+pcGolfBall->GetRadius()))
				continue ;

			//이것!!!!!!!!!!!!!
			//홀컵인경우 평면의 방향과 공의 진행방향 관계에서 이상해지는데... -_- 일단 이렇게 하고 나중에 다시 보자규
			//if(!(sIntersectedTriangle[i].dwAttr & COLLISION_IN_HOLECUP) && float_greater(psTrace->v.dot(sIntersectedTriangle[i].sPlane.GetNormal()), 0.0f))
			if(float_greater(psTrace->v.dot(m_psIntersectedTriangleCT[i].sPlane.GetNormal()), 0.0f))
				continue ;

			float prevDist = sProjTriangle.sPlane.ToPosition(pcGolfBall->GetPrevState()->vPos) ;
			float curDist = sProjTriangle.sPlane.ToPosition(pcGolfBall->GetCurState()->vPos) ;

			//이 충돌테스트에서 나온 위치가 바로 실제 공이 충돌한 위치
			if(IntersectLinetoTriangle(*psTrace, sProjTriangle, vIntersect, true, true, COLDET_EPSILON) == geo::INTERSECT_POINT)
			{
				float ddd = psTrace->d ;
				float dist = (vIntersect - psTrace->s).Magnitude() ;
				if(dist < fClosest)
				{
					fClosest = dist ;
					vClosestIntersect = vIntersect ;
					sClosestIntersectedTriangle = m_psIntersectedTriangleCT[i] ;
					sClosestIntersectedTriangle.dwAttr = m_psIntersectedTriangleCT[i].dwAttr ;
					sClosestIntersectedTriangle.bSelected = true ;
				}
			}
		}
		if(sClosestIntersectedTriangle.bSelected)
		{
			//1차 충돌검색으로 선택된 삼각형 충돌처리
			bool bCouldbeSleep=false ;
			nResponseOption = COLLIDED_TRIANGLE ;
			if(pcTerrain)
			{
				if(sClosestIntersectedTriangle.dwAttr & COLLISION_IN_HOLECUP)
				{
					DWORD dwIndex = sClosestIntersectedTriangle.dwAttr&0xffff ;
					pcTerrain->m_sHeightMap.GetSurfaceInfofromHolecup(dwIndex, &pcGolfBall->m_sContactSurfaceInfo) ;

					if((dwIndex >= 16) && (dwIndex <= 33))
					{
						if(dwIndex <= 31)//실린더는 구르지 않음
							nResponseOption |= DONTROLLING ;
						bCouldbeSleep = true ;
					}
				}
				else
				{
					if(m_pcRoadManager->Intersect(Vector3(vClosestIntersect.x, 0.0f, vClosestIntersect.z)))//해저드속성이지만 로드에 사용함.
					{
						pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::HAZARD, &pcGolfBall->m_sContactSurfaceInfo) ;
						pcGolfBall->m_vOffset = m_vOffsetGolfBallonRoad ;
					}
					else
					{
						pcTerrain->m_sHeightMap.GetSurfaceInfo(vClosestIntersect.x, vClosestIntersect.z, &pcGolfBall->m_sContactSurfaceInfo) ;
						if(pcGolfBall->m_sContactSurfaceInfo.bGroundHazard)//Ground Hazard
						{
							int kind = pcGolfBall->m_sContactSurfaceInfo.nKind ;
							pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::OB, &pcGolfBall->m_sContactSurfaceInfo) ;
							pcGolfBall->m_sContactSurfaceInfo.nKind = kind ;
							pcGolfBall->m_sContactSurfaceInfo.bGroundHazard = true ;
						}
						pcGolfBall->m_vOffset.set(0.0f, 0.0f, 0.0f) ;
					}
				}
			}
			else if(psTRObject)
				pcGolfBall->m_sContactSurfaceInfo = psTRObject->sContactSurfaceInfo ;
				//pcGolfBall->m_sContactSurfaceInfo.set(SContactSurfaceInfo::TREE, 0.7f, 0.05f, 1.0f) ;//임시로 설정

			//TRACE("intersect with Triangle restitution=%g friction=%g\r\n", pcGolfBall->m_sContactSurfaceInfo.fRestitution, pcGolfBall->m_sContactSurfaceInfo.fFriction) ;

			if(_CollisionResponse((CGolfBall *)pcGolfBall, psTrace, &sClosestIntersectedTriangle.sPlane, vClosestIntersect, nResponseOption))
			{
				//TRACE("intersect with Triangle\r\n") ;
				TRACE("intersect triangle plane(%g %g %g) normal(%g %g %g)\r\n", enumVector(vIntersect), enumVector(sClosestIntersectedTriangle.sPlane.GetNormal())) ;
				if(pcGolfBall->GetMoveKind() == CSecretRigidbody::ROLLING)//충돌반응이 Rolling일경우 알맞는 처리를 해줘야함.
				{
					pcGolfBall->m_bContactedPlane = true ;
					pcGolfBall->m_sContactedPlane = sClosestIntersectedTriangle.sPlane ;
					pcGolfBall->m_bContactedTriangle = true ;
					pcGolfBall->m_sContactedTriangle = sClosestIntersectedTriangle ;

					_RollingTriangles(pcGolfBall, psTrace, psRangedTriangles, nNumRangedTriangle, pcTerrain, psTRObject) ;

					if(bCouldbeSleep && (pcGolfBall->GetMoveStatus() == CSecretRigidbody::SLEEP))
					{
						if(isWrongSleepingHolecup(&sClosestIntersectedTriangle, pcGolfBall, m_pcTerrain))
						{
							pcGolfBall->m_bSleepInHolecup = true ;
							TRACE("m_bSleepInHolecup is true\r\n") ;
							if(m_pfuncPlaySound)
								m_pfuncPlaySound(pcGolfBall) ;
							return true ;
						}
					}
					return true ;
				}

				m_nNumCollidedTriangle = 0 ;
				m_psCollidedTriangles[m_nNumCollidedTriangle++] = sClosestIntersectedTriangle ;

				bCollision = true ;

				if( (float_abs(psTrace->d) <= COLDET_EPSILON) )
					return bCollision ;
			}
		}
		else
			break ;
	}

	//if(nNumIntersectedTriangle == 0)//평면과 충돌하지 않았으면
	{
		////TRACE("no hit plane\r\n") ;

		int n ;
		geo::SCylinder *psBoundingCylinder ;

		float fNearest=FLT_MAX, ftemp ;
		geo::SCylinder *psNearestCylinder=NULL ;
		Vector3 vNearestIntersect ;

		for(n=0 ; n<nNumCylinder ; n++)//가장 가까운 실린더를 찾는다.
		{
			psBoundingCylinder = &m_psBoundingCylindersCT[n] ;
			if(psBoundingCylinder->Intersect(*psTrace, vIntersect, COLDET_EPSILON) == geo::INTERSECT_POINT)
			{
				float fOldEpsilon = g_fEpsilon ;
				//g_fEpsilon = COLDET_EPSILON ;
				ftemp = (vIntersect - psTrace->s).Magnitude() ;
				if(fNearest > ftemp)
				{
					fNearest = ftemp ;
					vNearestIntersect = vIntersect ;
					psNearestCylinder = psBoundingCylinder ;
				}

				g_fEpsilon = fOldEpsilon ;				
			}
		}
		if(psNearestCylinder)
		{
			psBoundingCylinder = psNearestCylinder ;
			vIntersect = vNearestIntersect ;

			geo::SPlane sIntersectedPlane ;
			Vector3 va = vIntersect-psBoundingCylinder->vStart ;
			Vector3 vb = psBoundingCylinder->vEnd-psBoundingCylinder->vStart ;
			Vector3 vc = ProjectionVector(va, vb) ;
			if(vc.IsZero())
				sIntersectedPlane.set(vIntersect, (vIntersect-psBoundingCylinder->vStart).Normalize()) ;
			else
				sIntersectedPlane.set(vIntersect, (va-vc).Normalize()) ;

			TRACE("intersect whth cylinder start(%g %g %g) end(%g %g %g)\r\n", enumVector(psBoundingCylinder->vStart), enumVector(psBoundingCylinder->vEnd)) ;
			TRACE("Trail Golfball start(%g %g %g) end(%g %g %g) dir(%g %g %g)\r\n",
				enumVector(psTrace->s), enumVector(psTrace->GetEndPos()), enumVector(psTrace->v) ) ;
			TRACE("intersect pos(%g %g %g) reflect plane(%g %g %g) \r\n", enumVector(pcGolfBall->GetCurState()->vPos), enumVector(sIntersectedPlane.GetNormal()) ) ;

			m_nNumCollidedCylinder = 0 ;
			_GetCylinderMatrix(psBoundingCylinder, &m_pmatCylinders[m_nNumCollidedCylinder]) ;
			m_pvCylinderIntersectPos[m_nNumCollidedCylinder] = vIntersect ;
			m_nNumCollidedCylinder++ ;

			//float dist = sIntersectedPlane.ToPosition(vIntersect) ;
			////TRACE("ball dist from intersectPlane=%g\r\n", dist) ;
			//if(float_eq(dist, 0.0f))
			//	int xxxxx=0 ;//continue ;

			//nResponseOption = COLLIDED_CYLINDER ;

			if(pcTerrain)
			{
				if(psBoundingCylinder->dwAttr & COLLISION_IN_HOLECUP)
				{
					DWORD dwIndex = psBoundingCylinder->dwAttr&0xffff ;
					pcTerrain->m_sHeightMap.GetSurfaceInfofromHolecup(dwIndex, &pcGolfBall->m_sContactSurfaceInfo) ;
					nResponseOption |= PUREREFLECT ;
				}
				else
				{
					if(m_pcRoadManager->Intersect(Vector3(vIntersect.x, 0.0f, vIntersect.z)))//해저드속성이지만 로드에 사용함.
					{
						pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::HAZARD, &pcGolfBall->m_sContactSurfaceInfo) ;
						pcGolfBall->m_vOffset = m_vOffsetGolfBallonRoad ;
					}
					else
					{
						pcTerrain->m_sHeightMap.GetSurfaceInfo(vIntersect.x, vIntersect.z, &pcGolfBall->m_sContactSurfaceInfo) ;
						if(pcGolfBall->m_sContactSurfaceInfo.bGroundHazard)//Ground Hazard
						{
							int kind = pcGolfBall->m_sContactSurfaceInfo.nKind ;
							pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::OB, &pcGolfBall->m_sContactSurfaceInfo) ;
							pcGolfBall->m_sContactSurfaceInfo.nKind = kind ;
							pcGolfBall->m_sContactSurfaceInfo.bGroundHazard = true ;
						}
						pcGolfBall->m_vOffset.set(0.0f, 0.0f, 0.0f) ;
					}
				}
			}
			else if(psTRObject)
				pcGolfBall->m_sContactSurfaceInfo = psTRObject->sContactSurfaceInfo ;
				//pcGolfBall->m_sContactSurfaceInfo.set(SContactSurfaceInfo::TREE, 0.7f, 0.05f, 1.0f) ;//임시로 설정

			////TRACE("intersect with Cylinder restitution=%g friction=%g\r\n", pcGolfBall->m_sContactSurfaceInfo.fRestitution, pcGolfBall->m_sContactSurfaceInfo.fFriction) ;

			if(_CollisionResponse((CGolfBall *)pcGolfBall, psTrace, &sIntersectedPlane, vIntersect, nResponseOption))
			{
				TRACE("intersect cylinder (%07.03f %07.03f %07.03f)\r\n", enumVector(vIntersect)) ;
				if(pcGolfBall->GetMoveKind() == CSecretRigidbody::ROLLING)
				{
					pcGolfBall->m_bContactedPlane = true ;
					pcGolfBall->m_sContactedPlane = sIntersectedPlane ;
				}
				bCollision = true ;

				if( (float_abs(psTrace->d) <= COLDET_EPSILON) )
					return bCollision ;
			}
		}

		float fClosestDist = FLT_MAX ;
		Vector3 vClosestSpherePos ;
		
		geo::SSphere *psBoundingSphere ;
		for(n=0 ; n<nNumSphere ; n++)
		{
			psBoundingSphere = &m_psBoundingSpheresCT[n] ;

			Vector3 vDist = pcGolfBall->GetCurState()->vPos - psBoundingSphere->vPos ;
			if(fClosestDist > vDist.Magnitude())
			{
				fClosestDist = vDist.Magnitude() ;
				vClosestSpherePos = psBoundingSphere->vPos ;
			}

			//if(IntersectLineToSphere(psTrace, psBoundingSphere->vPos, psBoundingSphere->fRadius, COLDET_EPSILON))
			if(IntersectLineToSphere(psTrace, psBoundingSphere, vIntersect, COLDET_EPSILON))
			//if(float_less(vDist.Magnitude(), pcGolfBall->GetRadius()))
			{
				IntersectLineToSphere(psTrace, psBoundingSphere, vIntersect, COLDET_EPSILON) ;
				////TRACE("vDistLength=%07.03f\r\n", vDist.Magnitude()) ;
				////TRACE("vertex to golfball dir(%07.03f %07.03f %07.03f)\r\n", enumVector(vDist)) ;
				////TRACE("vDistNormal(%07.03f %07.03f %07.03f)\r\n", enumVector(vDist.Normalize())) ;

				//vIntersect = sBoundingSpheres[n].vPos + (vDist.Normalize()*pcGolfBall->GetRadius()) ;
				vDist = vIntersect - psBoundingSphere->vPos ;
				geo::SPlane sIntersectedPlane(vIntersect, vDist.Normalize()) ;

				//vIntersect = sBoundingSpheres[n].vPos + (-psTrace->v*pcGolfBall->GetRadius()) ;
				//geo::SPlane sIntersectedPlane(vIntersect, -psTrace->v) ;

				////TRACE("Trail Golfball start(%g %g %g) end(%g %g %g) dir(%g %g %g)\r\n",
				//	enumVector(psTrace->s), enumVector(psTrace->GetEndPos()), enumVector(psTrace->v) ) ;

				////TRACE("intersect (%g %g %g)\r\n", enumVector(vIntersect)) ;
				////TRACE("intersectPlane nor(%g %g %g)\r\n", enumVector(sIntersectedPlane.GetNormal())) ;

				nResponseOption = COLLIDED_SPHERE ;

				if(pcTerrain)
				{
					if(psBoundingSphere->dwAttr & COLLISION_IN_HOLECUP)
					{
						pcTerrain->m_sHeightMap.GetSurfaceInfofromHolecup(psBoundingSphere->dwAttr&0xffff, &pcGolfBall->m_sContactSurfaceInfo) ;
						nResponseOption |= PUREREFLECT ;
					}
					else
					{
						if(m_pcRoadManager->Intersect(Vector3(vIntersect.x, 0.0f, vIntersect.z)))//해저드속성이지만 로드에 사용함.
						{
							pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::HAZARD, &pcGolfBall->m_sContactSurfaceInfo) ;
							pcGolfBall->m_vOffset = m_vOffsetGolfBallonRoad ;
						}
						else
						{
							pcTerrain->m_sHeightMap.GetSurfaceInfo(vIntersect.x, vIntersect.z, &pcGolfBall->m_sContactSurfaceInfo) ;
							if(pcGolfBall->m_sContactSurfaceInfo.bGroundHazard)//Ground Hazard
							{
								int kind = pcGolfBall->m_sContactSurfaceInfo.nKind ;
								pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::OB, &pcGolfBall->m_sContactSurfaceInfo) ;
								pcGolfBall->m_sContactSurfaceInfo.nKind = kind ;
								pcGolfBall->m_sContactSurfaceInfo.bGroundHazard = true ;
							}
							pcGolfBall->m_vOffset.set(0.0f, 0.0f, 0.0f) ;
						}
					}
				}
				else if(psTRObject)
					pcGolfBall->m_sContactSurfaceInfo = psTRObject->sContactSurfaceInfo ;
					//pcGolfBall->m_sContactSurfaceInfo.set(SContactSurfaceInfo::TREE, 0.7f, 0.05f, 1.0f) ;//임시로 설정

				//TRACE("intersect with Sphere restitution=%g friction=%g\r\n", pcGolfBall->m_sContactSurfaceInfo.fRestitution, pcGolfBall->m_sContactSurfaceInfo.fFriction) ;

				if(_CollisionResponse((CGolfBall *)pcGolfBall, psTrace, &sIntersectedPlane, vIntersect, nResponseOption))
				{
					TRACE("intersect sphere (%07.03f %07.03f %07.03f)\r\n", enumVector(vIntersect)) ;
					if(pcGolfBall->GetMoveKind() == CSecretRigidbody::ROLLING)
					{
						pcGolfBall->m_bContactedPlane = true ;
						pcGolfBall->m_sContactedPlane = sIntersectedPlane ;
					}
					bCollision = true ;

					if( (float_abs(psTrace->d) <= COLDET_EPSILON) )
						return bCollision ;
				}
			}
		}
	}

	////TRACE("_CollidingTriangles golfballpos(%g %g %g) ####\r\n", enumVector(pcGolfBall->GetCurState()->vPos)) ;

	return bCollision ;
	//return false ;
}
void CSecretCollisionDetection::_isSafe_colTri_to_rolTri(CGolfBall *pcGolfBall, geo::STriangle *psRolledTriangle)
{

}

bool CSecretCollisionDetection::_RollingTriangles(CGolfBall *pcGolfBall, geo::SLine *psTrace, geo::STriangle *psRangedTriangles, int nNumRangedTriangle, CSecretTerrain *pcTerrain, STrueRenderingObject *psTRObject)
{
	int i ;
	Vector3 vProj, vIntersect ;
	geo::STriangle sProjTriangle ;
	bool bRollingEvent=false ;
	float dist ;

	//TRACE("_RollingTriangles numTriangle=%d golfballpos(%g %g %g)\r\n", nNumRangedTriangle, enumVector(pcGolfBall->GetCurState()->vPos)) ;

	//for(i=0 ; i<nNumRangedTriangle ; i++)
	//{
	//	vProj = psRangedTriangles[i].sPlane.GetNormal()*pcGolfBall->GetRadius() ;
	//	sProjTriangle.set(psRangedTriangles[i].avVertex[0].vPos+vProj, psRangedTriangles[i].avVertex[1].vPos+vProj, psRangedTriangles[i].avVertex[2].vPos+vProj) ;



	//	dist = sProjTriangle.sPlane.ToPosition(pcGolfBall->GetCurState()->vPos) ;
	//	if(float_greater(dist, 0.0f) && float_less_eq(dist, pcGolfBall->GetRadius()))
	//	{
	//		//TRACE("#######################################\r\n") ;
	//		//TRACE("intersect with plane in rolling motion\r\n") ;
	//		//TRACE("#######################################\r\n") ;

	//		//TRACE("dist=%g\r\n", dist) ;
	//	}
	//}

	for(i=0 ; i<nNumRangedTriangle ; i++)
	{
		if( float_greater(psRangedTriangles[i].sPlane.ToPosition(pcGolfBall->GetCurState()->vPos), pcGolfBall->GetRadius()) )
			continue ;

		if(psTrace->d > COLDET_EPSILON)
		{
			vProj = psRangedTriangles[i].sPlane.GetNormal()*pcGolfBall->GetRadius() ;
			sProjTriangle.set(psRangedTriangles[i].avVertex[0].vPos+vProj, psRangedTriangles[i].avVertex[1].vPos+vProj, psRangedTriangles[i].avVertex[2].vPos+vProj) ;		

			//if(IntersectLinetoTriangle(*psTrace, sProjTriangle, vIntersect, true) == geo::INTERSECT_POINT)
			if(IntersectLinetoTriangle(*psTrace, sProjTriangle, vIntersect, true, true, COLDET_EPSILON) == geo::INTERSECT_POINT)
			{
				float dot = pcGolfBall->m_sContactedPlane.GetNormal().dot(sProjTriangle.sPlane.GetNormal()) ;
				////TRACE("Intersect Point dot=%g\r\n", dot) ;
				if( float_less_eq(dot, 0.15f) )//수직벽 or 앞으로 기울어진벽
				{
					dist = (vIntersect - pcGolfBall->GetCurState()->vPos).Magnitude() ;
					if(dist <= COLDET_EPSILON)//현재 공의 위치가 충돌된 지점과 같음.
					{
						//TRACE("not yet rolling collision barrier wall\r\n") ;
					}
					else//충돌처리
					{
						//충돌한표면의 정보를 얻는다.
						if(pcTerrain)
						{
							if(psRangedTriangles[i].dwAttr & COLLISION_IN_HOLECUP)
								pcTerrain->m_sHeightMap.GetSurfaceInfofromHolecup(psRangedTriangles[i].dwAttr&0xffff, &pcGolfBall->m_sContactSurfaceInfo) ;
							else
							{
								if(m_pcRoadManager->Intersect(Vector3(vIntersect.x, 0.0f, vIntersect.z)))//해저드속성이지만 로드에 사용함.
								{
									pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::HAZARD, &pcGolfBall->m_sContactSurfaceInfo) ;
									pcGolfBall->m_vOffset = m_vOffsetGolfBallonRoad ;
								}
								else
								{
									pcTerrain->m_sHeightMap.GetSurfaceInfo(vIntersect.x, vIntersect.z, &pcGolfBall->m_sContactSurfaceInfo) ;
									if(pcGolfBall->m_sContactSurfaceInfo.bGroundHazard)//Ground Hazard
									{
										int kind = pcGolfBall->m_sContactSurfaceInfo.nKind ;
										pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::OB, &pcGolfBall->m_sContactSurfaceInfo) ;
										pcGolfBall->m_sContactSurfaceInfo.nKind = kind ;
										pcGolfBall->m_sContactSurfaceInfo.bGroundHazard = true ;
									}
									pcGolfBall->m_vOffset.set(0.0f, 0.0f, 0.0f) ;
								}
							}
						}
						else if(psTRObject)
							pcGolfBall->m_sContactSurfaceInfo = psTRObject->sContactSurfaceInfo ;
							//pcGolfBall->m_sContactSurfaceInfo.set(SContactSurfaceInfo::TREE, 0.7f, 0.05f, 1.0f) ;//임시로 설정

						//Vector3 vReflect = _CalculateReflection(pcGolfBall, psTrace->v, sProjTriangle.sPlane.GetNormal(), pcGolfBall->GetCurState()->vLinearVel.Magnitude()) ;
						//Vector3 vReflect ;
						//if(psRangedTriangles[i].dwAttr & COLLISION_IN_HOLECUP)
						//	vReflect = _CalculatePureReflection(pcGolfBall, psTrace->v, sProjTriangle.sPlane.GetNormal(), 1.0f) ;
						//else
						//	Vector3 vReflect = _CalculatePureReflection(pcGolfBall, psTrace->v, sProjTriangle.sPlane.GetNormal(), pcGolfBall->m_sContactSurfaceInfo.fRestitution) ;

						Vector3 vReflect = _CalculatePureReflection(pcGolfBall, psTrace->v, sProjTriangle.sPlane.GetNormal(), 1.0f) ;

						vReflect *= dist ;

						Vector3 vNewPos = vIntersect + vReflect ;

						//geo::STriangle *psClosestTriangle ;
						//float fClosest ;
						//_FindTriangleInRolling(vNewPos, pcGolfBall->GetRadius(), psRangedTriangles, nNumRangedTriangle, &psClosestTriangle, fClosest) ;

						pcGolfBall->GetPrevState()->vPos = vIntersect ;
						pcGolfBall->GetPrevState()->vLinearMomentum = pcGolfBall->GetCurState()->vLinearMomentum ;

						pcGolfBall->GetCurState()->vPos = vNewPos ;
						pcGolfBall->GetCurState()->vLinearMomentum = vReflect.Normalize() * pcGolfBall->GetCurState()->vLinearMomentum.Magnitude() ;// * pcGolfBall->m_sContactSurfaceInfo.fRestitution ;

						pcGolfBall->m_bContactedPlane = true ;//예전에 접촉했던 평면 그대로

						_GetVelocityTrace(psTrace, pcGolfBall) ;

						TRACE("수직벽Barrier Wall Collision prevMomentum=%g curMomemtum=%g planeNormal(%g %g %g\r\n",
							pcGolfBall->GetPrevState()->vLinearMomentum.Magnitude(), pcGolfBall->GetCurState()->vLinearMomentum.Magnitude(), enumVector(sProjTriangle.sPlane.GetNormal())) ;

						return true ;
					}
				}
				else if(float_less(dot, 1.0f))
				{
					dist = (vIntersect - pcGolfBall->GetCurState()->vPos).Magnitude() ;
					if(dist <= COLDET_EPSILON)//현재 공의 위치가 충돌된 지점과 같음.
					{
						//TRACE("not yet rolling collision barrier wall\r\n") ;
					}
					else//충돌처리 이건 타고 올라가야 된다.
					{
						//충돌한표면의 정보를 얻는다.
						DWORD dwIndex=0 ;
						if(pcTerrain)
						{
							if(psRangedTriangles[i].dwAttr & COLLISION_IN_HOLECUP)
							{
								dwIndex = psRangedTriangles[i].dwAttr&0xffff ;
								pcTerrain->m_sHeightMap.GetSurfaceInfofromHolecup(dwIndex, &pcGolfBall->m_sContactSurfaceInfo) ;
							}
							else
							{
								if(m_pcRoadManager->Intersect(Vector3(vIntersect.x, 0.0f, vIntersect.z)))//해저드속성이지만 로드에 사용함.
								{
									pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::HAZARD, &pcGolfBall->m_sContactSurfaceInfo) ;
									pcGolfBall->m_vOffset = m_vOffsetGolfBallonRoad ;
								}
								else
								{
									pcTerrain->m_sHeightMap.GetSurfaceInfo(vIntersect.x, vIntersect.z, &pcGolfBall->m_sContactSurfaceInfo) ;
									if(pcGolfBall->m_sContactSurfaceInfo.bGroundHazard)//Ground Hazard
									{
										int kind = pcGolfBall->m_sContactSurfaceInfo.nKind ;
										pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::OB, &pcGolfBall->m_sContactSurfaceInfo) ;
										pcGolfBall->m_sContactSurfaceInfo.nKind = kind ;
										pcGolfBall->m_sContactSurfaceInfo.bGroundHazard = true ;
									}
									pcGolfBall->m_vOffset.set(0.0f, 0.0f, 0.0f) ;
								}
							}
						}
						else if(psTRObject)
							pcGolfBall->m_sContactSurfaceInfo = psTRObject->sContactSurfaceInfo ;
							//pcGolfBall->m_sContactSurfaceInfo.set(SContactSurfaceInfo::TREE, 0.7f, 0.05f, 1.0f) ;//임시로 설정

						float d1 = sProjTriangle.sPlane.ToPosition(pcGolfBall->GetCurState()->vPos) ;
						float d2 = sProjTriangle.sPlane.ToPosition(psTrace->GetEndPos()) ;

						if(d1 > 0.0f)
							int wiofjwf=0 ;

						//닮은꼴 삼각형의 비를 이용햇지롱
						float fEmbeddedLength = (pcGolfBall->GetCurState()->vPos-vIntersect).Magnitude() ;
						Vector3 vNewEnd = vIntersect + psTrace->v ;

						Vector3 vOnPlane = sProjTriangle.sPlane.GetOnPos(vNewEnd) ;
						Vector3 vMomentumDir = vOnPlane - vIntersect ;
						float fMomentumLength = vMomentumDir.Magnitude() ;
						fMomentumLength *= fEmbeddedLength ;

						vMomentumDir = vMomentumDir.Normalize() ;

						pcGolfBall->GetPrevState()->vPos = vIntersect ;
						pcGolfBall->GetPrevState()->vLinearMomentum = pcGolfBall->GetCurState()->vLinearMomentum ;
						//위치보정
						Vector3 vNewPos = vIntersect + (vMomentumDir*fMomentumLength) ;
						float d = sProjTriangle.sPlane.ToPosition(vNewPos) ;
						if(float_abs(d) > FLT_EPSILON)
							vNewPos -= (sProjTriangle.sPlane.GetNormal()*d) ;

						pcGolfBall->GetCurState()->vPos = vNewPos ;
						
						//Vector3 vOnPlane = sProjTriangle.sPlane.GetOnPos(pcGolfBall->GetCurState()->vPos) ;

						//pcGolfBall->GetPrevState()->vPos = vIntersect ;
						//pcGolfBall->GetPrevState()->vLinearMomentum = pcGolfBall->GetCurState()->vLinearMomentum ;

						////20100120 vIntersect 와 vOnPlane 가 너무 작을때 ROLLINGSPACE를 더하니까 전혀다른 방향으로 갔었네ㅠㅠ
						////vOnPlane += (sProjTriangle.sPlane.GetNormal()*ROLLINGSPACE) ;
						//pcGolfBall->GetCurState()->vPos = vOnPlane ;

						//Vector3 vMomentumDir = (vOnPlane-vIntersect).Normalize() ;

						//Vector3 vcurMomentumDir = pcGolfBall->GetCurState()->vLinearMomentum.Normalize() ;
						//if(vMomentumDir.dot(vcurMomentumDir) < 0)
						//	int wwdfw=0 ;

						pcGolfBall->GetCurState()->vLinearMomentum = vMomentumDir * pcGolfBall->GetCurState()->vLinearMomentum.Magnitude() ;// * pcGolfBall->m_sContactSurfaceInfo.fRestitution ;

						_GetVelocityTrace(psTrace, pcGolfBall) ;

						//if((dwIndex >= 16) && (dwIndex <= 31))
						//{
						//}
						//else
						float angle = IncludedAngle(pcGolfBall->m_sContactedTriangle.sPlane.GetNormal(), psRangedTriangles[i].sPlane.GetNormal()) ;
						angle = rad_deg(angle) ;

						pcGolfBall->m_bContactedPlane = true ;
						pcGolfBall->m_sContactedPlane = psRangedTriangles[i].sPlane ;
						pcGolfBall->m_bContactedTriangle = true ;
						pcGolfBall->m_sContactedTriangle = psRangedTriangles[i] ;
						bRollingEvent = true ;

						m_nNumCollidedTriangle = 0 ;
						m_psCollidedTriangles[m_nNumCollidedTriangle++] = psRangedTriangles[i] ;

						TRACE("완만한 오르막 Barrier Wall Collision planeNormal(%g %g %g) includeAngle=%g\r\n",
							enumVector(sProjTriangle.sPlane.GetNormal()), angle) ;
						return true ;
					}
				}
			}
		}
	}

	////TRACE("intersectedTriangles=%d\r\n", nNumRangedTriangle) ;

	//static geo::SCylinder m_psBoundingCylinders[MAXNUM_STATICBOUNDINGBOLUME] ;
	//static geo::SSphere sBoundingSpheres[MAXNUM_STATICBOUNDINGBOLUME] ;

	int nNumIntersectedTriangle=0, nNumCylinder=0, nNumSphere=0, nTotalCylinderCount=0, nTotalSphereCount=0 ;

	for(i=0 ; i<nNumRangedTriangle ; i++)
	{
		//일단 실린더와 스피어를 만들어둠.
		for(int n=0 ; n<3 ; n++)//Create Bounding-Cylinders
		{
			bool bExistCylinder=false, bExistSphere=false ;
			int nStart = n ;
			int nEnd = ((n+1)<3) ? n+1 : 0 ;
			//sCylinder.set(psRangedTriangles[i].avVertex[nStart].vPos, psRangedTriangles[i].avVertex[nEnd].vPos, pcGolfBall->GetRadius()) ;
			nTotalCylinderCount++ ;

			for(int cylinder_count=0 ; cylinder_count<nNumCylinder ; cylinder_count++)
			{
				if( (vector_eq(psRangedTriangles[i].avVertex[nStart].vPos, m_psBoundingCylindersRT[cylinder_count].vStart) && vector_eq(psRangedTriangles[i].avVertex[nEnd].vPos, m_psBoundingCylindersRT[cylinder_count].vEnd))
					|| (vector_eq(psRangedTriangles[i].avVertex[nEnd].vPos, m_psBoundingCylindersRT[cylinder_count].vStart) && vector_eq(psRangedTriangles[i].avVertex[nStart].vPos, m_psBoundingCylindersRT[cylinder_count].vEnd)) )
				{
					bExistCylinder=true ;
					break ;
				}
			}
			if(!bExistCylinder)
			{
				m_psBoundingCylindersRT[nNumCylinder].set(psRangedTriangles[i].avVertex[nStart].vPos, psRangedTriangles[i].avVertex[nEnd].vPos, pcGolfBall->GetRadius()) ;
				m_psBoundingCylindersRT[nNumCylinder++].dwAttr = psRangedTriangles[i].dwAttr ;
				assert(nNumCylinder < MAXNUM_STATICBOUNDINGBOLUME) ;
			}

			nTotalSphereCount++ ;
			for(int sphere_count=0 ; sphere_count<nNumSphere ; sphere_count++)
			{
				if(vector_eq(psRangedTriangles[i].avVertex[n].vPos, m_psBoundingSpheresRT[sphere_count].vPos))
				{
					bExistSphere = true ;
					break ;
				}
			}
			if(!bExistSphere)
			{
				m_psBoundingSpheresRT[nNumSphere].set(psRangedTriangles[i].avVertex[n].vPos, pcGolfBall->GetRadius()) ;
				m_psBoundingSpheresRT[nNumSphere++].dwAttr = psRangedTriangles[i].dwAttr ;
				assert(nNumSphere < MAXNUM_STATICBOUNDINGBOLUME) ;
			}
		}
	}

	int nVertIntersectedTriangleCount=0 ;
	geo::STriangle *psVertIntersectedTriangle=NULL ;

	for(i=0 ; i<nNumRangedTriangle ; i++)
	{
		if( float_greater(psRangedTriangles[i].sPlane.ToPosition(pcGolfBall->GetCurState()->vPos), pcGolfBall->GetRadius()+0.0001f) )
			continue ;

		if(psTrace->v.dot(psRangedTriangles[i].sPlane.GetNormal()) >= 0.85f)
			continue ;

		////TRACE("m_psCollidedTriangles[%d].set(Vector3(%gf, %gf, %gf), Vector3(%gf, %gf, %gf), Vector3(%gf, %gf, %gf));\r\n",
		//	i, 
		//	enumVector(psRangedTriangles[i].avVertex[0].vPos), enumVector(psRangedTriangles[i].avVertex[1].vPos), enumVector(psRangedTriangles[i].avVertex[2].vPos)) ;

		//geo::SLine sVertline(pcGolfBall->GetCurState()->vPos, -psRangedTriangles[i].sPlane.GetNormal(), pcGolfBall->GetRadius()*2.0f) ;

		geo::SLine sVertline ;

		//수직아래로 교차된 삼각형을 찾아낸다. 요거는 한개만 나와야됨
		sVertline.set(pcGolfBall->GetCurState()->vPos, Vector3(0, -1, 0), pcGolfBall->GetRadius()*2.0f) ;
		if(IntersectLinetoTriangle(sVertline, psRangedTriangles[i], vIntersect, true, true, COLDET_EPSILON) == geo::INTERSECT_POINT)
		{
			psVertIntersectedTriangle = &psRangedTriangles[i] ;
			nVertIntersectedTriangleCount++ ;
		}

		sVertline.set(pcGolfBall->GetCurState()->vPos, -psRangedTriangles[i].sPlane.GetNormal(), pcGolfBall->GetRadius()*2.0f) ;

		if(IntersectLinetoTriangle(sVertline, psRangedTriangles[i], vIntersect, true, true, COLDET_EPSILON) == geo::INTERSECT_POINT)
		{
			bool could_be_sleep_in_holecup=false ;
			int nResponseOption=COLLIDED_TRIANGLE ;

			////TRACE("sVertline\r\n") ;
			if(pcTerrain)
			{
				if(psRangedTriangles[i].dwAttr & COLLISION_IN_HOLECUP)
				{
					DWORD dwIndex = psRangedTriangles[i].dwAttr&0xffff ;
					pcTerrain->m_sHeightMap.GetSurfaceInfofromHolecup(dwIndex, &pcGolfBall->m_sContactSurfaceInfo) ;
					if((dwIndex >= 16) && (dwIndex <= 39))
					{
						//if(dwIndex <= 31)//실린더일경우는 구르지 않고
						//	continue ;
							//nResponseOption |= DONTROLLING ;

						if(dwIndex <= 31)//실린더일경우는 구르지 않고
							nResponseOption |= DONTROLLING ;

						could_be_sleep_in_holecup = true ;
					}
				}
				else
				{
					if(m_pcRoadManager->Intersect(Vector3(vIntersect.x, 0.0f, vIntersect.z)))//해저드속성이지만 로드에 사용함.
					{
						pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::HAZARD, &pcGolfBall->m_sContactSurfaceInfo) ;
						pcGolfBall->m_vOffset = m_vOffsetGolfBallonRoad ;
					}
					else
					{
						pcTerrain->m_sHeightMap.GetSurfaceInfo(vIntersect.x, vIntersect.z, &pcGolfBall->m_sContactSurfaceInfo) ;
						if(pcGolfBall->m_sContactSurfaceInfo.bGroundHazard)//Ground Hazard
						{
							int kind = pcGolfBall->m_sContactSurfaceInfo.nKind ;
							pcTerrain->m_sHeightMap.getTerrainSurfaceProperty(SContactSurfaceInfo::OB, &pcGolfBall->m_sContactSurfaceInfo) ;
							pcGolfBall->m_sContactSurfaceInfo.nKind = kind ;
							pcGolfBall->m_sContactSurfaceInfo.bGroundHazard = true ;
						}
						pcGolfBall->m_vOffset.set(0.0f, 0.0f, 0.0f) ;
					}
				}
			}
			else if(psTRObject)
				pcGolfBall->m_sContactSurfaceInfo = psTRObject->sContactSurfaceInfo ;
				//pcGolfBall->m_sContactSurfaceInfo.set(SContactSurfaceInfo::TREE, 0.7f, 0.05f, 1.0f) ;//임시로 설정

			//수직벽을 타고 구르는 공을 잡아내야함.
			if( float_greater_eq(psRangedTriangles[i].sPlane.GetNormal().dot(Vector3(0, -1, 0)), 0.0f) )//중력의 방향과 비교
				nResponseOption |= DONTROLLING ;

			if(_CollisionResponse((CGolfBall *)pcGolfBall, psTrace, &psRangedTriangles[i].sPlane, vIntersect, nResponseOption))
			{
				//TRACE("Rolling CollisionResponse\r\n") ;

				//TRACE("INTERSECTED Triangle (%gf, %gf, %gf) (%gf, %gf, %gf) (%gf, %gf, %gf) normal(%gf, %gf, %gf) index=%d\r\n",
				//	enumVector(psRangedTriangles[i].avVertex[0].vPos), enumVector(psRangedTriangles[i].avVertex[1].vPos), enumVector(psRangedTriangles[i].avVertex[2].vPos), enumVector(psRangedTriangles[i].sPlane.GetNormal()), psRangedTriangles[i].dwAttr&0xffff) ;

				m_nNumCollidedTriangle = 0 ;
				m_psCollidedTriangles[m_nNumCollidedTriangle++] = psRangedTriangles[i] ;

				if(could_be_sleep_in_holecup && (pcGolfBall->GetMoveStatus() == CGolfBall::SLEEP))
				{
					if(isWrongSleepingHolecup(&psRangedTriangles[i], pcGolfBall, m_pcTerrain))
					{
						pcGolfBall->m_bSleepInHolecup = true ;
						TRACE("m_bSleepInHolecup is true\r\n") ;
						if(m_pfuncPlaySound)
							m_pfuncPlaySound(pcGolfBall) ;
					}
				}

				float angle = IncludedAngle(pcGolfBall->m_sContactedTriangle.sPlane.GetNormal(), psRangedTriangles[i].sPlane.GetNormal()) ;
				angle = rad_deg(angle) ;

				//TRACE("includeAngle=%g\r\n", angle) ;

				pcGolfBall->m_bContactedPlane = true ;
				pcGolfBall->m_sContactedPlane = psRangedTriangles[i].sPlane ;
				pcGolfBall->m_bContactedTriangle = true ;
				pcGolfBall->m_sContactedTriangle = psRangedTriangles[i] ;
				bRollingEvent = true ;
				return true ;
			}
		}
	}

	if(nVertIntersectedTriangleCount > 0)
	{
		TRACE("###nVertIntersectedTriangleCount###\r\n") ;

		Vector3 vPos = pcGolfBall->m_sContactedPlane.GetOnPos(pcGolfBall->GetCurState()->vPos) ;
		//vPos = psVertIntersectedTriangle->sPlane.GetOnPos(vPos) ;
		//vPos += (psVertIntersectedTriangle->sPlane.GetNormal()*pcGolfBall->GetRadius()) ;
		pcGolfBall->GetCurState()->vPos = vPos ;

		if(_CollisionResponse((CGolfBall *)pcGolfBall, psTrace, &psVertIntersectedTriangle->sPlane, vIntersect, COLLIDED_TRIANGLE))
		{
			//TRACE("Rolling CollisionResponse in nVertIntersectedTriangleCount\r\n") ;

			//TRACE("INTERSECTED Triangle (%gf, %gf, %gf) (%gf, %gf, %gf) (%gf, %gf, %gf) normal(%gf, %gf, %gf) index=%d\r\n",
			//	enumVector(psRangedTriangles[i].avVertex[0].vPos), enumVector(psRangedTriangles[i].avVertex[1].vPos), enumVector(psRangedTriangles[i].avVertex[2].vPos), enumVector(psRangedTriangles[i].sPlane.GetNormal()), psRangedTriangles[i].dwAttr&0xffff) ;

			m_nNumCollidedTriangle = 0 ;
			m_psCollidedTriangles[m_nNumCollidedTriangle++] = *psVertIntersectedTriangle ;

			if(pcGolfBall->GetMoveStatus() == CGolfBall::SLEEP)
			{
				if(isWrongSleepingHolecup(&psRangedTriangles[i], pcGolfBall, m_pcTerrain))
				{
					pcGolfBall->m_bSleepInHolecup = true ;
					TRACE("m_bSleepInHolecup is true\r\n") ;
					if(m_pfuncPlaySound)
						m_pfuncPlaySound(pcGolfBall) ;
				}
			}

			float angle = IncludedAngle(pcGolfBall->m_sContactedTriangle.sPlane.GetNormal(), psVertIntersectedTriangle->sPlane.GetNormal()) ;
			angle = rad_deg(angle) ;

			//TRACE("includeAngle=%g\r\n", angle) ;

			pcGolfBall->m_bContactedPlane = true ;
			pcGolfBall->m_sContactedPlane = psVertIntersectedTriangle->sPlane ;
			pcGolfBall->m_bContactedTriangle = true ;
			pcGolfBall->m_sContactedTriangle = *psVertIntersectedTriangle ;
			bRollingEvent = true ;
			return true ;
		}
	}
	return true ;
}

void CSecretCollisionDetection::_GetVelocityTrace(geo::SLine *psLine, CSecretRigidbody *pcRigidbody, Matrix4 *pmatWorld)
{
	Vector3 vStart, vEnd ;

	if(pmatWorld)
	{
		vStart = pcRigidbody->GetPrevState()->vPos * (*pmatWorld) ;
		vEnd = pcRigidbody->GetCurState()->vPos * (*pmatWorld) ;
	}
	else
	{
		vStart = pcRigidbody->GetPrevState()->vPos ;
		vEnd = pcRigidbody->GetCurState()->vPos ;
	}

	Vector3 vTRACE = vEnd-vStart ;

	psLine->s = vStart ;
	psLine->d = vTRACE.Magnitude() ;
	if(psLine->d <= COLDET_EPSILON)
	{
		psLine->d = 0.0f ;
		psLine->v.set(0, 0, 0) ;
	}
	else
	{
		psLine->v = vTRACE.Normalize() ;
	}
}

void CSecretCollisionDetection::_GetCylinderMatrix(geo::SCylinder *psCylinder, Matrix4 *pmat)
{
	Vector3 dist = psCylinder->vEnd - psCylinder->vStart ;
	Vector3 vCenter = psCylinder->vStart + (dist.Normalize()*dist.Magnitude()*0.5f) ;

	Matrix4 matT, matR, matS ;

	float fScaling = psCylinder->fRadius/0.2f ;
	matS.m11 = matS.m33 = fScaling ;
	matS.m22 = dist.Magnitude() ;

	Quaternion q(dist.Normalize(), Vector3(0, 1, 0)) ;
	q.GetMatrix(matR) ;

	matT.m41 = vCenter.x ;
	matT.m42 = vCenter.y ;
	matT.m43 = vCenter.z ;
	(*pmat) = matS*matR*matT ;
}

void CSecretCollisionDetection::_IntersectTerrain(CGolfBall *pcGolfBall)
{
	//if(vector_eq(pcGolfBall->GetCurState()->vPos, pcGolfBall->GetPrevState()->vPos))
	//	return ;

	geo::SLine sTRACE ;
	_GetVelocityTrace(&sTRACE, pcGolfBall) ;

	if(pcGolfBall->GetMoveKind() == CSecretRigidbody::FLYING)
	{
		Vector3 vPos = (pcGolfBall->GetCurState()->vPos + pcGolfBall->GetPrevState()->vPos)*0.5f ;
		float r = (pcGolfBall->GetCurState()->vPos - pcGolfBall->GetPrevState()->vPos).Magnitude()*0.5f ;
		if(float_eq(r, 0.0f))
			r = 1.0f ;
		m_nNumRangedTriangleIT = m_pcTerrain->m_sHeightMap.GetTriangles(vPos, r, m_psRangedTrianglesIT, true, true, 512) ;
		Vector3 vIntersect ;

		if(m_nNumRangedTriangleIT == 0)
			return ;

		if(_CollidingTriangles(pcGolfBall, &sTRACE, m_psRangedTrianglesIT, m_nNumRangedTriangleIT, m_pcTerrain, NULL))
		{
			_CollidingTriangles(pcGolfBall, &sTRACE, m_psRangedTrianglesIT, m_nNumRangedTriangleIT, m_pcTerrain, NULL) ;//2번에 걸쳐 충돌테스트
			return ;
		}
	}
	else if(pcGolfBall->GetMoveKind() == CSecretRigidbody::ROLLING)
	{
		Vector3 vPos = pcGolfBall->GetCurState()->vPos ;
		float r = (vPos - pcGolfBall->GetPrevState()->vPos).Magnitude() ;
		if(float_eq(r, 0.0f))
			r = 1.0f ;
		m_nNumRangedTriangleIT = m_pcTerrain->m_sHeightMap.GetTriangles(vPos, r, m_psRangedTrianglesIT, true, true, 512) ;
		////TRACE("intersectTerrain Rolling numTriangle=%d\r\n", nNumTriangle) ;

		//nNumTriangle = m_pcTerrain->m_sHeightMap.GetTriangles(vPos, pcGolfBall->GetRadius(), sTriangles) ;
		Vector3 vIntersect ;

		if(m_nNumRangedTriangleIT == 0)
			return ;
		
		_RollingTriangles(pcGolfBall, &sTRACE, m_psRangedTrianglesIT, m_nNumRangedTriangleIT, m_pcTerrain, NULL) ;
	}
}

void CSecretCollisionDetection::_IntersectRigidbody(CGolfBall *pcGolfBall)
{
	//if(vector_eq(pcGolfBall->GetCurState()->vPos, pcGolfBall->GetPrevState()->vPos))
	//	return ;

	if(m_nNumRigidbody <= 0)
		return ;

	int i, n ;
	CSecretMeshObject *pcMeshObject ;
	CSecretMesh *pcMesh ;
	CollisionModel3D *pcCollision3d ;
	D3DXMATRIX mat ;
	float m[16], ori[3], dir[3] ;

	geo::SLine sTrace ;
	_GetVelocityTrace(&sTrace, pcGolfBall) ;

	for(i=0 ; i<m_nNumRigidbody ; i++)
	{
		if(m_ppcRigidbodies[i]->GetBodykind() == CSecretRigidbody::PLANT)// 폴리곤 테스트
		{
			pcMeshObject = m_ppcRigidbodies[i]->GetTRObject()->apcObject[0] ;
			for(n=0 ; n<pcMeshObject->GetNumMesh() ; n++)
			{
				pcMesh = pcMeshObject->GetMesh(n) ;
				{
					pcCollision3d = pcMesh->GetCollisionModel() ;
					if(pcCollision3d == NULL)
						continue ;

					//mat = (*pcMesh->GetmatLocal()) * m_ppcRigidbodies[i]->GetTRObject()->matWorld ;
					mat = m_ppcRigidbodies[i]->GetTRObject()->matWorld ;
					m[0] =  mat._11 ; m[1]  = mat._12 ; m[2]  = mat._13 ; m[3]  = mat._14 ; 
					m[4] =  mat._21 ; m[5]  = mat._22 ; m[6]  = mat._23 ; m[7]  = mat._24 ; 
					m[8] =  mat._31 ; m[9]  = mat._32 ; m[10] = mat._33 ; m[11] = mat._34 ; 
					m[12] = mat._41 ; m[13] = mat._42 ; m[14] = mat._43 ; m[15] = mat._44 ; 
					pcCollision3d->setTransform(m) ;

					static geo::STriangle sRangedTriangles[512] ;
					int nNumCollidingTriangle ;
					float fScaling = 10.0f ;

					if(float_greater(sTrace.d, 0.0f))
					//if(sTrace.d > COLDET_EPSILON)
					{
						ori[0] = sTrace.s.x ;
						ori[1] = sTrace.s.y ;
						ori[2] = sTrace.s.z ;

						dir[0] = sTrace.v.x ;
						dir[1] = sTrace.v.y ;
						dir[2] = sTrace.v.z ;
					}
					else//롤링일때 공의 움직임이 멈춤.
					{
						ori[0] = pcGolfBall->GetCurState()->vPos.x ;
						ori[1] = pcGolfBall->GetCurState()->vPos.y ;
						ori[2] = pcGolfBall->GetCurState()->vPos.z ;

						dir[0] = 0 ;
						dir[1] = pcGolfBall->GetRadius() ;
						dir[2] = 0 ;
					}

					nNumCollidingTriangle = pcCollision3d->intersectBoundingBox(ori, dir, pcGolfBall->GetRadius()*fScaling, 0.0f, sTrace.d*fScaling) ;
					if(nNumCollidingTriangle >= 512)
					{
						OutputStringforRelease("############nNumCollidingTriangle=%d#################\r\n", nNumCollidingTriangle) ;
						continue ;
					}
					else
					{
						if(nNumCollidingTriangle)
						{
							pcCollision3d->getCollidingTriangles(m_pfCollidingVertices, false) ;

							Matrix4 matTransform = MatrixConvert(mat) ;
							float *pfCollidingVertices = m_pfCollidingVertices ;
							int nVertexCount=0 ;
							Vector3 avPos[3] ;

							for(int c=0 ; c<nNumCollidingTriangle ; c++)
							{
								avPos[0].x = *pfCollidingVertices++ ;
								avPos[0].y = *pfCollidingVertices++ ;
								avPos[0].z = *pfCollidingVertices++ ;

								avPos[1].x = *pfCollidingVertices++ ;
								avPos[1].y = *pfCollidingVertices++ ;
								avPos[1].z = *pfCollidingVertices++ ;

								avPos[2].x = *pfCollidingVertices++ ;
								avPos[2].y = *pfCollidingVertices++ ;
								avPos[2].z = *pfCollidingVertices++ ;

								sRangedTriangles[c].set(avPos[0], avPos[1], avPos[2]) ;
								sRangedTriangles[c].MakeBoundingShpere() ;

								//m_psCollidedTriangles[m_nNumCollidedTriangle++] = sRangedTriangles[c] ;
							}
						}
					}

					bool bcollide ;
					if(pcGolfBall->GetMoveKind() == CSecretRigidbody::FLYING)
					{
						bcollide = _CollidingTriangles(pcGolfBall, &sTrace, sRangedTriangles, nNumCollidingTriangle, NULL, m_ppcRigidbodies[i]->GetTRObject()) ;
						if(bcollide)//1번더
							_CollidingTriangles(pcGolfBall, &sTrace, sRangedTriangles, nNumCollidingTriangle, NULL, m_ppcRigidbodies[i]->GetTRObject()) ;
					}
					else if(pcGolfBall->GetMoveKind() == CSecretRigidbody::ROLLING)
						bcollide = _RollingTriangles(pcGolfBall, &sTrace, sRangedTriangles, nNumCollidingTriangle, NULL, m_ppcRigidbodies[i]->GetTRObject()) ;

					//현재 충돌된 오브젝트를 저장한다
					pcGolfBall->m_psContactedObject = m_ppcRigidbodies[i]->GetTRObject() ;
					//TRACE("ball contactedobject is %x\r\n", pcGolfBall->m_psContactedObject) ;
				}
			}
		}
	}
}
bool CSecretCollisionDetection::_IntersectWater(CGolfBall *pcGolfBall)
{
	Vector3 vIntersect ;
	geo::SLine sTrace ;
	_GetVelocityTrace(&sTrace, pcGolfBall) ;
	int nNum = m_pcWaterManager->Intersect(&sTrace, vIntersect) ;
	if(nNum >= 0)
	{
		pcGolfBall->GetCurState()->vLinearMomentum.set(0, 0, 0) ;
		pcGolfBall->SetMoveStatus(CSecretRigidbody::SLEEP) ;
		pcGolfBall->m_fSleepCount = 0.0f ;
		pcGolfBall->AddGolfBallAttr(CGolfBall::SLEEPINWATERHAZARD) ;
		//TRACE("SLEEP\r\n") ;

		pcGolfBall->m_pcIntersectedWater = m_pcWaterManager->GetWater(nNum) ;

		m_dwCollisionEvent |= COLLISIONEVENT_INTERSECTWATER ;

		if(m_pfuncPlaySound)
			m_pfuncPlaySound(pcGolfBall) ;

		return true ;
	}
	return false ;
}

void CSecretCollisionDetection::Process()
{
	int n ;

	//TRACE("### COLLISION DETECTION START ###\r\n") ;

	CGolfBall *pcGolfBall ;
	for(n=0 ; n<m_nNumRigidbody ; n++)
	{
		//rigidbody GolfBall
		if((m_ppcRigidbodies[n]->GetBodykind() == CSecretRigidbody::GOLFBALL) && (m_ppcRigidbodies[n]->GetMoveStatus() == CSecretRigidbody::ACTIVITY))
		{
			pcGolfBall = (CGolfBall *)m_ppcRigidbodies[n] ;

			pcGolfBall->m_bContactedPlane = false ;
			pcGolfBall->m_bContactedTriangle = false ;

			if(!_IntersectWater(pcGolfBall))
			{
				_IntersectRigidbody(pcGolfBall) ;
				_IntersectTerrain(pcGolfBall) ;
			}

			if(!pcGolfBall->m_bContactedPlane && pcGolfBall->GetMoveKind() == CSecretRigidbody::ROLLING)
			{
				pcGolfBall->SetMoveKind(CSecretRigidbody::FLYING) ;
				TRACE("FLYING\r\n") ;
			}
		}
	}
	//TRACE("### COLLISION DETECTION END ###\r\n") ;
}

void CSecretCollisionDetection::Render(D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj)
{
	if(!m_nNumCollidedTriangle)
		return ;

	m_pcTerrain->m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;
	D3DXMATRIX matVP = (*pmatView) * (*pmatProj) ;

	D3DXVECTOR3 avPos[4] ;
	int i, n, nCount=0 ;

	//m_pLine->SetWidth(3.0f) ;
	//i = m_motherfucker ;
	//{
	//	for(n=0 ; n<3 ; n++)
	//		avPos[n] = D3DXVECTOR3(m_psCollidedTriangles[i].avVertex[n].vPos.x, m_psCollidedTriangles[i].avVertex[n].vPos.y, m_psCollidedTriangles[i].avVertex[n].vPos.z) ;

	//	avPos[n] = D3DXVECTOR3(m_psCollidedTriangles[i].avVertex[0].vPos.x, m_psCollidedTriangles[i].avVertex[0].vPos.y, m_psCollidedTriangles[i].avVertex[0].vPos.z) ;

	//	m_pLine->Begin() ;
	//	m_pLine->DrawTransform(avPos, 4, &matVP, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)) ;
	//	m_pLine->End() ;
	//}

	//m_pLine->SetWidth(0.8f) ;
	for(i=0 ; i<m_nNumCollidedTriangle ; i++)
	{
		for(n=0 ; n<3 ; n++)
			avPos[n] = D3DXVECTOR3(m_psCollidedTriangles[i].avVertex[n].vPos.x, m_psCollidedTriangles[i].avVertex[n].vPos.y, m_psCollidedTriangles[i].avVertex[n].vPos.z) ;

		avPos[n] = D3DXVECTOR3(m_psCollidedTriangles[i].avVertex[0].vPos.x, m_psCollidedTriangles[i].avVertex[0].vPos.y, m_psCollidedTriangles[i].avVertex[0].vPos.z) ;

		m_pLine->Begin() ;
		m_pLine->DrawTransform(avPos, 4, &matVP, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)) ;
		m_pLine->End() ;
	}

	m_pcTerrain->m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) ;
}

void CSecretCollisionDetection::ClearCollidedTriangle()
{
	m_nNumCollidedTriangle = 0 ;
}

void CSecretCollisionDetection::AddRigidbody(CSecretRigidbody *pcRigidbody)
{
	if(m_nNumRigidbody >= MAXNUM_RIGIDBODY)
		return ;

	m_ppcRigidbodies[m_nNumRigidbody++] = pcRigidbody ;
}
void CSecretCollisionDetection::AddPlane(geo::SPlane *psPlane)
{
	if(m_nNumPlane >= MAXNUM_PLANE)
		return ;

	m_ppsPlanes[m_nNumPlane++] = psPlane ;
}
void CSecretCollisionDetection::ClearRigidbody()
{
	m_nNumRigidbody = 0 ;
}
void CSecretCollisionDetection::ClearPlane()
{
	m_nNumPlane = 0 ;
}
void CSecretCollisionDetection::SetTerrain(CSecretTerrain *pcTerrain)
{
	m_pcTerrain = pcTerrain ;

	D3DXCreateLine(m_pcTerrain->m_pd3dDevice, &m_pLine) ;
	m_pLine->SetWidth(2.0f) ;
	m_pLine->SetAntialias(TRUE) ;
}
void CSecretCollisionDetection::SetHolecup(SHolecup *psHolecup)
{
	m_motherfucker = 0 ;
	m_nNumCollidedTriangle = psHolecup->sCollisionHolecup.nNumTriangle ;
	for(int i=0 ; i<m_nNumCollidedTriangle ; i++)
		m_psCollidedTriangles[i] = psHolecup->sCollisionHolecup.psTriangles[i] ;
}
void CSecretCollisionDetection::SetRoadManager(CSecretRoadManager *pcRoadManager)
{
	m_pcRoadManager = pcRoadManager ;
}
void CSecretCollisionDetection::SetWaterManager(CSecretWaterManager *pcWaterManager)
{
	m_pcWaterManager = pcWaterManager ;
}
void CSecretCollisionDetection::ResetCollisionEvent()
{
	m_dwCollisionEvent = 0 ;
}
void CSecretCollisionDetection::AddCollisionEvent(DWORD dwEvent, bool bEnable)
{
	if(bEnable)
		m_dwCollisionEvent |= dwEvent ;
	else
		m_dwCollisionEvent &= (~dwEvent) ;
}
bool CSecretCollisionDetection::IsCollisionEvent(DWORD dwEvent)
{
	if(m_dwCollisionEvent & dwEvent)
		return true ;
	return false ;
}