#include "Matrix.h"
#include "Vector3.h"
#include "Vector4.h"


Matrix4::~Matrix4()
{
}

Matrix4::Matrix4()
{
	Identity() ;    
}

Matrix4::Matrix4(float _m11, float _m12, float _m13, float _m14, float _m21, float _m22, float _m23, float _m24,
		float _m31, float _m32, float _m33, float _m34, float _m41, float _m42, float _m43, float _m44)
{
	m11 = _m11 ;	m12 = _m12 ;	m13 = _m13 ;	m14 = _m14 ;
	m21 = _m21 ;	m22 = _m22 ;	m23 = _m23 ;	m24 = _m24 ;
	m31 = _m31 ;	m32 = _m32 ;	m33 = _m33 ;	m34 = _m34 ;
	m41 = _m41 ;	m42 = _m42 ;	m43 = _m43 ;	m44 = _m44 ;
}

Matrix4::Matrix4(const Matrix4 &mat)
{
	m11 = mat.m11 ;	m12 = mat.m12 ;	m13 = mat.m13 ;	m14 = mat.m14 ;
	m21 = mat.m21 ;	m22 = mat.m22 ;	m23 = mat.m23 ;	m24 = mat.m24 ;
	m31 = mat.m31 ;	m32 = mat.m32 ;	m33 = mat.m33 ;	m34 = mat.m34 ;
	m41 = mat.m41 ;	m42 = mat.m42 ;	m43 = mat.m43 ;	m44 = mat.m44 ;
}

double Matrix4::GetElement(unsigned long row, unsigned long column) const
{
	if(row == 1)
	{
		if(column == 1) return (double)m11 ;
		if(column == 2) return (double)m12 ;
		if(column == 3) return (double)m13 ;
		if(column == 4) return (double)m14 ;
	}
	if(row == 2)
	{
		if(column == 1) return (double)m21 ;
		if(column == 2) return (double)m22 ;
		if(column == 3) return (double)m23 ;
		if(column == 4) return (double)m24 ;
	}
	if(row == 3)
	{
		if(column == 1) return (double)m31 ;
		if(column == 2) return (double)m32 ;
		if(column == 3) return (double)m33 ;
		if(column == 4) return (double)m34 ;
	}
	if(row == 4)
	{
		if(column == 1) return (double)m41 ;
		if(column == 2) return (double)m42 ;
		if(column == 3) return (double)m43 ;
		if(column == 4) return (double)m44 ;
	}

	return (double)m11 ;
}

void Matrix4::SetElement(unsigned long row, unsigned long column, float fValue)
{
	if(row == 1)
	{
		if(column == 1) m11 = fValue ;
		if(column == 2) m12 = fValue ;
		if(column == 3) m13 = fValue ;
		if(column == 4) m14 = fValue ;
	}
	if(row == 2)
	{
		if(column == 1) m21 = fValue ;
		if(column == 2) m22 = fValue ;
		if(column == 3) m23 = fValue ;
		if(column == 4) m24 = fValue ;
	}
	if(row == 3)
	{
		if(column == 1) m31 = fValue ;
		if(column == 2) m32 = fValue ;
		if(column == 3) m33 = fValue ;
		if(column == 4) m34 = fValue ;
	}
	if(row == 4)
	{
		if(column == 1) m41 = fValue ;
		if(column == 2) m42 = fValue ;
		if(column == 3) m43 = fValue ;
		if(column == 4) m44 = fValue ;
	}
}

Matrix4 &Matrix4::operator =(const Matrix4 &mat)
{
	m11 = mat.m11 ;	m12 = mat.m12 ;	m13 = mat.m13 ;	m14 = mat.m14 ;
	m21 = mat.m21 ;	m22 = mat.m22 ;	m23 = mat.m23 ;	m24 = mat.m24 ;
	m31 = mat.m31 ;	m32 = mat.m32 ;	m33 = mat.m33 ;	m34 = mat.m34 ;
	m41 = mat.m41 ;	m42 = mat.m42 ;	m43 = mat.m43 ;	m44 = mat.m44 ;

	return *this ;
}

