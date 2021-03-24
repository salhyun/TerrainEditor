#include "SecretIdMap.h"
#include "D3DEnvironment.h"
#include "SecretMeshObject.h"
#include "SecretSceneManager.h"
#include "SecretPicking.h"
#include "SecretDragSelection.h"

CSecretIdMap::CSecretIdMap() : MAXNUM_TROBJECT(1024)
{
	m_pcPicking = NULL ;
	m_psEffect = NULL ;
	m_nNumTRObject = 0 ;
	m_ppsTRObject = NULL ;
	m_nNumPickedObject = 0 ;
	m_ppsPickedTRObject = NULL ;
	m_nNumPickedIndex = 0 ;
	m_pfPickedEigenIndex = NULL ;
}
CSecretIdMap::~CSecretIdMap()
{
	Release() ;
}
void CSecretIdMap::Release()
{
	SAFE_DELETEARRAY(m_pfPickedEigenIndex) ;
    SAFE_DELETEARRAY(m_ppsPickedTRObject) ;
	SAFE_DELETEARRAY(m_ppsTRObject) ;
	m_sRenderTarget.Release() ;
}

void CSecretIdMap::Initialize(CD3DEnvironment *pEnvironment, CSecretPicking *pcPicking, CSecretDragSelection *pcDragSelection, SD3DEffect *psEffect, int nResolution)
{
	m_pEnvironment = pEnvironment ;
	m_pd3dDevice = m_pEnvironment->GetD3DDevice() ;
	m_sRenderTarget.Initialize(m_pd3dDevice, nResolution, nResolution, D3DFMT_R32F, D3DFMT_D16, 0.1f, 2048.0f) ;

	m_pcDragSelection = pcDragSelection ;
	m_pcPicking = pcPicking ;
	m_psEffect = psEffect ;

	m_nNumTRObject = 0 ;
    m_ppsTRObject = new STrueRenderingObject*[MAXNUM_TROBJECT] ;

	m_nNumPickedObject = 0 ;
	m_ppsPickedTRObject = new STrueRenderingObject*[MAXNUM_TROBJECT] ;

	m_nNumPickedIndex = 0 ;
	m_pfPickedEigenIndex = new float[MAXNUM_TROBJECT] ;
}

void CSecretIdMap::AddObject(STrueRenderingObject *psTRObject)
{
	if(m_nNumTRObject >= MAXNUM_TROBJECT)
		return ;

	m_ppsTRObject[m_nNumTRObject++] = psTRObject ;
}

