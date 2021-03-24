#include "SecretPhysicsManager.h"
#include "SecretTerrain.h"
#include "SecretSceneManager.h"
#include "SecretMeshObject.h"
#include "SecretTrail.h"

CSecretPhysicsManager::CSecretPhysicsManager()
{
	m_pcTerrain = NULL ;
	m_pcGolfBalls = NULL ;
	m_pcPlants = NULL ;
}
CSecretPhysicsManager::~CSecretPhysicsManager()
{
	Release() ;
}

void CSecretPhysicsManager::Release()
{
	//std::list<CSecretRigidbody *>::iterator it ;
	//for(it=m_Rigidbodies.begin() ; it!=m_Rigidbodies.end() ; it++)
	//{
	//	CSecretRigidbody *pcRigidbody = (*it) ;
	//	SAFE_DELETE(pcRigidbody) ;
	//}

	m_cCollisionDetection.Release() ;

	m_Rigidbodies.clear() ;

	SAFE_DELETEARRAY(m_pcGolfBalls) ;
	SAFE_DELETEARRAY(m_pcPlants) ;
}

void CSecretPhysicsManager::Initialize(CSecretTerrain *pcTerrain, CSecretRoadManager *pcRoadManager, CSecretWaterManager *pcWaterManager, CSecretTrailManager *pcTrailManager, int nUnitType)
{
	m_Rigidbodies.resize(0) ;

	m_pcTerrain = pcTerrain ;
	m_pcRoadManager = pcRoadManager ;
	m_pcWaterManager = pcWaterManager ;
	m_pcTrailManager = pcTrailManager ;

	m_cCollisionDetection.Initialize() ;
	m_cCollisionDetection.SetTerrain(pcTerrain) ;
	m_cCollisionDetection.SetRoadManager(pcRoadManager) ;
	m_cCollisionDetection.SetWaterManager(pcWaterManager) ;

	m_nNumGolfBall = 0 ;
	m_pcGolfBalls = new CGolfBall[MAXNUM_GOLFBALL] ;

	for(int i=0 ; i<MAXNUM_GOLFBALL ; i++)
		m_pcGolfBalls[i].m_nGAUnitType = nUnitType ;

	m_nNumPlant = 0 ;
	m_pcPlants = new CPlant[MAXNUM_PLANT] ;
}

