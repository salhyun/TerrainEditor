#pragma once

#include "Vector3.h"
#include "Matrix.h"

//#undef EPSILON
//#define EPSILON 0.005f
//
//#define FLOAT_EQ(x,v)	( ((v) - EPSILON) < (x) && (x) < ((v) + EPSILON) )		// float equality test

class Quaternion
{
public :
	double x, y, z, w ;

private :
	double QuatNorm() ;//Length

public :
	Quaternion() ;
	Quaternion(float x, float y, float z, float w) ;
    Quaternion(float x, float y, float z) ;// 각 축으로 회전량으로 쿼터니언을 초기화한다.
	Quaternion(Vector3 v, float ftheta) ;
	Quaternion(Vector3 v1, Vector3 v2) ;
	Quaternion(const Matrix4 &mat) ;//당연한거지만 [m41, m44]는 사용안한다는거
	~Quaternion() ;

	void Identity() ;
	void QuatNormalize() ;
	void GetValues(float &x, float &y, float &z, float &w) ;
	void EulerToQuat(float x, float y, float z) ;//x:x축으로의 회전량, y:y축으로의 회전량, z:z축으로의 회전량
	void AxisToQuat(Vector3 &v, float ftheta) ;
	void GetAxisAngle(Vector3 &v, float &ftheta) ;//현재의 축이 몇도 회전인가
	void GetMatrix(Matrix4 &mat) ;
	void RotatebyQuat(Vector3 &vDest, Vector3 &vSrc) ;
	void MatrixToQuat(const Matrix4 &mat) ;//당연한거지만 [m41, m44]는 사용안한다는거

	void TwoVectorsToQuat(Vector3 &v1, Vector3 &v2) ;//2개의 벡터의 사잇각으로 쿼터니언을 만듬.

	Quaternion operator*(const Quaternion &q) ;
	Quaternion &operator=(const Quaternion &q) ;

	float GetPitch() ;//by axis-x
	float GetYaw() ;//by axis-y
	float GetRoll() ;//by axis-z

	void GetPitchYawRoll(float &pitch, float &yaw, float &roll) ;
} ;