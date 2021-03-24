#pragma once

#include "SecretRigidBody.h"
#include "GeoLib.h"
#include "SecretTerrain.h"

#define GOLFBALL_RADIUS 0.02f
#define GOLFBALL_MASS 0.054f // 0.0459f

#define MAXNUM_GASTAIONPOSDATA 1000

class CSecretTrail ;
class CSecretWater ;

extern float g_fAirResistance ;
extern float g_fBackspin ;
extern float g_fNormalForce ;
extern float g_fGolfBallMass ;

struct SReflectParameter
{
	float fVelRange, fVelBase ;
	float fMaxIncidenceBase ;
	float fScaleRestitution ;
} ;

class CGolfBall : public CSecretRigidbody
{
public :
	enum GOLFBALL_ATTR
	{
		FORCEDGASTATION=0x01,	//골프아치바센서 데이타를 따라간다.
		READYTOSHOT=0x02,		//공을 생성하고 아직 치기전
		TRAILING=0x04,
		SLEEPINWATERHAZARD=0x08
	} ;
	enum GASTATION_STATUS {GASTATUS_INRANGE=0, GASTATUS_EXCEEDRANGE} ;
	enum GASTATION_UNITTYPE { GA_UNITTYPE_INCH=0, GA_UNITTYPE_CENTIMETER } ;

private :
	int m_nGolfball_Attr ;
	float m_fAirDensity ;//공기밀도
	float m_fAreaOfIntersect ;//공의 반지름에 대한 공기의 교차영역
	float m_fCp ;//공의 양력계수

	float m_fAngularVel ;
	Vector3 m_fAngularAxis ;
	float m_fRadius ;

	struct SDerivative
	{
		Vector3 vLinearVel ;//단위시간에 대한 속도. 위치정할때 사용된다.
		Vector3 vForce ;//단위시간에 대한 힘. 모멘텀계산에 사용된다.
	} ;

	struct SGAStationPosData
	{//x, y, z : position by inch, t : 0.001sec 
		int x, y, z, t ;
	} ;
	struct SGAStationPosDataf
	{//x, y, z : position by inch, t : 0.001sec 
		float x, y, z, t ;
	} ;

	int _GetPosfromGAStationData(float fElapsedTime, float fDruationTime) ;

	Vector3 _CalculateDragForce() ;
	float _CalculateLiftForce() ;
	Vector3 _CalculateMagnusFroce() ;

	void _CalculateForce(Vector3 *pvForce) ;
	SDerivative _Evaluate(SRigidbodyState *pstate) ;
	SDerivative _Evaluate(SRigidbodyState state, float dt, const CGolfBall::SDerivative &sInput) ;

	void _CalculateNormalForce(Vector3 &vNormal, float dt) ;
	void _correctPosinRolling() ;
	void _interpolateGAStationData(SGAStationPosData *psRecvData, int nNum, SGAStationPosDataf *psResultDataf) ;

public :
	bool m_bContactedPlane, m_bContactedTriangle ;
	geo::SPlane m_sContactedPlane ;
	geo::STriangle m_sContactedTriangle ;
	SContactSurfaceInfo m_sContactSurfaceInfo ;
	STrueRenderingObject *m_psContactedObject ;
	Vector3 m_vGravity ;
	Vector3 m_vNormalForce ;

	Vector3 m_vStartPos, m_vEndPos ;
	float m_fSleepCount ;
	//CSecretTrail *m_pcTrail ;
	CSecretTrail *m_pcFlyingTrail, *m_pcRollingTrail ;
	bool m_bSleepInHolecup ;
	int m_nCollisionCount ;
	float m_fTimeAfterFirstHit ;
	float m_fFirstVelocity ;

	CSecretWater *m_pcIntersectedWater ;

	bool m_bBoundEvent ;
	float m_fBoundForce ;

	SGAStationPosData m_asGAStationPosData[MAXNUM_GASTAIONPOSDATA] ;
	SGAStationPosDataf m_asGAStationPosDataf[MAXNUM_GASTAIONPOSDATA] ;
	int m_nMaxNumGAStationPos ;
	int m_nLatestGAStationNode ;
	Matrix4 m_matGAStationRot ;

	Vector3 m_vOriginDir, m_vOriginUp, m_vOriginAxis ;
	D3DXMATRIX m_matOriginRot ;
	Matrix4 m_matRot ;
	Quaternion m_qRot ;

	Vector3 m_vPrevPosAtLastFrame ;//이전 프레임에서의 위치

	int m_nGAUnitType ;//inch or cm

public :
	CGolfBall() ;
	virtual ~CGolfBall() ;

	//Runge-Kutta method	
	virtual void Integrate(float dt) ;

	void SetRadius(float radius) ;
	float GetRadius() ;

	void SetAngularVelbyClub(Vector3 &vAxis, float vel) ;

	void CalculateGAStaionRot(Vector3 &vDir) ;
	void SetGAStationData(int *pnPosData, int nNumPos) ;
	Vector3 GetGAStationData(int n) ;
	void ImportSwingData(char *pszFileName) ;
	void SetFlyingTrail(CSecretTrail *pcTrail) ;
	void SetRollingTrail(CSecretTrail *pcTrail) ;
	void ReleaseTrails() ;
	void AddGolfBallAttr(int nAttr, bool bEnable=true) ;
	bool IsGolfBallAttr(int nAttr) ;
	void InitVariable() ;
	void setSpinningAxies() ;
	void spinning(Vector3 vMovement) ;
} ;