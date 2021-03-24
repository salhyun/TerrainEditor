#pragma once

#include "D3Ddef.h"
#include "GeoLib.h"
#include "ASEData.h"
#include "LightScatteringData.h"

#define D3DFVF_SkyDomeVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

struct SSkyDomeVertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR2 t ;
} ;

struct SSkyDomeIndex
{
	WORD anIndex[3] ;
} ;

class CSecretSkyDomeEffect ;

class CSecretSkyDome
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;

	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pIB ;
	LPDIRECT3DTEXTURE9 m_pTex ;
	SSkyDomeVertex *m_psVertices ;
	SSkyDomeIndex *m_psIndices ;
	unsigned long m_lNumVertex, m_lNumTriangle ;
	bool m_bWireFrame ;

public :
	float m_fWorldScale ;
	D3DXVECTOR3 m_vSkydomPos ;
	D3DXVECTOR4 m_vTexAni ;

public :
	CSecretSkyDome() ;
	~CSecretSkyDome() ;

	bool Initialize(CASEData *pcASEData, LPDIRECT3DDEVICE9 pd3dDevice) ;
	void Process(float fTimeSinceLastFrame) ;

	void Render(CSecretSkyDomeEffect *pcSkyDomeEffect) ;

	void Release() ;
	void SetWireFrame(bool bEnable) ;

	void SetWorldScale(float fScale) {m_fWorldScale=fScale;}
	float GetScale() {return m_fWorldScale;}

	void SetTexture(LPDIRECT3DTEXTURE9 pTex) ;

	void AttachPlanet(Vector3 vSunDir) ;
} ;

#include "SecretD3DEffect.h"

class CSecretSkyDomeEffect : public CSecretD3DEffect
{
public :

	D3DXHANDLE m_hvTexAni ;
	D3DXHANDLE m_hvSunToCamera ;

	CSecretSkyDomeEffect() ;
	virtual ~CSecretSkyDomeEffect() ;

	virtual HRESULT Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName) ;
} ;