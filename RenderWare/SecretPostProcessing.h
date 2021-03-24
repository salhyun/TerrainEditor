#pragma once

#include "D3Ddef.h"

class CPostSceneEffect ;

#define D3DFVF_POSTPROCESSINGVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

struct SPostProcessing_Vertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR2 tex ;
} ;


class CSecretPostProcessing
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	SRenderTarget *m_psRenderTarget ;
	CPostSceneEffect *m_pcPostSceneEffect ;

public :
	LPDIRECT3DTEXTURE9 m_pTexPost0, m_pTexPost1 ;
	LPDIRECT3DSURFACE9 m_pSurfPost0, m_pSurfPost1 ;

	SPostProcessing_Vertex m_sVertices[4] ;

public :
	CSecretPostProcessing() ;
	~CSecretPostProcessing() ;

    void Initialize(SRenderTarget *psRenderTarget, CPostSceneEffect *pcPostSceneEffect, LPDIRECT3DDEVICE9 pd3dDevice) ;
	void Processing() ;

	void Release() ;

	SRenderTarget *GetRenderTarget() {return m_psRenderTarget;}
	CPostSceneEffect *GetSceneEffect() {return m_pcPostSceneEffect;}
} ;

class CPostSceneEffect
{
public :
	LPDIRECT3DVERTEXDECLARATION9 m_pDecl ;//정점선언
	LPD3DXEFFECT m_pEffect ;//셰이더
	D3DXHANDLE m_hTechnique ;//테크닉

	CPostSceneEffect() ;
	virtual ~CPostSceneEffect() ;

	virtual HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName) ;
	bool SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName) ;
	void Release() ;
} ;