bool CSecretIdMap::PickOut(int nPickingKind)
{
    D3DXMATRIX matOldView, matOldProj ;
	D3DVIEWPORT9 OldViewport, Viewport = {0, 0, m_sRenderTarget.lWidth, m_sRenderTarget.lHeight, 0.0f, 1.0f} ;
	m_pd3dDevice->GetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->GetTransform(D3DTS_PROJECTION, &matOldProj) ;

	//store the current back buffer and z-buffer
	LPDIRECT3DSURFACE9 pSurfBack, pSurfZBuffer ;
	m_pd3dDevice->GetRenderTarget(0, &pSurfBack) ;
	m_pd3dDevice->GetDepthStencilSurface(&pSurfZBuffer) ;
	m_pd3dDevice->GetViewport(&OldViewport) ;

	m_pd3dDevice->SetViewport(&Viewport) ;
	m_pd3dDevice->SetRenderTarget(0, m_sRenderTarget.pSurf) ;
	m_pd3dDevice->SetDepthStencilSurface(m_sRenderTarget.pSurfZBuffer) ;

	//Render 
	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );

	int i, n ;
	DWORD dwAlphaTest, dwAlphaFunc ;

	if(SUCCEEDED(m_pd3dDevice->BeginScene()))
	{
		m_psEffect->pEffect->SetTechnique(m_psEffect->hTechnique) ;
		m_psEffect->pEffect->Begin(NULL, 0) ;

		m_pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTest) ;
		m_pd3dDevice->GetRenderState(D3DRS_ALPHAFUNC, &dwAlphaFunc) ;

		m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
		m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

		STrueRenderingObject *psTRObject ;
		CSecretMeshObject *pcMeshObject ;
        for(i=0 ; i<m_nNumTRObject ; i++)
		{
			psTRObject = m_ppsTRObject[i] ;
			pcMeshObject = psTRObject->apcObject[0] ;
			m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, pcMeshObject->m_dwAddAlpha) ;

			for(n=0 ; n<pcMeshObject->GetNumMesh() ; n++)
			{
				pcMeshObject->AddAttr(CSecretMeshObject::ATTR_IDMAP) ;
				pcMeshObject->m_anTemporary[0] = psTRObject->nEigenIndex ;
				pcMeshObject->Render(m_psEffect, pcMeshObject->GetMesh(n), &psTRObject->matWorld) ;
				pcMeshObject->AddAttr(CSecretMeshObject::ATTR_IDMAP, false) ;
			}
		}

		m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTest) ;
		m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, dwAlphaFunc) ;

		m_psEffect->pEffect->End() ;
	}

	m_pd3dDevice->EndScene() ;

	//Restore RenderTarget
	m_pd3dDevice->SetRenderTarget(0, pSurfBack) ;
	m_pd3dDevice->SetDepthStencilSurface(pSurfZBuffer) ;
	m_pd3dDevice->SetViewport(&OldViewport) ;

	SAFE_RELEASE(pSurfBack) ;
	SAFE_RELEASE(pSurfZBuffer) ;

	m_pd3dDevice->SetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matOldProj) ;


    //texture lock
	LPDIRECT3DTEXTURE9 pTex = m_sRenderTarget.pTex ;

	D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	pTex->GetLevelDesc(0, &desc) ;

	LPDIRECT3DSURFACE9 pSurf=NULL ;
	m_pd3dDevice->CreateOffscreenPlainSurface(m_sRenderTarget.lWidth, m_sRenderTarget.lHeight, desc.Format, D3DPOOL_SYSTEMMEM, &pSurf, NULL) ;

	if(FAILED(m_pd3dDevice->GetRenderTargetData(m_sRenderTarget.pSurf, pSurf)))
		return false ;

	DWORD *pdwColor ;
	D3DLOCKED_RECT d3drect ;
	if(FAILED(pSurf->LockRect(&d3drect, NULL, D3DLOCK_READONLY)))
		return false ;
	pdwColor = (DWORD *)d3drect.pBits ;

	if(nPickingKind == 0)//intersect with point
	{
		int x, y, pos ;
		x = (int)(m_pcPicking->m_vScreenPos.x*((float)desc.Width/(float)m_pEnvironment->m_lScreenWidth)+0.5f) ;
		y = (int)(m_pcPicking->m_vScreenPos.y*((float)desc.Height/(float)m_pEnvironment->m_lScreenHeight)+0.5f) ;
		pos = (y*desc.Width)+x ;

		if(pdwColor[pos] > 0)
		{
			DWORD *pdwValue = (DWORD *)&m_pfPickedEigenIndex[m_nNumPickedIndex++] ;
			memcpy(pdwValue, &pdwColor[pos], 4) ;
		}
	}
	else if(nPickingKind == 1)//intersect with drag-selection
	{
		float fValue ;
		int x, y, x1, x2, y1, y2, temp, pos ;

		x1 = (int)(m_pcDragSelection->m_vPrevScreenPos.x*((float)desc.Width/(float)m_pEnvironment->m_lScreenWidth)+0.5f) ;
		y1 = (int)(m_pcDragSelection->m_vPrevScreenPos.y*((float)desc.Height/(float)m_pEnvironment->m_lScreenHeight)+0.5f) ;
		x2 = (int)(m_pcDragSelection->m_vCurScreenPos.x*((float)desc.Width/(float)m_pEnvironment->m_lScreenWidth)+0.5f) ;
		y2 = (int)(m_pcDragSelection->m_vCurScreenPos.y*((float)desc.Height/(float)m_pEnvironment->m_lScreenHeight)+0.5f) ;

		if(x1 > x2)
		{
			temp = x1 ;
			x1 = x2 ;
			x2 = temp ;
		}
		if(y1 > y2)
		{
			temp = y1 ;
			y1 = y2 ;
			y2 = temp ;
		}

        for(y=y1; y<y2; y++)
		{
			for(x=x1; x<x2; x++)
			{
				pos = (y*desc.Width)+x ;
				if(pdwColor[pos] > 0)
				{
					DWORD *pdwValue = (DWORD *)&fValue ;
					memcpy(pdwValue, &pdwColor[pos], 4) ;
					if(!_IsPickedIndex(fValue))
						m_pfPickedEigenIndex[m_nNumPickedIndex++] = fValue ;
				}
			}
		}

		//for(int y=0; y<(int)desc.Height; y++)
		//{
		//	for(int x=0; x<(int)desc.Width; x++, pdwColor++)
		//	{
		//		if((*pdwColor) != 0)
		//		{
		//			//using D3DFMT_R32F
		//			fValue=0 ;
		//			DWORD *pdwValue = (DWORD *)&fValue ;
		//			memcpy(pdwValue, pdwColor, 4) ;

		//			//if(m_fColorFromRT < fValue)
		//			//	m_fColorFromRT = fValue ;

		//			//using D3DFMT_G16R16
		//			//if(m_nColorFromRT < (*pdwColor))
		//			//{
		//			//	D3DXFLOAT16 f16Color ;
		//			//	WORD *pwColor = (WORD *)&f16Color ;

		//			//	WORD wColor = (WORD)((*pdwColor)&0x0000ffff) ;
		//			//	memcpy(pwColor, &wColor, sizeof(WORD)) ;

		//			//	m_fColorFromRT = f16Color ;

		//			//	wColor = (WORD)(((*pdwColor)>>16)&0x0000ffff) ;
		//			//	memcpy(pwColor, &wColor, sizeof(WORD)) ;

		//			//	xx = f16Color ;
		//			//}
		//		}
		//	}
		//}
	}

	for(i=0 ; i<m_nNumPickedIndex ; i++)
	{
		for(n=0 ; n<m_nNumTRObject ; n++)
		{
			if(m_pfPickedEigenIndex[i] == m_ppsTRObject[n]->nEigenIndex)
			{
				m_ppsPickedTRObject[m_nNumPickedObject++] = m_ppsTRObject[n] ;
				break ;
			}
		}
	}

	pSurf->Release() ;

	return true ;
}
bool CSecretIdMap::_IsPickedIndex(float fIndex)
{
	for(int i=m_nNumPickedIndex-1 ; i>=0 ; i--)
	{
		if(m_pfPickedEigenIndex[i] == fIndex)
			return true ;
	}
	return false ;
}
STrueRenderingObject *CSecretIdMap::GetPickedObject()
{
	return m_ppsTRObject[m_nNumTRObject] ;
}
SRenderTarget *CSecretIdMap::GetRenderTarget()
{
	return &m_sRenderTarget ;
}
LPDIRECT3DTEXTURE9 CSecretIdMap::GetTex()
{
	return m_sRenderTarget.pTex ;
}
void CSecretIdMap::Reset()
{
	m_nNumTRObject = 0 ;
	m_nNumPickedObject = 0 ;
	m_nNumPickedIndex = 0 ;
}