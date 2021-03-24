#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

//지속되는 힘(Constant Force)만 Runge-Kutta에 적용된다.

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
	
	//Vector3 acceleration() ;//이 함수에 지속되는 힘이 계산되어져야 한다.
	SRK4Derivative evaluate(float t, Vector3 &acc) ;
	SRK4Derivative evaluate(float t, float dt, Vector3 &acc, const SRK4Derivative &d) ;
	void integrate(float t, float dt, Vector3 &acc) ;
} ;