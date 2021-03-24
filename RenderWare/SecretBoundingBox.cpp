#include "SecretBoundingBox.h"
#include "def.h"
#include "MathOrdinary.h"


CSecretBoundingBox::CSecretBoundingBox()
{
}

CSecretBoundingBox::~CSecretBoundingBox()
{
}

void CSecretBoundingBox::Initialize(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice ;
}

void CSecretBoundingBox::SetSize(Vector3 *pvDistfromObject, float width, float height, float depth)
{
	if(!pvDistfromObject)
		m_vDistfromObject.set(0, 0, 0) ;
	else
		m_vDistfromObject = (*pvDistfromObject) ;
	//geo::SAACube cube(vCenter, width+0.2f, height+0.2f, depth+0.2f) ;
	m_sOriginCube.set(Vector3(0, 0, 0), width+0.2f, height+0.2f, depth+0.2f) ;

	for(int i=0 ; i<8 ; i++)
		m_avOriginPos[i] = m_sOriginCube.GetPos(i) ;

	m_vCenter = m_sOriginCube.vCenter ;
	m_fWidth = m_sOriginCube.fWidth ;
	m_fHeight = m_sOriginCube.fHeight ;
	m_fDepth = m_sOriginCube.fDepth ;
	m_fLineLength = m_fWidth/5.0f ;
	m_fRadius = (m_avOriginPos[7]-m_sOriginCube.vCenter).Magnitude() ;

	//for(int i=0 ; i<8 ; i++)
	//	m_avPos[i] = cube.GetPos(i) ;

	//m_sTransform.Translate(vCenterPos.x, vCenterPos.y, vCenterPos.z) ;
	//m_vCenter = cube.vCenter ;
	//m_fWidth = cube.fWidth ;
	//m_fHeight = cube.fHeight ;
	//m_fDepth = cube.fDepth ;
	//m_fLineLength = width/5.0f ;
	//m_fRadius = (cube.vMax-cube.vCenter).Magnitude() ;
}

void CSecretBoundingBox::SetTransform(D3DXMATRIX *pmat)
{
	D3DXMATRIX matTrans ;
	D3DXMatrixIdentity(&matTrans) ;

	D3DXVECTOR4 v4(m_vDistfromObject.x, m_vDistfromObject.y, m_vDistfromObject.z, 0) ;
	D3DXVec4Transform(&v4, &v4, pmat) ;
	matTrans._41 = v4.x ;
	matTrans._42 = v4.y ;
	matTrans._43 = v4.z ;

	m_dxmatTransform = (*pmat)*matTrans ;
	m_matTransform = MatrixConvert(m_dxmatTransform) ;

    for(int i=0 ; i<8 ; i++)
        m_avPos[i] = m_avOriginPos[i]*m_matTransform ;

	//                0            1            2            3            4            5            6            7
	//enum {SAACUBE_LLB, SAACUBE_LLT, SAACUBE_LRT, SAACUBE_LRB, SAACUBE_ULB, SAACUBE_ULT, SAACUBE_URT, SAACUBE_URB} ;
    
	//m_vCenter *= m_matTransform ;
	m_vCenter.x = m_matTransform.m41 ;
	m_vCenter.y = m_matTransform.m42 ;
	m_vCenter.z = m_matTransform.m43 ;

	m_fWidth = (m_avPos[0]-m_avPos[3]).Magnitude() ;
	m_fHeight = (m_avPos[0]-m_avPos[4]).Magnitude() ;
	m_fDepth = (m_avPos[0]-m_avPos[1]).Magnitude() ;
	m_fLineLength = m_fWidth/5.0f ;
	m_fRadius = (m_avPos[7]-m_vCenter).Magnitude() ;
}

