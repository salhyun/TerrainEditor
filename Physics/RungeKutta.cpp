#include "RungeKutta.h"

SRungeKutta::SRungeKutta()
{
}
SRungeKutta::~SRungeKutta()
{
}
void SRungeKutta::initialize()
{
	state.pos.set(0, 0, 0) ;
	state.vel.set(0, 0, 0) ;
	elapsed_time = 0.0f ;
}
//Vector3 SRungeKutta::acceleration()
//{
//	//Gravity
//	return Vector3(0.0f, 80.0f, 0.0f) ;
//}
SRK4Derivative SRungeKutta::evaluate(float t, Vector3 &acc)
{
	SRK4Derivative derivative ;
	derivative.dp = state.vel ;
	derivative.dv = acc ;
	return derivative ;
}
SRK4Derivative SRungeKutta::evaluate(float t, float dt, Vector3 &acc, const SRK4Derivative &d)
{
	SRK4State sta ;
	sta.pos = state.pos + d.dp*dt ;//위치 변화량 = 위치+속도*시간
	sta.vel = state.vel + d.dv*dt ;//속도 변화량 = 속도+가속도*시간

	SRK4Derivative derivative ;
	derivative.dp = sta.vel ;
	derivative.dv = acc ;

	return derivative ;
}
void SRungeKutta::integrate(float t, float dt, Vector3 &acc)
{
	SRK4Derivative a, b, c, d ;

	a = evaluate(t, acc) ;
	b = evaluate(t, dt*0.5f, acc, a) ;
	c = evaluate(t, dt*0.5f, acc, b) ;
	d = evaluate(t, dt, acc, c) ;

	Vector3 dpdt, dvdt ;
	float coef=1.0f/6.0f ;
	
	dpdt = (a.dp+(b.dp+c.dp)*2.0f+d.dp)*coef ;
	dvdt = (a.dv+(b.dv+c.dv)*2.0f+d.dv)*coef ;

	state.pos += dpdt*dt ;
	state.vel += dvdt*dt ;

	state.vel *= powf(0.9f, dt) ;
}