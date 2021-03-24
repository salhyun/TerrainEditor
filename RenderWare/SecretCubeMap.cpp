#include "SecretCubeMap.h"
#include "MathOrdinary.h"
#include "SecretMeshObject.h"

CSecretCubeMap::CSecretCubeMap()
{
	m_pd3dDevice = NULL ;
	m_pCubeMap = NULL ;
}

CSecretCubeMap::~CSecretCubeMap()
{
}

HRESULT CSecretCubeMap::LoadCubeMap(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName)// using DDS file format
{
	m_pd3dDevice = pd3dDevice ;

    if(FAILED(D3DXCreateCubeTextureFromFile(m_pd3dDevice, pszFileName, &m_pCubeMap)))
		return E_FAIL ;

    return S_OK ;
}

HRESULT CSecretCubeMap::BuildCubeMap(D3DXVECTOR3 vCameraPos, LPDIRECT3DDEVICE9 pd3dDevice, int nNumObject, CSecretMeshObject *pcSecretMeshObject)
{
	m_pd3dDevice = pd3dDevice ;

    D3DXMATRIX matOldView, matOldProj, matIdentity ;
	D3DVIEWPORT9 OldViewport ;

	D3DXMatrixIdentity(&matIdentity) ;

	m_pd3dDevice->GetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->GetTransform(D3DTS_PROJECTION, &matOldProj) ;

	//store the current back buffer and z-buffer
	LPDIRECT3DSURFACE9 pSurfBack, pSurfZBuffer ;
	m_pd3dDevice->GetRenderTarget(0, &pSurfBack) ;
	m_pd3dDevice->GetDepthStencilSurface(&pSurfZBuffer) ;
	m_pd3dDevice->GetViewport(&OldViewport) ;

	D3DXMATRIX matProj ;
	D3DXMatrixPerspectiveFovLH(&matProj, 3.141592f/2.0f, 1.0f, 0.1f, 100.0f) ;
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj) ;

	D3DVIEWPORT9 Viewport = {0, 0, 256, 256, 0.0f, 1.0f} ;
	m_pd3dDevice->SetViewport(&Viewport) ;

    for(int i=0 ; i<6 ; i++)
	{
		//Standard view that will be overridden below
		D3DXVECTOR3 vEye = vCameraPos ;

		D3DXVECTOR3 vLookat, vUp ;

		switch(i)
		{
		case D3DCUBEMAP_FACE_POSITIVE_X :
			vLookat = D3DXVECTOR3(1, 0, 0) ;
			vUp = D3DXVECTOR3(0, 1, 0) ;
			break ;
		case D3DCUBEMAP_FACE_NEGATIVE_X :
			vLookat = D3DXVECTOR3(-1, 0, 0) ;
			vUp = D3DXVECTOR3(0, 1, 0) ;
			break ;
		case D3DCUBEMAP_FACE_POSITIVE_Y :
			vLookat = D3DXVECTOR3(0, 1, 0) ;
			vUp = D3DXVECTOR3(0, 0, -1) ;
			break ;
		case D3DCUBEMAP_FACE_NEGATIVE_Y :
			vLookat = D3DXVECTOR3(0, -1, 0) ;
			vUp = D3DXVECTOR3(0, 0, 1) ;
			break ;
		case D3DCUBEMAP_FACE_POSITIVE_Z :
			vLookat = D3DXVECTOR3(0, 0, 1) ;
			vUp = D3DXVECTOR3(0, 1, 0) ;
			break ;
		case D3DCUBEMAP_FACE_NEGATIVE_Z :
			vLookat = D3DXVECTOR3(0, 0, -1) ;
			vUp = D3DXVECTOR3(0, 1, 0) ;
			break ;
		}

		vLookat = vEye+vLookat ;

		D3DXMATRIX matView ;
		D3DXMatrixLookAtLH(&matView, &vEye, &vLookat, &vUp) ;
		m_pd3dDevice->SetTransform(D3DTS_VIEW, &matView) ;

		LPDIRECT3DSURFACE9 pSurfFace ;
		m_pCubeMap->GetCubeMapSurface((D3DCUBEMAP_FACES)i, 0, &pSurfFace) ;
		//pSurfFace = pSurfVirtual ;
		m_pd3dDevice->SetRenderTarget(0, pSurfFace) ;
		m_pd3dDevice->SetDepthStencilSurface(pSurfZBuffer) ;
		SAFE_RELEASE(pSurfFace) ;

		m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L) ;

        for(int i=0 ; i<nNumObject ; i++)
			pcSecretMeshObject->Render(&matIdentity) ;

		//m_cSecretTerrain.ProcessAllInOctTree() ;
		//m_cSecretTerrain.SetWireFrame(false) ;
		//m_cSecretTerrain.RenderOctTree() ;
	}

	//Restore RenderTarget
	m_pd3dDevice->SetRenderTarget(0, pSurfBack) ;
	m_pd3dDevice->SetDepthStencilSurface(pSurfZBuffer) ;
	m_pd3dDevice->SetViewport(&OldViewport) ;
	SAFE_RELEASE(pSurfBack) ;
	SAFE_RELEASE(pSurfZBuffer) ;

	m_pd3dDevice->SetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matOldProj) ;

	return S_OK ;
}