geo::SPlane CSecretBoundingBox::GetFace(int nFace, Matrix4 *pmatAdd)
{
	Vector3 vPos[3] ;
	geo::SPlane plane ;

	Matrix4 mat ;

	if(pmatAdd)
        mat = (*pmatAdd) ;//mat = m_matTransform*(*pmatAdd) ;
	else
		mat.Identity() ;//mat = m_matTransform ;


	if(nFace == 0)//left face
	{
		//sPlane.MakePlane(GetPos(SAACUBE_LLB), GetPos(SAACUBE_LLT), GetPos(SAACUBE_ULT)) ;
		vPos[0] = m_avPos[BBOX_LLB]*mat ;		vPos[1] = m_avPos[BBOX_LLT]*mat ;		vPos[2] = m_avPos[BBOX_ULT]*mat ;
	}
	else if(nFace == 1)//right face
	{
		//sPlane.MakePlane(GetPos(SAACUBE_LRB), GetPos(SAACUBE_URB), GetPos(SAACUBE_URT)) ;
		vPos[0] = m_avPos[BBOX_LRB]*mat ;		vPos[1] = m_avPos[BBOX_URB]*mat ;		vPos[2] = m_avPos[BBOX_URT]*mat ;
	}
	else if(nFace == 2)//top face
	{
		//sPlane.MakePlane(GetPos(SAACUBE_ULB), GetPos(SAACUBE_ULT), GetPos(SAACUBE_URT)) ;
		vPos[0] = m_avPos[BBOX_ULB]*mat ;		vPos[1] = m_avPos[BBOX_ULT]*mat ;		vPos[2] = m_avPos[BBOX_URT]*mat ;
	}
	else if(nFace == 3)//bottom face
	{
		//sPlane.MakePlane(GetPos(SAACUBE_LLB), GetPos(SAACUBE_LRB), GetPos(SAACUBE_LRT)) ;
		vPos[0] = m_avPos[BBOX_LLB]*mat ;		vPos[1] = m_avPos[BBOX_LRB]*mat ;		vPos[2] = m_avPos[BBOX_LRT]*mat ;
	}
	else if(nFace == 4)//front face
	{
		//sPlane.MakePlane(GetPos(SAACUBE_LLB), GetPos(SAACUBE_ULB), GetPos(SAACUBE_URB)) ;
		vPos[0] = m_avPos[BBOX_LLB]*mat ;		vPos[1] = m_avPos[BBOX_ULB]*mat ;		vPos[2] = m_avPos[BBOX_URB]*mat ;
	}
	else if(nFace == 5)//back face
	{
		//sPlane.MakePlane(GetPos(SAACUBE_LLT), GetPos(SAACUBE_LRT), GetPos(SAACUBE_URT)) ;
		vPos[0] = m_avPos[BBOX_LLT]*mat ;		vPos[1] = m_avPos[BBOX_LRT]*mat ;		vPos[2] = m_avPos[BBOX_URT]*mat ;
	}

	//TRACE("pos[0]=(%10.7f %10.7f %10.7f) pos[1]=(%10.7f %10.7f %10.7f) pos[2]=(%10.7f %10.7f %10.7f)\r\n",
	//	enumVector(vPos[0]), enumVector(vPos[1]), enumVector(vPos[2])) ;

	plane.MakePlane(vPos[0], vPos[1], vPos[2]) ;

	return plane ;
}

