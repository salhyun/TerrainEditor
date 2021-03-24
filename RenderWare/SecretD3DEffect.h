#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#include "../RenderWare/D3DEnvironment.h"

enum D3DEFFECTKIND
{
    EFFECTKIND_TERRAIN=1,
	EFFECTKIND_ATOMSPHERE,
	EFFECTKIND_CLOUD,
	EFFECTKIND_SUNLIT_OBJECT,
} ;


struct SEffectEssentialElements//셰이더 사용시 필수 요소
{
	D3DXMATRIX matWorld, matView, matProj, matClipProj ;//월드, 뷰, 투영 행렬
	D3DXVECTOR4 vDiffuse ;
	D3DXVECTOR4 vSunDir ;//광원방향벡터
	D3DXVECTOR4 vSunColor ;//광원색상
	D3DXVECTOR4 vCameraPos ;//카메라 위치벡터
	D3DXVECTOR4 vLookAt ;//카메라 보는 방향벡터
	
	SEffectEssentialElements() ;
} ;

class CSecretD3DEffect
{
public :
    int m_nEffectKind ;
	LPDIRECT3DVERTEXDECLARATION9 m_pDecl ;//정점선언
	SEffectEssentialElements *m_psEssentialElements ;

    LPD3DXEFFECT m_pEffect ;//셰이더
	D3DXHANDLE m_hTechnique ;//테크닉
	D3DXHANDLE m_hmatWVP ;//월드-뷰-투영 변환을 위한 행렬
	D3DXHANDLE m_hvDiffuse ;
	D3DXHANDLE m_hvSunDir ;//태양의 방향
	D3DXHANDLE m_hvSunColor ;
	D3DXHANDLE m_hvLookAt ;//시선방향
	D3DXHANDLE m_hvCameraPos ;//카메라위치

public :
	CSecretD3DEffect() ;
	virtual ~CSecretD3DEffect() ;

	virtual HRESULT Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName) ;
	void Release() ;
	bool SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName) ;
} ;