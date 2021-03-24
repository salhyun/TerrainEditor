#include "SecretGeoRenderer.h"

CSecretGeoRenderer::CSecretGeoRenderer() :
MAXNUM_TRIANGLE(64),
MAXNUM_LINE(64)
{
	m_pd3dDevice = NULL ;
	m_nNumTriangle = m_nNumLine = 0 ;
	m_psTriangles = NULL ;
	m_psLines = NULL ;
	m_pd3dLine = NULL ;
}
CSecretGeoRenderer::~CSecretGeoRenderer()
{
	SAFE_RELEASE(m_pd3dLine) ;

	SAFE_DELETEARRAY(m_psTriangles) ;
	SAFE_DELETEARRAY(m_psLines) ;
}

void CSecretGeoRenderer::Initialize(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice ;

	m_psTriangles = new SGRTriangle[MAXNUM_TRIANGLE] ;
	m_nNumTriangle = 0 ;

	m_psLines = new SGRLine[MAXNUM_LINE] ;
	m_nNumLine = 0 ;

	D3DXCreateLine(m_pd3dDevice, &m_pd3dLine) ;
	m_pd3dLine->SetWidth(2.0f) ;
	m_pd3dLine->SetAntialias(TRUE) ;
}

void CSecretGeoRenderer::AddTriangle(geo::STriangle *psTriangle, DWORD dwColor)
{
	SGRTriangle *psGRTriangle = &m_psTriangles[m_nNumTriangle++] ;

	psGRTriangle->dwColor = dwColor ;
	geo::STriangle *ptri = (geo::STriangle *)psGRTriangle ;
	(*ptri) = (*psTriangle) ;

}
void CSecretGeoRenderer::AddLine(geo::SLine *psLine, DWORD dwColor)
{
	SGRLine *psGRLine = &m_psLines[m_nNumLine++] ;

	psGRLine->dwColor = dwColor ;
	geo::SLine *pline = (geo::SLine *)psGRLine ;
	(*pline) = (*psLine) ;
}

void CSecretGeoRenderer::Render(D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj)
{
	static D3DXVECTOR3 avPositions[128] ;

	int i ;
	Vector3 p ;
	D3DXMATRIX matVP = (*pmatView) * (*pmatProj) ;

	for(i=0 ; i<m_nNumLine ; i++)
	{
		avPositions[0] = D3DXVECTOR3(m_psLines[i].s.x, m_psLines[i].s.y, m_psLines[i].s.z) ;
		p = m_psLines[i].GetEndPos() ;
		avPositions[1] = D3DXVECTOR3(p.x, p.y, p.z) ;

		m_pd3dLine->Begin() ;
		m_pd3dLine->DrawTransform(avPositions, 2, &matVP, m_psLines[i].dwColor) ;
		m_pd3dLine->End() ;
	}

	static SCVertex asCVertex[64] ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;

	int nCount=0, n ;
	for(i=0 ; i<m_nNumTriangle ; i++)
	{
		for(n=0 ; n<3 ; n++, nCount++)
		{
			asCVertex[nCount].pos.x = m_psTriangles[i].avVertex[n].vPos.x ;
			asCVertex[nCount].pos.y = m_psTriangles[i].avVertex[n].vPos.y ;
			asCVertex[nCount].pos.z = m_psTriangles[i].avVertex[n].vPos.z ;
			asCVertex[nCount].color = m_psTriangles[i].dwColor ;
		}
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, &asCVertex[nCount-3], sizeof(SCVertex)) ;
	}
}