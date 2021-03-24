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
    Quaternion(float x, float y, float z) ;// �� ������ ȸ�������� ���ʹϾ��� �ʱ�ȭ�Ѵ�.
	Quaternion(Vector3 v, float ftheta) ;
	Quaternion(Vector3 v1, Vector3 v2) ;
	Quaternion(const Matrix4 &mat) ;//�翬�Ѱ����� [m41, m44]�� �����Ѵٴ°�
	~Quaternion() ;

	void Identity() ;
	void QuatNormalize() ;
	void GetValues(float &x, float &y, float &z, float &w) ;
	void EulerToQuat(float x, float y, float z) ;//x:x�������� ȸ����, y:y�������� ȸ����, z:z�������� ȸ����
	void AxisToQuat(Vector3 &v, float ftheta) ;
	void GetAxisAngle(Vector3 &v, float &ftheta) ;//������ ���� � ȸ���ΰ�
	void GetMatrix(Matrix4 &mat) ;
	void RotatebyQuat(Vector3 &vDest, Vector3 &vSrc) ;
	void MatrixToQuat(const Matrix4 &mat) ;//�翬�Ѱ����� [m41, m44]�� �����Ѵٴ°�

	void TwoVectorsToQuat(Vector3 &v1, Vector3 &v2) ;//2���� ������ ���հ����� ���ʹϾ��� ����.

	Quaternion operator*(const Quaternion &q) ;
	Quaternion &operator=(const Quaternion &q) ;

	float GetPitch() ;//by axis-x
	float GetYaw() ;//by axis-y
	float GetRoll() ;//by axis-z

	void GetPitchYawRoll(float &pitch, float &yaw, float &roll) ;
} ;