Matrix4 Matrix4::operator +(const Matrix4 &mat)
{
	Matrix4 matRet(
		m11+mat.m11, m12+mat.m12, m13+mat.m13, m14+mat.m14,
		m21+mat.m21, m22+mat.m22, m23+mat.m23, m24+mat.m24,
		m31+mat.m31, m32+mat.m32, m33+mat.m33, m34+mat.m34,
		m41+mat.m41, m42+mat.m42, m43+mat.m43, m44+mat.m44
		) ;

	return matRet ;
}

Matrix4 Matrix4::operator -(const Matrix4 &mat)
{
	Matrix4 matRet(
		m11-mat.m11, m12-mat.m12, m13-mat.m13, m14-mat.m14,
		m21-mat.m21, m22-mat.m22, m23-mat.m23, m24-mat.m24,
		m31-mat.m31, m32-mat.m32, m33-mat.m33, m34-mat.m34,
		m41-mat.m41, m42-mat.m42, m43-mat.m43, m44-mat.m44
		) ;

	return matRet ;
}

Matrix4 &Matrix4::operator +=(const Matrix4 &mat)
{
	m11 += mat.m11 ;	m12 += mat.m12 ;	m13 += mat.m13 ;	m14 += mat.m14 ;
	m21 += mat.m21 ;	m22 += mat.m22 ;	m23 += mat.m23 ;	m24 += mat.m24 ;
	m31 += mat.m31 ;	m32 += mat.m32 ;	m33 += mat.m33 ;	m34 += mat.m34 ;
	m41 += mat.m41 ;	m42 += mat.m42 ;	m43 += mat.m43 ;	m44 += mat.m44 ;

	return *this ;
}

Matrix4 &Matrix4::operator -=(const Matrix4 &mat)
{
	m11 -= mat.m11 ;	m12 -= mat.m12 ;	m13 -= mat.m13 ;	m14 -= mat.m14 ;
	m21 -= mat.m21 ;	m22 -= mat.m22 ;	m23 -= mat.m23 ;	m24 -= mat.m24 ;
	m31 -= mat.m31 ;	m32 -= mat.m32 ;	m33 -= mat.m33 ;	m34 -= mat.m34 ;
	m41 -= mat.m41 ;	m42 -= mat.m42 ;	m43 -= mat.m43 ;	m44 -= mat.m44 ;

	return *this ;
}

Matrix4 Matrix4::operator *(const float fScale)
{
	Matrix4 matRet(
		m11*fScale, m12*fScale, m13*fScale, m14*fScale,
		m21*fScale, m22*fScale, m23*fScale, m24*fScale,
		m31*fScale, m32*fScale, m33*fScale, m34*fScale,
		m41*fScale, m42*fScale, m43*fScale, m44*fScale
		) ;

    return matRet ;	
}

Matrix4 &Matrix4::operator *=(const float fScale)
{
	m11 *= fScale ;	m12 *= fScale ;	m13 *= fScale ;	m14 *= fScale ;
	m21 *= fScale ;	m22 *= fScale ;	m23 *= fScale ;	m24 *= fScale ;
	m31 *= fScale ;	m32 *= fScale ;	m33 *= fScale ;	m34 *= fScale ;
	m41 *= fScale ;	m42 *= fScale ;	m43 *= fScale ;	m44 *= fScale ;

	return *this ;
}