void CSecretPhysicsManager::Process(float fTime)//여기에서는 m_pcGolfBalls, m_pcPlants를 직접액세스하지 않는다.
{
	int i ;
	std::list<CSecretRigidbody *>::iterator it ;

	m_dwEvent = 0 ;

	const float fFixedTime = 0.01f ;
	static float fAccumTime = 0.0f ;

	static int physics_count=0 ;

	m_nIntegrateCount = 0 ;

	for(i=0 ; i<MAXNUM_GOLFBALL ; i++)
	{
		if(m_pcGolfBalls[i].IsEnable())
			m_pcGolfBalls[i].m_bBoundEvent = false ;
	}
	
	fAccumTime += fTime ;
	//fAccumTime = fTime ;

	//while(fAccumTime >= fTime)// ㅋㅋ 이건 한프레임에 한번
	while(fAccumTime >= fFixedTime)
	{
		_ClearPlants() ;

		for(it=m_Rigidbodies.begin() ; it!=m_Rigidbodies.end() ; it++)
		{
			if((*it)->GetMoveStatus() == CSecretRigidbody::ACTIVITY)
				(*it)->Integrate(fFixedTime) ;

			else if((*it)->GetMoveStatus() == CSecretRigidbody::SLEEP)//임시로 해놓은 것임
				(*it)->m_fElapsedTime += fFixedTime ;
		}

		for(i=0 ; i<MAXNUM_GOLFBALL ; i++)
		{
			if(m_pcGolfBalls[i].IsEnable())
			{
				//TRACE("[%d]GolfBall findaroundbody       ", i) ;
				_FindAroundbody(&m_pcGolfBalls[i]) ;

				if(m_pcGolfBalls[i].GetMoveStatus() == CSecretRigidbody::SLEEP)
					m_pcGolfBalls[i].m_fSleepCount += fFixedTime ;
			}
		}

		for(it=m_Rigidbodies.begin() ; it!=m_Rigidbodies.end() ; it++)
		{
			if((*it)->IsEnable() && (*it)->GetMoveStatus() == CSecretRigidbody::ACTIVITY)
				m_cCollisionDetection.AddRigidbody((*it)) ;
		}

		m_cCollisionDetection.ResetCollisionEvent() ;
		m_cCollisionDetection.Process() ;
		if(m_cCollisionDetection.IsCollisionEvent(CSecretCollisionDetection::COLLISIONEVENT_INTERSECTWATER))
		{
			m_dwEvent |= EVENT_SPLASHWATER ;
		}

		m_cCollisionDetection.ClearRigidbody() ;
		m_cCollisionDetection.ClearPlane() ;

		fAccumTime -= fFixedTime ;

		m_nIntegrateCount++ ;

		//physics_count++ ;
		//TRACE("physics_count=%d\r\n", physics_count) ;
	}

	//translate
	STrueRenderingObject *psTRObject ;
	for(it=m_Rigidbodies.begin() ; it!=m_Rigidbodies.end() ; it++)
	{
		if((*it)->GetMoveStatus() == CSecretRigidbody::ACTIVITY)
		{
			psTRObject = (*it)->GetTRObject() ;
			psTRObject->ClearLinktoQuadTree() ;
			(*it)->UpdateTransformation() ;
			m_pcTerrain->m_pcQuadTree->PlaceObjects(1, psTRObject) ;
		}
	}

	//operation trail golfball
	for(i=0 ; i<MAXNUM_GOLFBALL ; i++)
	{
		if(m_pcGolfBalls[i].IsEnable())
		{
			if(m_pcGolfBalls[i].IsGolfBallAttr(CGolfBall::TRAILING))
			{
				CSecretTrail *pcFlyingTrail = m_pcGolfBalls[i].m_pcFlyingTrail ;
				CSecretTrail *pcRollingTrail = m_pcGolfBalls[i].m_pcRollingTrail ;

				if(pcFlyingTrail)
				{
					if(pcFlyingTrail->IsAttr(CSecretTrail::ATTR_FADEOUT) && (pcFlyingTrail->m_fAlphaValue < 0.5f) && !pcRollingTrail)
					{
						m_pcGolfBalls[i].SetRollingTrail(m_pcTrailManager->GetEmptyTrail()) ;
						pcRollingTrail = m_pcGolfBalls[i].m_pcRollingTrail ;
						if(pcRollingTrail)
						{
							pcRollingTrail->SetKind(CSecretTrail::ROLLINGTRAIL) ;
							pcRollingTrail->m_dwZEnable = D3DZB_FALSE ;
						}
					}

					if((m_pcGolfBalls[i].m_nCollisionCount > 0) && (pcFlyingTrail->IsStatus(CSecretTrail::TRAILING)))
					{
						m_pcGolfBalls[i].m_fTimeAfterFirstHit += (m_nIntegrateCount*fFixedTime) ;
						if(m_pcGolfBalls[i].m_fTimeAfterFirstHit > 1.5f)
							pcFlyingTrail->AddAttr(CSecretTrail::ATTR_FADEOUT) ;
					}
				}

				psTRObject = m_pcGolfBalls[i].GetTRObject() ;

				if(pcRollingTrail)
				{
					if((pcRollingTrail->IsStatus(CSecretTrail::TRAILING)) && m_pcGolfBalls[i].GetMoveKind() == CGolfBall::ROLLING)
					{
						Vector3 vNormal = m_pcGolfBalls[i].m_sContactedPlane.GetNormal() ;
						Vector3 vPos(psTRObject->matWorld._41, psTRObject->matWorld._42, psTRObject->matWorld._43) ;
						vPos -= vNormal*(m_pcGolfBalls[i].GetRadius()*0.99f) ;
						pcRollingTrail->AddPos(vPos, &vNormal) ;
					}
				}
				if(pcFlyingTrail)
				{
					if(pcFlyingTrail->IsStatus(CSecretTrail::TRAILING))
						pcFlyingTrail->AddPos(Vector3(psTRObject->matWorld._41, psTRObject->matWorld._42, psTRObject->matWorld._43)) ;
				}
			}
				//m_pcGolfBalls[i].m_pcTrail->AddPos(m_pcGolfBalls[i].GetCurState()->vPos) ;

			//if( (m_pcGolfBalls[i].GetMoveStatus() == CSecretRigidbody::ACTIVITY)
			//	&& (m_pcGolfBalls[i].GetMoveKind() == CSecretRigidbody::FLYING)
			//	&& (m_pcGolfBalls[i].m_pcTrail) )
			//	m_pcGolfBalls[i].m_pcTrail->AddPos(m_pcGolfBalls[i].GetCurState()->vPos) ;
		}
	}
}
CSecretCollisionDetection *CSecretPhysicsManager::GetCollisionDetection()
{
	return &m_cCollisionDetection ;
}
CGolfBall *CSecretPhysicsManager::_GetEmptyGolfBall()
{
	if(m_nNumGolfBall >= MAXNUM_GOLFBALL)
		return NULL ;

	int i = m_nNumGolfBall, count=0 ;
	while(count++ <= MAXNUM_GOLFBALL)
	{
		if(m_pcGolfBalls[i].IsEnable() == false)
		{
			m_nNumGolfBall++ ;

			m_pcGolfBalls[i].InitVariable() ;
			return &m_pcGolfBalls[i] ;
		}

		if(++i >= MAXNUM_GOLFBALL)
			i = 0 ;
	}
	return NULL ;
}
CPlant *CSecretPhysicsManager::_GetEmptyPlant()
{
	if(m_nNumPlant >= MAXNUM_PLANT)
		return NULL ;

	int i = m_nNumPlant, count=0 ;
	while(count++ <= MAXNUM_PLANT)
	{
		if(m_pcPlants[i].IsEnable() == false)
		{
			m_nNumPlant++ ;
			return &m_pcPlants[i] ;
		}

		if(++i >= MAXNUM_PLANT)
			i = 0 ;
	}
	return NULL ;
}

