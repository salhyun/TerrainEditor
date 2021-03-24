#pragma once

#include "SecretMath.h"

struct Matrix4 ;

struct Vector4
{
public :
	float x, y, z, w ;

	//destructor
	~Vector4() ;

	//constructors
	Vector4() ;
	Vector4(float fx, float fy, float fz, float fw=1.f) ;

	//copy operator
	Vector4(const Vector4 &v) ;
	Vector4 &operator =(const Vector4 &v) ;

	//+, - operator
	Vector4 operator +(const Vector4 &v) ;
	Vector4 operator -(const Vector4 &v) ;
	Vector4 operator -(void) ;
    Vector4 &operator +=(const Vector4 &v) ;
	Vector4 &operator -=(const Vector4 &v) ;

	Vector4 operator +(const Vector4 &v) const ;

	//Multiply operator
	Vector4 operator *(const float fScale) const ;
	Vector4 &operator *=(const float fScale) ;

	Vector4 operator *(const Matrix4 &mat) ;//DirectX 타입의 행렬계산, 행기준행렬(Row major Matrix)
	Vector4 &operator *=(const Matrix4 &mat) ;//DirectX 타입의 행렬계산, 행기준행렬(Row major Matrix)

	Vector4 operator *(const Vector4 &v) const ;

	//dot
	float dot(const Vector4 &v) ;

	//set variable
	void set(float fx, float fy, float fz, float fw=1.f) ;

	//misc
	float Magnitude() ;
	Vector4 Normalize() ;	
} ;