Matrix4 Matrix4::operator *(const Matrix4 &mat)
{
	Matrix4 matRet ;

	matRet.m11 = m11*mat.m11 + m12*mat.m21 + m13*mat.m31 + m14*mat.m41 ;
	matRet.m12 = m11*mat.m12 + m12*mat.m22 + m13*mat.m32 + m14*mat.m42 ;
	matRet.m13 = m11*mat.m13 + m12*mat.m23 + m13*mat.m33 + m14*mat.m43 ;
	matRet.m14 = m11*mat.m14 + m12*mat.m24 + m13*mat.m34 + m14*mat.m44 ;

	matRet.m21 = m21*mat.m11 + m22*mat.m21 + m23*mat.m31 + m24*mat.m41 ;
	matRet.m22 = m21*mat.m12 + m22*mat.m22 + m23*mat.m32 + m24*mat.m42 ;
	matRet.m23 = m21*mat.m13 + m22*mat.m23 + m23*mat.m33 + m24*mat.m43 ;
	matRet.m24 = m21*mat.m14 + m22*mat.m24 + m23*mat.m34 + m24*mat.m44 ;

	matRet.m31 = m31*mat.m11 + m32*mat.m21 + m33*mat.m31 + m34*mat.m41 ;
	matRet.m32 = m31*mat.m12 + m32*mat.m22 + m33*mat.m32 + m34*mat.m42 ;
	matRet.m33 = m31*mat.m13 + m32*mat.m23 + m33*mat.m33 + m34*mat.m43 ;
	matRet.m34 = m31*mat.m14 + m32*mat.m24 + m33*mat.m34 + m34*mat.m44 ;

	matRet.m41 = m41*mat.m11 + m42*mat.m21 + m43*mat.m31 + m44*mat.m41 ;
	matRet.m42 = m41*mat.m12 + m42*mat.m22 + m43*mat.m32 + m44*mat.m42 ;
	matRet.m43 = m41*mat.m13 + m42*mat.m23 + m43*mat.m33 + m44*mat.m43 ;
	matRet.m44 = m41*mat.m14 + m42*mat.m24 + m43*mat.m34 + m44*mat.m44 ;

    return matRet ;	
}

Matrix4 &Matrix4::operator *=(const Matrix4 &mat)
{
	Matrix4 matRet ;

	matRet.m11 = m11*mat.m11 + m12*mat.m21 + m13*mat.m31 + m14*mat.m41 ;
	matRet.m12 = m11*mat.m12 + m12*mat.m22 + m13*mat.m32 + m14*mat.m42 ;
	matRet.m13 = m11*mat.m13 + m12*mat.m23 + m13*mat.m33 + m14*mat.m43 ;
	matRet.m14 = m11*mat.m14 + m12*mat.m24 + m13*mat.m34 + m14*mat.m44 ;

	matRet.m21 = m21*mat.m11 + m22*mat.m21 + m23*mat.m31 + m24*mat.m41 ;
	matRet.m22 = m21*mat.m12 + m22*mat.m22 + m23*mat.m32 + m24*mat.m42 ;
	matRet.m23 = m21*mat.m13 + m22*mat.m23 + m23*mat.m33 + m24*mat.m43 ;
	matRet.m24 = m21*mat.m14 + m22*mat.m24 + m23*mat.m34 + m24*mat.m44 ;

	matRet.m31 = m31*mat.m11 + m32*mat.m21 + m33*mat.m31 + m34*mat.m41 ;
	matRet.m32 = m31*mat.m12 + m32*mat.m22 + m33*mat.m32 + m34*mat.m42 ;
	matRet.m33 = m31*mat.m13 + m32*mat.m23 + m33*mat.m33 + m34*mat.m43 ;
	matRet.m34 = m31*mat.m14 + m32*mat.m24 + m33*mat.m34 + m34*mat.m44 ;

	matRet.m41 = m41*mat.m11 + m42*mat.m21 + m43*mat.m31 + m44*mat.m41 ;
	matRet.m42 = m41*mat.m12 + m42*mat.m22 + m43*mat.m32 + m44*mat.m42 ;
	matRet.m43 = m41*mat.m13 + m42*mat.m23 + m43*mat.m33 + m44*mat.m43 ;
	matRet.m44 = m41*mat.m14 + m42*mat.m24 + m43*mat.m34 + m44*mat.m44 ;

    *this = matRet ;
	
	return *this ;
}

