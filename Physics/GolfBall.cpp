#include "GolfBall.h"
#include "SecretTrail.h"
#include "SecretSceneManager.h"
#include "D3DMathLib.h"

#include <stdio.h>

//float g_fAirResistance = 0.000625f ;
float g_fAirResistance = 0.0008f ;
float g_fBackspin = 0.227f ;// 0.19275f ;//11.0f ;
float g_fNormalForce = 0.5f ;
float g_fGolfBallMass = GOLFBALL_MASS ;

CGolfBall::CGolfBall()
{
	m_bContactedPlane = false ;

	m_fAirDensity = 1.225f ;//공기밀도
	m_fCp = 0.22f ;//투피스공의 항력계수
	m_fAreaOfIntersect = pi*0.02f*0.02f ;
	//m_vGravity.set(0, -0.48f, 0) ;
	m_vGravity.set(0, -0.5f, 0) ;

	m_nGolfball_Attr = 0 ;
	m_nMaxNumGAStationPos = 0 ;
	m_nLatestGAStationNode = 0 ;
	m_pcFlyingTrail = NULL ;
	m_pcRollingTrail = NULL ;

	m_nGAUnitType = GA_UNITTYPE_INCH ;
}
CGolfBall::~CGolfBall()
{
}

void CGolfBall::InitVariable()
{
	CSecretRigidbody::InitVariable() ;

	m_nGolfball_Attr = 0 ;
	m_nMaxNumGAStationPos = 0 ;
	m_nLatestGAStationNode = 0 ;
	m_pcFlyingTrail = NULL ;
	m_pcRollingTrail = NULL ;
}
Vector3 CGolfBall::_CalculateDragForce()
{
	float vv = m_sCurState.vLinearVel.dot(m_sCurState.vLinearVel) ;
	//Vector3 vDrag = -m_sCurState.vLinearVel*(0.5f*m_fAirDensity*m_fAreaOfIntersect*m_fCp*vv) ;

	//Vector3 vDrag = -m_sCurState.vLinearVel.Normalize()*(0.000125f*vv) ;
	//Vector3 vDrag = -m_sCurState.vLinearVel*(0.0125f*vv) ;//중력95일때 적당

	//Vector3 vDrag = -m_sCurState.vLinearVel.Normalize()*(0.00125f*vv) ;

	//float D = 0.000625f ;//0.00125f/2.0f ;
	float D = g_fAirResistance ;

	Vector3 vDrag = -m_sCurState.vLinearVel.Normalize()*(D*vv) ;

	return vDrag ;
}
float CGolfBall::_CalculateLiftForce()
{
	float fLift=0.0f ;

	float div ;

	if(float_less_eq(m_fAngularVel, 0.0f))
		return fLift ;

	if(float_eq(m_sCurState.vLinearVel.Magnitude(), 0.0f))
		return fLift ;

	div = m_fRadius*m_fAngularVel/m_sCurState.vLinearVel.Magnitude() ;
	fLift = div*1.0f ;

	m_fAngularVel *= 0.99f ;//시간이 지날수록 감소되는 각속도
	//m_fAngularVel *= g_fAirResistance ;//시간이 지날수록 감소되는 각속도

	//fLift = -0.05f+sqrtf(0.0025f+0.36f*div) ;

	return fLift ;
}
Vector3 CGolfBall::_CalculateMagnusFroce()
{
	Vector3 vMagnus ;

	//angularmomentum = torque * dt
	//angularvelocity = angularmomentum * inertia_tensor
	//orientation.normalize()
	//spin = 0.5 * quaternion(0, angularvelocity.x, angularvelocity.y, angularvelocity.z)*orientation
	//state.orientation += 1.0f/6.0f * dt * (a.spin + 2.0f*(b.spin + c.spin) + d.spin)

	float fLift = _CalculateLiftForce() ;

	Vector3 vAxis = m_sCurState.vLinearVel.cross(m_fAngularAxis) ;
	//vMagnus = vAxis*(0.5f*m_fAirDensity*m_fAreaOfIntersect*fLift*m_sCurState.vLinearVel.dot(m_sCurState.vLinearVel)) ;

	vMagnus = vAxis*(fLift*m_sCurState.vLinearVel.dot(m_sCurState.vLinearVel)) ;

	return vMagnus ;
}

