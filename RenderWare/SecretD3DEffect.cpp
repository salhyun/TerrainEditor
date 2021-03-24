#include "SecretD3DEffect.h"
#include <assert.h>
#include "../RenderWare/D3DDef.h"
#include <iostream>

SEffectEssentialElements::SEffectEssentialElements()
{
	D3DXMatrixIdentity(&matWorld) ;
	D3DXMatrixIdentity(&matView) ;
	D3DXMatrixIdentity(&matProj) ;
	D3DXMatrixIdentity(&matClipProj) ;

	vDiffuse = D3DXVECTOR4(1, 1, 1, 1) ;
	vSunDir = D3DXVECTOR4(0, 0, 0, 0) ;
	vSunColor = D3DXVECTOR4(1, 1, 1, 1) ;
	vCameraPos = D3DXVECTOR4(0, 0, 0, 1) ;
	vLookAt = D3DXVECTOR4(0, 0, 0, 0) ;
}

CSecretD3DEffect::CSecretD3DEffect()
{
	m_pDecl = NULL ;
	m_pEffect = NULL ;
	m_psEssentialElements = NULL ;
}

CSecretD3DEffect::~CSecretD3DEffect()
{
	Release() ;
}

void CSecretD3DEffect::Release()
{
	SAFE_RELEASE(m_pEffect) ;
	SAFE_RELEASE(m_pDecl) ;
}

HRESULT CSecretD3DEffect::Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName)
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
        
		m_psEssentialElements = psEssentialElements ;

        SetD3DXHANDLE(&m_hmatWVP, "matWVP") ;
		SetD3DXHANDLE(&m_hvDiffuse, "vDiffuse") ;
		SetD3DXHANDLE(&m_hvSunDir, "vSunDir") ;
		SetD3DXHANDLE(&m_hvSunColor, "vSunColor") ;
		SetD3DXHANDLE(&m_hvCameraPos, "vCameraPos") ;
		SetD3DXHANDLE(&m_hvLookAt, "vLookAt") ;
	}
	SAFE_RELEASE(pError) ;

	return S_OK ;
}

bool CSecretD3DEffect::SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName)
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