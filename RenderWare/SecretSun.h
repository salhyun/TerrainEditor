#pragma once

#include "D3Ddef.h"
#include "GeoLib.h"

class CSecretSkyDomeEffect ;

#define D3DFVF_SUNVERTEX (D3DFVF_XYZ|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1))

struct SSun_Vertex
{
    D3DXVECTOR3		pos ;
    D3DXVECTOR2 t ;
	float rightFactor, upFactor ;
} ;

struct SSun_Index
{
	unsigned short anIndex[3] ;
} ;

class CSecretSun
{
public :

private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pIB ;
	CSecretSkyDomeEffect *m_pcEffect ;
	LPDIRECT3DTEXTURE9 m_pTex ;

	D3DXVECTOR3 m_vPosition ;
	int m_nNumVertex, m_nNumIndex ;

public :
	CSecretSun() ;
	~CSecretSun() ;

	bool initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretSkyDomeEffect *pcEffect) ;
	void render() ;
	void release() ;

	bool setPosition(D3DXVECTOR4 vSunDir, float width, float height, float dist=1000.0f) ;
	void setTexture(LPDIRECT3DTEXTURE9 pTex) ;
} ;