CGolfBall *CSecretPhysicsManager::AddGolfBall(STrueRenderingObject *psTRObject, bool bGAStationData)
{
	CGolfBall *pcGolfBall = _GetEmptyGolfBall() ;
	CSecretRigidbody *pcRigidbody = (CSecretRigidbody *)pcGolfBall ;
	if(pcRigidbody)
	{
		//pcRigidbody->SetMass(GOLFBALL_MASS) ;//골프공질량
		pcRigidbody->SetMass(g_fGolfBallMass) ;//골프공질량

		pcRigidbody->SetPos(Vector3(psTRObject->matWorld._41, psTRObject->matWorld._42, psTRObject->matWorld._43)) ;
		pcRigidbody->SetLinearVel(Vector3(0.0f, 0.0f, 0.0f)) ;
		pcRigidbody->SetLinearMomentum(Vector3(0.0f, 0.0f, 0.0f)) ;

		pcRigidbody->m_sConstantForce.SetForce(Vector3(0.0f, -15.0f, 0.0f)) ;
		pcGolfBall->SetRadius(GOLFBALL_RADIUS) ;
		pcRigidbody->SetTRObject(psTRObject) ;
		pcRigidbody->AddAttr(CSecretRigidbody::ATTR_MOVABLE) ;
		pcRigidbody->SetBodykind(CSecretRigidbody::GOLFBALL) ;
		pcRigidbody->SetMoveStatus(CSecretRigidbody::SLEEP) ;
		pcRigidbody->m_fElapsedTime = 0.0f ;
		pcGolfBall->m_fSleepCount = 0.0f ;

		pcRigidbody->SetEnable() ;
		if(bGAStationData)
			pcGolfBall->AddGolfBallAttr(CGolfBall::FORCEDGASTATION) ;

		pcGolfBall->AddGolfBallAttr(CGolfBall::READYTOSHOT) ;
		pcGolfBall->m_bSleepInHolecup = false ;

		AddRigidbody(pcRigidbody) ;

		//((CGolfBall *)pcRigidbody)->ImportSwingData("swingdata.txt") ;
		//pcGolfBall->AddGolfBallAttr(CGolfBall::FORCEDGASTATION) ;
	}
	return pcGolfBall ;
}
void CSecretPhysicsManager::HitGolfBall(CGolfBall *pcGolfBall, Vector3 &vImpluse, Vector3 &vAngularAxis, float fAngularVel, float fAzimuthAngle, int nMoveKind)
{
	TRACE("IMPLUS FORCE=(%g %g %g) length=%g ANGULAR VELOCITY=%g VERT_ANGLE=%G\r\n", enumVector(vImpluse), vImpluse.Magnitude(), fAngularVel, fAngularVel/8.0f) ;

	if(nMoveKind == CGolfBall::ROLLING)//처음부터 롤링인경우
	{
		int c, nNumIntersectedTriangle=0 ;
		float radius = pcGolfBall->GetRadius() ;
		Vector3 vBallPos = pcGolfBall->GetPos(), vIntersect ;
		geo::STriangle triangles[128], sIntersectedTriangles[128] ;
		m_pcTerrain->m_sHeightMap.GetTriangles(vBallPos, radius, triangles, true, true, 100) ;

		for(c=0 ; c<128 ; c++)
		{
			//먼저 삼각형의 바운딩박스와 골프공충돌테스트
			Vector3 vDist = vBallPos - triangles[c].sBoundingSphere.vPos ;
			if(vDist.Magnitude() <= (triangles[c].sBoundingSphere.fRadius+radius))
			{
				sIntersectedTriangles[nNumIntersectedTriangle++] = triangles[c] ;
				assert(nNumIntersectedTriangle < 128) ;
			}
		}

		geo::SLine sVertline ;
		for(c=0 ; c<nNumIntersectedTriangle ; c++)
		{
			sVertline.set(vBallPos, -sIntersectedTriangles[c].sPlane.GetNormal(), radius*2.0f) ;
			if(IntersectLinetoTriangle(sVertline, sIntersectedTriangles[c], vIntersect, true, true, FLT_EPSILON) == geo::INTERSECT_POINT)
			{
				pcGolfBall->m_bContactedPlane = true ;
				pcGolfBall->m_sContactedPlane = sIntersectedTriangles[c].sPlane ;
				m_pcTerrain->m_sHeightMap.GetSurfaceInfo(vBallPos.x, vBallPos.z, &pcGolfBall->m_sContactSurfaceInfo) ;
				break ;
			}
		}

		Vector3 vnPlane = pcGolfBall->m_sContactedPlane.GetNormal() ;
		Vector3 vNewImpluse = GetNewVectorfromTwoVector(vnPlane, vImpluse.Normalize(), deg_rad(90)) ;

		//퍼팅 Azimuth 각도에 따른 방향
		Matrix4 matRot ;
		Quaternion q(vnPlane, fAzimuthAngle) ;
		q.GetMatrix(matRot) ;
		vNewImpluse *= matRot ;

		vNewImpluse *= vImpluse.Magnitude() ;
		vImpluse = vNewImpluse ;
		pcGolfBall->m_sImplusForce.SetForce(vImpluse) ;
		pcGolfBall->AddGolfBallAttr(CGolfBall::FORCEDGASTATION, false) ;
	}

	if(pcGolfBall->IsGolfBallAttr(CGolfBall::FORCEDGASTATION))
	{
		pcGolfBall->m_sImplusForce.SetForce(Vector3(0, 0, 0)) ;

		//Impluse 노말라이징이 현재 카메라가 바라보는 방향
		Vector3 vLookat(vImpluse.x, 0, vImpluse.z) ;
		vLookat = vLookat.Normalize() ;
		pcGolfBall->CalculateGAStaionRot(vLookat) ;
	}
	else
		pcGolfBall->m_sImplusForce.SetForce(vImpluse) ;

	pcGolfBall->m_vStartPos = pcGolfBall->GetCurState()->vPos ;
	pcGolfBall->SetAngularVelbyClub(vAngularAxis, fAngularVel) ;

	pcGolfBall->m_fElapsedTime = 0.0f ;
	pcGolfBall->m_fSleepCount = 0.0f ;

	pcGolfBall->SetMoveStatus(CSecretRigidbody::ACTIVITY) ;
	pcGolfBall->SetMoveKind(nMoveKind) ;
	pcGolfBall->AddGolfBallAttr(CGolfBall::READYTOSHOT, false) ;

	if(pcGolfBall->GetMoveKind() == CGolfBall::FLYING)
	{
		pcGolfBall->m_nCollisionCount = 0 ;
		pcGolfBall->AddGolfBallAttr(CGolfBall::TRAILING) ;
	}
	else if(pcGolfBall->GetMoveKind() == CGolfBall::ROLLING)
	{
		pcGolfBall->m_nCollisionCount = 1 ;
		pcGolfBall->AddGolfBallAttr(CGolfBall::TRAILING) ;
	}
}

