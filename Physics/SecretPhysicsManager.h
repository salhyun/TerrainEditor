#pragma once

#include "SecretRigidBody.h"
#include "GolfBall.h"
#include "Plant.h"
#include "SecretCollisionDetection.h"
#include <list>

class CSecretTerrain ;
struct STrueRenderingObject ;
class CSecretRoadManager ;
class CSecretWaterManager ;
class CSecretTrailManager ;

#define MAXNUM_PLANT 128
#define MAXNUM_GOLFBALL 4
#define MAXNUM_RIGIDBODY MAXNUM_PLANT+MAXNUM_GOLFBALL

class CSecretPhysicsManager
{
public :
	enum EVENT {EVENT_SPLASHWATER=0x01} ;

private :
	CSecretCollisionDetection m_cCollisionDetection ;

	DWORD m_dwEvent ;
	int m_nNumGolfBall ;
	CGolfBall *m_pcGolfBalls ;

	int m_nNumPlant ;
	CPlant *m_pcPlants ;

	std::list<CSecretRigidbody *> m_Rigidbodies ;
	
	CSecretTerrain *m_pcTerrain ;
	CSecretRoadManager *m_pcRoadManager ;
	CSecretWaterManager *m_pcWaterManager ;
	CSecretTrailManager *m_pcTrailManager ;

	void _FindAroundbody(CGolfBall *pcGolfBall) ;

	CGolfBall *_GetEmptyGolfBall() ;
	CPlant *_GetEmptyPlant() ;
	void _ClearPlants() ;

public :
	int m_nIntegrateCount ;

public :
	CSecretPhysicsManager() ;
	~CSecretPhysicsManager() ;

	void Initialize(CSecretTerrain *pcTerrain, CSecretRoadManager *pcRoadManager, CSecretWaterManager *pcWaterManager, CSecretTrailManager *pcTrailManager, int nUnitType) ;
	void Process(float fTime) ;

	CGolfBall *AddGolfBall(STrueRenderingObject *psTRObject, bool bGAStationData) ;
	void HitGolfBall(CGolfBall *pcGolfBall, Vector3 &vImpluse, Vector3 &vAngularAxis, float fAngularVel, float fAzimuthAngle, int nMoveKind) ;
	void DisableGolfBall(int nNum) ;
	void DisableGolfBall(CGolfBall *pcGolfBall) ;

	CPlant *AddPlant(STrueRenderingObject *psTRObject) ;

	void AddRigidbody(CSecretRigidbody *pcRigidbody) ;
	std::list<CSecretRigidbody *> *GetRigidbodies() ;
	void DeleteRigidbody(CSecretRigidbody *pcRigidbody) ;

	int GetNumGolfBall() ;
	CGolfBall *GetGolfBalls() ;

	int GetNumPlant() ;
	CPlant *GetPlants() ;
	CSecretCollisionDetection *GetCollisionDetection() ;

	bool IsEvent(DWORD dwEvent) ;

	void Release() ;
} ;