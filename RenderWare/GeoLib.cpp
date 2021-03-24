#include "def.h"
#include "GeoLib.h"
#include "assert.h"
#include <float.h>

using namespace geo ;

SLine::SLine()
{
	d=0 ;
}

SLine::SLine(Vector3 _s, Vector3 _v, float _d)
{
	s = _s ;
	v = _v ;
	d = _d ;
}

void SLine::set(Vector3 _s, Vector3 _v, float _d)
{
	s = _s ;
	v = _v ;
	d = _d ;
}

Vector3 SLine::GetEndPos()
{
	Vector3 vEnd ;
	vEnd = s + (v*d) ;
	return vEnd ;
}
float SLine::GetDist(Vector3 &pos)
{
	Vector3 p = (pos-s) ;

	if(p.IsZero())
		return 0.0f ;

	if(vector_eq(v, p.Normalize()))
	{
		return p.Magnitude() ;

		//이게 도대체 머야? 씨발
		//float assesdontlie = v.GreatestElement(false) ;
		//if(!float_eq(assesdontlie, 0.0f))
		//	return (pos.x-s.x)/assesdontlie ;
	}
	return 0.0f ;
}
Vector3 SLine::GetPos(float t)//t=[0, 1]
{
	return (s + (v*t*d)) ;
}

SVertex::SVertex()
{
}

SVertex::SVertex(Vector3 _vPos, Vector3 _vNormal, Vector2 _vTex)
{
	vPos = _vPos ;
	vNormal = _vNormal ;
	vTex = _vTex ;
}

void SVertex::set(Vector3 _vPos, Vector3 _vNormal, Vector2 _vTex)
{
	vPos = _vPos ;
	vNormal = _vNormal ;
	vTex = _vTex ;
}

//#####################################################################//
//                          STriangle                                  //
//#####################################################################//

STriangle::STriangle()
{
	for(int i=0 ; i<3 ; i++)
		avVertex[i].set(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector2(0, 0)) ;
    bSelected = false ;
	dwAttr = 0 ;
}

STriangle::STriangle(Vector3 &v0, Vector3 &v1, Vector3 &v2)
{
	set(v0, v1, v2) ;
	bSelected = false ;
}

void STriangle::set(Vector3 &v0, Vector3 &v1, Vector3 &v2)
{
	avVertex[0].vPos = v0 ;
	avVertex[1].vPos = v1 ;
	avVertex[2].vPos = v2 ;

	sPlane.MakePlane(v0, v1, v2) ;
	dwAttr = 0 ;
}

void STriangle::MakeBoundingShpere()
{
	Vector3 vCenter = GetMidPos() ;

	float l, fLongest = (vCenter-avVertex[0].vPos).Magnitude() ;
	for(int i=1 ; i<3 ; i++)
	{
		l = (vCenter-avVertex[i].vPos).Magnitude() ;
		if(fLongest < l)
			fLongest = l ;
	}

	sBoundingSphere.vPos = vCenter ;
	sBoundingSphere.fRadius = fLongest ;
}

void STriangle::GetLine(int nNum, SLine *psLine)
{
	int nStart, nEnd ;

	if(nNum == 0)
	{
		nStart = 0 ;
		nEnd = 1 ;
	}
	else if(nNum == 1)
	{
		nStart = 1 ;
		nEnd = 2 ;
	}
	else if(nNum == 2)
	{
		nStart = 2 ;
		nEnd = 0 ;
	}
	else
		return ;

	psLine->s.x = avVertex[nStart].vPos.x ;
	psLine->s.y = avVertex[nStart].vPos.y ;
	psLine->s.z = avVertex[nStart].vPos.z ;

	Vector3 vDist, vNormal ;
	vDist = (avVertex[nEnd].vPos - avVertex[nStart].vPos) ;
	vNormal = vDist.Normalize() ;
	psLine->v.x = vNormal.x ;
	psLine->v.y = vNormal.y ;
	psLine->v.z = vNormal.z ;

	psLine->d = vDist.Magnitude() ;
}

Vector3 STriangle::GetNormalbyWeight(STriangleWeight *psWeight)
{
	Vector3 vNormal = (avVertex[0].vNormal*psWeight->afweight[0])+(avVertex[1].vNormal*psWeight->afweight[1])+(avVertex[2].vNormal*psWeight->afweight[2]) ;
	return vNormal.Normalize() ;
}

Vector3 STriangle::GetPosbyWeight(STriangleWeight *psWeight)
{
	return ( (avVertex[0].vPos*psWeight->afweight[0])+(avVertex[1].vPos*psWeight->afweight[1])+(avVertex[2].vPos*psWeight->afweight[2]) ) ;
}

Vector3 STriangle::GetMidPos()
{
	Vector3 vPos ;
	vPos.x = (avVertex[0].vPos.x+avVertex[1].vPos.x+avVertex[2].vPos.x)/3.0f ;
	vPos.y = (avVertex[0].vPos.y+avVertex[1].vPos.y+avVertex[2].vPos.y)/3.0f ;
	vPos.z = (avVertex[0].vPos.z+avVertex[1].vPos.z+avVertex[2].vPos.z)/3.0f ;
	return vPos ;
}

Vector2 STriangle::GetTexbyWeight(STriangleWeight *psWeight)
{
	return ( (avVertex[0].vTex*psWeight->afweight[0])+(avVertex[1].vTex*psWeight->afweight[1])+(avVertex[2].vTex*psWeight->afweight[2]) ) ;
}

SSphere *STriangle::GetBoundingSphere()
{
	return &sBoundingSphere ;
}

//#####################################################################//
//                             SPlane                                  //
//#####################################################################//

SPlane::SPlane()
{
	a = b = c = d = 0.0f ;
}

SPlane::SPlane(float _a, float _b, float _c, float _d)
{
	a = _a ;
	b = _b ;
	c = _c ;
	d = _d ;
}

SPlane::SPlane(Vector3 &vPos, Vector3 &vNormal)
{
	a = vNormal.x ;
	b = vNormal.y ;
	c = vNormal.z ;
	d = -(vNormal.dot(vPos)) ;
}

void SPlane::set(float _a, float _b, float _c, float _d)
{
	a = _a ;
	b = _b ;
	c = _c ;
	d = _d ;
}

void SPlane::set(Vector3 &vPos, Vector3 &vNormal)
{
	a = vNormal.x ;
	b = vNormal.y ;
	c = vNormal.z ;
	d = -(vNormal.dot(vPos)) ;
}

bool SPlane::MakePlane(Vector3 &_v1, Vector3 &_v2, Vector3 &_v3)
{
	//Vector3 vtemp, v1=(_v2 - _v1).Normalize(), v2=(_v3 - _v1).Normalize() ;
	Vector3 vtemp, v1=(_v2 - _v1), v2=(_v3 - _v1) ;

	if(float_less_eq(v1.Magnitude() , 0.0f) || float_less_eq(v2.Magnitude() , 0.0f))
		return false ;

	v1 = v1.Normalize() ;
	v2 = v2.Normalize() ;

	//D3DXVec3Cross(&vtemp, &v1, &v2) ;
	vtemp = v1.cross(v2) ;

	//D3DXVec3Normalize(&vtemp, &vtemp) ;
	//vtemp = vtemp.Normalize() ;

	a = vtemp.x ;
	b = vtemp.y ;
	c = vtemp.z ;
	//sPlane.d = -(D3DXVec3Dot(&avPos[0], &vtemp)) ;
	d = -(_v1.dot(vtemp)) ;

	return true ;
}

int SPlane::Classify(Vector3 &vPos) const
{
	Vector3 vNormal(a, b, c) ;
	float fSideValue = vNormal.dot(vPos) ;//현재좌표(원점에서부터)를 평면의 법선벡터에다가 반영했다고 생각하면 쉬울것이다.

	if(float_eq(fSideValue, -d))
		return SPLANE_COINCLIDING ;
	else if(float_less(fSideValue, -d))
		return SPLANE_BEHIND ;
	
	return SPLANE_INFRONT ;
}

float SPlane::ToPosition(Vector3 &vPos) const
{
	return (a*vPos.x)+(b*vPos.y)+(c*vPos.z)+d ;
}

Vector3 SPlane::GetOnPos(Vector3 &vPos)
{
	return (vPos - (GetNormal()*ToPosition(vPos))) ;
}

Vector3 SPlane::GetNormal() const
{
	return Vector3(a, b, c).Normalize() ;
}

SPlane SPlane::GetPlanebyTransform(Matrix4 *pmat) const
{
	SPlane plane ;
	Vector3 n, pos ;

	n = GetNormal() ;
	Vector4 v4(n.x, n.y, n.z, 0) ;
	v4 = (v4*(*pmat)) ;
	n = Vector3(v4.x, v4.y, v4.z).Normalize() ;
	pos = (GetNormal()*d)*(*pmat) ;

	plane.a = n.x ;
	plane.b = n.y ;
	plane.c = n.z ;
	plane.d = -n.dot(pos) ;
	return plane ;
}

SAACube::SAACube(Vector3 &vMinPos, Vector3 &vMaxPos)
{
	set(vMinPos, vMaxPos) ;
}

SAACube::SAACube(Vector3 &vCenterPos, float width, float height, float depth)
{
	set(vCenterPos, width, height, depth) ;
}

void SAACube::set(Vector3 &vMinPos, Vector3 &vMaxPos)
{
	vMin = vMinPos ;
	vMax = vMaxPos ;
	vCenter = vMin + ((vMax-vMin)*0.5f) ;
	fWidth = fHeight = fDepth = fabs(vMax.x-vMin.x) ;

	fWidth = fabs(vMax.x-vMin.x) ;
	fHeight = fabs(vMax.y-vMin.y) ;
	fDepth = fabs(vMax.z-vMin.z) ;
	fRadius = (vCenter-vMin).Magnitude() ;
	fDiagonal = sqrtf(fWidth*fWidth + fDepth*fDepth)/2.0f ;//Axis-Aligned 이기 때문에 높이값을 제외하고 계산하면 됨
}

void SAACube::set(Vector3 &vCenterPos, float width, float height, float depth)
{
	Vector3 vtemp(width/2.0f, height/2.0f, depth/2.0f) ;

	vMin = vCenterPos-vtemp ;
	vMax = vCenterPos+vtemp ;
	vCenter = vCenterPos ;

	fWidth = width ;
	fHeight = height ;
	fDepth = depth ;
	fRadius = (vCenter-vMin).Magnitude() ;
	fDiagonal = sqrtf(fWidth*fWidth + fDepth*fDepth)/2.0f ;//Axis-Aligned 이기 때문에 높이값을 제외하고 계산하면 됨
}

