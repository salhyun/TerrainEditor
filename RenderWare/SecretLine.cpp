#include "SecretLine.h"

CSecretLine::CSecretLine()
{
	m_pmatView = NULL ;
	m_pmatProj = NULL ;
	m_pvPositions = NULL ;
	m_pvd3dPositions = NULL ;
	m_pLine = NULL ;
}

CSecretLine::~CSecretLine()
{
	Release() ;
}

void CSecretLine::Release()
{
	SAFE_DELETE(m_pLine) ;
	SAFE_DELETEARRAY(m_pvPositions) ;
	SAFE_DELETEARRAY(m_pvd3dPositions) ;
}

bool CSecretLine::Initialize(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice ;
	D3DXCreateLine(m_pd3dDevice, &m_pLine) ;
	m_pLine->SetWidth(3) ;
	m_pLine->SetAntialias(TRUE) ;

	return true ;
}

void CSecretLine::SetPositions(Vector3 *pvPositions, int nNumPosition)
{
	m_nNumPosition = nNumPosition ;
	m_pvPositions = new Vector3[m_nNumPosition] ;
	m_pvd3dPositions = new D3DXVECTOR3[m_nNumPosition] ;
	for(int i=0 ; i<m_nNumPosition ; i++)
		m_pvPositions[i] = pvPositions[i] ;
}

void CSecretLine::UpdateMatrices(Matrix4 *pmatView, D3DXMATRIX *pmatProj)
{
	m_pmatView = pmatView ;
	m_pmatProj = pmatProj ;

	Vector3 v ;
    for(int i=0 ; i<m_nNumPosition ; i++)
	{
		v = m_pvPositions[i]*(*m_pmatView) ;
		m_pvd3dPositions[i].x = v.x ;
		m_pvd3dPositions[i].y = v.y ;
		m_pvd3dPositions[i].z = v.z ;
	}
}

void CSecretLine::Render()
{
	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;

	m_pLine->Begin() ;
	m_pLine->DrawTransform(m_pvd3dPositions, m_nNumPosition, m_pmatProj, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)) ;
	m_pLine->End() ;

	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) ;
}