#pragma once

#include "SecretEffect.h"

struct SEffectEssentialElements ;
class CSecretBillboardEffectEffect ;

#define D3DFVF_BILLBOARDEFFECTVERTEX (D3DFVF_XYZ|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1))

struct SBillboardEffect_Vertex
{
    D3DXVECTOR3		pos ;
    D3DXVECTOR2 t ;
	float rightFactor, upFactor ;
} ;

struct SBillboardEffect_Index
{
	unsigned short anIndex[3] ;
} ;

class CSecretBillboardEffect : public CSecretEffect
{
public :

private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pIB ;
	LPDIRECT3DVOLUMETEXTURE9 m_pVTex ;
	CSecretBillboardEffectEffect *m_pcEffect ;

	int m_nNumVertex, m_nNumIndex ;

public :
	CSecretBillboardEffect() ;
	virtual ~CSecretBillboardEffect() ;

	bool initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName, float fOffsetY_inPixel=0.0f) ;

	virtual void process(float time) ;
	virtual void render(float fElapsedTime, D3DXMATRIX *pmatWorld) ;
	virtual void release() ;

	void loadVolumeTexture(char *pszFileName, UINT nDepth=D3DX_FROM_FILE) ;

	void setEffect(CSecretBillboardEffectEffect *pcEffect) ;
} ;

class CSecretBillboardEffectEffect
{
public :
	LPDIRECT3DVERTEXDECLARATION9 m_pDecl ;//정점선언
	LPD3DXEFFECT m_pEffect ;//셰이더
	D3DXHANDLE m_hTechnique ;//테크닉
	D3DXHANDLE m_hmatWVP ;//월드-뷰-투영 변환을 위한 행렬
	D3DXHANDLE m_hvLookat ;//카메라위치
	D3DXHANDLE m_hfElapsedTime ;

	SEffectEssentialElements *m_psEssentialElements ;

	CSecretBillboardEffectEffect() ;
	~CSecretBillboardEffectEffect() ;

	HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName) ;
	bool SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName) ;
	void Release() ;
} ;