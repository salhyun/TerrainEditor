#include "Vector4.h"
#include "Matrix.h"

Vector4::~Vector4()
{
}

Vector4::Vector4()
{
	x=y=z=w=0.f ;
}

Vector4::Vector4(float fx, float fy, float fz, float fw)
{
	x=fx ;	y=fy ;	z=fz,	w=fw ;
}

Vector4::Vector4(const Vector4 &v)
{
	if(this != &v)
	{
		x=v.x ;		y=v.y ;		z=v.z ;		w=v.w ;
	}
}

void Vector4::set(float fx, float fy, float fz, float fw)
{
	x=fx ;	y=fy ;	z=fz,	w=fw ;
}

Vector4 &Vector4::operator =(const Vector4 &v)
{
	if(this != &v)
	{
		x=v.x ;		y=v.y ;		z=v.z ;		w=v.w ;
	}
	return *this ;
}

Vector4 Vector4::operator +(const Vector4 &v)
{
	Vector4 vRet ;
	vRet.x = x+v.x ;	vRet.y = y+v.y ;	vRet.z = z+v.z ;	vRet.w = z+v.w ;
	return vRet ;
}

Vector4 Vector4::operator -(const Vector4 &v)
{
	Vector4 vRet ;
	vRet.x = x-v.x ;	vRet.y = y-v.y ;	vRet.z = z-v.z ;	vRet.w = z-v.w ;
	return vRet ;
}

Vector4 Vector4::operator -(void)
{
	Vector4 vRet ;
	vRet.x = -x ;	vRet.y = -y ;	vRet.z = -z ;	vRet.w = -w ;
    return vRet ;
}

Vector4 &Vector4::operator +=(const Vector4 &v)
{
	x += v.x ;	y += v.y ;	z += v.z ;	w += v.w ;
	return *this ;
}

Vector4 &Vector4::operator -=(const Vector4 &v)
{
	x -= v.x ;	y -= v.y ;	z -= v.z ;	w -= v.w ;
	return *this ;
}

Vector4 Vector4::operator +(const Vector4 &v) const
{
	Vector4 vRet ;
	vRet.x = x+v.x ;	vRet.y = y+v.y ;	vRet.z = z+v.z ;	vRet.w = w+v.w ;
	return vRet ;
}

Vector4 Vector4::operator *(const float fScale) const
{
	Vector4 vRet(x*fScale, y*fScale, z*fScale, w*fScale) ;
	return vRet ;
}

Vector4 &Vector4::operator *=(const float fScale)
{
	x *= fScale ;	y *= fScale ;	z *= fScale ;	w *= fScale ;
    return *this ;
}

Vector4 Vector4::operator *(const Matrix4 &mat)
{
	Vector4 vRet ;

	vRet.x = x*mat.m11 + y*mat.m21 + z*mat.m31 + w*mat.m41 ;
	vRet.y = x*mat.m12 + y*mat.m22 + z*mat.m32 + w*mat.m42 ;
	vRet.z = x*mat.m13 + y*mat.m23 + z*mat.m33 + w*mat.m43 ;
	vRet.w = x*mat.m14 + y*mat.m24 + z*mat.m34 + w*mat.m44 ;

	return vRet ;
}

Vector4 &Vector4::operator *=(const Matrix4 &mat)
{
	Vector4 vRet ;

	vRet.x = x*mat.m11 + y*mat.m21 + z*mat.m31 + w*mat.m41 ;
	vRet.y = x*mat.m12 + y*mat.m22 + z*mat.m32 + w*mat.m42 ;
	vRet.z = x*mat.m13 + y*mat.m23 + z*mat.m33 + w*mat.m43 ;
	vRet.w = x*mat.m14 + y*mat.m24 + z*mat.m34 + w*mat.m44 ;

	*this = vRet ;

	return *this ;
}

Vector4 Vector4::operator *(const Vector4 &v) const
{
    Vector4 vRet ;
	vRet.x = x*v.x ;	vRet.y = y*v.y ;	vRet.z = z*v.z ;	vRet.w = w*v.w ;
	return vRet ;
}

float Vector4::dot(const Vector4 &v)
{
    return (x*v.x+y*v.y+z*v.z+w*v.w) ;
}