#include "SecretMath.h"
#include "Quaternion.h"


Quaternion::Quaternion()
{
	Identity() ;
}

Quaternion::Quaternion(float x, float y, float z, float w)
{
	this->x = (double)x ;
	this->y = (double)y ;
	this->z = (double)z ;

	this->w = (double)w ;
}

Quaternion::Quaternion(float x, float y, float z)
{
	EulerToQuat(x, y, z) ;
}

Quaternion::Quaternion(Vector3 v, float ftheta)
{
	AxisToQuat(v, ftheta) ;
}
Quaternion::Quaternion(Vector3 v1, Vector3 v2)
{
	TwoVectorsToQuat(v1, v2) ;
}
Quaternion::Quaternion(const Matrix4 &mat)
{
	MatrixToQuat(mat) ;
}

Quaternion::~Quaternion()
{
}

void Quaternion::QuatNormalize()
{
	double Norm = QuatNorm() ;

    assert(!float_eq(Norm, 0.0f)) ;//Must be nonZero

	x /= Norm ;
	y /= Norm ;
	z /= Norm ;
	w /= Norm ;

	assert(float_eq((float)QuatNorm(), 1.0f)) ;//Must be 1
}

double Quaternion::QuatNorm()
{
	return sqrt(sqr(x)+sqr(y)+sqr(z)+sqr(w)) ;
}

void Quaternion::Identity()
{
	x = y = z = 0.f ;
	w = 1.f ;
}

void Quaternion::GetValues(float &x, float &y, float &z, float &w)
{
	x = (float)this->x;
	y = (float)this->y;
	z = (float)this->z;

	w = (float)this->w;
}

void Quaternion::EulerToQuat(float x, float y, float z)
{
	double ex = x / 2.0;	// convert to rads and half them
	double ey = y / 2.0;
	double ez = z / 2.0;

	double c1 = cos(ey) ;
	double s1 = sin(ey) ;
	double c2 = cos(ez) ;
	double s2 = sin(ez) ;
	double c3 = cos(ex) ;
	double s3 = sin(ex) ;

	double c1c2 = c1*c2;
	double s1s2 = s1*s2;

	this->w =c1c2*c3 - s1s2*s3;
  	this->x =c1c2*s3 + s1s2*c3;
	this->y =s1*c2*c3 + c1*s2*s3;
	this->z =c1*s2*c3 - s1*c2*s3;
}
/*
void Quaternion::EulerToQuat(float x, float y, float z)
{
	double	ex, ey, ez;		// temp half euler angles
	double	cr, cp, cy, sr, sp, sy, cpcy, spsy;		// temp vars in roll,pitch yaw

	ex = x / 2.0;	// convert to rads and half them
	ey = y / 2.0;
	ez = z / 2.0;

	cr = cos(ex);
	cp = cos(ey);
	cy = cos(ez);

	sr = sin(ex);
	sp = sin(ey);
	sy = sin(ez);

	cpcy = cp * cy;
	spsy = sp * sy;

	this->w = cr * cpcy + sr * spsy;

	this->x = sr * cpcy - cr * spsy;
	this->y = cr * sp * cy + sr * cp * sy;
	this->z = cr * cp * sy - sr * sp * cy;

	QuatNormalize();
}
*/
void Quaternion::AxisToQuat(Vector3 &v, float ftheta)
{
	//double axis_x,axis_y,axis_z;			// temp vars of vector
	double rad, scale;		// temp vars

	if (float_eq(0.0f, v.Magnitude()))			// if axis is zero, then return quaternion (1,0,0,0)
	{
		w	= 1.0;
		x	= 0.0;
		y	= 0.0;
		z	= 0.0;
		return;
	}

	rad		= (double)ftheta / 2.0 ;

	w		= cos(rad);

	scale	= sin(rad);

	x = ((double)v.x * scale);
	y = ((double)v.y * scale);
	z = ((double)v.z * scale);

	QuatNormalize();		// make sure a unit quaternion turns up
}