Vector3 SAACube::GetPos(int nPos) const
{
	Vector3 vPos ;

	if(nPos == SAACUBE_LLB)//Lower Left Bottom
		vPos = vMin ;
	else if(nPos == SAACUBE_LLT)//Lower Left Top
		vPos = Vector3(vMin.x, vMin.y, vMin.z+fDepth) ;
	else if(nPos == SAACUBE_LRT)//Lower Right Top
		vPos = Vector3(vMin.x+fWidth, vMin.y, vMin.z+fDepth) ;
	else if(nPos == SAACUBE_LRB)//Lower Right Bottom
		vPos = Vector3(vMin.x+fWidth, vMin.y, vMin.z) ;

	else if(nPos == SAACUBE_ULB)//Upper Left Bottom
		vPos = Vector3(vMax.x-fWidth, vMax.y, vMax.z-fDepth) ;
	else if(nPos == SAACUBE_ULT)//Upper Left Top
		vPos = Vector3(vMax.x-fWidth, vMax.y, vMax.z) ;
	else if(nPos == SAACUBE_URT)//Upper Right Top
		vPos = vMax ;
	else if(nPos == SAACUBE_URB)//Upper Right Bottom
		vPos = Vector3(vMax.x, vMax.y, vMax.z-fDepth) ;

    return vPos ;
}
/*
Vector3 SAACube::GetPos(int nPos)
{
	Vector3 vPos ;

	if(nPos == SAACUBE_LLB)//Lower Left Bottom
		vPos = vMin ;
	else if(nPos == SAACUBE_LLT)//Lower Left Top
		vPos = Vector3(vMin.x, vMin.y, vMin.z+fSideLength) ;
	else if(nPos == SAACUBE_LRT)//Lower Right Top
		vPos = Vector3(vMin.x+fSideLength, vMin.y, vMin.z+fSideLength) ;
	else if(nPos == SAACUBE_LRB)//Lower Right Bottom
		vPos = Vector3(vMin.x+fSideLength, vMin.y, vMin.z) ;

	else if(nPos == SAACUBE_ULB)//Upper Left Bottom
		vPos = Vector3(vMax.x-fSideLength, vMax.y, vMax.z-fSideLength) ;
	else if(nPos == SAACUBE_ULT)//Upper Left Top
		vPos = Vector3(vMax.x-fSideLength, vMax.y, vMax.z) ;
	else if(nPos == SAACUBE_URT)//Upper Right Top
		vPos = vMax ;
	else if(nPos == SAACUBE_URB)//Upper Right Bottom
		vPos = Vector3(vMax.x, vMax.y, vMax.z-fSideLength) ;

    return vPos ;
}
*/

SPlane SAACube::GetFace(int nFace) const
{
	SPlane sPlane ;

	if(nFace == SAACUBE_LEFTFACE)
	{
		//TRACE("pos[0]=(%10.7f %10.7f %10.7f) pos[1]=(%10.7f %10.7f %10.7f) pos[2]=(%10.7f %10.7f %10.7f)\r\n",
		//enumVector(GetPos(SAACUBE_LLB)), enumVector(GetPos(SAACUBE_LLT)), enumVector(GetPos(SAACUBE_ULT))) ;
		sPlane.MakePlane(GetPos(SAACUBE_LLB), GetPos(SAACUBE_LLT), GetPos(SAACUBE_ULT)) ;
	}
	else if(nFace == SAACUBE_RIGHTFACE)
	{
		//TRACE("pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f)\r\n",
		//enumVector(GetPos(SAACUBE_LRB)), enumVector(GetPos(SAACUBE_URB)), enumVector(GetPos(SAACUBE_URT))) ;
		sPlane.MakePlane(GetPos(SAACUBE_LRB), GetPos(SAACUBE_URB), GetPos(SAACUBE_URT)) ;
	}
	else if(nFace == SAACUBE_TOPFACE)
	{
		//TRACE("pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f)\r\n",
		//enumVector(GetPos(SAACUBE_ULB)), enumVector(GetPos(SAACUBE_ULT)), enumVector(GetPos(SAACUBE_URT))) ;
		sPlane.MakePlane(GetPos(SAACUBE_ULB), GetPos(SAACUBE_ULT), GetPos(SAACUBE_URT)) ;
	}
	else if(nFace == SAACUBE_BOTTOMFACE)
	{
		//TRACE("pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f)\r\n",
		//enumVector(GetPos(SAACUBE_LLB)), enumVector(GetPos(SAACUBE_LRB)), enumVector(GetPos(SAACUBE_LRT))) ;
		sPlane.MakePlane(GetPos(SAACUBE_LLB), GetPos(SAACUBE_LRB), GetPos(SAACUBE_LRT)) ;
	}
	else if(nFace == SAACUBE_FRONTFACE)
	{
		//TRACE("pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f)\r\n",
		//enumVector(GetPos(SAACUBE_LLB)), enumVector(GetPos(SAACUBE_ULB)), enumVector(GetPos(SAACUBE_URB))) ;
		sPlane.MakePlane(GetPos(SAACUBE_LLB), GetPos(SAACUBE_ULB), GetPos(SAACUBE_URB)) ;
	}
	else if(nFace == SAACUBE_BACKFACE)
	{
		//TRACE("pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f) pos[0]=(%10.7f %10.7f %10.7f)\r\n",
		//enumVector(GetPos(SAACUBE_LLT)), enumVector(GetPos(SAACUBE_LRT)), enumVector(GetPos(SAACUBE_URT))) ;
		sPlane.MakePlane(GetPos(SAACUBE_LLT), GetPos(SAACUBE_LRT), GetPos(SAACUBE_URT)) ;
	}

	return sPlane ;
}

void STransform::Reset()
{
	matTrans.Identity() ;
	matRot.Identity() ;
	matScale.Identity() ;
	matTransform.Identity() ;
	OriginQuat.Identity() ;
	ResultQuat.Identity() ;
    fPitch = fYaw = fRoll = 0.0f ;
}

void STransform::Translate(float x, float y, float z, bool bAdd)
{
	if(bAdd)
	{
		matTrans.m41 += x ;
		matTrans.m42 += y ;
		matTrans.m43 += z ;
	}
	else
	{
		matTrans.m41 = x ;
		matTrans.m42 = y ;
		matTrans.m43 = z ;
	}
}
void STransform::Rotate(float pitch, float yaw, float roll)
{
    fPitch = fmod(fPitch+pitch, 360) ;
	fYaw = fmod(fYaw+yaw, 360) ;
	fRoll = fmod(fRoll+roll, 360) ;

	Quaternion quat(fPitch*3.141592f/180.0f, fYaw*3.141592f/180.0f, fRoll*3.141592f/180.0f) ;
	ResultQuat = quat*OriginQuat ;
	ResultQuat.GetMatrix(matRot) ;
}
void STransform::Scale(float x, float y, float z)
{
	matScale.m11 *= x ;
	matScale.m22 *= y ;
	matScale.m33 *= z ;
}
Matrix4 *STransform::ComputeTransform()
{
	matTransform = matScale*matRot*matTrans ;
	return &matTransform ;
}
Matrix4 *STransform::GetTransform()
{
	return &matTransform ;
}
void STransform::SetTransform(Matrix4 *pmatTransform)
{
	matTransform = (*pmatTransform) ;
}

SCube::SCube()
{
	fSmallestX = fLargestX = fSmallestY = fLargestY = fSmallestZ = fLargestZ = 0.0f ;
}

//void SCube::set(Vector3 &vCenterPos, float width, float height, float depth, float pitch, float yaw, float roll)
//{
//	Vector3 vtemp(width/2.0f, height/2.0f, depth/2.0f) ;
//
//	vMin = vCenterPos-vtemp ;
//	vMax = vCenterPos+vtemp ;
//	vCenter = vCenterPos ;
//
//	fWidth = width ;
//	fHeight = height ;
//	fDepth = depth ;
//	fRadius = (vCenter-vMin).Magnitude() ;
//	fDiagonal = sqrtf(fWidth*fWidth + fDepth*fDepth)/2.0f ;//Axis-Aligned 이기 때문에 높이값을 제외하고 계산하면 됨
//
//	sTransform.Translate(vCenterPos.x, vCenterPos.y, vCenterPos.z) ;
//	sTransform.Rotate(pitch, yaw, roll) ;
//	Matrix4 *pmat = sTransform.ComputeTransform() ;
//
//	for(int i=0; i<8; i++)
//	{
//		vtemp = _GetPos(i) ;
//		vVertices[i] = vtemp*(*pmat) ;
//	}
//
//	//Matrix4 matRot ;
//	//OriginQuat.EulerToQuat(fPitch=deg_rad(pitch), fYaw=deg_rad(yaw), fRoll=deg_rad(roll)) ;
//	//OriginQuat.GetMatrix(matRot) ;
//
//	//for(int i=0; i<8; i++)
//	//{
//	//	vtemp = _GetPos(i) ;
//	//	vVertices[i] = vtemp*matRot ;
//	//}
//}

void SCube::set(Vector3 &center, Vector3 basis[3], Vector3 &min, Vector3 &max)
{
	vCenter = center ;

	Vector3 max_min = (max-min) ;

	fWidth = max_min.x ;
	fHeight = max_min.y ;
	fDepth = max_min.z ;

	vVertices[SCUBE_LLB] = center-(basis[0]*(fWidth/2.0f))-(basis[1]*(fHeight/2.0f))+(basis[2]*(fDepth/2.0f)) ;
	vVertices[SCUBE_LLT] = center-(basis[0]*(fWidth/2.0f))-(basis[1]*(fHeight/2.0f))-(basis[2]*(fDepth/2.0f)) ;
	vVertices[SCUBE_LRT] = center+(basis[0]*(fWidth/2.0f))-(basis[1]*(fHeight/2.0f))-(basis[2]*(fDepth/2.0f)) ;
	vVertices[SCUBE_LRB] = center+(basis[0]*(fWidth/2.0f))-(basis[1]*(fHeight/2.0f))+(basis[2]*(fDepth/2.0f)) ;

	vVertices[SCUBE_ULB] = center-(basis[0]*(fWidth/2.0f))+(basis[1]*(fHeight/2.0f))+(basis[2]*(fDepth/2.0f)) ;
	vVertices[SCUBE_ULT] = center-(basis[0]*(fWidth/2.0f))+(basis[1]*(fHeight/2.0f))-(basis[2]*(fDepth/2.0f)) ;
	vVertices[SCUBE_URT] = center+(basis[0]*(fWidth/2.0f))+(basis[1]*(fHeight/2.0f))-(basis[2]*(fDepth/2.0f)) ;
	vVertices[SCUBE_URB] = center+(basis[0]*(fWidth/2.0f))+(basis[1]*(fHeight/2.0f))+(basis[2]*(fDepth/2.0f)) ;

	nKind = geo::CUBE ;
}

void SCube::ComputeBoundary()
{
	fSmallestX = fLargestX = vVertices[0].x ;
	fSmallestY = fLargestY = vVertices[0].y ;
	fSmallestZ = fLargestZ = vVertices[0].z ;
	for(int i=1; i<8; i++)
	{
		if(fSmallestX > vVertices[i].x)
			fSmallestX = vVertices[i].x ;
		if(fSmallestY > vVertices[i].y)
			fSmallestY = vVertices[i].y ;
		if(fSmallestZ > vVertices[i].z)
			fSmallestZ = vVertices[i].z ;

		if(fLargestX < vVertices[i].x)
			fLargestX = vVertices[i].x ;
		if(fLargestY < vVertices[i].y)
			fLargestY = vVertices[i].y ;
		if(fLargestZ < vVertices[i].z)
			fLargestZ = vVertices[i].z ;
	}
}

Vector3 SCube::_GetPos(int nPos)
{
	Vector3 vPos ;

	if(nPos == SCUBE_LLB)//Lower Left Bottom
		vPos = vMin ;
	else if(nPos == SCUBE_LLT)//Lower Left Top
		vPos = Vector3(vMin.x, vMin.y, vMin.z+fDepth) ;
	else if(nPos == SCUBE_LRT)//Lower Right Top
		vPos = Vector3(vMin.x+fWidth, vMin.y, vMin.z+fDepth) ;
	else if(nPos == SCUBE_LRB)//Lower Right Bottom
		vPos = Vector3(vMin.x+fWidth, vMin.y, vMin.z) ;

	else if(nPos == SCUBE_ULB)//Upper Left Bottom
		vPos = Vector3(vMax.x-fWidth, vMax.y, vMax.z-fDepth) ;
	else if(nPos == SCUBE_ULT)//Upper Left Top
		vPos = Vector3(vMax.x-fWidth, vMax.y, vMax.z) ;
	else if(nPos == SCUBE_URT)//Upper Right Top
		vPos = vMax ;
	else if(nPos == SCUBE_URB)//Upper Right Bottom
		vPos = Vector3(vMax.x, vMax.y, vMax.z-fDepth) ;

    return vPos ;
}