void CGolfBall::_CalculateForce(Vector3 *pvForce)
{
	Vector3 vDrag = _CalculateDragForce() ;
	Vector3 vMagnus = _CalculateMagnusFroce() ;

	//TRACE("dragforce=(%g %g %g) length=%g\r\n", enumVector(vDrag), vDrag.Magnitude()) ;
	//TRACE("magnusforce=(%g %g %g) length=%g\r\n", enumVector(vMagnus), vMagnus.Magnitude()) ;

	if(m_lMoveKind == FLYING)
	{
		(*pvForce) = m_sImplusForce.vForce + m_vGravity + vDrag + vMagnus ;// m_sConstantForce.vForce ;
	}
	else if(m_lMoveKind == ROLLING)
	{
		//지면의 경사도와 중력의 합해서 계산
		Vector3 vConstant, vNormalForce ;
		if(m_bContactedPlane)
		{
			vNormalForce = ProjectionVector(m_vGravity, -m_sContactedPlane.GetNormal(), false) ;
			vConstant = m_vGravity-vNormalForce ;
		}
		else
			vConstant = m_vGravity ;

		(*pvForce) = m_sImplusForce.vForce + vConstant ;
	}
}

CGolfBall::SDerivative CGolfBall::_Evaluate(SRigidbodyState *pstate)
{
	SDerivative sDerivative ;

	sDerivative.vLinearVel = pstate->vLinearVel ;
	_CalculateForce(&sDerivative.vForce) ;

	return sDerivative ;
}
CGolfBall::SDerivative CGolfBall::_Evaluate(SRigidbodyState state, float dt, const CGolfBall::SDerivative &sInput)
{
	//P(momentum)=Mess*Velocity
	//state.vPos += sInput.vLinearVel*dt ;
	state.vLinearMomentum += (sInput.vForce*dt) ;
	state.Recalculate() ;//linear velocity를 구하는 것임. vLinearVel = vLinearMomentum*fInvMass ;

	SDerivative sOutput ;
	sOutput.vLinearVel = state.vLinearVel ;
	_CalculateForce(&sOutput.vForce) ;//  (*pvForce) = m_sImplusForce.vForce + m_vGravity + vDrag + vMagnus;
	return sOutput ;
}