bool CSecretBoundingBox::isIn(Vector3 &pos)
{
	geo::SPlane plane ;

	for(int i=0 ; i<6 ; i++)
	{
		plane = GetFace(i) ;
		if(plane.Classify(pos) == geo::SPlane::SPLANE_INFRONT)
			return false ;
	}
	return true ;
}
bool CSecretBoundingBox::IntersectLine(geo::SLine *psLine)
{
	Matrix4 matAdd, matInv, matTrans ;

	matInv = m_matTransform.Inverse() ;

	matTrans.Identity() ;
	matTrans.m41 = m_sOriginCube.fWidth/2.0f ;
	matTrans.m42 = m_sOriginCube.fHeight/2.0f ;
	matTrans.m43 = m_sOriginCube.fDepth/2.0f ;

	matAdd = matInv*matTrans ;

	//geo::SLine line(psLine->s-vTrans, psLine->v, psLine->d) ;
	geo::SLine line(psLine->s, psLine->v, psLine->d) ;
	Vector4 v4 ;

	v4.set(psLine->s.x, psLine->s.y, psLine->s.z) ;
	v4 = v4*matAdd ;
	line.s.set(v4.x, v4.y, v4.z) ;

	v4.set(psLine->v.x, psLine->v.y, psLine->v.z, 0) ;
	v4 = v4*matAdd ;
	line.v.set(v4.x, v4.y, v4.z) ;

    int i, count=0 ;
	geo::SPlane asPlane[3], sPlane ;
	for(i=0 ; i<6 ; i++)
	{
		if(line.v.dot(GetFace(i, &matAdd).GetNormal()) < 0)
		{
			if(count >= 3)
				return false ;

			asPlane[count++] = GetFace(i, &matAdd) ;
		}
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
			if(float_greater_eq(y, 0.0f) && float_less_eq(y, m_sOriginCube.fHeight) && float_greater_eq(z, 0.0f) && float_less_eq(z, m_sOriginCube.fDepth))
			{
				x = line.s.x+line.v.x*t ;
				if(float_greater_eq(x, 0.0f) && float_less_eq(x, m_sOriginCube.fWidth))
					return true ;
			}
		}
		if(!float_eq(line.v.y, 0))
		{
            t = (-asPlane[i].d-line.s.y)/line.v.y ;

			x = line.s.x+line.v.x*t ;
			z = line.s.z+line.v.z*t ;
			//if( (x>=0) && (x<=m_fWidth) && (z>=0) && (z<=m_fDepth) )
			if(float_greater_eq(x, 0.0f) && float_less_eq(x, m_sOriginCube.fWidth) && float_greater_eq(z, 0.0f) && float_less_eq(z, m_sOriginCube.fDepth))
			{
				y = line.s.y+line.v.y*t ;
				if(float_greater_eq(y, 0.0f) && float_less_eq(y, m_sOriginCube.fHeight))
					return true ;
			}
		}
		if(!float_eq(line.v.z, 0))
		{
            t = (-asPlane[i].d-line.s.z)/line.v.z ;

			x = line.s.x+line.v.x*t ;
			y = line.s.y+line.v.y*t ;
			//if( (x>=0) && (x<=m_fWidth) && (y>=0) && (y<=m_fHeight) )
			if(float_greater_eq(x, 0.0f) && float_less_eq(x, m_sOriginCube.fWidth) && float_greater_eq(y, 0.0f) && float_less_eq(y, m_sOriginCube.fHeight))
			{
				z = line.s.z+line.v.z*t ;
				if(float_greater_eq(z, 0.0f) && float_less_eq(z, m_sOriginCube.fDepth))
					return true ;
			}
		}
	}
	return false ;
}
bool CSecretBoundingBox::IntersectLine(geo::SLine *psLine, Vector3 *pvIntersect)
{
	bool bResult=false ;
	Vector3 a ;
	float fNearest=FLT_MAX ;
	Matrix4 matAdd, matInv, matTrans ;

	matInv = m_matTransform.Inverse() ;

	matTrans.Identity() ;
	matTrans.m41 = m_sOriginCube.fWidth/2.0f ;
	matTrans.m42 = m_sOriginCube.fHeight/2.0f ;
	matTrans.m43 = m_sOriginCube.fDepth/2.0f ;

	matAdd = matInv*matTrans ;

	geo::SLine line(psLine->s, psLine->v, psLine->d) ;
	Vector4 v4 ;

	v4.set(psLine->s.x, psLine->s.y, psLine->s.z) ;
	v4 = v4*matAdd ;
	line.s.set(v4.x, v4.y, v4.z) ;

	v4.set(psLine->v.x, psLine->v.y, psLine->v.z, 0) ;
	v4 = v4*matAdd ;
	line.v.set(v4.x, v4.y, v4.z) ;
	line.v = line.v.Normalize() ;

    int i, count=0 ;
	geo::SPlane asPlane[3], sPlane ;
	for(i=0 ; i<6 ; i++)
	{
		if(line.v.dot(GetFace(i, &matAdd).GetNormal()) < 0)
		{
			if(count >= 3)
				return false ;

			asPlane[count++] = GetFace(i, &matAdd) ;
		}

		//assert(count < 3) ;
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
			if(float_greater_eq(y, 0.0f) && float_less_eq(y, m_sOriginCube.fHeight) && float_greater_eq(z, 0.0f) && float_less_eq(z, m_sOriginCube.fDepth))
			{
				x = line.s.x+line.v.x*t ;
				if(float_greater_eq(x, 0.0f) && float_less_eq(x, m_sOriginCube.fWidth))
				{
					if(t < fNearest && t > 0.0f)
					{
						fNearest = t ;
						a.set(x, y, z) ;
						bResult = true ;
					}
				}
			}
		}
		if(!float_eq(line.v.y, 0))
		{
            t = (-asPlane[i].d-line.s.y)/line.v.y ;

			x = line.s.x+line.v.x*t ;
			z = line.s.z+line.v.z*t ;
			//if( (x>=0) && (x<=m_fWidth) && (z>=0) && (z<=m_fDepth) )
			if(float_greater_eq(x, 0.0f) && float_less_eq(x, m_sOriginCube.fWidth) && float_greater_eq(z, 0.0f) && float_less_eq(z, m_sOriginCube.fDepth))
			{
				y = line.s.y+line.v.y*t ;
				if(float_greater_eq(y, 0.0f) && float_less_eq(y, m_sOriginCube.fHeight))
				{
					if(t < fNearest && t > 0.0f)
					{
						fNearest = t ;
						a.set(x, y, z) ;
						bResult = true ;
					}
				}
			}
		}
		if(!float_eq(line.v.z, 0))
		{
            t = (-asPlane[i].d-line.s.z)/line.v.z ;

			x = line.s.x+line.v.x*t ;
			y = line.s.y+line.v.y*t ;
			//if( (x>=0) && (x<=m_fWidth) && (y>=0) && (y<=m_fHeight) )
			if(float_greater_eq(x, 0.0f) && float_less_eq(x, m_sOriginCube.fWidth) && float_greater_eq(y, 0.0f) && float_less_eq(y, m_sOriginCube.fHeight))
			{
				z = line.s.z+line.v.z*t ;
				if(float_greater_eq(z, 0.0f) && float_less_eq(z, m_sOriginCube.fDepth))
				{
					if(t < fNearest && t > 0.0f)
					{
						fNearest = t ;
						a.set(x, y, z) ;
						bResult = true ;
					}
				}
			}
		}
	}

	(*pvIntersect) = a * matAdd.Inverse() ;
	return bResult ;
}

