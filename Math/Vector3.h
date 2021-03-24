#pragma once

#include "SecretMath.h"

//(%03.03f %03.03f %03.03f)
#define enumVector(v) v.x, v.y, v.z

#ifndef vector_eq
#define vector_eq(v1, v2) ( float_eq(v1.x, v2.x) && float_eq(v1.y, v2.y) && float_eq(v1.z, v2.z) )
#endif

struct Matrix4 ;

struct Vector3
{
public :
	float x, y, z ;

	//destructor
	~Vector3() ;

	//constructors
	Vector3() ;
	Vector3(float fx, float fy, float fz) ;

	//copy operator
	Vector3(const Vector3 &v) ;
	Vector3 &operator =(const Vector3 &v) ;

	//+, - operator
	Vector3 operator +(const Vector3 &v) const ;
	Vector3 operator -(const Vector3 &v) const ;
	Vector3 operator -(void) const ;
    Vector3 &operator +=(const Vector3 &v) ;
	Vector3 &operator -=(const Vector3 &v) ;
	Vector3 operator +(const float a) const ;
	Vector3 operator -(const float a) const ;
	Vector3 &operator +=(const float a) ;
	Vector3 &operator -=(const float a) ;

	//Multiply operator
	Vector3 operator *(const float fScale) const ;
	Vector3 &operator *=(const float fScale) ;

	Vector3 operator /(const float fScale) const ;
	Vector3 operator /=(const float fScale) ;

	Vector3 operator *(const Matrix4 &mat) ;
	Vector3 &operator *=(const Matrix4 &mat) ;

	Vector3 operator *(const Vector3 &v) const ;

	bool operator <(const Vector3 &v) const ;
	bool operator <=(const Vector3 &v) const ;
	bool operator >(const Vector3 &v) const ;
	bool operator >=(const Vector3 &v) const ;

	// indexing
    float operator[](unsigned int i) const;
    float& operator[](unsigned int i);

	//dot, cross operator
	float dot(const Vector3 &v) const ;
	Vector3 cross(const Vector3 &v) const ;//¿Þ¼Õ¹ýÄ¢

	//Projection 
	Vector3 Projection(Vector3 &vdest) ;//do not need Normalized

	//set variable
	void set(float fx, float fy, float fz) ;

	//misc
	float Magnitude() const ;
	Vector3 Normalize() const ;
	float GreatestElement(bool bSign=true) ;
	void roundup() ;
	bool IsZero() ;
} ;