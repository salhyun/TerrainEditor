#include "Vector2.h"

Vector2::~Vector2()
{
}

Vector2::Vector2()
{
	x=y=0.f ;
}

Vector2::Vector2(float fx, float fy)
{
	x=fx ;	y=fy ;
}

Vector2::Vector2(const Vector2 &v)
{
	if(this != &v)
	{
		x=v.x ;		y=v.y ;
	}
}

Vector2 &Vector2::operator =(const Vector2 &v)
{
	if(this != &v)
	{
		x=v.x ;		y=v.y ;
	}
	return *this ;
}

Vector2 Vector2::operator +(const Vector2 &v) const
{
	Vector2 vRet ;
	vRet.x = x+v.x ;	vRet.y = y+v.y ;
	return vRet ;
}

Vector2 Vector2::operator -(const Vector2 &v) const
{
	Vector2 vRet ;
	vRet.x = x-v.x ;	vRet.y = y-v.y ;
	return vRet ;
}

Vector2 Vector2::operator -(void) const
{
	Vector2 vRet ;
	vRet.x = -x ;	vRet.y = -y ;
    return vRet ;
}

Vector2 &Vector2::operator +=(const Vector2 &v)
{
	x += v.x ;	y += v.y ;
	return *this ;
}

Vector2 &Vector2::operator -=(const Vector2 &v)
{
	x -= v.x ;	y -= v.y ;
	return *this ;
}

Vector2 Vector2::operator *(const float fScale) const
{
	Vector2 vRet(x*fScale, y*fScale) ;
	return vRet ;
}

Vector2 &Vector2::operator *=(const float fScale)
{
	x *= fScale ;	y *= fScale ;
    return *this ;
}

bool Vector2::operator <(const Vector2 &v) const
{
	return ((x < v.x) && (y < v.y)) ;
}

bool Vector2::operator <=(const Vector2 &v) const
{
	return ((x <= v.x) && (y <= v.y)) ;
}
bool Vector2::operator >(const Vector2 &v) const
{
	return ((x > v.x) && (y > v.y)) ;
}

bool Vector2::operator >=(const Vector2 &v) const
{
	return ((x >= v.x) && (y >= v.y)) ;
}

void Vector2::set(float fx, float fy)
{
	x=fx ;	y=fy ;
}

float Vector2::Magnitude() const
{
	return sqrtf(x*x+y*y) ;
}

Vector2 Vector2::Normalize() const
{
	float fMagnitude = Magnitude() ;
	assert(!float_eq(fMagnitude, 0.f)) ;
	Vector2 vRet(x/fMagnitude, y/fMagnitude) ;
	return vRet ;
}