SPlane SCube::GetFace(int nFace)
{
    SPlane sPlane ;

	if(nFace == SCUBE_LEFTFACE)
		sPlane.MakePlane(vVertices[SCUBE_LLB], vVertices[SCUBE_LLT], vVertices[SCUBE_ULT]) ;
	else if(nFace == SCUBE_RIGHTFACE)
		sPlane.MakePlane(vVertices[SCUBE_LRB], vVertices[SCUBE_URB], vVertices[SCUBE_URT]) ;
	else if(nFace == SCUBE_TOPFACE)
		sPlane.MakePlane(vVertices[SCUBE_ULB], vVertices[SCUBE_ULT], vVertices[SCUBE_URT]) ;
	else if(nFace == SCUBE_BOTTOMFACE)
		sPlane.MakePlane(vVertices[SCUBE_LLB], vVertices[SCUBE_LRB], vVertices[SCUBE_LRT]) ;
	else if(nFace == SCUBE_FRONTFACE)
		sPlane.MakePlane(vVertices[SCUBE_LLB], vVertices[SCUBE_ULB], vVertices[SCUBE_URB]) ;
	else if(nFace == SCUBE_BACKFACE)
		sPlane.MakePlane(vVertices[SCUBE_LLT], vVertices[SCUBE_LRT], vVertices[SCUBE_URT]) ;

	return sPlane ;
}

//################################################################################//
//                                    SCylinder                                   //
//################################################################################//

SCylinder::SCylinder(Vector3 start, Vector3 end, float radius)
{
	set(start, end, radius) ;
}
void SCylinder::set(Vector3 &start, Vector3 &end, float radius)
{
	vStart = start ;
	vEnd = end ;
	fRadius = radius ;

	sBoundingSphere.vPos = (vStart+vEnd)*0.5f ;

	float l = (sBoundingSphere.vPos-vStart).Magnitude() ;
	sBoundingSphere.fRadius = sqrtf((l*l) + (fRadius*fRadius)) ;
	dwAttr = 0 ;
}
int SCylinder::Intersect(geo::SLine &line, Vector3 &vIntersect, float precision)
{
	//1. 경계구 테스트
	if(!IntersectLineToSphere(&line, sBoundingSphere.vPos, sBoundingSphere.fRadius, precision))
		return INTERSECT_NONE ;

	//2. 시작평면, 끝평면과 각각 충돌테스트
	Vector3 vl, vn, p, vMove ;
	geo::SPlane plane ;

	/*
	//시작평면
	vn = (vStart-vEnd).Normalize() ;
	plane.set(vStart, vn) ;

	if(float_greater_eq(line.v.dot(plane.GetNormal()), 0.0f))
		return INTERSECT_NONE ;

	d = plane.ToPosition(line.s) ;
	if(float_eq(d, 0.0f))//라인의 시작점이 평면위에 있는경우
	{
		vIntersect = line.s ;
		return INTERSECT_POINT ;
	}
	else if(float_greater(d, 0.0f))//시작점의 바깥쪽에서 라인이 들어오는 경우
	{
		if(IntersectLinetoPlane(line, plane, vIntersect, false) == INTERSECT_POINT)//평면과 충돌하고
		{
			if((vIntersect-vStart).Magnitude() <= fRadius)//그 점이 반지름보다 작다면
				return INTERSECT_POINT ;
		}
		return INTERSECT_NONE ;
	}
	//끝평면
	vn = (vEnd-vStart).Normalize() ;
	plane.set(vEnd, vn) ;

	if(float_greater_eq(line.v.dot(plane.GetNormal()), 0.0f))
		return INTERSECT_NONE ;

	d = plane.ToPosition(line.s) ;
	if(float_eq(d, 0.0f))//라인의 시작점이 평면위에 있는경우
	{
		vIntersect = line.s ;
		return INTERSECT_POINT ;
	}
	else if(float_greater(d, 0.0f))//끝점의 바깥쪽에서 라인이 들어오는 경우
	{
		//TRACE("line s(%07.03f %07.03f %07.03f)\r\n", enumVector(line.s)) ;
		//TRACE("line end(%07.03f %07.03f %07.03f)\r\n", enumVector(line.GetEndPos())) ;

		if(IntersectLinetoPlane(line, plane, vIntersect, false) == INTERSECT_POINT)//평면과 충돌하고
		{
			if((vIntersect-vEnd).Magnitude() <= fRadius)//그 점이 반지름보다 작다면
				return INTERSECT_POINT ;
		}
		return INTERSECT_NONE ;
	}
	*/

	//3. 라인의 시작점과 끝점이 시작평면과 끝평면 사이에 존재하는 경우

	Vector3 start, end ;
	start = vStart-vStart ;
	end = vEnd-vStart ;

	Vector3 A, s0 ;
	A = end-start ;

	float a, b, c ;
	float q, AA ;

	s0 = line.s-vStart ;
	AA = A.dot(A) ;

	q = line.v.dot(A) ;
	a = line.v.dot(line.v) - (q*q)/AA ;
	b = s0.dot(line.v) - (s0.dot(A)*line.v.dot(A))/AA ;

	q = s0.dot(A) ;
	c = s0.dot(s0)-(fRadius*fRadius)-(q*q)/AA ;

	float D = ((b*b)-(a*c))/4.0f ;

	if(float_abs(D) <= precision)
		D = 0.0f ;

	if(D < 0.0f)
	//if( (D < 0.0f) && !(float_abs(D) <= precision) )
	//if(float_less(D, 0.0f))
		return INTERSECT_NONE ;
	//else if(float_eq(D, 0.0f))//방정식의 근이 0인경우 마지막으로 원기둥표면과 접하는지 검사
	//{
	//	//시작점에서의 평면판정
	//	Vector3 vl, vn, p, vMove ;
	//	vn = (vStart-vEnd).Normalize() ;
	//	geo::SPlane plane(vStart, vn) ;

	//	vl = line.v*line.d ;
	//	Vector3 va = ProjectionVector(vl, vn, false) ;
	//	Vector3 vb = vl-va ;

	//	vMove = ((-plane.GetNormal())*plane.ToPosition(line.s)) ;

	//	p = line.s+vMove ;
	//	geo::SLine sProjLine(p, vb.Normalize(), vb.Magnitude()) ;
	//	d = IntersectLinetoPoint(sProjLine, vStart, &vIntersect) ;
	//	if(float_less_eq(d, fRadius))
	//	{
	//		vIntersect -= vMove ;
	//		return INTERSECT_POINT ;
	//	}
	//}
	else
	{
		D = (b*b)-(a*c) ;
		if(float_abs(D) <= precision)
			D = 0.0f ;

		float t = (-b - sqrtf(D))/a ;
		//TRACE("intersect cylinder D=%g t=%g\r\n", D, t) ;

		//if(float_less(t, 0.0f) || float_greater(t, line.d))
		if( ((t<0.0f) && !(float_abs(t)<=precision)) || ((t>line.d) && !(float_abs(t-line.d)<=precision)) )
			return INTERSECT_NONE ;

		vIntersect = line.GetPos(t/line.d) ;

		float pe = (vIntersect-vStart).dot(A) ;
		if( (pe<0.0f) && !(float_abs(pe)<=precision) )
			return INTERSECT_NONE ;

		//if(float_less(pe, 0.0f) || float_greater(pe, AA))
		if( ((pe<0.0f) && !(float_abs(pe)<=precision)) || ((pe>AA) && !(float_abs(pe-AA)<=precision)) )
			return INTERSECT_NONE ;

		//TRACE("cylinder intersect (%07.03f %07.03f %07.03f)\r\n", enumVector(vIntersect)) ;

		return INTERSECT_POINT ;
	}
	return INTERSECT_NONE ;
}

//#######################################################################################################################################
// struct SMovement
//#######################################################################################################################################

void SMovement::set(Vector3 &start, Vector3 &end, float time_limit, float time_stay)
{
	vStart = start ;
	vEnd = end ;
	vPos = vStart ;

	Vector3 v = vEnd-vStart ;
	sPath.set(vStart, v.Normalize(), v.Magnitude()) ;

	fTimeLimit = time_limit ;
	fTimeStay = time_stay ;
	fTimeRatio = 1.0f/fTimeLimit ;
	fDistRatio = 1.0f/sPath.d ;
	fElapsedTime = 0.0f ;
	nActionNode = 0 ;
	nCurAction = ACTION_IDLE ;
	pFunction = NULL ;
}
void SMovement::setActions(int *pnActions, int nNum)
{
	if(nNum > 30)
		nNum = 30 ;

	nNumAction = nNum ;
	memcpy(anActions, pnActions, nNumAction*sizeof(int)) ;
	setAction(anActions[nActionNode++]) ;
}
void SMovement::_NextAction()
{
	if(nActionNode >= nNumAction)
	{
		if(nAttr & ATTR_LOOP)
		{
			nActionNode = 0 ;
			setAction(anActions[nActionNode++]) ;
		}
		else
			setAction(ACTION_FINISH) ;
	}
	else
		setAction(anActions[nActionNode++]) ;
}
Vector3 SMovement::getPos()
{
	return vPos ;
}
int SMovement::getCurAction()
{
	return nCurAction ;
}
void SMovement::addAttr(int attr, bool enable)
{
	if(enable)
		nAttr |= attr ;
	else
		nAttr &= (~attr) ;
}
int SMovement::getAttr()
{
	return nAttr ;
}
void SMovement::setAction(int action)
{
	if(action == ACTION_IDLE)
	{
	}
	else if(action == ACTION_GO)
	{
		Vector3 v = vEnd-vStart ;
		sPath.set(vStart, v.Normalize(), v.Magnitude()) ;
	}
	else if(action == ACTION_STAY)
	{
	}
	else if(action == ACTION_BACK)
	{
		Vector3 v = vStart-vEnd ;
		sPath.set(vEnd, v.Normalize(), v.Magnitude()) ;
	}
	else if(action == ACTION_FINISH)
	{
	}
	fElapsedTime = 0.0f ;
	nCurAction = action ;
}
void SMovement::process(float time)
{
	if(nCurAction == ACTION_FINISH)
		return ;

	if(nCurAction == ACTION_IDLE)
	{
	}
	else if(nCurAction == ACTION_GO)
	{
		fElapsedTime += time ;

		float t = fElapsedTime * fTimeRatio ;
		float d = pFunction(t) ;
		vPos = sPath.GetPos(d) ;

		if((fElapsedTime >= fTimeLimit))
		{
			vPos = vEnd ;
			_NextAction() ;
		}
	}
	else if(nCurAction == ACTION_STAY)
	{
		fElapsedTime += time ;
		if(fElapsedTime >= fTimeStay)
			_NextAction() ;
	}
	else if(nCurAction == ACTION_BACK)
	{
		fElapsedTime += time ;

		float t = fElapsedTime * fTimeRatio ;
		float d = pFunction(t) ;
		vPos = sPath.GetPos(d) ;

		if((fElapsedTime >= fTimeLimit))
		{
			vPos = vStart ;
			_NextAction() ;
		}
	}
}
void SMovement::setFunction(float (*pfunc)(float))
{
	pFunction = pfunc ;
}