void CSecretPhysicsManager::DisableGolfBall(int nNum)
{
	m_pcGolfBalls[nNum].ReleaseTrails() ;
	m_pcGolfBalls[nNum].SetEnable(false) ;
	m_nNumGolfBall-- ;
	m_Rigidbodies.remove(&m_pcGolfBalls[nNum]) ;
}
void CSecretPhysicsManager::DisableGolfBall(CGolfBall *pcGolfBall)
{
	pcGolfBall->ReleaseTrails() ;
	pcGolfBall->SetEnable(false) ;
	m_nNumGolfBall-- ;
	m_Rigidbodies.remove(pcGolfBall) ;
}

CPlant *CSecretPhysicsManager::AddPlant(STrueRenderingObject *psTRObject)
{
	CPlant *pcPlant = _GetEmptyPlant() ;
	CSecretRigidbody *pcRigidbody = (CSecretRigidbody *)pcPlant ;
	if(pcRigidbody)
	{
		pcRigidbody->SetMass(10.0f) ;
		pcRigidbody->SetPos(Vector3(psTRObject->matWorld._41, psTRObject->matWorld._42, psTRObject->matWorld._43)) ;
		pcRigidbody->SetLinearVel(Vector3(0.0f, 0.0f, 0.0f)) ;
		pcRigidbody->SetLinearMomentum(Vector3(0.0f, 0.0f, 0.0f)) ;

		pcRigidbody->m_sConstantForce.SetForce(Vector3(0.0f, -15.0f, 0.0f)) ;
		((CGolfBall *)pcRigidbody)->SetRadius(0.02f) ;
		pcRigidbody->SetTRObject(psTRObject) ;
		pcRigidbody->AddAttr(CSecretRigidbody::ATTR_IMMOVABLE) ;
		pcRigidbody->SetBodykind(CSecretRigidbody::PLANT) ;
		pcRigidbody->m_fElapsedTime = 0.0f ;
		pcRigidbody->SetEnable() ;
	}
	return pcPlant ;
}
void CSecretPhysicsManager::AddRigidbody(CSecretRigidbody *pcRigidbody)
{
	m_Rigidbodies.push_back(pcRigidbody) ;
}
std::list<CSecretRigidbody *> *CSecretPhysicsManager::GetRigidbodies()
{
	return &m_Rigidbodies ;
}

