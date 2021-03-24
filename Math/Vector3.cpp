#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include <float.h>

Vector3::~Vector3()
{
}

Vector3::Vector3()
{
	x=y=z=0.f ;
}

Vector3::Vector3(float fx, float fy, float fz)
{
	x=fx ;	y=fy ;	z=fz ;
}

Vector3::Vector3(const Vector3 &v)
{
	if(this != &v)
	{
		x=v.x ;		y=v.y ;		z=v.z ;
	}
}

Vector3 &Vector3::operator =(const Vector3 &v)
{
	if(this != &v)
	{
		x=v.x ;		y=v.y ;		z=v.z ;
	}
	return *this ;
}

Vector3 Vector3::operator +(const Vector3 &v) const
{
	Vector3 vRet ;
	vRet.x = x+v.x ;	vRet.y = y+v.y ;	vRet.z = z+v.z ;
	return vRet ;
}

Vector3 Vector3::operator -(const Vector3 &v) const
{
	Vector3 vRet ;
	vRet.x = x-v.x ;	vRet.y = y-v.y ;	vRet.z = z-v.z ;	
	return vRet ;
}

Vector3 Vector3::operator -(void) const
{
	Vector3 vRet ;
	vRet.x = -x ;	vRet.y = -y ;	vRet.z = -z ;
    return vRet ;
}

Vector3 &Vector3::operator +=(const Vector3 &v)
{
	x += v.x ;	y += v.y ;	z += v.z ;
	return *this ;
}

Vector3 &Vector3::operator -=(const Vector3 &v)
{
	x -= v.x ;	y -= v.y ;	z -= v.z ;
	return *this ;
}

Vector3 Vector3::operator +(const float a) const
{
	Vector3 vRet ;
	vRet.x = x+a ; vRet.y = y+a ; vRet.z = z+a ;
	return vRet ;
}
Vector3 Vector3::operator -(const float a) const
{
	Vector3 vRet ;
	vRet.x = x-a ; vRet.y = y-a ; vRet.z = z-a ;
	return vRet ;
}
Vector3 &Vector3::operator +=(const float a)
{
	x += a ; y += a ; z += a ;
	return *this ;
}
Vector3 &Vector3::operator -=(const float a)
{
	x -= a ; y -= a ; z -= a ;
	return *this ;
}

Vector3 Vector3::operator *(const float fScale) const
{
	Vector3 vRet(x*fScale, y*fScale, z*fScale) ;
	return vRet ;
}

Vector3 &Vector3::operator *=(const float fScale)
{
	x *= fScale ;	y *= fScale ;	z *= fScale ;
    return *this ;
}

Vector3 Vector3::operator /(const float fScale) const
{
	Vector3 vRet(x/fScale, y/fScale, z/fScale) ;
	return vRet ;
}

Vector3 Vector3::operator /=(const float fScale)
{
	x /= fScale ;	y /= fScale ;	z /= fScale ;
    return *this ;
}

Vector3 Vector3::operator *(const Matrix4 &mat)
{
	Vector4 v4(x, y, z, 1.0f) ;

	v4 *= mat ;

	Vector3 v3(v4.x/v4.w, v4.y/v4.w, v4.z/v4.w) ;
	return v3 ;
}

Vector3 &Vector3::operator *=(const Matrix4 &mat)
{
	Vector4 v4(x, y, z, 1.0f) ;

	v4 *= mat ;

	Vector3 v3(v4.x/v4.w, v4.y/v4.w, v4.z/v4.w) ;

	*this = v3 ;
	return *this ;
}

Vector3 Vector3::operator *(const Vector3 &v) const
{
    Vector3 vRet ;
	vRet.x = x*v.x ;	vRet.y = y*v.y ;	vRet.z = z*v.z ;
	return vRet ;
}

bool Vector3::operator <(const Vector3 &v) const
{
	return ((x < v.x) && (y < v.y) && (z < v.z)) ;
}

bool Vector3::operator <=(const Vector3 &v) const
{
	return ((x <= v.x) && (y <= v.y) && (z <= v.z)) ;
}
bool Vector3::operator >(const Vector3 &v) const
{
	return ((x > v.x) && (y > v.y) && (z > v.z)) ;
}

bool Vector3::operator >=(const Vector3 &v) const
{
	return ((x >= v.x) && (y >= v.y) && (z >= v.z)) ;
}

float Vector3::operator[](unsigned int i) const
{
    return (&x)[i];
}

float& Vector3::operator[](unsigned int i)
{
   return (&x)[i];
}

float Vector3::dot(const Vector3 &v) const
{
	return (x*v.x+y*v.y+z*v.z) ;
}

Vector3 Vector3::cross(const Vector3 &v) const//왼손법칙 cross product
{
	//왼손법칙일경우 : z*v.x-x*v.z
	//오른손법칙일경우 x*v.z-z*v.x
	Vector3 vRet(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x) ;
	if(float_eq(vRet.Magnitude(), 0.0f))
		return vRet ;
	return vRet.Normalize() ;
}

void Vector3::set(float fx, float fy, float fz)
{
	x=fx ;	y=fy ;	z=fz ;
}

float Vector3::Magnitude() const
{
	return sqrtf(x*x+y*y+z*z) ;
}

Vector3 Vector3::Normalize() const
{
	Vector3 vRet ;
	float fMagnitude = Magnitude() ;

	if(fMagnitude <= FLT_EPSILON)
		return vRet ;

	//assert(!(float_abs(fMagnitude-0.0f) <= FLT_EPSILON)) ;
	//assert(!float_eq(fMagnitude, 0.f)) ;

	vRet.set(x/fMagnitude, y/fMagnitude, z/fMagnitude) ;
	return vRet ;
}

//Vector3 Vector3::Projection(Vector3 &vdest)
//{
//	float dot = this->dot(vdest) ;
//	if(dot <= 0.0f)
//		return Vector3(0, 0, 0) ;
//
//	//vdest가 this보다 커야 Projection할수 있음
//    // 투영벡터 = (A*B/|B|)(B/|B|) = (A*B/|B|*|B|)B
//	float fProjLength = dot/(vdest.Magnitude()*vdest.Magnitude()) ;
//	return vdest*fProjLength ;
//}

Vector3 Vector3::Projection(Vector3 &vdest)
{
	float dot = this->dot(vdest) ;
	if(dot <= 0.0f)
		return Vector3(0, 0, 0) ;

	float l = float_round(dot/vdest.dot(vdest)) ;
	return vdest*l ;
}
float Vector3::GreatestElement(bool bSign)
{
	if(bSign)
	{
		if(float_greater(x, y))
		{
			if(float_greater(x, z))
				return x ;
			else
				return z ;
		}
		else
		{
			if(float_greater(y, z))
				return y ;
			else
				return z ;
		}
	}
	else
	{
		float abs_x=float_abs(x), abs_y=float_abs(y), abs_z=float_abs(z) ;

		if(float_greater(abs_x, abs_y))
		{
			if(float_greater(abs_x, abs_z))
				return x ;
			else
				return z ;
		}
		else
		{
			if(float_greater(abs_y, abs_z))
				return y ;
			else
				return z ;
		}
	}
	return 0.0f ;
}
void Vector3::roundup()
{
	x = float_round(x) ;
	y = float_round(y) ;
	z = float_round(z) ;
}
bool Vector3::IsZero()
{
	if(float_eq(x, 0.0f) && float_eq(y, 0.0f) && float_eq(z, 0.0f))
		return true ;
	return false ;
}