void CGolfBall::Integrate(float dt)
{
	m_sPrevState = m_sCurState ;

	if(m_nGolfball_Attr & FORCEDGASTATION)//아치바데이타를 따르고 있는경우
	{
		m_fElapsedTime += dt ;
		int nGAStationStatus = _GetPosfromGAStationData(m_fElapsedTime, dt) ;

		if(nGAStationStatus == GASTATUS_INRANGE)
		{
			Vector3 vDir = m_sCurState.vPos - m_sPrevState.vPos ;
			m_sCurState.vLinearVel = vDir.Normalize() * (vDir.Magnitude()/dt) ;
			m_sCurState.vLinearMomentum = m_sCurState.vLinearVel*m_sCurState.fMass ;
			return ;
		}
		else if(nGAStationStatus == GASTATUS_EXCEEDRANGE)
		{
			m_nGolfball_Attr &= (~FORCEDGASTATION) ;
			return ;
		}
	}

	SDerivative a, b, c, d ;

	//runge-kutta integrate
	a = _Evaluate(&m_sCurState) ;
	b = _Evaluate(m_sCurState, dt*0.5f, a) ;
	c = _Evaluate(m_sCurState, dt*0.5f, b) ;
	d = _Evaluate(m_sCurState, dt, c) ;

	//TRACE("a vel(%g %g %g), force(%g %g %g) \r\n", enumVector(a.vLinearVel), enumVector(a.vForce)) ;
	//TRACE("b vel(%g %g %g), force(%g %g %g) \r\n", enumVector(b.vLinearVel), enumVector(b.vForce)) ;
	//TRACE("c vel(%g %g %g), force(%g %g %g) \r\n", enumVector(c.vLinearVel), enumVector(c.vForce)) ;
	//TRACE("d vel(%g %g %g), force(%g %g %g) \r\n", enumVector(d.vLinearVel), enumVector(d.vForce)) ;

	static const float coef=1.0f/6.0f ;
	Vector3 dpdt, dmdt ;
	dpdt = (a.vLinearVel+(b.vLinearVel+c.vLinearVel)*2.0f+d.vLinearVel)*coef*dt ;
	dmdt = (a.vForce+(b.vForce+c.vForce)*2.0f+d.vForce)*coef*dt ;

	//TRACE("dpdt(%g %g %g) dmdt(%g %g %g)\r\n", enumVector(dpdt), enumVector(dmdt)) ;

	m_sCurState.vPos += dpdt ;
	m_sCurState.vLinearMomentum += dmdt ;

	//TRACE("pos(%g %g %g) momemtum(%g %g %g)\r\n", enumVector(m_sCurState.vPos), enumVector(m_sCurState.vLinearMomentum)) ;

	if(m_lMoveKind == ROLLING)
	{
		_CalculateNormalForce(m_sContactedPlane.GetNormal(), dt) ;
		_correctPosinRolling() ;
	}

	m_fElapsedTime += dt ;

	m_sCurState.Recalculate() ;

	m_sImplusForce.ClearForce() ;
}
void CGolfBall::SetRadius(float radius)
{
	m_fRadius = radius ;
	m_fAreaOfIntersect = pi*m_fRadius*m_fRadius ;// pi*r*r 골프공의 반지름에 대한 공기와 교차영역
}
float CGolfBall::GetRadius()
{
	return m_fRadius ;
}
void CGolfBall::SetAngularVelbyClub(Vector3 &vAxis, float vel)
{
	m_fAngularVel = vel ;
	m_fAngularAxis = vAxis ;
}
void CGolfBall::_CalculateNormalForce(Vector3 &vNormal, float dt)
{
	float length = m_vGravity.Normalize().dot(-vNormal) * g_fNormalForce ;
	m_vNormalForce = ProjectionVector(m_vGravity, -vNormal, false) ;
	m_vNormalForce = m_vNormalForce.Normalize() * length * dt ;

	//Vector3 vnGravity = m_vGravity.Normalize() ;
	//Vector3 a = ProjectionVector(vnGravity, -vNormal, false) ;
	//m_vNormalForce = vnGravity - a ;
	//m_vNormalForce *= dt ;
}
void CGolfBall::_correctPosinRolling()
{
	float a = m_sContactedPlane.ToPosition(m_sCurState.vPos) - GOLFBALL_RADIUS ;
	//if(a < 0.0f)
	//{
	//	TRACE("wrong dist=%g\r\n", a) ;
	//	Vector3 vNewPos = m_sContactedPlane.GetOnPos(m_sCurState.vPos) + (m_sContactedPlane.GetNormal()*GOLFBALL_RADIUS) ;
	//	m_sCurState.vPos = vNewPos ;
	//	a = m_sContactedPlane.ToPosition(m_sCurState.vPos) ;
	//	TRACE("correction pos(%g %g %g) dist=%g\r\n", enumVector(m_sCurState.vPos), a) ;

	//}
}
void CGolfBall::CalculateGAStaionRot(Vector3 &vDir)
{
	Quaternion q(Vector3(0, 0, 1), vDir) ;
	q.GetMatrix(m_matGAStationRot) ;
}
void CGolfBall::SetGAStationData(int *pnPosData, int nNumPos)
{
	//memcpy(m_asGAStationPosData, pnPosData, sizeof(SGAStationPosData)*nNumPos) ;
	_interpolateGAStationData((SGAStationPosData *)pnPosData, nNumPos, m_asGAStationPosDataf) ;
	m_nMaxNumGAStationPos = nNumPos ;
	m_nLatestGAStationNode = 0 ;
}
void CGolfBall::_interpolateGAStationData(SGAStationPosData *psRecvData, int nNum, SGAStationPosDataf *psResultDataf)
{
	int i ;

	int nSameXCount ;
	int nSameXStart=0 ;
	int nSameXEnd=0 ;
	int nXValue ;

	int nSameYCount ;
	int nSameYStart=0 ;
	int nSameYEnd=0 ;
	int nYValue ;

	int nSameZCount ;
	int nSameZStart=0 ;
	int nSameZEnd=0 ;
	int nZValue ;

	nXValue = psRecvData[0].x ;
	nSameXCount = 1 ;

	nYValue = psRecvData[0].y ;
	nSameYCount = 1 ;

	nZValue = psRecvData[0].z ;
	nSameZCount = 1 ;

	for(i=1 ; i<nNum ; i++)
	{
		if(psRecvData[i].x == nXValue)
		{
			nSameXEnd = i ;
			psResultDataf[i].x = (float)psRecvData[i].x ;
			nSameXCount++ ;
		}
		else
		{
			float diff = (float)(psRecvData[i].x-nXValue) ;
			float length = (float)nSameXCount ;
			float dx = diff/length ;

			for(int n=nSameXStart ; n<=nSameXEnd ; n++)
			{
				psResultDataf[n].x = (float)nXValue + dx*(float)(n-nSameXStart) ;
			}

			nSameXCount = 1 ;
			nXValue = psRecvData[i].x ;
			psResultDataf[i].x = (float)nXValue ;
			nSameXEnd = nSameXStart = i ;
		}

		if(psRecvData[i].y == nYValue)
		{
			nSameYEnd = i ;
			psResultDataf[i].y = (float)psRecvData[i].y ;
			nSameYCount++ ;
		}
		else
		{
			float diff = (float)(psRecvData[i].y-nYValue) ;
			float length = (float)nSameYCount ;
			float dy = diff/length ;

			for(int n=nSameYStart ; n<=nSameYEnd ; n++)
			{
				psResultDataf[n].y = (float)nYValue + dy*(float)(n-nSameYStart) ;
			}

			nSameYCount = 1 ;
			nYValue = psRecvData[i].y ;
			psResultDataf[i].y = (float)nYValue ;
			nSameYEnd = nSameYStart = i ;
		}

		if(psRecvData[i].z == nZValue)
		{
			nSameZEnd = i ;
			psResultDataf[i].z = (float)psRecvData[i].z ;
			nSameZCount++ ;
		}
		else
		{
			float diff = (float)(psRecvData[i].z-nZValue) ;
			float length = (float)nSameZCount ;
			float dz = diff/length ;

			for(int n=nSameZStart ; n<=nSameZEnd ; n++)
			{
				psResultDataf[n].z = (float)nZValue + dz*(float)(n-nSameZStart) ;
			}

			nSameZCount = 1 ;
			nZValue = psRecvData[i].z ;
			psResultDataf[i].z = (float)nZValue ;
			nSameZEnd = nSameZStart = i ;
		}

		psResultDataf[i].t = (float)psRecvData[i].t ;
	}
	//for(i=0 ; i<nNum ; i++)
	//	TRACE("ResultDataf[%03d] (%g %g %g)\r\n", i, psResultDataf[i].x, psResultDataf[i].y, psResultDataf[i].z) ;
}