int GetIntersectBoundingShpere(CGolfBall *pcGolfBall, STrueRenderingObject **ppsCollidedTRObject, int &nCollided, std::list<STrueRenderingObject *> *psTRObjects)
{
	bool already_exist ;
	int count=0 ;
	std::list<STrueRenderingObject *>::iterator it ;
	STrueRenderingObject *obj ;

	for(it=psTRObjects->begin(); it!=psTRObjects->end(); it++)
	{
		if( (pcGolfBall->GetTRObject()->nEigenIndex == (*it)->nEigenIndex) //골프공 자신일 경우
			|| (!strcmp((*it)->apcObject[0]->GetObjName(), "cylinder")) //실린더는 빼고
			|| (!strcmp((*it)->apcObject[0]->GetObjName(), "p1_kit_a1_240c_001")) //깃발은 빼고
			|| (!(*it)->IsAcquiredFlag(STrueRenderingObject::ACQUIREDFLAG_COLLISIONVOLUME)) //충돌속성이 없는경우
			|| ((*it)->IsFlag(STrueRenderingObject::FLAG_HIDE)) )
			continue ;

		obj = (*it) ;
		float length = (pcGolfBall->GetPos() - obj->cBBox.m_vCenter).Magnitude() ;

		if((pcGolfBall->GetPos() - (*it)->cBBox.m_vCenter).Magnitude() <= (pcGolfBall->GetRadius() + (*it)->cBBox.m_fRadius))//test bounding sphere
		{
			already_exist = false ;
			for(int c=0 ; c<nCollided ; c++)
			{
				if(ppsCollidedTRObject[c]->nEigenIndex == (*it)->nEigenIndex)
					already_exist = true ;
			}
			if(!already_exist)
			{
				ppsCollidedTRObject[nCollided++] = (*it) ;
				count++ ;
			}
		}
	}
	return count ;
}

