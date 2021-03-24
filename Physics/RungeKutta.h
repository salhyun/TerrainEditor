#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

//���ӵǴ� ��(Constant Force)�� Runge-Kutta�� ����ȴ�.

struct SRK4State
{
	Vector3 pos ;//position
	Vector3 vel ;//velocity
} ;
struct SRK4Derivative
{
	Vector3 dp ;//derivative of position : velocity
	Vector3 dv ;//derivative of velocity : acceleration
} ;
struct SRungeKutta
{
	SRK4State state ;

	float elapsed_time ;

	SRungeKutta() ;
	~SRungeKutta() ;

	void initialize() ;
	
	//Vector3 acceleration() ;//�� �Լ��� ���ӵǴ� ���� ���Ǿ����� �Ѵ�.
	SRK4Derivative evaluate(float t, Vector3 &acc) ;
	SRK4Derivative evaluate(float t, float dt, Vector3 &acc, const SRK4Derivative &d) ;
	void integrate(float t, float dt, Vector3 &acc) ;
} ;