bool IntesectXZProjTriangleToPoint(geo::STriangle &sTriangle, Vector3 vPos, float &u, float &v)
{
	float afMatrix[2][3] ;
	float afResult[2] ;

	//Test Point to Triangle Plane


	//t(x, y) = (1-u-v)V0 + uV1 + vV2
	//t(x, y) = V0 + u(V1-V0) + v(V2-V0)
	//x-V0.x = u(V1.x-V0.x) + v(V2.x-V0.x)
	//y-V0.y = u(V1.y-V0.y) + v(V2.y-V0.y)
	//삼각형 무게중심의 공식을 이용해서 연립일차방정식을 만든다.
	//afMatrix[0][0] = sTriangle.avPos[1].x - sTriangle.avPos[0].x ;
	//afMatrix[0][1] = sTriangle.avPos[2].x - sTriangle.avPos[0].x ;
	//afMatrix[0][2] = vPos.x - sTriangle.avPos[0].x ;
	afMatrix[0][0] = sTriangle.avVertex[1].vPos.x - sTriangle.avVertex[0].vPos.x ;
	afMatrix[0][1] = sTriangle.avVertex[2].vPos.x - sTriangle.avVertex[0].vPos.x ;
	afMatrix[0][2] = vPos.x - sTriangle.avVertex[0].vPos.x ;

	//afMatrix[1][0] = sTriangle.avPos[1].z - sTriangle.avPos[0].z ;
	//afMatrix[1][1] = sTriangle.avPos[2].z - sTriangle.avPos[0].z ;
	//afMatrix[1][2] = vPos.z - sTriangle.avPos[0].z ;
	afMatrix[1][0] = sTriangle.avVertex[1].vPos.z - sTriangle.avVertex[0].vPos.z ;
	afMatrix[1][1] = sTriangle.avVertex[2].vPos.z - sTriangle.avVertex[0].vPos.z ;
	afMatrix[1][2] = vPos.z - sTriangle.avVertex[0].vPos.z ;

	SolveLinearSystem((float *)afMatrix[0], 2, 3, 2, afResult) ;

	if( (afResult[0] >= 0.0f && afResult[0] <= 1.0f) && (afResult[1] >= 0.0f && afResult[1] <= 1.0f) && ((afResult[0]+afResult[1]) <= 1.0f) )
	{
		u = afResult[0] ;
		v = afResult[1] ;
		return true ;
	}

    return false ;
}

void SolveLinearSystem(float *pfInputMatrix, int nNumColumn, int nNumRow, int nNumVariable, float *pfResult)//using Guass-Jordan Elimination
{
	int i, n, t ;
	int nCol ;
	float afMatrix[3][4], afRow[4] ;
	float var0, var1 ;

	if((nNumColumn > 3) || (nNumRow > 4))
	{
		assert(false && "Too Large Column or Row in SolveLinearSystem") ;
		return ;
	}

	////Dynamic Allocate Double Arrays
	//ppfMatrix = (float **)( new float*[nNumColumn] ) ;
	//for(i=0 ; i<nNumColumn ; i++)
	//	ppfMatrix[i] = new float[nNumRow] ;

	//ex) 4x4 Matrix
	//[0][0], [0][1], [0][2], [0][3]
	//[1][0], [1][1], [1][2], [1][3]
	//[2][0], [2][1], [2][2], [2][3]
	//[3][0], [3][1], [3][2], [3][3]

	for(i=0 ; i<nNumColumn ; i++)
	{
		for(n=0 ; n<nNumRow ; n++)
			afMatrix[i][n] = pfInputMatrix[(i*nNumRow)+n] ;
	}

	//TRACE("Linear System\r\n") ;
	//for(i=0 ; i<nNumColumn ; i++)
	//{
	//	TRACE("|") ;
	//	for(n=0 ; n<nNumRow ; n++)
	//	{
	//		if(n == nNumRow-1)
	//			TRACE("|") ;

	//		TRACE("%+5.2f ", afMatrix[i][n]) ;
	//	}
	//	TRACE("|\r\n") ;
	//}
	//TRACE("\r\n") ;

	//Find Large Number in Current Column
	var0 = afMatrix[0][0] ;
	nCol = 0 ;
	for(i=0 ; i<nNumColumn ; i++)
	{
		if(var0 < afMatrix[i][0] && !float_eq(afMatrix[i][0], 0.0f))
		{
			var0 = afMatrix[i][0] ;
			nCol = i ;
		}
	}

	//Swaping Current Row with Large Number Row
	if(nCol != 0)
	{
		for(i=0 ; i<nNumRow ; i++)
		{
			afRow[i] = afMatrix[nCol][i] ;
			afMatrix[nCol][i] = afMatrix[0][i] ;
			afMatrix[0][i] = afRow[i] ;
		}
	}

	for(n=0 ; n<nNumVariable ; n++)
	{
		//[n][0] Value is make '1'
		assert(!float_eq(afMatrix[n][n], 0.0f)) ;// afMatrix[n][n] is not Zero

		var0 = float_round(1.0f/afMatrix[n][n]) ;
		for(i=0 ; i<nNumRow ; i++)
			afMatrix[n][i] *= var0 ;

		for(i=0 ; i<nNumColumn ; i++)
		{
            if(i == n)
				continue ;

			var0 = afMatrix[n][n] * (-afMatrix[i][n]) ;
            for(t=n ; t<nNumRow ; t++)
			{
                var1 = var0 * afMatrix[n][t] ;
				afMatrix[i][t] += var1 ;
			}
		}
	}

	for(i=0 ; i<nNumColumn ; i++)
		pfResult[i] = afMatrix[i][nNumRow-1] ;

	//for(i=0 ; i<nNumColumn ; i++)
	//{
	//	TRACE("|") ;
	//	for(n=0 ; n<nNumRow ; n++)
	//	{
	//		if(n == nNumRow-1)
	//			TRACE("|") ;

	//		TRACE("%+5.2f ", afMatrix[i][n]) ;
	//	}
	//	TRACE("|\r\n") ;
	//}
}

int CloseToExp(float fInput, float fExponent, bool bReverse)
{
	if(fInput > 0.0f && fInput <= 1.0f)
		return 0 ;

    float fResult = log(fabs(fInput))/log(fExponent) ;

	int nResult = (int)fResult ;

	float fEpsilon = fabs(fResult - (float)((int)fResult)) ;

	if(float_eq(fEpsilon, 0.0f))
		return (int)pow(fExponent, nResult) ;

	if(bReverse)
	{
		nResult = (int)pow(fExponent, nResult+1) ;
	}
	else
        nResult = (int)pow(fExponent, nResult+1) ;

	if(fInput < 0.0f)
		return -nResult ;

	return nResult ;
}

//float IntersectLinetoPoint(geo::SLine &line, Vector3 &vPos, Vector3 &vIntersected)
//{
//	Vector3 qs = vPos-line.s ;
//	Vector3 proj = qs.Projection((line.v*line.d)) ;
//	float t = proj.Magnitude() ;
//	if((t<0.0f) || float_greater(t, line.d))
//		return -1 ;
//
//	float d = sqrtf( qs.dot(qs) - (t*t) ) ;
//
//	vIntersected = line.s + (line.v*t) ;
//	return d ;
//}

float IntersectLinetoPoint(geo::SLine &line, Vector3 &vPos, Vector3 *pvIntersected)
{
	Vector3 qs = vPos-line.s ;
	Vector3 v = line.v*line.d ;
	float p = qs.dot(v) ;
	//float t = p*float_round(p/v.dot(v)) ;
	float t = (p*p)/v.dot(v) ;

	float d = sqrt(qs.dot(qs) - t) ;

    if(pvIntersected)
		(*pvIntersected) = line.s + (line.v*sqrt(t)) ;
    return d ;
}

int IntersectLineToLine(geo::SLine *psLine1, geo::SLine *psLine2, Vector3 *pvPos)
{
	Vector3 v1, v2, vZero(0, 0, 0) ;
	v1 = (psLine2->s-psLine1->s) ;
	v2 = (psLine1->v-psLine2->v) ;

	if(vector_eq(v1, vZero) && vector_eq(v2, vZero))
	{
		TRACE("infinite\r\n") ;
		return geo::INTERSECT_INFINITE ;
	}

	if(float_less(v1.Magnitude(), 0.0f) || float_greater(v1.Magnitude(), psLine1->d*v2.Magnitude()))
	{
		TRACE("none\r\n") ;
		return geo::INTERSECT_NONE ;
	}

	float t = v1.Magnitude()/v2.Magnitude() ;

	TRACE("t=%g\r\n", t) ;

	if(float_less(t, 0.0f) || float_greater(t, psLine1->d) )// || float_greater(t, psLine2->d))
		return geo::INTERSECT_NONE ;
	
	(*pvPos) = psLine1->s+psLine1->v*t ;
    
	return geo::INTERSECT_POINT ;
}

float DistLinetoLine(geo::SLine *psLine1, geo::SLine *psLine2)
{
	//"3D게임 프로그래밍 & 컴퓨터그래픽을 위한 수학 제2판" 139p 참조
	//P1(t1) = S1 + t1V1
	//P2(t2) = S2 + t2V2

	//F(t1, t2) = |P1(t1)-P2(t2)|^2 함수F(t1, t2) ---식1 는 P1(t1)과 P2(t2)의 거리에 대한 함수이다.

	//위 F함수를 t1, t2에 대해 각각 미분해서 최소값을 찾는다.
	//식1 을 전개하고
	//t1에 대해 편미분하면 F'(t1) = 2t1V1^2 + 2S1V1 - 2V1S2 - 2t2V1V2 = 0 ---식2
	//t2에 대해 편미분하면 F'(t2) = 2t2V2^2 + 2S2V2 - 2V2S1 - 2t1V1V2 = 0 ---식3

	//위의 식2,식3 를 행렬의 형태로 표현하면
	// | V1^2  -V1V2 |   | t1 |    | (S2-S1)V1 |
	// |             | * |    |  = |           |
	// | V1V2  -V2^2 |   | t2 |    | (S2-S1)V2 | 식3 에다가는 각항에 -1을 곱함.

	// | t1 |   | V1^2  -V1V2 |-1  | (S2-S1)V1 |
	// |    | = |             |  * |           |
	// | t2 |   | V1V2  -V2^2 |    | (S2-S1)V2 |

	//              1            | -V2^2  V1V2 |   | (S2-S1)V1 |   | t1 |
	// ----------------------- * |             | * |           | = |    |
	//  (V1V2)^2 - (V1^2V2^2)   | -V1V2  V1^2 |   | (S2-S1)V2 |   | t2 |

	float V1pow = psLine1->v.dot(psLine1->v), V2pow= psLine2->v.dot(psLine2->v) ;
	//float V1pow = 1.0f, V2pow = 1.0f ;
	float V1V2 = psLine1->v.dot(psLine2->v) ;
	float S2S1V1 = (psLine2->s-psLine1->s).dot(psLine1->v) ;
	float S2S1V2 = (psLine2->s-psLine1->s).dot(psLine2->v) ;
	float dist ;

	if((float_eq((V1V2*V1V2)-(V1pow*V2pow), 0.0f)))//이면 두 선이 평행하다.
	{
		dist = IntersectLinetoPoint(*psLine2, psLine1->s) ;
		return dist ;
	}

	float det = ((V1V2*V1V2) - (V1pow*V2pow)) ;

	float t1 = (-V2pow*S2S1V1 + V1V2*S2S1V2)/det ;
	float t2 = (-V1V2*S2S1V1 + V1pow*S2S1V2)/det ;

	if(float_less(t1, 0.0f))
		t1 = 0 ;
	else if(float_greater(t1, psLine1->d))
		t1 = psLine1->d ;

	if(float_less(t2, 0.0f))
		t2 = 0 ;
	else if(float_greater(t2, psLine2->d))
		t2 = psLine2->d ;

	Vector3 v1 = psLine1->s + (psLine1->v*t1) ;
	Vector3 v2 = psLine2->s + (psLine2->v*t2) ;

	dist = (v1-v2).Magnitude() ;

    return dist ;
}