Vector3 CGolfBall::GetGAStationData(int n)
{
	//float fChangeUnit=0.01f ;
	float fChangeUnit=0.0254f ;
	if(m_nGAUnitType == GA_UNITTYPE_INCH)
		fChangeUnit=0.0254F ;
	else if(m_nGAUnitType == GA_UNITTYPE_CENTIMETER)
		fChangeUnit=0.01f ;

	Vector3 p ;
	p.x = m_asGAStationPosData[n].x*fChangeUnit ;
	p.y = m_asGAStationPosData[n].y*fChangeUnit ;
	p.z = m_asGAStationPosData[n].z*fChangeUnit ;
	p *= m_matGAStationRot ;
	p = m_vStartPos + p ;
	return p ;
}
int CGolfBall::_GetPosfromGAStationData(float fElapsedTime, float fDruationTime)
{
	if(m_nLatestGAStationNode >= m_nMaxNumGAStationPos)
		return GASTATUS_EXCEEDRANGE ;

	float fChangeUnit=0.0254f, fChangeTime=0.001f ;

	if(m_nGAUnitType == GA_UNITTYPE_INCH)
		fChangeUnit=0.0254F ;
	else if(m_nGAUnitType == GA_UNITTYPE_CENTIMETER)
		fChangeUnit=0.01f ;

	Vector3 vPos, vDir ;
	float w, cur_t, prev_t ;
	SGAStationPosDataf *prev, *cur ;

	for(int i=m_nLatestGAStationNode ; i<m_nMaxNumGAStationPos ; i++)
	//for(int i=0 ; i<m_nMaxNumGAStationPos ; i++)
	{
		cur = &m_asGAStationPosDataf[i] ;
		cur_t = (float)cur->t*fChangeTime ;
		if(cur_t >= fElapsedTime)
		{
			prev = &m_asGAStationPosDataf[i-1] ;
			prev_t = (float)prev->t*fChangeTime ;

			w = WeightInterpolation(prev_t, cur_t, fElapsedTime) ;
			//TRACE("weight=%g elapsedTime=%g in _GetPosfromGAStationData\r\n", w, fElapsedTime) ;

			//inch to meter
			Vector3 vCurPos(cur->x*fChangeUnit, cur->y*fChangeUnit, cur->z*fChangeUnit) ;
			Vector3 vPrevPos(prev->x*fChangeUnit, prev->y*fChangeUnit, prev->z*fChangeUnit) ;

			vCurPos *= m_matGAStationRot ;
			vPrevPos *= m_matGAStationRot ;
			vDir = vCurPos-vPrevPos ;

			m_sCurState.vPos = m_vStartPos + vPrevPos + (vDir.Normalize()*w*vDir.Magnitude()) ;

			//Vector3 a = m_vStartPos+vPrevPos ;
			//Vector3 b = m_vStartPos+vCurPos ;
			//TRACE("[%d - %d] weight=%g prevPos(%g %g %g) curPos(%g %g %g) golfball pos (%g %g %g)\r\n",
			//	i-1, i, w, enumVector(a), enumVector(b), enumVector(m_sCurState.vPos)) ;

			m_nLatestGAStationNode = i ;
			return GASTATUS_INRANGE ;
		}
	}

	m_nLatestGAStationNode = m_nMaxNumGAStationPos ;

	//맨마지막데이타를 넘어갔을경우
	//prev = &m_asGAStationPosData[m_nLatestGAStationNode-2] ;
	//cur = &m_asGAStationPosData[m_nLatestGAStationNode-1] ;
	//vDir.set( (cur->x-prev->x)*fChangeUnit, (cur->y-prev->y)*fChangeUnit,(cur->z-prev->z)*fChangeUnit ) ;
	//vDir *= m_matGAStationRot ;

	//Vector3 vPrevPos(prev->x*fChangeUnit, prev->y*fChangeUnit, prev->z*fChangeUnit) ;
	//vPrevPos *= m_matGAStationRot ;

	//float vel = vDir.Magnitude()/(cur->t-prev->t) ;
	//float dist = vel*fDruationTime ;

	//m_sCurState.vPos = m_vStartPos + vPrevPos + (vDir.Normalize()*dist) ;
	return GASTATUS_EXCEEDRANGE ;
}

