#pragma once

#include "D3Ddef.h"
#include "GeoLib.h"
#include "ASEData.h"
#include "LightScatteringData.h"

#define D3DFVF_SKYVERTEX (D3DFVF_XYZ)

struct SSkyVertex
{
	D3DXVECTOR3 pos ;
} ;

struct SSkyIndex
{
	WORD anIndex[3] ;
} ;

class CSecretSkyLightScatteringData : public cLightScatteringData
{
public :
	CSecretSkyLightScatteringData() {} ;
	~CSecretSkyLightScatteringData() {} ;

	void CalculateShaderParameterSkyOptimized() ;
} ;

class CSecretSkyEffect ;

class CSecretSky
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;

	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pIB ;
	SSkyVertex *m_psVertices ;
	SSkyIndex *m_psIndices ;
	unsigned long m_lNumVertex, m_lNumTriangle ;
	bool m_bWireFrame ;

	//cLightScatteringData *m_pcAtm ;
	CSecretSkyLightScatteringData *m_pcAtm ;
	D3DXVECTOR4 m_vCloudMove ;
	float m_fWorldScale ;

public :
	CSecretSky() ;
	~CSecretSky() ;

	bool Initialize(CASEData *pcASEData, LPDIRECT3DDEVICE9 pd3dDevice) ;
	//void Process(float fTimeSinceLastFrame) ;

	void Render(CSecretSkyEffect *pcSkyEffect) ;

	void Release() ;
	void SetWireFrame(bool bEnable) ;

	CSecretSkyLightScatteringData *GetAtm() {return m_pcAtm;}
	void SetWorldScale(float fScale) {m_fWorldScale=fScale;}
	float GetScale() {return m_fWorldScale;}

} ;

#include "SecretD3DEffect.h"

class CSecretSkyEffect : public CSecretD3DEffect
{
public :

	D3DXHANDLE m_hAtm ;

	CSecretSkyEffect() ;
	virtual ~CSecretSkyEffect() ;

	virtual HRESULT Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName) ;
} ;