bool IntersectLinetoPlane(Vector3 &v1, Vector3 &v2, geo::SPlane &plane, Vector3 &vOut, float &t, float precision)
{
	//ax+by+cz+d=0
	//vStart + vNormal*t = vPositon
	//a*(vStart.x + vNormal.x*t) + b*(vStart.y + vNormal.y*t) + c*(vStart.z + vNormal.z*t) + d = 0
	//a*vStart.x + b*vStart.y + c*vStart.z + a*(vNormal.x*t) + b*(vNormal.y*t) + c*(vNormal.z*t) + d = 0
	//a*vStart.x + b*vStart.y + c*vStart.z + t*(a*vNormal.x + b*vNormal.y + c*vNormal.z) + d = 0
	//t = -((a*vStart.x + b*vStart.y + c*vStart.z + d)/((a*vNormal.x + b*vNormal.y + c*vNormal.z)))

    Vector3 v = v2-v1 ;
	t = -((plane.a*v1.x + plane.b*v1.y + plane.c*v1.z + plane.d)/(plane.a*v.x + plane.b*v.y + plane.c*v.z)) ;
	//if(t >= 0.0f && t <= 1.0f)
	//if(float_greater_eq(t, 0.0f) && float_less_eq(t, 1.0f))
	if( ((t>0.0f) || (float_abs(t)<=precision)) && ((t<1.0f) || (float_abs(t-1.0f)<=precision)) )
	{
		vOut.x = v1.x+v.x*t ;
		vOut.y = v1.y+v.y*t ;
		vOut.z = v1.z+v.z*t ;
        return true ;
	}
	return false ;
}

int IntersectLinetoPlane(geo::SLine &line, geo::SPlane &plane, Vector3 &vPos, bool bCullingTest, bool bDirectional, float precision)
{
	//N*P(t)+D=0          P(t)에다가 직선의 방정식 S+tV 를 대입하면
	//N*S+(N*V)t+D=0      이것을 t에 대해서 풀면
	//t=-(N*S+D)/(N*V)    가 된다.

	//N*V 가 0 이 되면 평면과 직선은 평행이고,
	//이때 N*S+D=0 이 되면 직선은 평면에 있는것이다. 따라서 교점이 무한히 많게 되고 그게 아니면 교점은 없다.
	//위의 식을 4D벡터로 표현 하면 단, L=(N,D) 이다.
	//t=L*S/L*V

	Vector4 L(plane.a, plane.b, plane.c, plane.d) ;
	float a = L.dot(Vector4(line.s.x, line.s.y, line.s.z, 1)) ;
	float b = L.dot(Vector4(line.v.x, line.v.y, line.v.z, 0)) ;

	float abs_a = float_abs(a) ;

	//if(float_eq(b, 0.0f))
	if(float_abs(b) < precision)
	{
		//if(float_eq(a, 0.0f))
		if(abs_a <= precision)
			return geo::INTERSECT_INFINITE ;
		else
			return geo::INTERSECT_NONE ;
	}

  //  float t = -a/b ;
  //  if(t < 0.0f || t > line.d)
		//return geo::INTERSECT_NONE ;

	if(bCullingTest)
	{
		//선분의 시작점이 평면의 뒤인지 아닌지 테스트
		//if(!float_greater_eq(a, 0.0f))
		if( (a<0.0f) && (abs_a<=precision) )
			return geo::INTERSECT_NONE ;
	}
	if(bDirectional)
	{
		//선분의 시작점이 평면에 붙은 상태가 아니고 선분의 방향이 평면의 노말방향과 같을때
		float directional = line.v.dot(plane.GetNormal()) ;
		if( (directional>0.0f) || (float_abs(directional)<=precision) )
			return geo::INTERSECT_NONE ;

		//if((!float_eq(a, 0.0f)) && (!float_less(a, -b*line.d)))
		//float c = -b*line.d ;
		//if( !(abs_a<=precision) && ((abs_a>c) || (float_abs(abs_a-c)<=precision)) )
		//	return geo::INTERSECT_NONE ;
	}

	//float t = -float_round(a/b) ;
	float t = -a/b ;
	if( ((t<0.0f) && !(float_abs(t)<=precision)) || ((t>line.d) && !(float_abs(t-line.d)<=precision)) )
			return geo::INTERSECT_NONE ;

	vPos.set(line.s.x+(line.v.x*t), line.s.y+(line.v.y*t), line.s.z+(line.v.z*t)) ;

	return geo::INTERSECT_POINT ;
}

int IntersectLinetoPlane(geo::SLine &line, geo::SPlane &plane, Vector3 &vPos, float &t, bool bCullingTest, float precision)
{
	//N*P(t)+D=0          P(t)에다가 직선의 방정식 S+tV 를 대입하면
	//N*S+(N*V)t+D=0      이것을 t에 대해서 풀면
	//t=-(N*S+D)/(N*V)    가 된다.

	//N*V 가 0 이 되면 평면과 직선은 평행이고,
	//이때 N*S+D=0 이 되면 직선은 평면에 있는것이다. 따라서 교점이 무한히 많게 되고 그게 아니면 교점은 없다.
	//위의 식을 4D벡터로 표현 하면 단, L=(N,D) 이다.
	//t=L*S/L*V

	Vector4 L(plane.a, plane.b, plane.c, plane.d) ;
	float a = L.dot(Vector4(line.s.x, line.s.y, line.s.z, 1)) ;
	float b = L.dot(Vector4(line.v.x, line.v.y, line.v.z, 0)) ;

	//if(float_eq(b, 0.0f))
	if(float_abs(b) < precision)
	{
		//if(float_eq(a, 0.0f))
		if(float_abs(a) <= precision)
			return geo::INTERSECT_INFINITE ;
		else
			return geo::INTERSECT_NONE ;
	}

    //t = -float_round(a/b) ;
	t = -a/b ;
	if(bCullingTest)
	{
		//if(t < 0.0f || t > line.d)
		//if(float_less(t, 0.0f) || float_greater(t, line.d))
		if( ((t<0.0f) && !(float_abs(t)<=precision)) || ((t>line.d) && !(float_abs(t-line.d)<=precision)) )
			return geo::INTERSECT_NONE ;
	}

	vPos.set(line.s.x+(line.v.x*t), line.s.y+(line.v.y*t), line.s.z+(line.v.z*t)) ;

	return geo::INTERSECT_POINT ;
}

bool TriangleIntersectLinearSystem(Vector3 &r, Vector3 &q1, Vector3 &q2, Vector3 &vBaryCenter, float precision)
{
	// r = u*q1 + v*q2    이렇게 쓸수 있고, 양변에 q1, q2 로 내적해서 방정식 두개를 만들어낸다.

	// r*q1 = u*(q1*q1) + v*(q1*q2)
	// r*q2 = u*(q1*q2) + v*(q2*q2)    이제 이 두 연립방정식을 행렬로 풀어낸다.

	// | q1*q1   q1*q2 |   | u |    | r*q1 |
	// |               | * |   |  = |      |
	// | q1*q2   q2*q2 |   | v |    | r*q2 |

	float q1pow, q2pow, q1q2,  rq1, rq2 ;
	q1pow = q1.dot(q1) ;
	q2pow = q2.dot(q2) ;
	q1q2 = q1.dot(q2) ;
	rq1 = r.dot(q1) ;
	rq2 = r.dot(q2) ;

	// | u |   | q1*q1   q1*q2 |-1  | r*q1 |
	// |   | = |               |  * |      |
	// | v |   | q1*q2   q2*q2 |    | r*q2 |

	//              1                 | q2*q2  -q1*q2 |   | r*q1 |   | u |
	// ---------------------------- * |               | * |      | = |   |
	//  q1*q1*q2*q2 - (q1*q2*q1*q2)   |-q1*q2   q1*q1 |   | r*q2 |   | v |

	if(float_eq(((q1pow*q2pow) - (q1q2*q1q2)), 0.0f))
		return false ;

	float u, v ;
	float d = (q1pow*q2pow) - (q1q2*q1q2) ;

	u = (q2pow*rq1) - (q1q2*rq2) ;
	if( ((u<0.0f) && !(float_abs(u-0.0f)<=precision)) || ((u>d) && !(float_abs(u-d)<=precision)) )
		return false ;

	v = -(q1q2*rq1) + (q1pow*rq2) ;
	if( ((v<0.0f) && !(float_abs(v-0.0f)<=precision)) || ((v>d) && !(float_abs(v-d)<=precision)) )
		return false ;

	float det = 1 / ( (q1pow*q2pow) - (q1q2*q1q2) ) ;

	u = (q2pow*det*rq1) - (q1q2*det*rq2) ;
	v = -(q1q2*det*rq1) + (q1pow*det*rq2) ;

	float u_plus_v = u+v ;
	if( ((u_plus_v>1.0f) && !(float_abs(u_plus_v-1.0f)<=precision)) )
		return false ;

	vBaryCenter.x = 1.0f-u-v ;
	vBaryCenter.y = u ;
	vBaryCenter.z = v ;
	return true ;
}

int IntersectLinetoTriangle(geo::SLine &line, geo::STriangle &triangle, Vector3 &vBaryCenter, bool bDirectlyUse, bool bCullingTest, float precision)
{
	int nResult ;
	Vector3 vPos ;
	nResult = IntersectLinetoPlane(line, triangle.sPlane, vPos, bCullingTest) ;
	if(nResult == geo::INTERSECT_INFINITE || nResult == geo::INTERSECT_NONE)
		return nResult ;

    Vector3 r, q1, q2 ;

	// P = t*P0 + u*P1 + v*P2                 t+u+v=1 이 되므로
	// P = (1-u-v)*P0 + u*P1 + v*P2
	// P = P0 + u*(P1-P0) + v*(P2-P0)
	// P-P0 = u*(P1-P0) + v*(P2-P0)

	// r = P-P0
	// q1 = P1-P0
	// q2 = P2-P0
	r = vPos-triangle.avVertex[0].vPos ;
	q1 = triangle.avVertex[1].vPos-triangle.avVertex[0].vPos ;
	q2 = triangle.avVertex[2].vPos-triangle.avVertex[0].vPos ;

	// r = u*q1 + v*q2    이렇게 쓸수 있고, 양변에 q1, q2 로 내적해서 방정식 두개를 만들어낸다.

	// r*q1 = u*(q1*q1) + v*(q1*q2)
	// r*q2 = u*(q1*q2) + v*(q2*q2)    이제 이 두 연립방정식을 행렬로 풀어낸다.

	// | q1*q1   q1*q2 |   | u |    | r*q1 |
	// |               | * |   |  = |      |
	// | q1*q2   q2*q2 |   | v |    | r*q2 |

	float q1pow, q2pow, q1q2,  rq1, rq2 ;
	q1pow = q1.dot(q1) ;
	q2pow = q2.dot(q2) ;
	q1q2 = q1.dot(q2) ;
	rq1 = r.dot(q1) ;
	rq2 = r.dot(q2) ;

	// | u |   | q1*q1   q1*q2 |-1  | r*q1 |
	// |   | = |               |  * |      |
	// | v |   | q1*q2   q2*q2 |    | r*q2 |

	//              1                 | q2*q2  -q1*q2 |   | r*q1 |   | u |
	// ---------------------------- * |               | * |      | = |   |
	//  q1*q1*q2*q2 - (q1*q2*q1*q2)   |-q1*q2   q1*q1 |   | r*q2 |   | v |

	//if( !((q1pow*q2pow) - (q1q2*q1q2)) )
	//	return geo::INTERSECT_NONE ;
	//if(float_eq(((q1pow*q2pow) - (q1q2*q1q2)), 0.0f))
	//float xxxxx = ((q1pow*q2pow) - (q1q2*q1q2)) ;
	if( float_abs((q1pow*q2pow) - (q1q2*q1q2)) <= precision )
	{
		return geo::INTERSECT_NONE ;
	}

	float u, v ;

	float d = (q1pow*q2pow) - (q1q2*q1q2) ;

	u = (q2pow*rq1) - (q1q2*rq2) ;
	//u = float_round(u) ;
	//if(float_less(u, 0.0f) || float_greater(u, d))
	if( ((u<0.0f) && !(float_abs(u-0.0f)<=precision)) || ((u>d) && !(float_abs(u-d)<=precision)) )
	{
		//TRACE("intersect 1\r\n") ;
		return geo::INTERSECT_NONE ;
	}

	v = -(q1q2*rq1) + (q1pow*rq2) ;
	//v = float_round(v) ;
	//if(float_less(v, 0.0f) || float_greater(v, d))
	if( ((v<0.0f) && !(float_abs(v-0.0f)<=precision)) || ((v>d) && !(float_abs(v-d)<=precision)) )
	{
		//TRACE("intersect 2\r\n") ;
		return geo::INTERSECT_NONE ;
	}

	float det = 1 / ( (q1pow*q2pow) - (q1q2*q1q2) ) ;

	u = (q2pow*det*rq1) - (q1q2*det*rq2) ;
    //if(u < 0.0f || u > 1.0f)
	//if( float_less(u, 0.0f) || float_greater(u, 1.0f) )
	//	return geo::INTERSECT_NONE ;

	v = -(q1q2*det*rq1) + (q1pow*det*rq2) ;
	//if(v < 0.0f || (u+v) > 1.0f)
	//if( float_less(v, 0.0f) || float_greater((u+v), 1.0f) )
	//	return geo::INTERSECT_NONE ;

	//if(float_greater((u+v), 1.0f))
	float u_plus_v = u+v ;
	if( ((u_plus_v>1.0f) && !(float_abs(u_plus_v-1.0f)<=precision)) )
	{
		//TRACE("intersect 3\r\n") ;
		return geo::INTERSECT_NONE ;
	}

	if(bDirectlyUse)
	{
		vBaryCenter = triangle.avVertex[0].vPos*(1-u-v) + triangle.avVertex[1].vPos*u + triangle.avVertex[2].vPos*v ;
	}
	else
	{
		//vPos = triangle.avVertex[0].vPos*(1-u-v) + triangle.avVertex[1].vPos*u + triangle.avVertex[2].vPos*v ;
		vBaryCenter.x = (1-u-v) ;
		vBaryCenter.y = u ;
		vBaryCenter.z = v ;
	}

	return geo::INTERSECT_POINT ;
}

