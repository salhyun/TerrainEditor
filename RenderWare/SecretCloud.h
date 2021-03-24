#pragma once

#include "D3Ddef.h"
#include "GeoLib.h"
#include "ASEData.h"

#define D3DFVF_CLOUDVERTEX (D3DFVF_XYZ)

struct SCloudVertex
{
	D3DXVECTOR3 pos ;
} ;

struct SCloudIndex
{
	WORD anIndex[3] ;
} ;

class CSecretCloudEffect ;

class CSecretCloud
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;

	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pIB ;
	LPDIRECT3DTEXTURE9 m_pTexLayer ;
	SCloudVertex *m_psVertices ;
	SCloudIndex *m_psIndices ;
	unsigned long m_lNumVertex, m_lNumTriangle ;
	bool m_bWireFrame ;
	float m_fWorldScale ;

	D3DXVECTOR4 m_vCloudMove ;

public :
	CSecretCloud() ;
	~CSecretCloud() ;

	bool Initialize(CASEData *pcASEData, LPDIRECT3DDEVICE9 pd3dDevice) ;
	void Process(float fTimeSinceLastFrame) ;

	void Render(CSecretCloudEffect *pcCloudEffect) ;

	void Release() ;
	void SetWireFrame(bool bEnable) ;
	void SetWorldScale(float fScale) {m_fWorldScale=fScale;}
	float GetScale() {return m_fWorldScale;}

} ;

#include "SecretD3DEffect.h"

class CSecretCloudEffect : public CSecretD3DEffect
{
public :

	D3DXHANDLE m_hvCloudMove ;

	CSecretCloudEffect() ;
	virtual ~CSecretCloudEffect() ;

	virtual HRESULT Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName) ;
} ;