void CGolfBall::ImportSwingData(char *pszFileName)
{
	FILE *pfile = fopen(pszFileName, "r") ;
	char str[256] ;

	int c, t ;
	int x, y, z ;

	while(!feof(pfile))
	{
		fgets(str, 256, pfile) ;
		sscanf(str, "%d%d%d%d%d", &c, &x, &y, &z, &t) ;

		m_asGAStationPosData[c].x = x ;
		m_asGAStationPosData[c].y = y ;
		m_asGAStationPosData[c].z = z ;
		m_asGAStationPosData[c].t = t ;
	}
	m_nMaxNumGAStationPos = c+1 ;
	m_nLatestGAStationNode = 0 ;
	fclose(pfile) ;

	_interpolateGAStationData(m_asGAStationPosData, m_nMaxNumGAStationPos, m_asGAStationPosDataf) ;
}
void CGolfBall::SetFlyingTrail(CSecretTrail *pcTrail)
{
	m_pcFlyingTrail = pcTrail ;
	m_pcFlyingTrail->SetKind(CSecretTrail::FLYINGTRAIL) ;
}
void CGolfBall::SetRollingTrail(CSecretTrail *pcTrail)
{
	m_pcRollingTrail = pcTrail ;
	m_pcRollingTrail->SetKind(CSecretTrail::ROLLINGTRAIL) ;
}
void CGolfBall::ReleaseTrails()
{
	if(m_pcFlyingTrail)
	{
		m_pcFlyingTrail->Reset() ;
		m_pcFlyingTrail->SetStatus(CSecretTrail::IDLING) ;
		m_pcFlyingTrail = NULL ;
	}

	if(m_pcRollingTrail)
	{
		m_pcRollingTrail->Reset() ;
		m_pcRollingTrail->SetStatus(CSecretTrail::IDLING) ;
		m_pcRollingTrail = NULL ;
	}
}
void CGolfBall::AddGolfBallAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_nGolfball_Attr |= nAttr ;
	else
		m_nGolfball_Attr &= (~nAttr) ;
}
bool CGolfBall::IsGolfBallAttr(int nAttr)
{
	if(m_nGolfball_Attr & nAttr)
		return true ;
	return false ;
}
void CGolfBall::setSpinningAxies()
{
	D3DXMatrixDecomposeRotation(&m_matOriginRot, &m_psTRObject->matWorld) ;
	m_matRot.Identity() ;
	m_qRot.Identity() ;
}
void CGolfBall::spinning(Vector3 vMovement)
{
	Vector3 vUp(0, 1, 0), vAxis ;
	Vector3 vnMovement = vMovement.Normalize() ;
	float fMovement = vMovement.Magnitude()*0.7f ;
	float fArc = fMovement/m_fRadius ;
	if((fMovement <= FLT_EPSILON) || (fArc <= FLT_EPSILON))
	{
		//m_matRot.Identity() ;
		//TRACE("no rotation\r\n") ;
	}
	else
	{
		vAxis = (vUp.cross(vnMovement)).Normalize() ;
		Quaternion q(vAxis, fArc) ;
		m_qRot = m_qRot*q ;
		m_qRot.GetMatrix(m_matRot) ;
		//TRACE("axis(%g %g %g) arc=%g\r\n", enumVector(vAxis), fArc) ;
	}
}