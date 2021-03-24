#pragma once

#include "D3Ddef.h"
#include "GeoLib.h"

class CSecretBoundingBox
{
public :
	enum BBOXPOS {BBOX_LLB=0, BBOX_LLT, BBOX_LRT, BBOX_LRB, BBOX_ULB, BBOX_ULT, BBOX_URT, BBOX_URB} ;

	LPDIRECT3DDEVICE9 m_pd3dDevice ;

	geo::SAACube m_sOriginCube ;
	Vector3 m_vCenter, m_vDistfromObject ;
	float m_fWidth, m_fHeight, m_fDepth ;
	Vector3 m_avOriginPos[8], m_avPos[8] ;
	D3DXMATRIX m_dxmatTransform ;
	Matrix4 m_matTransform ;
	float m_fLineLength ;
	float m_fRadius ;

public :
	CSecretBoundingBox() ;
	~CSecretBoundingBox() ;

	void Initialize(LPDIRECT3DDEVICE9 pd3dDevice) ;
	void SetSize(Vector3 *pvDistfromObject, float width, float height, float depth) ;
	geo::SPlane GetFace(int nFace, Matrix4 *pmatAdd=NULL) ;
	bool isIn(Vector3 &pos) ;
	bool IntersectLine(geo::SLine *psLine) ;
	bool IntersectLine(geo::SLine *psLine, Vector3 *pvIntersect) ;

	void SetTransform(D3DXMATRIX *pmat) ;

	void Render(DWORD dwColor=0xffffffff, D3DXMATRIX *pmat=NULL) ;
	void RenderGeometry() ;
} ;