void Quaternion::GetAxisAngle(Vector3 &v, float &ftheta)
{
	double	temp_angle;		// temp angle
	double	scale;			// temp vars

	temp_angle = acos(w);

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Another version where scale is sqrt (x2 + y2 + z2)
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	scale = sqrt(sqr(x) + sqr(y) + sqr(z));

	assert(0 <= temp_angle);		// make sure angle is 0 - PI
	assert(3.14159265358979323846f >= temp_angle);

	if (float_eq(0.0f, scale))		// angle is 0 or 360 so just simply set axis to 0,0,1 with angle 0
	{
		ftheta = 0.0f;

		//v.SetValues(0.0f, 0.0f, 1.0f);		// any axis will do
		v.x = 0.f ;
		v.y = 0.f ;
		v.z = 1.f ;
	}
	else
	{
		ftheta = (float)(temp_angle * 2.0);		// angle in radians

		//v.SetValues(float(x / scale), float(y / scale), float(z / scale));
		v.x = (float)(x/scale) ;
		v.y = (float)(y/scale) ;
		v.z = (float)(z/scale) ;

		//v.Normalize();
		v = v.Normalize() ;
		

		assert(0.0f <= ftheta);			// make sure rotation around axis is 0 - 360
		assert(2.f*3.14159265358979323846f >= ftheta);
		//assert(v.IsUnit());				// make sure a unit axis comes up
	}
}

void Quaternion::GetMatrix(Matrix4 &mat)
{
	double x2, y2, z2, w2, xy, xz, yz, wx, wy, wz;

	mat.m14 = mat.m24 = mat.m34 = 0.0f;
    mat.m41 = mat.m42 = mat.m43 = 0.0f;
    mat.m44 = 1.0f;

	x2	= sqr(x);
	y2	= sqr(y);
	z2	= sqr(z);
	w2	= sqr(w);

	xy	= x * y;
	xz	= x * z;
	yz	= y * z;
	wx	= w * x;
	wy	= w * y;
	wz	= w * z;

	// should be slightly more accurate than above due to rearranging for unit quaternions only
	mat.m11 =	float(1 - 2*(y2 + z2));
	mat.m12 =	float(2 * (xy + wz));
	mat.m13 =	float(2 * (xz - wy));

	mat.m21 =	float(2 * (xy - wz));
	mat.m22 =	float(1 - 2*(x2 + z2));
	mat.m23 =	float(2 * (yz + wx));

	mat.m31 =	float(2 * (xz + wy));
	mat.m32 =	float(2 * (yz - wx));
	mat.m33 =	float(1 - 2*(x2 + y2));

	/*
	if( mat.m11 + mat.m22 + mat.m33 > 0.0f )
    {
        FLOAT s = sqrtf( mat.m11 + mat.m22 + mat.m33 + mat.m44 );

        x = (mat.m23-mat.m32) / (2*s);
        y = (mat.m31-mat.m13) / (2*s);
        z = (mat.m12-mat.m21) / (2*s);
        w = 0.5f * s;
    }

    FLOAT xx = x*x; FLOAT yy = y*y; FLOAT zz = z*z;
    FLOAT xy = x*y; FLOAT xz = x*z; FLOAT yz = y*z;
    FLOAT wx = w*x; FLOAT wy = w*y; FLOAT wz = w*z;
    
    mat.m11 = 1 - 2 * ( yy + zz ); 
    mat.m12 =     2 * ( xy - wz );
    mat.m13 =     2 * ( xz + wy );

    mat.m21 =     2 * ( xy + wz );
    mat.m22 = 1 - 2 * ( xx + zz );
    mat.m23 =     2 * ( yz - wx );

    mat.m31 =     2 * ( xz - wy );
    mat.m32 =     2 * ( yz + wx );
    mat.m33 = 1 - 2 * ( xx + yy );

    mat.m14 = mat.m24 = mat.m34 = 0.0f;
    mat.m41 = mat.m42 = mat.m43 = 0.0f;
    mat.m44 = 1.0f;
	*/
}