int TriangleIntersect(const geo::SLine *psLine, const geo::STriangle *psTriangle, Vector3 *pvResult, bool bUsable)
{
	Vector3 e1, e2, p, q, s ;
	float a, f, u, v, w ;

	e1 = psTriangle->avVertex[1].vPos - psTriangle->avVertex[0].vPos ;
	e2 = psTriangle->avVertex[2].vPos - psTriangle->avVertex[0].vPos ;
	p = psLine->v.cross(e2) ;
	a = e1.dot(p) ;
	if(float_eq(a, 0.0f))
		return geo::INTERSECT_NONE ;

	f = 1.0f/a ;

	s = psLine->s-psTriangle->avVertex[0].vPos ;

	u = f*s.dot(p) ;
	if(float_less(u, 0.0f) || float_greater(u, 1.0f))
		return geo::INTERSECT_NONE ;

	q = s.cross(e1) ;
	v = f*psLine->v.dot(q) ;
    if(float_less(v, 0.0f) || float_greater(u+v, 1.0f))
        return geo::INTERSECT_NONE ;

	w = f*e2.dot(q) ;
	if(float_less(w, 0.0f))
		return geo::INTERSECT_NONE ;

	if(bUsable)
	{
		*pvResult = psTriangle->avVertex[0].vPos*(1.0f-u-v) + psTriangle->avVertex[1].vPos*u + psTriangle->avVertex[2].vPos*v ;
	}
	else
	{
		pvResult->x = (1-u-v) ;
		pvResult->y = u ;
		pvResult->z = v ;
	}

	return geo::INTERSECT_POINT ;
}

bool IsPointOnTriangle(Vector3 &vPos, geo::STriangle &sTriangle, Vector3 &vResult, float precision)
{
	// P = t*P0 + u*P1 + v*P2                 t+u+v=1 이 되므로
	// P = (1-u-v)*P0 + u*P1 + v*P2
	// P = P0 + u*(P1-P0) + v*(P2-P0)
	// P-P0 = u*(P1-P0) + v*(P2-P0)

	return TriangleIntersectLinearSystem(vPos-sTriangle.avVertex[0].vPos,
		sTriangle.avVertex[1].vPos-sTriangle.avVertex[0].vPos,
		sTriangle.avVertex[2].vPos-sTriangle.avVertex[0].vPos,
		vResult) ;
}
/*
bool IntersectLineToCube(geo::SLine *psLine, geo::SAACube *psCube)
{
	Vector3 vTrans = psCube->vCenter ;

	geo::SAACube cube(psCube->vCenter-vTrans, psCube->fWidth, psCube->fHeight, psCube->fDepth) ;
	geo::SLine line(psLine->s-vTrans, psLine->v, psLine->d) ;

	Vector3 c = line.s+(line.v*(line.d/2.0f)) ;
	Vector3 w = line.GetEndPos()-c ;
	float hx, hy, hz ;
	hx = cube.fWidth/2.0f ;
	hy = cube.fHeight/2.0f ;
	hz = cube.fDepth/2.0f ;

	if(fabs(c.x) > (w.x+hx))
		return false ;
	if(fabs(c.y) > (w.y+hy))
		return false ;
	if(fabs(c.z) > (w.z+hz))
		return false ;

	if(fabs(c.y*w.z - c.z*w.y) > (hy*w.z + hz*w.y))
		return false ;
	if(fabs(c.x*w.z - c.z*w.x) > (hx*w.z + hz*w.x))
		return false ;
	if(fabs(c.x*w.y - c.y*w.x) > (hx*w.y + hy*w.x))
		return false ;

    return true ;
}
*/

bool isInCube(Vector3 &pos, geo::SAACube *psCube)
{
	geo::SPlane sPlane ;

	for(int i=0 ; i<6 ; i++)
	{
		sPlane = psCube->GetFace(i) ;
		if(sPlane.Classify(pos) == geo::SPlane::SPLANE_INFRONT)
			return false ;
	}
	return true ;
}
bool IntersectLineToCube(geo::SLine *psLine, geo::SCube *psCube)
{
	int i ;

	Vector3 vTrans(psCube->vCenter.x-(psCube->fWidth/2), psCube->vCenter.y-(psCube->fHeight/2), psCube->vCenter.z-(psCube->fDepth/2)) ;

	geo::SAACube cube(psCube->vCenter-vTrans, psCube->fWidth, psCube->fHeight, psCube->fDepth) ;
	//geo::SCube cube ;
	//cube.set(psCube->vCenter-vTrans, psCube->fWidth, psCube->fHeight, psCube->fDepth,

	geo::SLine line(psLine->s-vTrans, psLine->v, psLine->d) ;

	//geo::SPlane plane ;
	//for(i=0 ; i<6 ; i++)
	//{
	//	plane = cube.GetFace(i) ;
	//	TRACE("plane[%02d] a=%g b=%g c=%g d=%g\r\n", i, plane.a, plane.b, plane.c, plane.d) ;
	//}

	int count=0 ;
	geo::SPlane asPlane[3] ;
	for(i=0 ; i<6 ; i++)
	{
		if(line.v.dot(cube.GetFace(i).GetNormal()) < 0)
            asPlane[count++] = cube.GetFace(i) ;

		assert(count <= 3) ;
	}

	float t, x, y, z ;
	for(i=0 ; i<count ; i++)
	{
		if(!float_eq(line.v.x, 0))
		{
			t = (-asPlane[i].d-line.s.x)/line.v.x ;

			y = line.s.y+line.v.y*t ;
			z = line.s.z+line.v.z*t ;
			//if( (y>=0) && (y<=cube.fHeight) && (z>=0) && (z<=cube.fDepth) )
			if(float_greater_eq(y, 0.0f) && float_less_eq(y, cube.fHeight) && float_greater_eq(z, 0.0f) && float_less_eq(z, cube.fDepth))
			{
				x = line.s.x+line.v.x*t ;
				if(float_greater_eq(x, 0.0f) && float_less_eq(x, cube.fWidth))
					return true ;
			}
		}
		if(!float_eq(line.v.y, 0))
		{
            t = (-asPlane[i].d-line.s.y)/line.v.y ;

			x = line.s.x+line.v.x*t ;
			z = line.s.z+line.v.z*t ;
			//if( (x>=0) && (x<=cube.fWidth) && (z>=0) && (z<=cube.fDepth) )
			if(float_greater_eq(x, 0.0f) && float_less_eq(x, cube.fWidth) && float_greater_eq(z, 0.0f) && float_less_eq(z, cube.fDepth))
			{
				y = line.s.y+line.v.y*t ;
				if(float_greater_eq(y, 0.0f) && float_less_eq(y, cube.fHeight))
					return true ;
			}
		}
		if(!float_eq(line.v.z, 0))
		{
            t = (-asPlane[i].d-line.s.z)/line.v.z ;

			x = line.s.x+line.v.x*t ;
			y = line.s.y+line.v.y*t ;
			//if( (x>=0) && (x<=cube.fWidth) && (y>=0) && (y<=cube.fHeight) )
			if(float_greater_eq(x, 0.0f) && float_less_eq(x, cube.fWidth) && float_greater_eq(y, 0.0f) && float_less_eq(y, cube.fHeight))
			{
				z = line.s.z+line.v.z*t ;
				if(float_greater_eq(z, 0.0f) && float_less_eq(z, cube.fDepth))
					return true ;
			}
		}
	}
	return false ;
}
bool IntersectLineToCube(geo::SLine *psLine, geo::SAACube *psCube)
{
	int i ;

	Vector3 vTrans(psCube->vCenter.x-(psCube->fWidth/2), psCube->vCenter.y-(psCube->fHeight/2), psCube->vCenter.z-(psCube->fDepth/2)) ;

	geo::SAACube cube(psCube->vCenter-vTrans, psCube->fWidth, psCube->fHeight, psCube->fDepth) ;
	geo::SLine line(psLine->s-vTrans, psLine->v, psLine->d) ;

	//geo::SPlane plane ;
	//for(i=0 ; i<6 ; i++)
	//{
	//	plane = cube.GetFace(i) ;
	//	TRACE("plane[%02d] a=%g b=%g c=%g d=%g\r\n", i, plane.a, plane.b, plane.c, plane.d) ;
	//}

	int count=0 ;
	geo::SPlane asPlane[3] ;
	for(i=0 ; i<6 ; i++)
	{
		if(line.v.dot(cube.GetFace(i).GetNormal()) < 0)
            asPlane[count++] = cube.GetFace(i) ;

		assert(count <= 3) ;
	}

	//for(i=0 ; i<count ; i++)
	//	TRACE("plane n(%10.7f %10.7f %10.7f), d=%10.7f \r\n", asPlane[i].a, asPlane[i].b, asPlane[i].c, asPlane[i].d) ;

	float t, x, y, z ;
	for(i=0 ; i<count ; i++)
	{
		if(!float_eq(line.v.x, 0))
		{
			t = (-asPlane[i].d-line.s.x)/line.v.x ;

			y = line.s.y+line.v.y*t ;
			z = line.s.z+line.v.z*t ;
			//if( (y>=0) && (y<=cube.fHeight) && (z>=0) && (z<=cube.fDepth) )
			if(float_greater_eq(y, 0.0f) && float_less_eq(y, cube.fHeight) && float_greater_eq(z, 0.0f) && float_less_eq(z, cube.fDepth))
			{
				x = line.s.x+line.v.x*t ;
				if(float_greater_eq(x, 0.0f) && float_less_eq(x, cube.fWidth))
					return true ;
			}
		}
		if(!float_eq(line.v.y, 0))
		{
            t = (-asPlane[i].d-line.s.y)/line.v.y ;

			x = line.s.x+line.v.x*t ;
			z = line.s.z+line.v.z*t ;
			//if( (x>=0) && (x<=cube.fWidth) && (z>=0) && (z<=cube.fDepth) )
			if(float_greater_eq(x, 0.0f) && float_less_eq(x, cube.fWidth) && float_greater_eq(z, 0.0f) && float_less_eq(z, cube.fDepth))
			{
				y = line.s.y+line.v.y*t ;
				if(float_greater_eq(y, 0.0f) && float_less_eq(y, cube.fHeight))
					return true ;
			}
		}
		if(!float_eq(line.v.z, 0))
		{
            t = (-asPlane[i].d-line.s.z)/line.v.z ;

			x = line.s.x+line.v.x*t ;
			y = line.s.y+line.v.y*t ;
			//if( (x>=0) && (x<=cube.fWidth) && (y>=0) && (y<=cube.fHeight) )
			if(float_greater_eq(x, 0.0f) && float_less_eq(x, cube.fWidth) && float_greater_eq(y, 0.0f) && float_less_eq(y, cube.fHeight))
			{
				z = line.s.z+line.v.z*t ;
				if(float_greater_eq(z, 0.0f) && float_less_eq(z, cube.fDepth))
					return true ;
			}
		}
	}
	return false ;
}

