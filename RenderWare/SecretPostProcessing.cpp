#include "SecretPostProcessing.h"
#include <stdio.h>
#include <assert.h>

CSecretPostProcessing::CSecretPostProcessing()
{
	m_pd3dDevice = NULL ;
	m_psRenderTarget = NULL ;
	m_pcPostSceneEffect = NULL ;
	m_pTexPost0 = NULL ;
	m_pSurfPost0 = NULL ;
}
CSecretPostProcessing::~CSecretPostProcessing()
{
	Release() ;
}
void CSecretPostProcessing::Release()
{
	SAFE_RELEASE(m_pTexPost0) ;
	SAFE_RELEASE(m_pSurfPost0) ;
	SAFE_RELEASE(m_pTexPost1) ;
	SAFE_RELEASE(m_pSurfPost1) ;
	SAFE_DELETE(m_psRenderTarget) ;
    SAFE_DELETE(m_pcPostSceneEffect) ;
}
void CSecretPostProcessing::Initialize(SRenderTarget *psRenderTarget, CPostSceneEffect *pcPostSceneEffect, LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcPostSceneEffect = pcPostSceneEffect ;
	m_psRenderTarget = psRenderTarget ;

    D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	m_psRenderTarget->pTex->GetLevelDesc(0, &desc) ;
	SetRenderTarget(m_psRenderTarget->lWidth, m_psRenderTarget->lHeight, desc.Format, &m_pTexPost0, &m_pSurfPost0, m_pd3dDevice) ;
	SetRenderTarget(m_psRenderTarget->lWidth, m_psRenderTarget->lHeight, desc.Format, &m_pTexPost1, &m_pSurfPost1, m_pd3dDevice) ;

	m_sVertices[0].pos = D3DXVECTOR3(-1.0f, +1.0f, 0.1f) ;
	m_sVertices[0].tex = D3DXVECTOR2(0, 0) ;
	m_sVertices[1].pos = D3DXVECTOR3(+1.0f, +1.0f, 0.1f) ;
	m_sVertices[1].tex = D3DXVECTOR2(1, 0) ;
	m_sVertices[2].pos = D3DXVECTOR3(+1.0f, -1.0f, 0.1f) ;
	m_sVertices[2].tex = D3DXVECTOR2(1, 1) ;
	m_sVertices[3].pos = D3DXVECTOR3(-1.0f, -1.0f, 0.1f) ;
	m_sVertices[3].tex = D3DXVECTOR2(0, 1) ;
}
void CSecretPostProcessing::Processing()
{
}
//#############################################################//
// Glow Scene Effect
//#############################################################//

CPostSceneEffect::CPostSceneEffect()
{
	m_pDecl = NULL ;
	m_pEffect = NULL ;
	m_hTechnique = NULL ;
}
CPostSceneEffect::~CPostSceneEffect()
{
	Release() ;
}
void CPostSceneEffect::Release()
{
	SAFE_RELEASE(m_pEffect) ;
	SAFE_RELEASE(m_pDecl) ;
}
HRESULT CPostSceneEffect::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName)
{
	HRESULT hr ;
	LPD3DXBUFFER pError=NULL ;
	DWORD dwShaderFlag=NULL ;

#ifdef _SHADERDEBUG_
	dwShaderFlag = D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION ;//|D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT|D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT|D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION|D3DXSHADER_SKIPVALIDATION ;
	if(FAILED(hr=D3DXCreateEffectFromFile(pd3dDevice, pszFileName, NULL, NULL, dwShaderFlag, NULL, &m_pEffect, &pError)))
	{
		MessageBox(NULL, (LPCTSTR)pError->GetBufferPointer(), "Error", MB_OK) ;
		return E_FAIL ;
	}
#else
	if(FAILED(hr=D3DXCreateEffectFromFile(pd3dDevice, pszFileName, NULL, NULL, 0, NULL, &m_pEffect, &pError)))
	{
		MessageBox(NULL, (LPCTSTR)pError->GetBufferPointer(), "Error", MB_OK) ;
		return E_FAIL ;
	}
#endif
	else
	{
		if((m_hTechnique = m_pEffect->GetTechniqueByName("TShader")) == NULL)
		{
			assert(false && "hTechnique is NULL (TShader)") ;
			return E_FAIL ;
		}
 	}
	SAFE_RELEASE(pError) ;

	return S_OK ;
}
bool CPostSceneEffect::SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName)
{
	if((*pHandle = m_pEffect->GetParameterByName(NULL, pszName)) == NULL)
	{
		char err[128] ;
		sprintf(err, "%s Handle is NULL", pszName) ;
		assert(false && err) ;
		return false ;
	}
	return true ;
}