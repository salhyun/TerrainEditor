#pragma once

#include "SecretPhysicsDef.h"
#include "RungeKutta.h"

struct STrueRenderingObject ;

struct SForce
{
	Vector3 vForce, vPos ;

	void SetForce(Vector3 &force) ;
	Vector3 GetForce() ;
	void SetPos(Vector3 &pos) ;
	Vector3 GetPos() ;
	void ClearForce() ;
} ;

struct SRigidbodyState
{
	//linear motion properties
	Vector3 vPos ;//center of mass location
	Vector3 vLinearVel ;
	Vector3 vLinearMomentum ;

	//rotational motion properties
	Vector3 vCurOrientation ;
	Vector3 vAngularVel ;
	Vector3 vAngularMomentum ;
	Vector3 vRotationalInertia ;
	Vector3 vTorque ;

	//constant state
	float fMass, fInvMass ;
	float fInertiaTensor, fInvInertiaTensor ;
	float m_fDamping ;

	void Recalculate() ;
} ;

class CSecretRigidbody
{
public :
	enum ATTR {ATTR_IMMOVABLE=0x01, ATTR_MOVABLE=0x02, ATTR_INVISIBLE=0x04} ;
	enum BODYKIND {PLANT=0, GOLFBALL, STONE} ;

	enum MOVESTATUS {SLEEP=0, ACTIVITY} ;
	enum MOVEKIND {FLYING=0, ROLLING} ;

protected :
	bool m_bEnable ;
	unsigned long m_lAttr ;
	unsigned long m_lBodykind ;
	unsigned long m_lMoveStatus ;
	unsigned long m_lMoveKind ;

	char m_szName[64] ;

	SRigidbodyState m_sCurState, m_sPrevState ;

	STrueRenderingObject *m_psTRObject ;

public :
	SForce m_sImplusForce, m_sConstantForce ;
	float m_fElapsedTime ;

	Vector3 m_vOffset ;

public :
	CSecretRigidbody() ;
	virtual ~CSecretRigidbody() ;

	virtual void Integrate(float dt) {}

	void SetMass(float fMass) ;
	float GetMass() ;
	float GetInvMass() ;

	void SetLinearVel(Vector3 &vel) ;
	Vector3 GetLinearVel() ;

	void SetLinearMomentum(Vector3 &momentum) ;
	Vector3 GetLinearMomentum() ;

	void SetPos(Vector3 &pos) ;
	Vector3 GetPos() ;

	void SetEnable(bool bEnable=true) ;
	bool IsEnable() ;

	void AddAttr(unsigned long lAttr, bool bEnable=true) ;
	unsigned long GetAttr() ;

	void SetBodykind(unsigned long lBodykind) ;
	unsigned long GetBodykind() ;

	void SetMoveStatus(unsigned long lMoveStatus) ;//MOVESTATUS {SLEEP=0, ACTIVITY}
	unsigned long GetMoveStatus() ;//MOVESTATUS {SLEEP=0, ACTIVITY}
	
	void SetMoveKind(unsigned long lMoveKind) ;//MOVEKIND {FLYING=0, ROLLING}
	unsigned long GetMoveKind() ;//MOVEKIND {FLYING=0, ROLLING}

	void SetName(char *pszName) ;
	char *GetName() ;

	void SetTRObject(STrueRenderingObject *psTRObject) ;
	STrueRenderingObject *GetTRObject() ;
	void UpdateTransformation() ;

	SRigidbodyState *GetCurState() ;
	SRigidbodyState *GetPrevState() ;

	void InitVariable() ;

} ;