#include "SecretMeshObject.h"

void CSecretPhysicsManager::_FindAroundbody(CGolfBall *pcGolfBall)
{
	std::list<STrueRenderingObject *> *psTRObjects ;
	std::list<STrueRenderingObject *>::iterator it ;

	int i, nCollidedCount=0 ;
	CSecretQuadTree *pcQuadTree, *pcNeighbor ;
	STrueRenderingObject *apsCollidedTRObject[256], *psTRObject = pcGolfBall->GetTRObject() ;
	for(i=0 ; i<psTRObject->nNumQuadTree ; i++)
	{
		CSecretQuadTree *pcQuadTree = psTRObject->apcQuadTrees[i] ;

		pcNeighbor = pcQuadTree->GetNeighbor(CSecretQuadTree::NEIGHBOR_UP) ;//바로 위쪽
		if(pcNeighbor)
		{
			GetIntersectBoundingShpere(pcGolfBall, apsCollidedTRObject, nCollidedCount, pcNeighbor->GetTRObjects()) ;

			CSecretQuadTree *pcNeighborSide ;
			pcNeighborSide = pcNeighbor->GetNeighbor(CSecretQuadTree::NEIGHBOR_LEFT) ;//왼쪽상단 트리
			if(pcNeighborSide)
				GetIntersectBoundingShpere(pcGolfBall, apsCollidedTRObject, nCollidedCount, pcNeighborSide->GetTRObjects()) ;

			pcNeighborSide = pcNeighbor->GetNeighbor(CSecretQuadTree::NEIGHBOR_RIGHT) ;//오른쪽상단 트리
			if(pcNeighborSide)
				GetIntersectBoundingShpere(pcGolfBall, apsCollidedTRObject, nCollidedCount, pcNeighborSide->GetTRObjects()) ;
		}
		pcNeighbor = pcQuadTree->GetNeighbor(CSecretQuadTree::NEIGHBOR_DOWN) ;//바로 아래쪽
		if(pcNeighbor)
		{
			GetIntersectBoundingShpere(pcGolfBall, apsCollidedTRObject, nCollidedCount, pcNeighbor->GetTRObjects()) ;

			CSecretQuadTree *pcNeighborSide ;
			pcNeighborSide = pcNeighbor->GetNeighbor(CSecretQuadTree::NEIGHBOR_LEFT) ;//왼쪽상단 트리
			if(pcNeighborSide)
				GetIntersectBoundingShpere(pcGolfBall, apsCollidedTRObject, nCollidedCount, pcNeighborSide->GetTRObjects()) ;

			pcNeighborSide = pcNeighbor->GetNeighbor(CSecretQuadTree::NEIGHBOR_RIGHT) ;//오른쪽상단 트리
			if(pcNeighborSide)
				GetIntersectBoundingShpere(pcGolfBall, apsCollidedTRObject, nCollidedCount, pcNeighborSide->GetTRObjects()) ;
		}
		pcNeighbor = pcQuadTree->GetNeighbor(CSecretQuadTree::NEIGHBOR_LEFT) ;//바로 왼쪽
		if(pcNeighbor)
		{
			GetIntersectBoundingShpere(pcGolfBall, apsCollidedTRObject, nCollidedCount, pcNeighbor->GetTRObjects()) ;
		}
		pcNeighbor = pcQuadTree->GetNeighbor(CSecretQuadTree::NEIGHBOR_RIGHT) ;//바로 오른쪽
		if(pcNeighbor)
		{
			GetIntersectBoundingShpere(pcGolfBall, apsCollidedTRObject, nCollidedCount, pcNeighbor->GetTRObjects()) ;
		}

		GetIntersectBoundingShpere(pcGolfBall, apsCollidedTRObject, nCollidedCount, pcQuadTree->GetTRObjects()) ;//지금 있는곳
	}

	if(nCollidedCount)
	{
		CSecretRigidbody *pcRigidbody ;
		for(i=0 ; i<nCollidedCount ; i++)
		{
			pcRigidbody = AddPlant(apsCollidedTRObject[i]) ;
			if(pcRigidbody)
			{
				//TRACE("add rigidbody name=%s\r\n", pcRigidbody->GetTRObject()->apcObject[0]->GetObjName()) ;

				AddRigidbody(pcRigidbody) ;
				m_cCollisionDetection.AddRigidbody(pcRigidbody) ;
			}
		}
	}
	//TRACE("Collided Count=%d\r\n", nCollidedCount) ;
}