Vector4 Matrix4::operator *(const Vector4 &v)
{
	Vector4 vRet ;

	vRet.x = m11*v.x + m12*v.y + m13*v.z + m14*v.w ;
	vRet.y = m21*v.x + m22*v.y + m23*v.z + m24*v.w ;
	vRet.z = m31*v.x + m32*v.y + m33*v.z + m34*v.w ;
	vRet.w = m41*v.x + m42*v.y + m43*v.z + m44*v.w ;

	return vRet ;
}

void Matrix4::Set(float _m11, float _m12, float _m13, float _m14, float _m21, float _m22, float _m23, float _m24,
		float _m31, float _m32, float _m33, float _m34, float _m41, float _m42, float _m43, float _m44)
{
	m11 = _m11 ;	m12 = _m12 ;	m13 = _m13 ;	m14 = _m14 ;
	m21 = _m21 ;	m22 = _m22 ;	m23 = _m23 ;	m24 = _m24 ;
	m31 = _m31 ;	m32 = _m32 ;	m33 = _m33 ;	m34 = _m34 ;
	m41 = _m41 ;	m42 = _m42 ;	m43 = _m43 ;	m44 = _m44 ;
}

void Matrix4::Identity()
{
	m11 = m22 = m33 = m44 = 1.f ;

    m12 = m13 = m14 = 0.f ;
	m21 = m23 = m24 = 0.f ;
	m31 = m32 = m34 = 0.f ;
	m41 = m42 = m43 = 0.f ;
}

Matrix4 Matrix4::Transpose() const
{
	Matrix4 mat ;

	mat.m11 = m11 ;		mat.m21 = m12 ;		mat.m31 = m13 ;		mat.m41 = m14 ;
	mat.m12 = m21 ;		mat.m22 = m22 ;		mat.m32 = m23 ;		mat.m42 = m24 ;
	mat.m13 = m31 ;		mat.m23 = m32 ;		mat.m33 = m33 ;		mat.m43 = m34 ;
	mat.m14 = m41 ;		mat.m24 = m42 ;		mat.m34 = m43 ;		mat.m44 = m44 ;

	return mat ;
}

Matrix4 Matrix4::Inverse() const
{
	double m[4][8];
    int wi, wj, wk;
    double Pivot, wPivot, wf, wf2, wf3;
    int PivotRow=0;

	Matrix4 q ;

    for(wi=0 ; wi<4 ; wi++) {
        for(wj=0 ; wj<4 ; wj++) {
			m[wi][wj] = GetElement(wi+1, wj+1) ; // (*this)(wi+1, wj+1) ;
            m[wi][wj+4] = 0.0f;
        }
    }

    // Indentity Matrix
	for(wi=0 ; wi<4 ; wi++) {
        m[wi][wi+4] = 1.0f;
    }


    for(wk=0 ; wk<3 ; wk++) {
        Pivot = 0.0f;
        for(wj=wk ; wj<4 ; wj++) {
            if (fabs(Pivot) < fabs(m[wj][wk])) {
                Pivot = m[wj][wk];
                PivotRow = wj;
            }
        }
        if (wk != PivotRow) {
            for(int wi=0 ; wi<8 ; wi++) {
                wf = m[wk][wi];
                m[wk][wi] = m[PivotRow][wi];
                m[PivotRow][wi] = wf;
            }
        }

        for(wi=(wk+1) ; wi<4 ; wi++) {
            if (m[wi][wk]) {
                wPivot = -Pivot;
                wf = m[wi][wk];
                for(wj=0 ; wj<8 ; wj++) {
                    wf2 = wPivot * m[wi][wj];
                    wf3 = m[wk][wj] * wf;
                    m[wi][wj] = wf2 + wf3; //wPivot * m(wi, wj) + m(wk, wj) * wf;
                }
            }
        }
    }

    for(wk=3 ; wk>0 ; wk--) {
        for(wi=(wk-1) ; wi>=0 ; wi--) {
            Pivot = m[wk][wk];
            if (m[wi][wk]) {
                wPivot = -Pivot;
                wf = m[wi][wk];
                for(wj=0 ; wj<8 ; wj++) {
                    wf2 = wPivot * m[wi][wj];
                    wf3 = m[wk][wj] * wf;
                    m[wi][wj] = wf2 + wf3; //wPivot * m(wi, wj) + m(wk, wj) * wf;
                }
            }
        }
    }
        
    for(wi=0 ; wi<4 ; wi++) {
        wf = m[wi][wi];
        for(wj=0 ; wj<4 ; wj++) {
            m[wi][wj+4] /= wf;
			q.SetElement(wi+1, wj+1, (float)m[wi][wj+4]) ;
            //q(wi+1, wj+1) = (float)m[wi][wj+4];
        }
    }

	return q ;
}