void Quaternion::RotatebyQuat(Vector3 &vDest, Vector3 &vSrc)
{
    Matrix4 mat ;

	GetMatrix(mat) ;

	vDest.x = (vSrc.x*mat.m11)+(vSrc.y*mat.m21)+(vSrc.z*mat.m31)+(1*mat.m41) ;
	vDest.y = (vSrc.x*mat.m12)+(vSrc.y*mat.m22)+(vSrc.z*mat.m32)+(1*mat.m42) ;
	vDest.z = (vSrc.x*mat.m13)+(vSrc.y*mat.m23)+(vSrc.z*mat.m33)+(1*mat.m43) ;

	if(float_eq(vDest.x, 0.0f))
		vDest.x = 0.0f ;
	if(float_eq(vDest.y, 0.0f))
		vDest.y = 0.0f ;
	if(float_eq(vDest.z, 0.0f))
		vDest.z = 0.0f ;
}

void Quaternion::TwoVectorsToQuat(Vector3 &v1, Vector3 &v2)
{
    Vector3 vAxis ;
	float theta ;

	if( vector_eq(v1, v2) )// 같은 방향인 경우
	{
		AxisToQuat(vAxis, 0) ;
	}
	else
	{
		vAxis = v1.cross(v2) ;

		theta = acosf(v1.dot(v2)) ;

		AxisToQuat(vAxis, theta) ;
	}
}

Quaternion Quaternion::operator*(const Quaternion &q)
{
	double rx, ry, rz, rw;		// temp result

	rw	= q.w*w - q.x*x - q.y*y - q.z*z;

	rx	= q.w*x + q.x*w + q.y*z - q.z*y;
	ry	= q.w*y + q.y*w + q.z*x - q.x*z;
	rz	= q.w*z + q.z*w + q.x*y - q.y*x;

	return(Quaternion((float)rx, (float)ry, (float)rz, (float)rw));
}

Quaternion &Quaternion::operator=(const Quaternion &q)
{
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;

	return (*this) ;
}
void Quaternion::MatrixToQuat(const Matrix4 &mat)
{
	/*
	double trace = (double)(mat.m11 + mat.m22 + mat.m33) ;// removed +1.0f
	if(trace > 0)
	{
		double s = 0.5 / sqrt(trace+1.0) ;
		w = 0.25/s ;
		x = (mat.m32 - mat.m23)*s ;
		y = (mat.m13 - mat.m31)*s ;
		z = (mat.m21 - mat.m12)*s ;
	}
	else
	{
		if((mat.m11 > mat.m22) && (mat.m11 > mat.m33))
		{
			double s = 2.0*sqrt(1.0+mat.m11-mat.m22-mat.m33) ;
			double inv_s = 1.0/s ;
			w = (mat.m32 - mat.m23)*inv_s ;
			x = 0.25*s ;
			y = (mat.m12 + mat.m21)*inv_s ;
			z = (mat.m13 + mat.m31)*inv_s ;
		}
		else if(mat.m22 > mat.m33)
		{
			double s = 2.0*sqrt(1.0+mat.m22-mat.m11-mat.m33) ;
			double inv_s = 1.0/s ;
			w = (mat.m13 - mat.m31)*inv_s ;
			x = (mat.m12 + mat.m21)*inv_s ;
			y = 0.25*s ;
			z = (mat.m23 + mat.m32)*inv_s ;
		}
		else
		{
			double s = 2.0*sqrt(1.0+mat.m33-mat.m11-mat.m22) ;
			double inv_s = 1.0/s ;
			w = (mat.m21 - mat.m12)*inv_s ;
			x = (mat.m13 + mat.m31)*inv_s ;
			y = (mat.m23 + mat.m32)*inv_s ;
			z = 0.25*s ;
		}
	}
	*/
	double trace = (double)(mat.m11 + mat.m22 + mat.m33) ;// removed +1.0f
	if(trace > 0)
	{
		double s = 0.5 / sqrt(trace+1.0) ;
		w = 0.25/s ;
		x = (mat.m23 - mat.m32)*s ;
		y = (mat.m31 - mat.m13)*s ;
		z = (mat.m12 - mat.m21)*s ;
	}
	else
	{
		if((mat.m11 > mat.m22) && (mat.m11 > mat.m33))
		{
			double s = 2.0*sqrt(1.0+mat.m11-mat.m22-mat.m33) ;
			double inv_s = 1.0/s ;
			w = (mat.m23 - mat.m32)*inv_s ;
			x = 0.25*s ;
			y = (mat.m21 + mat.m12)*inv_s ;
			z = (mat.m31 + mat.m13)*inv_s ;
		}
		else if(mat.m22 > mat.m33)
		{
			double s = 2.0*sqrt(1.0+mat.m22-mat.m11-mat.m33) ;
			double inv_s = 1.0/s ;
			w = (mat.m31 - mat.m13)*inv_s ;
			x = (mat.m21 + mat.m12)*inv_s ;
			y = 0.25*s ;
			z = (mat.m32 + mat.m23)*inv_s ;
		}
		else
		{
			double s = 2.0*sqrt(1.0+mat.m33-mat.m11-mat.m22) ;
			double inv_s = 1.0/s ;
			w = (mat.m12 - mat.m21)*inv_s ;
			x = (mat.m31 + mat.m13)*inv_s ;
			y = (mat.m32 + mat.m23)*inv_s ;
			z = 0.25*s ;
		}
	}
}