int IntersectPlaneToCube(geo::SPlane *psPlane, geo::SAACube *psCube)
{
	int i, nPos=0 ;
	Vector3 v, vtemp, vPos ;
	float ftemp, fdot=0 ;

	//평면의 법선과 가장 근접한 대각선을 찾는다 큐브에서
	for(i=0 ; i<4 ; i++)
	{
		vtemp = psCube->GetPos(i) ;
		v = (psCube->vCenter-vtemp).Normalize() ;
		ftemp = fabs(psPlane->GetNormal().dot(v)) ;
		if(fdot < ftemp)
		{
			fdot = ftemp ;
            nPos = i ;
		}
	}

	Vector3 vMin, vMax ;

	//vMin = vPos ;
	//vMax = vMin+(psCube->vCenter-vMin)*2.0f ;
	vMin = psCube->GetPos(nPos) ;
	if(nPos == 0)
		vMax = psCube->GetPos(6) ;
	else if(nPos == 1)
		vMax = psCube->GetPos(7) ;
	else if(nPos == 2)
		vMax = psCube->GetPos(4) ;
	else if(nPos == 3)
		vMax = psCube->GetPos(5) ;


	int anClassify[2] ;
	anClassify[0] = psPlane->Classify(vMin) ;
	anClassify[1] = psPlane->Classify(vMax) ;

	if(anClassify[0] == SPlane::SPLANE_INFRONT && anClassify[1] == SPlane::SPLANE_INFRONT)
		return SPlane::SPLANE_INFRONT ;
	else if(anClassify[0] == SPlane::SPLANE_BEHIND && anClassify[1] == SPlane::SPLANE_BEHIND)
		return SPlane::SPLANE_BEHIND ;

	return SPlane::SPLANE_INTERSECT ;
}

bool IntersectLineToSphere(geo::SLine *psLine, Vector3 vPos, float fRadius, float precision)
{
	float b, c, D ;
	Vector3 vDist = psLine->s - vPos ;

	b = vDist.dot(psLine->v) ;
	c = vDist.dot(vDist) - fRadius*fRadius ;
	D = (b*b) - c ;
	//if(D < 0)
	if( (D < 0.0f) && !(float_abs(D) <= precision) )
	//if(float_less(D, 0.0f))
		return false ;
	return true ;
}
bool IntersectLineToSphere(geo::SLine *psLine, geo::SSphere *psSphere, Vector3 &vIntersect, float precision)
{
	Vector3 vLength = psSphere->vPos - psLine->s ;
	float s = vLength.dot(psLine->v) ;
	float sqrLength = vLength.Magnitude() ;
	sqrLength = sqr(sqrLength) ;

	float sqrRadius = sqr(psSphere->fRadius) ;

	//s<0 && sqrLength > sqrRadius
	if( ((s < 0.0f) && !(float_abs(s) <= precision)) && ((sqrLength > sqrRadius) && !(float_abs(sqrLength-sqrRadius) <= precision)) )
		return false ;

	float m = sqrLength - sqr(s) ;
	float diff = float_abs(m-sqrRadius) ;
	if( (m > sqrRadius) && !(diff <= precision) )
		return false ;

	float t, q ;
	if(diff <= precision)
		q = 0.0f ;
	else
		q = sqrt(sqrRadius-m) ;

	if( (sqrLength > sqrRadius) && !(float_abs(sqrLength-sqrRadius) <= precision) )
	{
		t = (s-q)/psLine->d ;
		if( ((t<0.0f) && !(float_abs(t)<=precision)) || ((t>1.0f) && !(float_abs(t-1.0f)<=precision)) )
			return false ;

		vIntersect = psLine->GetPos(t) ;
	}
	else
	{
		t = (s+q)/psLine->d ;
		if( ((t<0.0f) && !(float_abs(t)<=precision)) || ((t>1.0f) && !(float_abs(t-1.0f)<=precision)) )
			return false ;

		vIntersect = psLine->GetPos(t) ;
	}
	return true ;
}
bool IntersectDiskToRay(Vector3 &v1, Vector3 &v2, Vector3 &vDiskCenter, Vector3 &vDiskNormal, float fRadius)
{
	geo::SPlane plane(vDiskNormal.x, vDiskNormal.y, vDiskNormal.z, vDiskCenter.Magnitude()) ;

	Vector3 vIntersect ;
	float t ;
	if(IntersectLinetoPlane(v1, v2, plane, vIntersect, t))
	{
		float diff = (vIntersect-vDiskCenter).Magnitude() ;
		if(diff <= fRadius)
			return true ;
	}

    return false ;
}
bool IntersectSphereToBox(geo::SSphere *psSphere, geo::SAACube *psCube)
{
	float dist=0.0f, d ;

	for(int i=0 ; i<3 ; i++)
	{
		if(float_less(psSphere->vPos[i], psCube->vMin[i]))
		{
			d = psSphere->vPos[i] - psCube->vMin[i] ;
			dist += (d*d) ;
		}
		else if(float_greater(psSphere->vPos[i], psCube->vMax[i]))
		{
			d = psSphere->vPos[i] - psCube->vMax[i] ;
			dist += (d*d) ;
		}
	}
	return float_less_eq(dist, psSphere->fRadius*psSphere->fRadius) ;
}

int ComputeBigO(double length, double minsize, double treesystem)
{
	//전체길이를 절반씩 나눠서 트리를 만들어 가니까
	//minsize = (((length/2)/2)...) 이렇게 하면 2로 나눈 갯수가 트리의 높이가 된다.
	//위의 식을 반대로 쓰면 length = minsize * (2*2*2*...)
	//minsize * (2^h) = length 으로 볼수 있다.
	//(2^h) = (length/minsize)
	//log2(length/minsize) = h 그러나 이 식을 로그의 성질 logx(y) = logz(y)/logz(x) 을 이용해서
	//h = ln(length/minsize)/ln(2) 이렇게 될수 있다.

	double a = length/minsize ; //minsize/length ;
	double exp = fabs(log(a)/log(2.0)) ;
	double num=0 ;

    for(int i=0 ; i<=(int)exp ; i++)
	{
		//num += pow((int)treesystem, i) ;
		num += pow(treesystem, i) ;
	}
	return (int)num ;
}

Vector3 ReflectVector(Vector3 vIncident, Vector3 vNormal)
{
	return (vIncident - (vNormal*(vIncident.dot(vNormal)))*2.0f) ;
}

float IncludedAngle(Vector3 &v1, Vector3 &v2)
{
	float dot = v1.dot(v2) ;
	if(dot >= 1.0f)
		return 0.0f ;
	return acosf(dot) ;
}

Vector3 GetMidPoint(Vector3 &v1, Vector3 &v2, Vector3 &v3)
{
	Vector3 vPos ;

	vPos.x = (v1.x+v2.x+v3.x)/3.0f ;
	vPos.y = (v1.y+v2.y+v3.y)/3.0f ;
	vPos.z = (v1.z+v2.z+v3.z)/3.0f ;

	return vPos ;
}

float GetAreaOfTriangle(Vector3 v1, Vector3 v2, Vector3 v3)
{
	Vector3 va, vb, vc ;
	float a, b, c ;

	va = v2 - v1 ;
	vb = v3 - v2 ;
	vc = v1 - v3 ;

	a = va.Magnitude() ;
	b = vb.Magnitude() ;
	c = vc.Magnitude() ;

	float p = (a+b+c)/2.0f ;

	return sqrt(p*(p-a)*(p-b)*(p-c)) ;
}

Vector3 ProjectionVector(Vector3 &v1, Vector3 &v2, bool bRange)
{
	float dot = v1.dot(v2) ;
	if(bRange)
	{
		//if(dot <= 0.0f)
		if(float_less_eq(dot, 0.0f))
			return Vector3(0, 0, 0) ;
	}

	// 투영벡터 = (A*B/|B|)(B/|B|) = (A*B/|B|*|B|)B
	float fProjLength = dot/(v2.Magnitude()*v2.Magnitude()) ;
	return v2*fProjLength ;
}

Vector3 ComputeTangent(SVertex &vt0, SVertex &vt1, SVertex &vt2)
{
	//v1.pos-v0.pos = (v1.tex.u-v0.tex.u)*T + (v1.tex.v-v0.tex.v)*B ;
	//v2.pos-v0.pos = (v2.tex.u-v0.tex.u)*T + (v2.tex.v-v0.tex.v)*B ;

	//Refer to http://www.blacksmith-studios.dk/projects/downloads/tangent_matrix_derivation.php
	//Refer to DirectX9 셰이더프로그래밍 Chapter5 범프맵핑 p322

//	 ---------u
//	 |
//	 |
//	 |
//	 v

	//|p1-p0|   |(u1-u0) (v1-v0)| |Tangent |
	//|     | = |               |*|        |
	//|p2-p0|   |(u2-u0) (v2-v0)| |Binormal|
	//
	//    |(u1-u0) (v1-v0)|
	//det=|               |=( (u1-u0)*(v2-v0)-(v1-v0)*(u2-u0) )
	//    |(u2-u0) (v2-v0)|
	//
	//|Tangent |          | (v2-v0) -(v1-v0)| |p1-p0|
	//|        |= (1/det)*|                 |*|     |
	//|Binormal|          |-(u2-u0)  (u1-u0)| |p2-p0|
	//
	//
	//Tangent  = (1/det)*( (v2-v0)*(p1-p0)-(v1-v0)*(p2-p0) )
	//Binormal = (1/det)*(-(u2-u0)*(p1-p0)+(u1-u0)*(p2-p0) )
	//
	//각 성분별로 계산을 실시한다.
	//그러나 (1/det)는 나중에 Normalize 계산시에 모두 날라가 버리기 때문에 무용지물이다

	float   v0, v1, v2 ;
	float	u0, u1, u2 ;
	Vector3 p0, p1, p2 ;

	p0.set(vt0.vPos.x, vt0.vPos.y, vt0.vPos.z) ;
	p1.set(vt1.vPos.x, vt1.vPos.y, vt1.vPos.z) ;
	p2.set(vt2.vPos.x, vt2.vPos.y, vt2.vPos.z) ;

	u0 = vt0.vTex.x ;
	u1 = vt1.vTex.x ;
	u2 = vt2.vTex.x ;

	v0 = vt0.vTex.y ;
	v1 = vt1.vTex.y ;
	v2 = vt2.vTex.y ;

    float det = (u1-u0)*(v2-v0) - (v1-v0)*(u2-u0) ;

	Vector3 vTangent, vBinormal, vNormal ;

	//vTangent  = ( (p1-p0)*(v2-v0) - (p2-p0)*(v1-v0) )*(1/det) ;
	//vTangent = vTangent.Normalize() ;
	vBinormal = (-(p1-p0)*(u2-u0) + (p2-p0)*(u1-u0) )*(1.0f/det) ;
	vBinormal = vBinormal.Normalize() ;

	//vNormal.set(vt0.normal.x, vt0.normal.y, vt0.normal.z) ;
	vNormal.set(vt0.vNormal.x, vt0.vNormal.y, vt0.vNormal.z) ;

	vTangent = vBinormal.cross(vNormal) ;
	vBinormal = vTangent.cross(vNormal) ;

	//vt0.vTangent.set(vTangent.x, vTangent.y, vTangent.z) ;
	return Vector3(vTangent.x, vTangent.y, vTangent.z) ;

	//static int count=0 ;
	//TRACE("### Base Vector about [%2d] pos(%3.3f %3.3f %3.3f) ###\r\n", count++, enumVector(p0)) ;
	//TRACE("vTangent (%3.3f %3.3f %3.3f)\r\n", enumVector(vTangent)) ;
	//TRACE("vBinormal(%3.3f %3.3f %3.3f)\r\n", enumVector(vBinormal)) ;
	//TRACE("\r\n") ;
}

