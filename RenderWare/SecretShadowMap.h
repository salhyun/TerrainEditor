#pragma once

#include "D3Ddef.h"
#include "SecretFrustum.h"
#include "SecretCoordSystem.h"

class CSecretRenderingGroup ;
class CSecretTerrain ;
class CSecretD3DTerrainEffect ;
class CSecretShadowMapEffect ;
class CSecretMassiveBillboard ;

#define D3DFVF_SHADOWMAPVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

struct SShadowMap_Vertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR2 tex ;
} ;

class CSecretShadowMap
{
public :
	const int RENDERINGTIME ;

private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	D3DXVECTOR3 m_vLightPos ;
	D3DXMATRIX m_matLView, m_matProj, m_matLVP, m_matDistortionY ;
	SRenderTarget m_sRenderTarget ;
    LPDIRECT3DTEXTURE9 m_pTexEdge, m_pTexSoft ;
    LPDIRECT3DSURFACE9 m_pSurfEdge, m_pSurfSoft ;
	SD3DEffect *m_psEffect ;

	CSecretFrustum m_cFrustum ;
	CSecretCoordSystem m_cSystem ;

	CSecretRenderingGroup *m_pcRenderingGroup ;
	CSecretMassiveBillboard *m_pcMassiveBillboard ;
	CSecretTerrain *m_pcTerrain ;
	CSecretD3DTerrainEffect *m_pcTerrainEffect ;
	CSecretShadowMapEffect *m_pcShadowMapEffect ;

    SShadowMap_Vertex m_sVertices[4] ;

	geo::SAACube m_sFocusedBox ;

public :
	float m_fDistoWidth, m_fDistoHeight ;
	float m_fDistoNear, m_fDistoFar ;
	float m_fShaderEpsilon ;
	float m_fBias, m_fBiasFactor ;

	int m_nWaitCount ;

private :
	bool _SetRenderTarget(UINT lMapSize, D3DFORMAT D3DFMT_rgb, LPDIRECT3DTEXTURE9 *ppTex, LPDIRECT3DSURFACE9 *ppSurf) ;
	void _CalculateUpVector(D3DXVECTOR3 *pvUp, D3DXVECTOR3 *pvCameraDir, D3DXVECTOR3 *pvLightDir) ;
	void _CalculateBoundingPoint(D3DXVECTOR3 *pvMin, D3DXVECTOR3 *pvMax, D3DXMATRIX *pmat, CSecretFrustum *pcFrustum) ;
	void _BuildViewMatrix(D3DXMATRIX *pmat, D3DXVECTOR3 *pvPos, D3DXVECTOR3 *pvDir, D3DXVECTOR3 *pvUp) ;
	void _ScaleTranslateToFit(D3DXMATRIX *pmat, D3DXVECTOR3 *pvMin, D3DXVECTOR3 *pvMax, float zn, float zf) ;
	void _FilteringShadowMap() ;//SoftShadowMap

public :
	CSecretShadowMap() ;
	~CSecretShadowMap() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SD3DEffect *psEffect, CSecretRenderingGroup *pcRenderingGroup, CSecretMassiveBillboard *pcMassiveBillboard, UINT lMapSize) ;
	HRESULT RenderShadowTex(D3DXVECTOR3 vLightPos) ;
	void RenderSystem() ;
	void RenderFrustum() ;

	HRESULT RenderObjectId() ;
	SRenderTarget *GetRenderTarget() {return &m_sRenderTarget;}
	D3DXMATRIX *GetmatLVP() { return &m_matLVP;}
	D3DXMATRIX *GetmatLV() { return &m_matLView;}
	D3DXMATRIX *GetmatLView() { return &m_matLView;}
	D3DXMATRIX *GetmatLProj() { return &m_matProj;}
	LPDIRECT3DTEXTURE9 GetTexEdge() { return m_pTexEdge;}
	LPDIRECT3DTEXTURE9 GetTexSoft() { return m_pTexSoft;}

	void SetTerrain(CSecretTerrain *pcTerrain, CSecretD3DTerrainEffect *pcTerrainEffect) ;

	void BuildDistortionY(D3DXMATRIX *pmat, float w, float h, float zn, float zf) ;
	void BuildLispSM(D3DXMATRIX *pmatLV, D3DXVECTOR3 *pvLightDir, D3DXVECTOR3 *pvCamera, D3DXMATRIX *pmatV, CSecretFrustum *pcFrustum) ;
	void BuildUniformShadowMatrix(D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvLightDir, D3DXMATRIX *pmatV, D3DXMATRIX *pmatP, CSecretFrustum *pcFrustum) ;
	void BuildViewMatrix(D3DXMATRIX *pmatView, D3DXVECTOR3 *pvPos, D3DXVECTOR3 *pvDir, D3DXVECTOR3 *pvUp) ;
	void BuildLightViewMatrix(D3DXMATRIX *pmatLView, Matrix4 *pmatView, D3DXVECTOR3 *pvLightDir, CSecretFrustum *pcFrustum) ;

	void Release() ;
} ;

class CSecretShadowMapEffect
{
public :
	LPDIRECT3DVERTEXDECLARATION9 m_pDecl ;//정점선언
	LPD3DXEFFECT m_pEffect ;//셰이더
	D3DXHANDLE m_hTechnique ;//테크닉

	CSecretShadowMapEffect() ;
	~CSecretShadowMapEffect() ;

	HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName) ;
	void Release() ;
} ;