float Quaternion::GetPitch()
{
	//return (float)(atan2(2.0*(y*z + w*x), w*w - x*x - y*y + z*z)) ;

	return (float)(atan2(2.0*(y*z + w*x), 1.0-2.0*(x*x + y*y))) ;
}
float Quaternion::GetYaw()
{
	double angle = (-2.0*(x*z - w*y)) ;
	if( float_abs(float_abs(angle)-1.0) <= 0.0001 )//부동소수점오차
		angle = (angle>0) ? 1.0 : -1.0 ;

	return (float)(asin(angle)) ;

	//return (float)(asin(-2.0*(x*z - w*y))) ;
}
float Quaternion::GetRoll()
{
	return (float)(atan2(2.0*(x*y + w*z), 1.0-2.0*(y*y + z*z))) ;
	//return (float)(atan2(2.0*(x*y + w*z), w*w + x*x - y*y - z*z)) ;
}
void Quaternion::GetPitchYawRoll(float &pitch, float &yaw, float &roll)
{
	double sqx = x*x ;
	double sqy = y*y ;
	double sqz = z*z ;
	double sqw = w*w ;

	double unit = sqx + sqy + sqz + sqw ;
	double test = x*y + z*w ;

	if(test > 0.499*unit)//singulartiy at north pole
	{
		roll = 2 * atan2(x, w) ;
		yaw = pi/2 ;
		pitch = 0 ;
		return ;
	}
	if(test < -0.499*unit)//singulartiy at south pole
	{
		roll = -2 * atan2(x, w) ;
		yaw = -pi/2 ;
		pitch = 0 ;
		return ;
	}

	yaw = atan2(2*y*w - 2*x*z, sqx - sqy - sqz + sqw) ;
	roll = asin(2*test/unit) ;
	pitch = atan2(2*x*w - 2*y*z, -sqx + sqy - sqz + sqw) ;
}
/*
void Quaternion::GetPitchYawRoll(float &pitch, float &yaw, float &roll)
{
	double sqx = x*x ;
	double sqy = y*y ;
	double sqz = z*z ;
	double sqw = w*w ;

	roll = atan2(2*(x*y + w*z), sqw+sqx - sqy-sqz) ;
	yaw = asin(-2*(x*z - w*y)) ;
	pitch = atan2(2*(w*x + y*z), sqw-sqx - sqy-sqz) ;

	//yaw = atan( (2*(x*y + w*z)) / (sqw+sqx - sqy-sqz) ) ;
	//pitch = asin(-2*(x*z - w*y)) ;
	//roll = atan( (2*(w*x + y*z)) / (sqw-sqx - sqy-sqz) ) ;

	double test = x*y + z*w ;

	Vector3 vRad, vDeg ;

	if(test > 0.499)//singulartiy at north pole
	{
		roll = 2 * atan2(x, w) ;
		yaw = pi/2 ;
		pitch = 0 ;
		return ;
	}
	if(test > -0.499)//singulartiy at south pole
	{
		roll = -2 * atan2(x, w) ;
		yaw = -pi/2 ;
		pitch = 0 ;
		return ;
	}

	double sqx = x*x ;
	double sqy = y*y ;
	double sqz = z*z ;

	roll = atan2(2*y*w - 2*x*z, 1-2*sqy-2*sqz) ;
	yaw = asin(2*test) ;
	pitch = atan2(2*x*w - 2*y*z, 1-2*sqx-2*sqz) ;

}
*/