Vector3 GetHalfVector(Vector3 &vNormal0, Vector3 &vNormal1)
{
    Vector3 vHalfNormal = vNormal0+vNormal1 ;
	return vHalfNormal.Normalize() ;
}

Vector3 GetNewVectorfromTwoVector(Vector3 &v1, Vector3 &v2, float theta)//두벡터의 크로스벡터를 축으로 원하는 방향을 얻어낸다. 기준은 v1
{
	Vector3 v3 = v1.cross(v2) ;
	Quaternion q(v3, theta) ;
	Matrix4 mat ;
	q.GetMatrix(mat) ;
	Vector3 v4 = v1*mat ;
	return v4 ;
}
Vector3 GetNewVectorfromPerpendicularVector(Vector3 &vAxis, Vector3 &vFrom, float theta)
{
	Quaternion q(vAxis, theta) ;
	Matrix4 mat ;
	q.GetMatrix(mat) ;
	Vector3 vRet = vFrom*mat ;
	return vRet ;
}

Vector3 SlidingPlane(geo::SLine *psLine, geo::SPlane *psPlane)
{
	Vector3 vEndpos = psLine->GetEndPos() ;
	float dist = psPlane->ToPosition(vEndpos) ;

	//dist가 음수면 평면노말방향으로, dist가 양수면 평면노말의 반대방향으로 그러니까 결국엔 -dist가 됨.
	return (vEndpos - (psPlane->GetNormal()*dist)) ;
}

Vector3 LayingPlane(geo::SLine *psLine, geo::SPlane *psPlane, geo::SLine *psResultLine)
{
	if(psResultLine)
	{
		psResultLine->s = psPlane->GetOnPos(psLine->s) ;
		Vector3 vdist = psPlane->GetOnPos(psLine->GetEndPos()) - psResultLine->s ;
		psResultLine->v = vdist.Normalize() ;
		psResultLine->d = vdist.Magnitude() ;
	}
	else
		return (psPlane->GetOnPos(psLine->GetEndPos()) - psPlane->GetOnPos(psLine->s)) ;
}

Vector3 GetPosInXZPlane(Vector3 &p)
{
	return Vector3(p.x, 0.0f, p.z) ;
}

int Intersect2DLines(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float *x, float *y)
{
	float Ax, Bx, Cx, Ay, By, Cy, d, e, f, num, offset ;
	float x1lo, x1hi, y1lo, y1hi ;

	Ax = x2-x1 ;
	Bx = x3-x4 ;

	if(Ax < 0.0f)
	{
		x1lo = x2 ;
		x1hi = x1 ;
	}
	else
	{
		x1hi = x2 ;
		x1lo = x1 ;
	}

	if(Bx > 0.0f)
	{
		if(x1hi < x4 || x3 < x1lo)
			return geo::INTERSECT_NONE ;
	}
	else
	{
		if(x1hi < x3 || x4 < x1lo)
			return geo::INTERSECT_NONE ;
	}

	Ay = y2-y1 ;
	By = y3-y4 ;

	if(Ay < 0.0f)
	{
		y1lo = y2 ;
		y1hi = y1 ;
	}
	else
	{
		y1hi = y2 ;
		y1lo = y1 ;
	}

	if(By > 0.0f)
	{
		if(y1hi < y4 || y3 < y1lo)
			return geo::INTERSECT_NONE ;
	}
	else
	{
		if(y1hi < y3 || y4 < y1lo)
			return geo::INTERSECT_NONE ;
	}

	Cx = x1-x3 ;
	Cy = y1-y3 ;
	d = By*Cx - Bx*Cy ;
	f = Ay*Bx - Ax*By ;

	if(f > 0.0f)
	{
		if(d < 0.0f || d > f)
			return geo::INTERSECT_NONE ;
	}
	else
	{
		if(d > 0.0f || d < f)
			return geo::INTERSECT_NONE ;
	}

	e = Ax*Cy - Ay*Cx ;

	if(f > 0.0f)
	{
		if(e < 0.0f || e > f)
			return geo::INTERSECT_NONE ;
	}
	else
	{
		if(e > 0.0f || e < f)
			return geo::INTERSECT_NONE ;
	}

	if(float_abs(f) <= FLT_EPSILON)
		return geo::INTERSECT_INFINITE ;

	num = d*Ax ;
	if( (num >= 0.0f && f >= 0.0f) || (num < 0.0f && f < 0.0f) )
		offset = f * 0.5f ;
	else
		offset = -f * 0.5f ;

	if( (num < 0.0f && f < 0.0f) || (num >= 0.0f && f >= 0.0f) )
		*x = x1 + ((num+offset)/f) - 0.5f ;
	else
		*x = x1 + ((num+offset)/f) + 0.5f ;

	num = d*Ay ;
	if( (num >= 0.0f && f >= 0.0f) || (num < 0.0f && f < 0.0f) )
		offset = f * 0.5f ;
	else
		offset = -f * 0.5f ;

	if( (num < 0.0f && f < 0.0f) || (num >= 0.0f && f >= 0.0f) )
		*y = y1 + ((num+offset)/f) - 0.5f ;
	else
		*y = y1 + ((num+offset)/f) + 0.5f ;

	return geo::INTERSECT_POINT ;
}

void DrawCircle(geo::SPixel *psPixels, int cx, int cy, int R, DWORD dwColor)
{
    int x=0, y=R, h, nDeltaE, nDeltaNE ;
    h = 1-R ;
	nDeltaE = 3 ;
	nDeltaNE = 5-2*R ;

    while(y >= x)
	{
		//psPixels[x+cx][y+cy] = dwColor ;
		//psPixels[y+cx][x+cy] = dwColor ;
		//psPixels[y+cx][-x+cy] = dwColor ;
		//psPixels[x+cx][-y+cy] = dwColor ;
		//psPixels[-x+cx][-y+cy] = dwColor ;
		//psPixels[-y+cx][-x+cy] = dwColor ;
		//psPixels[-y+cx][x+cy] = dwColor ;
		//psPixels[-x+cx][y+cy] = dwColor ;

		if(h<0)
		{
			h += nDeltaE ;
			nDeltaE += 2 ;
			nDeltaNE += 2 ;
		}
		else
		{
			h += nDeltaNE ;
			nDeltaE += 2 ;
			nDeltaNE += 4 ;
			y-- ;
		}
		x++ ;
	}
}

int BresenhamLine(int x1, int y1, int x2, int y2, geo::SPixel *psPixelBuffer)
{
	int x, y;
	int delta_x, delta_y, d;
	int inc;
	int Einc, NEinc;
	int count=0 ;

	if (abs(x2 - x1) > abs(y2 - y1))   /* |slope| < 1 */
	{
		if (x1 > x2)
		{
			int temp ;

			temp = x1 ;
			x1 = x2 ;
			x2 = temp ;

			temp = y1 ;
			y1 = y2 ;
			y2 = temp ;
		}
		inc = (y2 > y1) ? 1 : -1;
		delta_x = x2 - x1;
		delta_y = abs(y2 - y1);
		d = 2*delta_y - delta_x;
		Einc = 2*delta_y;
		NEinc = 2*(delta_y - delta_x);

		psPixelBuffer[count].x = x = x1 ;
		psPixelBuffer[count++].y = y = y1 ;

		while (x < x2)
		{
			x++;
			if (d > 0)
			{
				y += inc;
				d += NEinc;
			}
			else
				d += Einc;

			psPixelBuffer[count].x = x ;
			psPixelBuffer[count++].y = y ;
		}
	}
	else
	{
		if (y1 > y2)
		{
			int temp ;

			temp = x1 ;
			x1 = x2 ;
			x2 = temp ;

			temp = y1 ;
			y1 = y2 ;
			y2 = temp ;
		}
		inc = (x2 > x1) ? 1 : -1;
		delta_x = abs(x2 - x1);
		delta_y = y2 - y1;
		d = 2*delta_x - delta_y;
		Einc = 2*delta_x;
		NEinc = 2*(delta_x - delta_y);

		psPixelBuffer[count].x = x = x1 ;
		psPixelBuffer[count++].y = y = y1 ;

		while (y < y2)
		{
			y++;
			if (d > 0)
			{
				x += inc;
				d += NEinc;
			}
			else
				d += Einc;

			psPixelBuffer[count].x = x ;
			psPixelBuffer[count++].y = y ;
		}
	}
	return count ;
}
float GaussianFunc(float x, float z, float area, float height)
{
	return height * pow(natural_log, -(pow(x,2.0f)+pow(z,2.0f))/(2.0f*pow(area,2.0f))) ;
}
float LinerInterpolation(float a, float b, float x)
{
    return a+x*(b-a) ;
}
float CosinInterpolation(float a, float b, float x)
{
	float t = x*3.1415927f ;
	float f = (1.0f-cosf(t))*0.5f ;
	return a*(1.0f-f)+b*f ;
}
float C2SCurveFunction(float t)
{
	return ( 6.0f*(t*t*t*t*t) - 15.0f*(t*t*t*t) + 10.0f*(t*t*t) ) ;
}
float SinDampingFunction(float t)
{
	return sinf(pi*0.5f*t) ;
}
float WeightInterpolation(float a, float b, float interpolation)
{
	return (interpolation-a)/(b-a) ;
}
void AxisAngleToEuler(Vector3 &axis, float theta, float &pitch, float &yaw, float &roll)
{
	float s = sinf(theta) ;
	float c = cosf(theta) ;
	float t = 1.0f-c ;

	if((axis.x*axis.y*t + axis.z*s) > 0.998f)
	{
		yaw = 2.0f*atan2f(axis.x*sinf(theta*0.5f), cosf(theta*0.5f)) ;
		roll = pi*0.5f ;
		pitch = 0.0f ;
		return ;
	}
	if((axis.x*axis.y*t + axis.z*s) < -0.998f)
	{
		yaw = -2.0f*atan2f(axis.x*sinf(theta*0.5f), cosf(theta*0.5f)) ;
		roll = -pi*0.5f ;
		pitch = 0.0f ;
		return ;
	}
	
	yaw = atan2f(axis.y*s - axis.x*axis.z*t, 1.0f-(sqr(axis.y)+sqr(axis.z))*t) ;
	roll = asinf(axis.x*axis.y*t + axis.z*s) ;
	pitch = atan2f(axis.x*s - axis.y*axis.z*t, 1.0f-(sqr(axis.x)+sqr(axis.z))*t) ;
}
float calculateAngle(float x, float y)
{
	float theta=0.0f ;
	Vector3 vOriginDir(1.0f, 0.0f, 0.0f), vDir(x, y, 0.0f) ;
	theta = IncludedAngle(vOriginDir, vDir) ;
	if(float_eq(vDir.y, 0.0f))
	{
		if(float_eq(vDir.x, 1.0f))
			theta = 0.0f ;
		else
			theta = pi ;
	}
	else if(float_less(vDir.y, 0.0f))
		theta = two_pi - theta ;

	return theta ;
}
void calculateVector(float theta, float &x, float &y)
{
	x = cosf(theta) ;
	y = sinf(theta) ;
}