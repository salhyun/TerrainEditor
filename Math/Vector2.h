#pragma once

#include "SecretMath.h"

struct Vector2
{
public :
	float x, y ;

	//destructor
	~Vector2() ;

	//constructors
	Vector2() ;
	Vector2(float fx, float fy) ;

	//copy operator
	Vector2(const Vector2 &v) ;
	Vector2 &operator =(const Vector2 &v) ;

	//+, - operator
	Vector2 operator +(const Vector2 &v) const ;
	Vector2 operator -(const Vector2 &v) const ;
	Vector2 operator -(void) const ;
    Vector2 &operator +=(const Vector2 &v) ;
	Vector2 &operator -=(const Vector2 &v) ;

	//Multiply operator
	Vector2 operator *(const float fScale) const ;
	Vector2 &operator *=(const float fScale) ;

	bool operator <(const Vector2 &v) const ;
	bool operator <=(const Vector2 &v) const ;
	bool operator >(const Vector2 &v) const ;
	bool operator >=(const Vector2 &v) const ;

	//set variable
	void set(float fx, float fy) ;

	//misc
	float Magnitude() const ;
	Vector2 Normalize() const ;
} ;