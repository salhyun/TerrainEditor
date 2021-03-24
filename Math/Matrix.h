#pragma once

#include "SecretMath.h"

struct Vector3 ;
struct Vector4 ;

struct Matrix4
{
public :
	float m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44 ;

	//destructor
	~Matrix4() ;

	//constructor
	Matrix4() ;
	Matrix4(float _m11, float _m12, float _m13, float _m14, float _m21, float _m22, float _m23, float _m24,
		float _m31, float _m32, float _m33, float _m34, float _m41, float _m42, float _m43, float _m44) ;

	double GetElement(unsigned long row, unsigned long column) const ;
	void SetElement(unsigned long row, unsigned long column, float fValue) ;

	//copy operator
	Matrix4(const Matrix4 &mat) ;
	Matrix4 &operator=(const Matrix4 &mat) ;

	//+, - operator
	Matrix4 operator +(const Matrix4 &mat) ;
	Matrix4 operator -(const Matrix4 &mat) ;
	Matrix4 &operator +=(const Matrix4 &mat) ;
	Matrix4 &operator -=(const Matrix4 &mat) ;

	//Scale
	Matrix4 operator *(const float fScale) ;
	Matrix4 &operator *=(const float fScale) ;

	//multiply
	Matrix4 operator *(const Matrix4 &mat) ;
	Matrix4 &operator *=(const Matrix4 &mat) ;

	Vector4 operator *(const Vector4 &v) ;//OpenGL 타입의 행렬계산, 열기준행렬(Column major Matrix)

	//set variable
	void Set(float _m11, float _m12, float _m13, float _m14, float _m21, float _m22, float _m23, float _m24,
		float _m31, float _m32, float _m33, float _m34, float _m41, float _m42, float _m43, float _m44) ;

	//Rotate
	void SetRotatebyAxisX(float ftheta, bool rad=false) ;
	void SetRotatebyAxisY(float ftheta, bool rad=false) ;
	void SetRotatebyAxisZ(float ftheta, bool rad=false) ;
	
	void SetTranslation(const Vector3 &v) ;
	void SetTranslation(float x, float y, float z) ;

	//Identity
	void Identity() ;

	//Transpose
    Matrix4 Transpose() const ;

    //Inverse
	Matrix4 Inverse() const ;

	float determinant() ;

	Matrix4 DecomposeTranslation() ;
	Matrix4 DecomposeRotation() ;
	Matrix4 DecomposeScaling() ;
} ;