void CSecretBoundingBox::Render(DWORD dwColor, D3DXMATRIX *pmat)
{
	int i, count ;
	SCVertex vertex[32] ;

	float fLineLength = m_sOriginCube.fWidth/5.0f ;

	count=0 ;
	//lower left bottom
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[0].x, m_avOriginPos[0].y, m_avOriginPos[0].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[0].x+fLineLength, m_avOriginPos[0].y, m_avOriginPos[0].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[0].x, m_avOriginPos[0].y+fLineLength, m_avOriginPos[0].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[0].x, m_avOriginPos[0].y, m_avOriginPos[0].z+fLineLength) ;
    //lower left top
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[1].x, m_avOriginPos[1].y, m_avOriginPos[1].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[1].x+fLineLength, m_avOriginPos[1].y, m_avOriginPos[1].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[1].x, m_avOriginPos[1].y+fLineLength, m_avOriginPos[1].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[1].x, m_avOriginPos[1].y, m_avOriginPos[1].z-fLineLength) ;
	//lower right top
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[2].x, m_avOriginPos[2].y, m_avOriginPos[2].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[2].x-fLineLength, m_avOriginPos[2].y, m_avOriginPos[2].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[2].x, m_avOriginPos[2].y+fLineLength, m_avOriginPos[2].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[2].x, m_avOriginPos[2].y, m_avOriginPos[2].z-fLineLength) ;
	//lower right bottom
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[3].x, m_avOriginPos[3].y, m_avOriginPos[3].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[3].x-fLineLength, m_avOriginPos[3].y, m_avOriginPos[3].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[3].x, m_avOriginPos[3].y+fLineLength, m_avOriginPos[3].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[3].x, m_avOriginPos[3].y, m_avOriginPos[3].z+fLineLength) ;

	//upper left bottom
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[4].x, m_avOriginPos[4].y, m_avOriginPos[4].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[4].x+fLineLength, m_avOriginPos[4].y, m_avOriginPos[4].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[4].x, m_avOriginPos[4].y-fLineLength, m_avOriginPos[4].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[4].x, m_avOriginPos[4].y, m_avOriginPos[4].z+fLineLength) ;
	//upper left top
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[5].x, m_avOriginPos[5].y, m_avOriginPos[5].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[5].x+fLineLength, m_avOriginPos[5].y, m_avOriginPos[5].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[5].x, m_avOriginPos[5].y-fLineLength, m_avOriginPos[5].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[5].x, m_avOriginPos[5].y, m_avOriginPos[5].z-fLineLength) ;
	//upper right top
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[6].x, m_avOriginPos[6].y, m_avOriginPos[6].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[6].x-fLineLength, m_avOriginPos[6].y, m_avOriginPos[6].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[6].x, m_avOriginPos[6].y-fLineLength, m_avOriginPos[6].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[6].x, m_avOriginPos[6].y, m_avOriginPos[6].z-fLineLength) ;
	//upper right bottom
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[7].x, m_avOriginPos[7].y, m_avOriginPos[7].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[7].x-fLineLength, m_avOriginPos[7].y, m_avOriginPos[7].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[7].x, m_avOriginPos[7].y-fLineLength, m_avOriginPos[7].z) ;
	vertex[count++].pos = D3DXVECTOR3(m_avOriginPos[7].x, m_avOriginPos[7].y, m_avOriginPos[7].z+fLineLength) ;

	D3DXVECTOR4 vPos ;
	D3DXMATRIX mat ;
	if(pmat)
		mat = (*pmat) * m_dxmatTransform ;
	else
		mat = m_dxmatTransform ;

	for(i=0 ; i<32 ; i++)
	{
		//D3DXVec3Transform(&vPos, &vertex[i].pos, &m_dxmatTransform) ;
		D3DXVec3Transform(&vPos, &vertex[i].pos, &mat) ;
		vertex[i].pos = D3DXVECTOR3(vPos.x, vPos.y, vPos.z) ;
		vertex[i].color = dwColor ;
	}

	unsigned short index[48] ;

	count=0 ;
	for(i=0 ; i<32 ; i += 4)
	{
		index[count++] = i ;
		index[count++] = i+1 ;

		index[count++] = i ;
		index[count++] = i+2 ;

		index[count++] = i ;
		index[count++] = i+3 ;
	}

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 32, 24, index, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;
}

void CSecretBoundingBox::RenderGeometry()
{
	int i ;
	DWORD dwColor = 0xffffffff ;//White Color
	SCVertex vertex[8] ;

	for(i=0 ; i<8 ; i++)
	{
		vertex[i].pos = D3DXVECTOR3(m_avPos[i].x, m_avPos[i].y, m_avPos[i].z) ;
		vertex[i].color = dwColor ;
	}

	//                0            1            2            3            4            5            6            7
	//enum {SAACUBE_LLB, SAACUBE_LLT, SAACUBE_LRT, SAACUBE_LRB, SAACUBE_ULB, SAACUBE_ULT, SAACUBE_URT, SAACUBE_URB} ;
	unsigned short index[] =
	{
		0, 1,
		1, 2,
		2, 3,
        3, 0,

		4, 5,
		5, 6,
		6, 7,
		7, 4,

		0, 4,
		1, 5,
		2, 6,
		3, 7,
	} ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 8, 12, index, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;
}