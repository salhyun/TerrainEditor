#pragma once
#include "SecretD3DEffect.h"

class CSecretD3DTerrainEffect : public CSecretD3DEffect
{
public :
	D3DXHANDLE m_hfTilingTex ;
	D3DXHANDLE m_hAtm ;
	D3DXHANDLE m_hmatWIT ;
	D3DXHANDLE m_hmatWLP ;//월드-광원-투영 행렬
	D3DXHANDLE m_hmatWLPT ;//월드-광원-투영-텍스쳐바이어스 행렬
	D3DXHANDLE m_hmatRotTex ;
	D3DXHANDLE m_hvWaterLevel ;
	D3DXHANDLE m_hvWaterColorTone ;
	D3DXHANDLE m_hvSpecular ;
	D3DXHANDLE m_hvHolecup ;

	CSecretD3DTerrainEffect() ;
	virtual ~CSecretD3DTerrainEffect() ;

	virtual HRESULT Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName) ;

} ;