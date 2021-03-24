#include "SecretRigidBody.h"
#include <stdio.h>
#include "SecretSceneManager.h"

//##############################################//
//                    SForce                    //
//##############################################//
void SForce::SetForce(Vector3 &force)
{
	vForce = force ;
}
Vector3 SForce::GetForce()
{
	return vForce ;
}
void SForce::SetPos(Vector3 &pos)
{
	vPos = pos ;
}
Vector3 SForce::GetPos()
{
	return vPos ;
}
void SForce::ClearForce()
{
	vForce.set(0, 0, 0) ;
}

//##############################################//
//               SRigidbodyState                //
//##############################################//
void SRigidbodyState::Recalculate()
{
	vLinearVel = vLinearMomentum*fInvMass ;
}

//##############################################//
//              CSecretRigidbody                //
//##############################################//

CSecretRigidbody::CSecretRigidbody()
{
	m_bEnable = false ;
	m_lAttr = 0 ;
	m_psTRObject = NULL ;
	m_lMoveStatus = SLEEP ;
	m_lMoveKind = FLYING ;
	m_fElapsedTime = 0.0f ;
}
CSecretRigidbody::~CSecretRigidbody()
{
}

void CSecretRigidbody::InitVariable()
{
	m_bEnable = false ;
	m_lAttr = 0 ;
	m_psTRObject = NULL ;
	m_lMoveStatus = SLEEP ;
	m_lMoveKind = FLYING ;
	m_fElapsedTime = 0.0f ;
}
void CSecretRigidbody::SetMass(float fMass)
{
	m_sCurState.fMass = fMass ;
	if(float_greater(m_sCurState.fMass, 0.0f))
		m_sCurState.fInvMass = 1.0f/m_sCurState.fMass ;
}
float CSecretRigidbody::GetMass()
{
	return m_sCurState.fMass ;
}
float CSecretRigidbody::GetInvMass()
{
	return m_sCurState.fInvMass ;
}
void CSecretRigidbody::SetLinearVel(Vector3 &vel)
{
	m_sCurState.vLinearVel = vel ;
}
void CSecretRigidbody::SetLinearMomentum(Vector3 &momentum)
{
	m_sCurState.vLinearMomentum = momentum ;
}
Vector3 CSecretRigidbody::GetLinearMomentum()
{
	return m_sCurState.vLinearMomentum ;
}
Vector3 CSecretRigidbody::GetLinearVel()
{
	return m_sCurState.vLinearVel ;
}
void CSecretRigidbody::SetPos(Vector3 &pos)
{
	m_sCurState.vPos = pos ;
}
Vector3 CSecretRigidbody::GetPos()
{
	return m_sCurState.vPos ;
}
void CSecretRigidbody::SetEnable(bool bEnable)
{
	m_bEnable = bEnable ;
}
bool CSecretRigidbody::IsEnable()
{
	return m_bEnable ;
}
void CSecretRigidbody::AddAttr(unsigned long lAttr, bool bEnable)
{
	if(bEnable)
		m_lAttr |= lAttr ;
	else
		m_lAttr &= (~lAttr) ;
}
unsigned long CSecretRigidbody::GetAttr()
{
	return m_lAttr ;
}
void CSecretRigidbody::SetBodykind(unsigned long lBodykind)
{
	m_lBodykind = lBodykind ;
}
unsigned long CSecretRigidbody::GetBodykind()
{
	return m_lBodykind ;
}
void CSecretRigidbody::SetMoveStatus(unsigned long lMoveStatus)
{
	m_lMoveStatus = lMoveStatus ;
}
unsigned long CSecretRigidbody::GetMoveStatus()
{
	return m_lMoveStatus ;
}
void CSecretRigidbody::SetMoveKind(unsigned long lMoveKind)
{
	m_lMoveKind = lMoveKind ;
}
unsigned long CSecretRigidbody::GetMoveKind()
{
	return m_lMoveKind ;
}
void CSecretRigidbody::SetName(char *pszName)
{
	sprintf_s(m_szName, 64, "%s", pszName) ;
}
char *CSecretRigidbody::GetName()
{
	return m_szName ;
}
void CSecretRigidbody::SetTRObject(STrueRenderingObject *psTRObject)
{
	m_psTRObject = psTRObject ;
}
STrueRenderingObject *CSecretRigidbody::GetTRObject()
{
	return m_psTRObject ;
}
void CSecretRigidbody::UpdateTransformation()
{
	m_psTRObject->matWorld._41 = m_sCurState.vPos.x + m_vOffset.x ;
	m_psTRObject->matWorld._42 = m_sCurState.vPos.y + m_vOffset.y ;
	m_psTRObject->matWorld._43 = m_sCurState.vPos.z + m_vOffset.z ;

	m_psTRObject->cBBox.SetTransform(&m_psTRObject->matWorld) ;
}
SRigidbodyState *CSecretRigidbody::GetCurState()
{
	return &m_sCurState ;
}
SRigidbodyState *CSecretRigidbody::GetPrevState()
{
	return &m_sPrevState ;
}