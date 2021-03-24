#pragma once

#include "D3DDef.h"
#include "GeoLib.h"

class CSecretGeoRenderer
{
public :
	int MAXNUM_TRIANGLE ;
	int MAXNUM_LINE ;

private :

	struct SGRTriangle : public geo::STriangle
	{
		DWORD dwColor ;
	} ;

	struct SGRLine : public geo::SLine
	{
		DWORD dwColor ;
	} ;

	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	int m_nNumTriangle ;
	SGRTriangle *m_psTriangles ;

	int m_nNumLine ;
	SGRLine *m_psLines ;

	LPD3DXLINE m_pd3dLine ;

public :
	CSecretGeoRenderer() ;
	~CSecretGeoRenderer() ;

	void Initialize(LPDIRECT3DDEVICE9 pd3dDevice) ;
	void AddTriangle(geo::STriangle *psTriangle, DWORD dwColor) ;
	void AddLine(geo::SLine *psLine, DWORD dwColor) ;

	void Render(D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj) ;
} ;