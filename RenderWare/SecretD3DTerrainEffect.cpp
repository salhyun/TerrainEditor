#include "SecretD3DTerrainEffect.h"

CSecretD3DTerrainEffect::CSecretD3DTerrainEffect()
{
}

CSecretD3DTerrainEffect::~CSecretD3DTerrainEffect()
{
}

HRESULT CSecretD3DTerrainEffect::Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName)
{
	CSecretD3DEffect::Initialze(pd3dDevice, psEssentialElements, pszFileName) ;

	SetD3DXHANDLE(&m_hfTilingTex, "afTilingTex") ;
	SetD3DXHANDLE(&m_hAtm, "atm") ;
	SetD3DXHANDLE(&m_hmatWIT, "matWIT") ;
	SetD3DXHANDLE(&m_hmatWLP, "matWLP") ;
	SetD3DXHANDLE(&m_hmatWLPT, "matWLPT") ;
	SetD3DXHANDLE(&m_hmatRotTex, "matRotTex") ;
	SetD3DXHANDLE(&m_hvWaterLevel, "vWaterLevel") ;
	SetD3DXHANDLE(&m_hvWaterColorTone, "vWaterColorTone") ;
	SetD3DXHANDLE(&m_hvSpecular, "specular") ;
	SetD3DXHANDLE(&m_hvHolecup, "vHolecup") ;

	return S_OK ;
}