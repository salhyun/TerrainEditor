#pragma once

#include "D3DDef.h"
#include "GeoLib.h"

class CSecretFrustum ;
class CD3DEnvironment ;
class CSecretBoundingBox ;

class CSecretDragSelection
{
public :
	enum STATUS { STATUS_IDLE=0, STATUS_DRAGSTART, STATUS_DRAGGING, STATUS_DRAGEND } ;

private :
	CD3DEnvironment *m_pcEnvironment ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;

    int m_nStatus ;
    CSecretFrustum *m_pcViewFrustum ;
	Vector3 m_avPos[8] ;
	geo::SPlane m_asPlane[6] ;

public :
	Vector2 m_vCurScreenPos, m_vPrevScreenPos ;
	bool m_bCurMouseLButton, m_bPrevMouseLButton ;
	bool m_bDrawLines ;

public :
	CSecretDragSelection() ;
	~CSecretDragSelection() ;

    void Initialize(CD3DEnvironment *pcEnvironment, LPDIRECT3DDEVICE9 pd3dDevice, CSecretFrustum *pcFrustum) ;
	void Process(Vector2 *pvScreenPos, bool bMouseLButton, Matrix4 &matV, Matrix4 &matP) ;
	void Render() ;

	bool Intersect(CSecretBoundingBox *pcBBox) ;
	int GetStatus() {return m_nStatus;}
	bool IsDrawLines() {return m_bDrawLines;}
} ;