void CSecretPhysicsManager::DeleteRigidbody(CSecretRigidbody *pcRigidbody)
{
	if(pcRigidbody->GetBodykind() == CSecretRigidbody::GOLFBALL)
	{
		CGolfBall *pcGolfBall = (CGolfBall *)pcRigidbody ;
		pcGolfBall->SetEnable(false) ;
	}
}
void CSecretPhysicsManager::_ClearPlants()
{
	static std::list<CSecretRigidbody *>::iterator it_erase[MAXNUM_PLANT] ;
	std::list<CSecretRigidbody *>::iterator it ;
	int count=0 ;

	for(it=m_Rigidbodies.begin() ; it!=m_Rigidbodies.end() ; it++)
	{
		if((*it)->GetBodykind() == CSecretRigidbody::PLANT)
		{
			(*it)->SetEnable(false) ;
			it_erase[count++] = it ;
		}
	}

	for(int i=0 ; i<count ; i++)
		m_Rigidbodies.erase(it_erase[i]) ;

	m_nNumPlant = 0 ;
}

int CSecretPhysicsManager::GetNumGolfBall()
{
	return m_nNumGolfBall ;
}
CGolfBall *CSecretPhysicsManager::GetGolfBalls()
{
	return m_pcGolfBalls ;
}
int CSecretPhysicsManager::GetNumPlant()
{
	return m_nNumPlant ;
}
CPlant *CSecretPhysicsManager::GetPlants()
{
	return m_pcPlants ;
}
bool CSecretPhysicsManager::IsEvent(DWORD dwEvent)
{
	if(m_dwEvent & dwEvent)
		return true ;
	return false ;
}