void Matrix4::SetRotatebyAxisX(float ftheta, bool rad)
{
	Identity() ;

	if(!rad)
		ftheta = ftheta*3.141592f/180.f ;

	m22 = cosf(ftheta) ;
	m23 = sinf(ftheta) ;

	m32 = -sinf(ftheta) ;
	m33 = cosf(ftheta) ;
}

void Matrix4::SetRotatebyAxisY(float ftheta, bool rad)
{
	Identity() ;

	if(!rad)
		ftheta = ftheta*3.141592f/180.f ;

	m11 = cosf(ftheta) ;
	m13 = -sinf(ftheta) ;

	m31 = sinf(ftheta) ;
	m33 = cosf(ftheta) ;
}

void Matrix4::SetRotatebyAxisZ(float ftheta, bool rad)
{
	Identity() ;

	if(!rad)
		ftheta = ftheta*3.141592f/180.f ;

	m11 = cosf(ftheta) ;
	m12 = sinf(ftheta) ;

	m21 = -sinf(ftheta) ;
	m22 = cosf(ftheta) ;
}

void Matrix4::SetTranslation(const Vector3 &v)
{
	m41 += v.x ;
	m42 += v.y ;
	m43 += v.z ;
}

void Matrix4::SetTranslation(float x, float y, float z)
{
	m41 += x ;
	m42 += y ;
	m43 += z ;
}

float Matrix4::determinant()
{
	return (m11*(m22*m33 - m23*m32)-m12*(m21*m33 - m23*m31)+m13*(m21*m32 - m22*m31)) ;
}
Matrix4 Matrix4::DecomposeTranslation()
{
	Matrix4 matTrans ;
	matTrans.m41 = this->m41 ;
	matTrans.m42 = this->m42 ;
	matTrans.m43 = this->m43 ;
	return matTrans ;
}
Matrix4 Matrix4::DecomposeRotation()
{
	Matrix4 matRot, matScale ;//스케일값이 음수인경우는 제외. exception, if scale value is negative
	float fScaleX, fScaleY, fScaleZ ;
	Vector3 vAxisX(m11, m21, m31), vAxisY(m12, m22, m32), vAxisZ(m13, m23, m33) ;
	fScaleX = 1.0f/vAxisX.Magnitude() ;
	fScaleY = 1.0f/vAxisY.Magnitude() ;
	fScaleZ = 1.0f/vAxisZ.Magnitude() ;

	matRot = *this ;
	matRot.m41 = matRot.m42 = matRot.m43 = 0.0f ;
	matRot.m11 *= fScaleX ;
	matRot.m21 *= fScaleX ;
	matRot.m31 *= fScaleX ;

	matRot.m12 *= fScaleY ;
	matRot.m22 *= fScaleY ;
	matRot.m32 *= fScaleY ;

	matRot.m13 *= fScaleZ ;
	matRot.m23 *= fScaleZ ;
	matRot.m33 *= fScaleZ ;

	return matRot ;
}
Matrix4 Matrix4::DecomposeScaling()
{
	Matrix4 matScale ;//스케일값이 음수인경우는 제외. exception, if scale value is negative
	Vector3 vAxisX(m11, m21, m31), vAxisY(m12, m22, m32), vAxisZ(m13, m23, m33) ;
	matScale.m11 = vAxisX.Magnitude() ;
	matScale.m22 = vAxisY.Magnitude() ;
	matScale.m33 = vAxisZ.Magnitude() ;
    return matScale ;
}