#include <assert.h>
#include "SecretShadowMap.h"
#include "SecretMeshObject.h"
#include "MathOrdinary.h"
#include "SecretSceneManager.h"
#include "SecretRenderingGroup.h"
#include "SecretMassiveBillboard.h"
#include "SecretTerrain.h"
//#include "SecretD3DEffect.h"

D3DVERTEXELEMENT9 decl_ShadowMap[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//법선에 대한 설정
		D3DDECL_END()
} ;
CSecretShadowMap::CSecretShadowMap() : RENDERINGTIME(5)
{
	m_pd3dDevice = NULL ;
    m_psEffect = NULL ;
	m_pcShadowMapEffect = NULL ;
	m_pcRenderingGroup = NULL ;
	m_pcMassiveBillboard = NULL ;
	m_pcTerrain = NULL ;
	m_pTexEdge = NULL ;
	m_pTexSoft = NULL ;
	m_pSurfEdge = NULL ;
	m_pSurfSoft = NULL ;
	m_fDistoWidth = m_fDistoHeight = 1.0f ;
	m_nWaitCount = RENDERINGTIME ;
	//m_fDistoNear = 1.0f ;
	//m_fDistoFar = 2.0f ;
	m_fDistoNear = 1.0f ;
	m_fDistoFar = 2.55f ;// 15.0f ;
	m_fShaderEpsilon = 0.005f ;
	m_fBias = 1.0f ;

	m_fBiasFactor = 0.8f ;// 0.6f ;

	m_fDistoWidth = 0.73f ;
	m_fDistoHeight = 1.0f ;
}
CSecretShadowMap::~CSecretShadowMap()
{
	Release() ;
}
void CSecretShadowMap::Release()
{
	SAFE_RELEASE(m_pSurfEdge) ;
	SAFE_RELEASE(m_pTexEdge) ;
	SAFE_RELEASE(m_pSurfSoft) ;
	SAFE_RELEASE(m_pTexSoft) ;	

	SAFE_DELETE(m_pcShadowMapEffect) ;

	m_sRenderTarget.Release() ;
}
bool CSecretShadowMap::_SetRenderTarget(UINT lMapSize, D3DFORMAT D3DFMT_rgb, LPDIRECT3DTEXTURE9 *ppTex, LPDIRECT3DSURFACE9 *ppSurf)
{
	if(FAILED(m_pd3dDevice->CreateTexture(lMapSize, lMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_rgb, D3DPOOL_DEFAULT, ppTex, NULL)))
		return false ;
	if(FAILED((*ppTex)->GetSurfaceLevel(0, ppSurf)))
		return false ;

	return true ;
}
bool CSecretShadowMap::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SD3DEffect *psEffect, CSecretRenderingGroup *pcRenderingGroup, CSecretMassiveBillboard *pcMassiveBillboard, UINT lMapSize)
{
	float fNear, fFar ;
	m_pd3dDevice = pd3dDevice ;
	m_psEffect = psEffect ;
	m_pcRenderingGroup = pcRenderingGroup ;
	m_pcMassiveBillboard = pcMassiveBillboard ;
	
	fNear = 1.0f ;
	fFar = 1024.0f ;

	//D3DFORMAT Format = D3DFMT_G16R16F ;
	//D3DFORMAT Format = D3DFMT_G16R16 ;
	D3DFORMAT Format = D3DFMT_A8R8G8B8 ;

	m_sRenderTarget.Initialize(pd3dDevice, lMapSize, lMapSize, Format, D3DFMT_D16, fNear, fFar) ;

	_SetRenderTarget(lMapSize, Format, &m_pTexEdge, &m_pSurfEdge) ;
	_SetRenderTarget(lMapSize, Format, &m_pTexSoft, &m_pSurfSoft) ;

	m_pcShadowMapEffect = new CSecretShadowMapEffect() ;
	m_pcShadowMapEffect->Initialize(m_pd3dDevice, "Shader/ShadowMap.fx") ;

	m_sVertices[0].pos = D3DXVECTOR3(-1.0f, +1.0f, 0.1f) ;
	m_sVertices[0].tex = D3DXVECTOR2(0, 0) ;
	m_sVertices[1].pos = D3DXVECTOR3(+1.0f, +1.0f, 0.1f) ;
	m_sVertices[1].tex = D3DXVECTOR2(1, 0) ;
	m_sVertices[2].pos = D3DXVECTOR3(+1.0f, -1.0f, 0.1f) ;
	m_sVertices[2].tex = D3DXVECTOR2(1, 1) ;
	m_sVertices[3].pos = D3DXVECTOR3(-1.0f, -1.0f, 0.1f) ;
	m_sVertices[3].tex = D3DXVECTOR2(0, 1) ;

	D3DXMatrixOrthoLH(&m_matProj, (float)lMapSize, (float)lMapSize, fNear, fFar) ;

	float w = 2.0f*fNear/m_matProj._11 ;
	float h = 2.0f*fNear/m_matProj._22 ;

	m_cSystem.Initialize(m_pd3dDevice) ;

    return true ;
}
HRESULT CSecretShadowMap::RenderShadowTex(D3DXVECTOR3 vLightPos)
{
	m_vLightPos = vLightPos ;

	//m_fDistoWidth = 2.0f*m_sRenderTarget.fZNear/m_matProj._11 ;
	//m_fDistoHeight = 2.0f*m_sRenderTarget.fZNear/m_matProj._22 ;
	//BuildDistortionY(&m_matDistortionY, m_fDistoWidth, m_fDistoHeight, m_fDistoNear, m_fDistoFar) ;

	//D3DXMATRIX matLVP ;
	//D3DXVECTOR3 vLookat(0, 0, 0) ;
	//D3DXVECTOR3 vUp(0, 1.0f, 0) ;
 //   D3DXMatrixLookAtLH(&m_matLView, &m_vLightPos, &vLookat, &vUp) ;

	m_matLVP = m_matLView*m_matProj ;

	//D3DXVECTOR4 vret ;
	//D3DXVECTOR3 v1(0, 90, -90), v2(0, 0, 0), v3(0, -90, 90) ;

	//D3DXVec3Transform(&vret, &v1, &m_matLVP) ;
	//TRACE("post proj v(%07.03f, %07.03f, %07.03f, %07.03f) z/w=%07.03f\r\n", vret.x, vret.y, vret.z, vret.w, vret.z/vret.w) ;
	//D3DXVec3Transform(&vret, &v2, &m_matLVP) ;
	//TRACE("post proj v(%07.03f, %07.03f, %07.03f, %07.03f) z/w=%07.03f\r\n", vret.x, vret.y, vret.z, vret.w, vret.z/vret.w) ;
	//D3DXVec3Transform(&vret, &v3, &m_matLVP) ;
	//TRACE("post proj v(%07.03f, %07.03f, %07.03f, %07.03f) z/w=%07.03f\r\n", vret.x, vret.y, vret.z, vret.w, vret.z/vret.w) ;

	D3DXMATRIX matInvView ;
	D3DXMatrixInverse(&matInvView, NULL, &m_matLView) ;

	m_cSystem.SetMatrix(matInvView) ;
	//m_cFrustum.BuildFrustum(MatrixConvert(m_matLVP), MatrixConvert(m_matLView), MatrixConvert(m_matProj)) ;
	m_cFrustum.BuildFrustum(MatrixConvert(m_matLView), MatrixConvert(m_matProj)) ;

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
	//m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00ffff00, 1.0f, 0L );
	//m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00ff00ff, 1.0f, 0L );
	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L );

	if(SUCCEEDED(m_pd3dDevice->BeginScene()))
	{
		//if(m_pcTerrain)
		//{
		//	m_pcTerrainEffect->m_pEffect->SetTechnique(m_pcTerrainEffect->m_hTechnique) ;
		//	m_pcTerrainEffect->m_pEffect->Begin(NULL, 0) ;

		//	DWORD dwAlphaTestEnable, dwAlphaFunc, dwAlphaRef ;
		//	m_pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTestEnable) ;
		//	m_pd3dDevice->GetRenderState(D3DRS_ALPHAFUNC, &dwAlphaFunc) ;
		//	m_pd3dDevice->GetRenderState(D3DRS_ALPHAREF, &dwAlphaRef) ;

		//	//m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
		//	//m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESS) ;
		//	//m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0) ;

		//	m_pcTerrain->AddAttr(CSecretTerrain::ATTR_RENDERSHADOW) ;
		//	m_pcTerrain->RenderQuadTree(m_pcTerrainEffect) ;
		//	m_pcTerrain->AddAttr(CSecretTerrain::ATTR_RENDERSHADOW, false) ;

		//	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, dwAlphaRef) ;
		//	m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, dwAlphaFunc) ;
		//	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTestEnable) ;

		//	m_pcTerrainEffect->m_pEffect->End() ;
		//}

		m_psEffect->pEffect->SetTechnique(m_psEffect->hTechnique) ;
		m_psEffect->pEffect->Begin(NULL, 0) ;

		//draw model here
		m_pcRenderingGroup->Render(m_psEffect, false, CSecretRenderingRoll::RROLL_ATTR_SHADOWRENDERING) ;

		m_psEffect->pEffect->End() ;

		m_pcMassiveBillboard->AddAttr(CSecretMassiveBillboard::ATTR_IDSHADOW) ;
		m_pcMassiveBillboard->Render() ;
		m_pcMassiveBillboard->AddAttr(CSecretMassiveBillboard::ATTR_IDSHADOW, false) ;

        DWORD dwZEnable, dwColorOp0, dwColorOp1, dwColorArg1 ;

		m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
		m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwColorOp0) ;
		m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &dwColorArg1) ;
		m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwColorOp1) ;

		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1) ;
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE) ;
		m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

		m_pd3dDevice->SetFVF(D3DFVF_SHADOWMAPVERTEX) ;

		m_pcShadowMapEffect->m_pEffect->SetTechnique(m_pcShadowMapEffect->m_hTechnique) ;
		m_pcShadowMapEffect->m_pEffect->Begin(NULL, 0) ;

		//if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_ShadowMap, &m_pcShadowMapEffect->m_pDecl )))
		//	assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in ShadowMap") ;

		//rendering for shadow edge
		//m_pd3dDevice->SetRenderTarget(0, m_pSurfEdge) ;
		//m_pcShadowMapEffect->m_pEffect->BeginPass(0) ;

		//m_pcShadowMapEffect->m_pEffect->SetTexture("SrcMap", m_sRenderTarget.pTex) ;

		//m_pcShadowMapEffect->m_pEffect->CommitChanges() ;
		//m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_sVertices, sizeof(SShadowMap_Vertex)) ;

		//m_pcShadowMapEffect->m_pEffect->EndPass() ;

		//rendering for blurring
		m_pd3dDevice->SetRenderTarget(0, m_pSurfSoft) ;
		m_pcShadowMapEffect->m_pEffect->BeginPass(1) ;

		//m_pcShadowMapEffect->m_pEffect->SetTexture("SrcMap", m_pTexEdge) ;
		m_pcShadowMapEffect->m_pEffect->SetTexture("SrcMap", m_sRenderTarget.pTex) ;

		m_pcShadowMapEffect->m_pEffect->CommitChanges() ;
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_sVertices, sizeof(SShadowMap_Vertex)) ;

		m_pcShadowMapEffect->m_pEffect->EndPass() ;

		m_pcShadowMapEffect->m_pEffect->End() ;

		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwColorOp0) ;
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, dwColorArg1) ;
		m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwColorOp1) ;

	}
	m_pd3dDevice->EndScene() ;

	//Restore RenderTarget
	m_pd3dDevice->SetRenderTarget(0, pSurfBack) ;
	m_pd3dDevice->SetDepthStencilSurface(pSurfZBuffer) ;
	m_pd3dDevice->SetViewport(&OldViewport) ;

	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00ff00ff, 1.0f, 0L );

	SAFE_RELEASE(pSurfBack) ;
	SAFE_RELEASE(pSurfZBuffer) ;

	//see render-target

	//D3DSURFACE_DESC desc ;
	//ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	//m_sRenderTarget.pTex->GetLevelDesc(0, &desc) ;

	//LPDIRECT3DSURFACE9 pSurf=NULL ;
	//m_pd3dDevice->CreateOffscreenPlainSurface(m_sRenderTarget.lWidth, m_sRenderTarget.lHeight, desc.Format, D3DPOOL_SYSTEMMEM, &pSurf, NULL) ;

	//if(FAILED(m_pd3dDevice->GetRenderTargetData(m_sRenderTarget.pSurf, pSurf)))
	//	return false ;

	//DWORD *pdwColor ;
	//D3DLOCKED_RECT d3drect ;
	//if(FAILED(pSurf->LockRect(&d3drect, NULL, D3DLOCK_READONLY)))
	//	return false ;
	//pdwColor = (DWORD *)d3drect.pBits ;

	//for(int z=0 ; z<m_sRenderTarget.lHeight ; z++)
	//{
	//	for(int x=0 ; x<m_sRenderTarget.lWidth ; x++, pdwColor++)
	//	{
	//		if((*pdwColor) != 0xffffffff)
	//		{
	//			WORD wValue = (WORD)((*pdwColor)&0x0000ffff) ;
	//			TRACE("dwColor[%d][%d]=%d\r\n", x, z, wValue) ;

	//			wValue = (WORD)(((*pdwColor)>>16)&0x0000ffff) ;
	//			TRACE("dwColor[%d][%d]=%d\r\n", x, z, wValue) ;

	//			//TRACE("dwColor[%d][%d]=0x%08x\r\n", x, z, *pdwColor) ;
	//		}
	//	}
	//}

	//SAFE_RELEASE(pSurf) ;

	m_pd3dDevice->SetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matOldProj) ;

	return S_OK ;
}
void CSecretShadowMap::RenderSystem()
{
	m_cSystem.Render() ;
}
void CSecretShadowMap::RenderFrustum()
{
	m_cFrustum.DrawFrustum(m_pd3dDevice) ;

	Vector3 v ;
    SCVertex asCVertex[8] ;

	for(int i=0 ; i<8 ; i++)
	{
		v = m_sFocusedBox.GetPos(i) ;
		asCVertex[i].pos.x = v.x ;
		asCVertex[i].pos.y = v.y ;
		asCVertex[i].pos.z = v.z ;
		asCVertex[i].color = 0xff630f01 ;
	}

	unsigned short index_edge[] = 
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		4, 5,
		5, 6,
		6, 7,
		7, 4,

		0, 4,
		3, 7,

		2, 6,
		1, 5
	} ;

	DWORD dwAlphaBlendEnable ;
	m_pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlendEnable) ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;


	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 8, 12, index_edge, D3DFMT_INDEX16, asCVertex, sizeof(asCVertex[0])) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;

	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlendEnable) ;


}
void CSecretShadowMap::SetTerrain(CSecretTerrain *pcTerrain, CSecretD3DTerrainEffect *pcTerrainEffect)
{
	m_pcTerrain = pcTerrain ;
	m_pcTerrainEffect = pcTerrainEffect ;
}
void CSecretShadowMap::_ScaleTranslateToFit(D3DXMATRIX *pmat, D3DXVECTOR3 *pvMin, D3DXVECTOR3 *pvMax, float zn, float zf)
{
	pmat->_11 = 2.0f/(pvMax->x-pvMin->x) ;
	pmat->_21 = 0 ;
	pmat->_31 = 0 ;
	pmat->_41 = (pvMax->x+pvMin->x)/(pvMax->x-pvMin->x) ;

	pmat->_12 = 0 ;
	pmat->_22 = 2.0f/(pvMax->y-pvMin->y) ;
	pmat->_32 = 0 ;
	pmat->_42 = (pvMax->y+pvMin->y)/(pvMax->y-pvMin->y) ;

	pmat->_13 = 0 ;
	pmat->_23 = 0 ;
	pmat->_33 = 2.0f/(pvMax->z-pvMin->z) ;
	pmat->_43 = -(pvMax->z+pvMin->z)/(pvMax->z-pvMin->z) ;

	pmat->_14 = 0 ;
	pmat->_24 = 0 ;
	pmat->_34 = 0 ;
	pmat->_44 = 1 ;
}
void CSecretShadowMap::BuildDistortionY(D3DXMATRIX *pmat, float w, float h, float zn, float zf)
{
	pmat->_11 = w ;
	pmat->_21 = 0.0f ;
	pmat->_31 = 0.0f ;
	pmat->_41 = 0.0f ;

	pmat->_12 = 0.0f ;
	//pmat->_22 = (zf+zn)/(zf-zn) ;
	pmat->_22 = zf/(zf-zn) ;
	pmat->_32 = 0.0f ;
	//pmat->_42 = (-2.0f*zf*zn)/(zf-zn) ;
	pmat->_42 = (2.0f*zf*zn)/(zn-zf) ;

	pmat->_13 = 0.0f ;
	pmat->_23 = 0.0f ;
	pmat->_33 = h ;
	pmat->_43 = 0.0f ;

	pmat->_14 = 0.0f ;
	pmat->_24 = 1.0f ;
	pmat->_34 = 0.0f ;
	pmat->_44 = 0.0f ;
}
void CSecretShadowMap::BuildUniformShadowMatrix(D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvLightDir, D3DXMATRIX *pmatV, D3DXMATRIX *pmatP, CSecretFrustum *pcFrustum)
{
	D3DXVECTOR3 vPos, vLightPos, vLookat, v ;
	D3DXVECTOR3 vMin, vMax ;
	D3DXMATRIX matIdentity ;
	D3DXMatrixIdentity(&matIdentity) ;
    
	D3DXMATRIX matLispsm ;
	//BuildLispSM(&matLispsm, pvLightDir, pvCamera, pmatV, pcFrustum) ;

    //_CalculateBoundingPoint(&vMin, &vMax, &matIdentity, pcFrustum) ;
    //vLightPos = (*pvCamera) ;// - ((*pvLightDir)*25.0f) ;

	//pcFrustum->m_pvBoundingPositions[pcFrustum->m_nNumBoundingPosition++].set(vLightPos.x, vLightPos.y, vLightPos.z) ;
	_CalculateBoundingPoint(&vMin, &vMax, &matIdentity, pcFrustum) ;

	m_sFocusedBox.set(Vector3(vMin.x, vMin.y, vMin.z), Vector3(vMax.x, vMax.y, vMax.z)) ;

	vPos.x = vMin.x+(vMax.x-vMin.x)/2.0f ;
	vPos.y = vMin.y+(vMax.y-vMin.y)/2.0f ;
	vPos.z = vMin.z+(vMax.z-vMin.z)/2.0f ;
	vLookat = vPos ;

	//TRACE("center of boundingbox(%07.03f %07.03f %07.03f)\r\n", vPos.x, vPos.y, vPos.z) ;
	//TRACE("vMin(%07.03f %07.03f %07.03f) vMax(%07.03f %07.03f %07.03f)\r\n", vMin.x, vMin.y, vMin.z, vMax.x, vMax.y, vMax.z) ;
	//TRACE("w=%07.03f h=%07.03f d=%07.03f\r\n", vMax.x-vMin.x, vMax.y-vMin.y, vMax.z-vMin.z) ;

	//vPos.x = vPos.x + (-pvLightDir->x * 25.0f) ;
	//vPos.y = vPos.y + (-pvLightDir->y * 25.0f) ;
	//vPos.z = vPos.z + (-pvLightDir->z * 25.0f) ;
	//vLightPos = vPos ;

	//TRACE("lightpos (%07.03f %07.03f %07.03f)\r\n", vPos.x, vPos.y, vPos.z) ;

	float fDistfromBBoxCentertoCamera ;
	_CalculateBoundingPoint(&vMin, &vMax, pmatV, pcFrustum) ;
    //fDistfromBBoxCentertoCamera = vMin.z+((vMax.z-vMin.z)*0.5f) ;
	//fDistfromBBoxCentertoCamera = (fDistfromBBoxCentertoCamera<12.0f) ? 12.0f : fDistfromBBoxCentertoCamera ;

	fDistfromBBoxCentertoCamera = (vMax.z-vMin.z)*0.5f ;
	fDistfromBBoxCentertoCamera = (fDistfromBBoxCentertoCamera<12.0f) ? 12.0f : fDistfromBBoxCentertoCamera ;

	vLightPos.x = vLookat.x-(pvLightDir->x*fDistfromBBoxCentertoCamera) ;
	vLightPos.y = vLookat.y-(pvLightDir->y*fDistfromBBoxCentertoCamera) ;
	vLightPos.z = vLookat.z-(pvLightDir->z*fDistfromBBoxCentertoCamera) ;

	D3DXVECTOR3 vUp ;
	vUp.x = vLookat.x-(*pvCamera).x ;
	vUp.y = vLookat.y-(*pvCamera).y ;
	vUp.z = vLookat.z-(*pvCamera).z ;
	D3DXVec3Normalize(&vUp, &vUp) ;
	_CalculateUpVector(&vUp, &vUp, pvLightDir) ;

    //D3DXMatrixLookAtLH(&m_matLView, &vPos, &vLookat, &vUp) ;
	BuildViewMatrix(&m_matLView, &vLightPos, pvLightDir, &vUp) ;

	_CalculateBoundingPoint(&vMin, &vMax, &m_matLView, pcFrustum) ;

	float bias = (vMax.y-vMin.y)*m_fBias ;
	vLightPos = vLightPos-(vUp*bias) ;
	BuildViewMatrix(&m_matLView, &vLightPos, pvLightDir, &vUp) ;
	_CalculateBoundingPoint(&vMin, &vMax, &m_matLView, pcFrustum) ;//라이트뷰

	bias = m_fBiasFactor-(vMin.y/(vMax.y-vMin.y)) ;
	if(bias > 0.0f)
        m_fDistoNear = 1.0f - bias ;

    D3DXVECTOR4 vRet ;
	D3DXVec3Transform(&vRet, pvCamera, &m_matLView) ;
	v.x = vRet.x/vRet.w ;
	v.y = vRet.y/vRet.w ;
	v.z = vRet.z/vRet.w ;
	float fDist = v.y*1.0f/vMin.y ;

	//TRACE("vMin(%07.03f %07.03f %07.03f) vMax(%07.03f %07.03f %07.03f)\r\n", vMin.x, vMin.y, vMin.z, vMax.x, vMax.y, vMax.z) ;
	//TRACE("w=%07.03f h=%07.03f d=%07.03f\r\n", vMax.x-vMin.x, vMax.y-vMin.y, vMax.z-vMin.z) ;

	float w, h, zn, zf ;
	w = vMax.x-vMin.x ;
	h = vMax.y-vMin.y ;
	zn = vMin.z ;
	zf = vMax.z+50.0f ;

	m_sRenderTarget.fZNear = zn ;
	m_sRenderTarget.fZFar = zf ;

	//h = vMax.z-vMin.z ;
	//zn = vMin.y ;
	//zf = vMax.y+50.0f ;
	//너무 작아질경우 투영행렬이 안 만들어짐.
	if(float_less_eq(w, 0.0f))
		w = 0.1f ;
	if(float_less_eq(h, 0.0f))
		h = 0.1f ;

	D3DXMatrixOrthoLH(&m_matProj, w, h, zn, zf) ;

	//D3DXMATRIX mat ;

	//D3DXMatrixOrthoLH(&m_matProj, w, h, zn, zf) ;
	//mat = m_matLView * m_matProj ;
	//D3DXVec3Transform(&vRet, &vLookat, &mat) ;
	//TRACE("Ortho vRet(%07.03f %07.03f %07.03f %07.03f)\r\n", vRet.x, vRet.y, vRet.z, vRet.w) ;

	//D3DXMatrixPerspectiveLH(&m_matProj, w, h, zn, zf) ;
	//mat = m_matLView * m_matProj ;
	//D3DXVec3Transform(&vRet, &vLookat, &mat) ;
	//TRACE("Pers vRet(%07.03f %07.03f %07.03f %07.03f)\r\n", vRet.x, vRet.y, vRet.z, vRet.w) ;

	//D3DXMatrixPerspectiveFovLH(&m_matProj, 3.141592f/4.f, 1.0f, zn, zf) ;
	//mat = m_matLView * m_matProj ;
	//D3DXVec3Transform(&vRet, &vLookat, &mat) ;
	//TRACE("Fov vRet(%07.03f %07.03f %07.03f %07.03f)\r\n", vRet.x, vRet.y, vRet.z, vRet.w) ;

	D3DXMATRIX matTrans, mat ;
	//_ScaleTranslateToFit(&matTrans, &vMin, &vMax, m_fDistoNear, m_fDistoFar) ;
	BuildDistortionY(&mat, m_fDistoWidth, m_fDistoHeight, m_fDistoNear, m_fDistoFar) ;
	m_matProj = m_matProj*mat ;

	int ee=0 ;
}
void CSecretShadowMap::BuildLispSM(D3DXMATRIX *pmatLV, D3DXVECTOR3 *pvLightDir, D3DXVECTOR3 *pvCamera, D3DXMATRIX *pmatV, CSecretFrustum *pcFrustum)
{
	D3DXVECTOR4 vRet ;
	D3DXVECTOR3 v1, v2, v3, vMin, vMax, vBBoxCenter ;
	D3DXMATRIX mat, matLView, matInv ;

	//Calculate New Up Vector
	v2.x = pmatV->_13 ;
	v2.y = pmatV->_23 ;
	v2.z = pmatV->_33 ;
	_CalculateUpVector(&v1, &v2, pvLightDir) ;

	//temporal light view matrix
	v2.x = pvCamera->x+v2.x ;
	v2.y = pvCamera->y+v2.y ;
	v2.z = pvCamera->z+v2.z ;
	D3DXMatrixLookAtLH(&matLView, pvCamera, &v2, &v1) ;

	////Calculate Bounding Points in temporal light View Matrix
	//_CalculateBoundingPoint(&vMin, &vMax, &matLView, pcFrustum) ;
	//fBoundExtent = vMax.z-vMin.z ;

	//v1.x = pmatV->_13 ;
	//v1.y = pmatV->_23 ;
	//v1.z = pmatV->_33 ;

	//v2.x = pvLightDir->x ;
	//v2.y = pvLightDir->y ;
	//v2.z = pvLightDir->z ;

	//if(vector_eq(v1, v2))
	//	fSinGamma = 0.0f ;
	//else
	//{
	//	ftemp = D3DXVec3Dot(&v1, &v2) ;
	//	fSinGamma = acosf(ftemp) ;
	//}

	//_CalculateBoundingPoint(&vMin, &vMax, pmatV, pcFrustum) ;

	//float z0, z1, fNear_opt ;

	//z0 = -vMin.z ;
	//z1 = -(vMin.z + fBoundExtent*fSinGamma) ;

	//fNear_opt = fBoundExtent / (sqrt(z1/z0)-1.0f) ;

	float n=10.0f, f=20.0f, fDistfromBBoxCentertoCamera ;
	D3DXVECTOR3 vLispsmUp, vLispsmDir, vLispsmPos, vLispsmLookat ;

	//calculate central point of the bounding box in the world space
	D3DXMatrixIdentity(&mat) ;
	_CalculateBoundingPoint(&vMin, &vMax, &mat, pcFrustum) ;
	vBBoxCenter.x = vMin.x+(vMax.x-vMin.x)/2.0f ;
	vBBoxCenter.y = vMin.y+(vMax.y-vMin.y)/2.0f ;
	vBBoxCenter.z = vMin.z+(vMax.z-vMin.z)/2.0f ;

	//calculate new up vector
	vLispsmUp = -(*pvLightDir) ;
    
	//calculate right vector
	v3.x = vBBoxCenter.x-pvCamera->x ;
	v3.y = vBBoxCenter.y-pvCamera->y ;
	v3.z = vBBoxCenter.z-pvCamera->z ;
	D3DXVec3Normalize(&v3, &v3) ;
	D3DXVec3Cross(&v2, &vLispsmUp, &v3) ;
	D3DXVec3Normalize(&v2, &v2) ;
	//calculate dir
	D3DXVec3Cross(&vLispsmDir, &v2, &vLispsmUp) ;
	D3DXVec3Normalize(&vLispsmDir, &vLispsmDir) ;

	_CalculateBoundingPoint(&vMin, &vMax, pmatV, pcFrustum) ;

    fDistfromBBoxCentertoCamera = vMin.z+((vMax.z-vMin.z)*0.5f) ;
	fDistfromBBoxCentertoCamera = (fDistfromBBoxCentertoCamera<12.0f) ? 12.0f : fDistfromBBoxCentertoCamera ;

	g_cDebugMsg.SetDebugMsg(5, "dist=%g\r\n", fDistfromBBoxCentertoCamera) ;

	//a point closest the near plane in the view space
    v1.x = 0.0f ;
	v1.y = 0.0f ;
	v1.z = vMin.z ;
	D3DXMatrixInverse(&matInv, NULL, pmatV) ;
	mat = matInv*matLView ;//view -> world -> light
	D3DXVec3Transform(&vRet, &v1, &mat) ;//transform the point (view -> world -> light)
	v2.x = vRet.x/vRet.w ;
	v2.y = vRet.y/vRet.w ;
	v2.z = vRet.z/vRet.w ;

	//get closest point to the light pos in the light space
	_CalculateBoundingPoint(&vMin, &vMax, &matLView, pcFrustum) ;
    v2.z = vMin.z ;
	D3DXMatrixInverse(&matInv, NULL, &matLView) ;
	D3DXVec3Transform(&vRet, &v2, &matInv) ;//transform the point (light -> world)
	v1.x = vRet.x/vRet.w ;
	v1.y = vRet.y/vRet.w ;
	v1.z = vRet.z/vRet.w ;

	vLispsmPos.x = vBBoxCenter.x-(vLispsmDir.x*fDistfromBBoxCentertoCamera) ;
	vLispsmPos.y = vBBoxCenter.y-(vLispsmDir.y*fDistfromBBoxCentertoCamera) ;
	vLispsmPos.z = vBBoxCenter.z-(vLispsmDir.z*fDistfromBBoxCentertoCamera) ;

	//v2.x = vMin.x+(vMax.x-vMin.x)/2.0f ;
	//v2.y = vMin.y+(vMax.y-vMin.y)/2.0f ;
	//v2.z = vMin.z+(vMax.z-vMin.z)/2.0f ;
	//D3DXVec3Transform(&vRet, &v2, &matInv) ;
	//vLispsmLookat.x = vRet.x/vRet.w ;
	//vLispsmLookat.y = vRet.y/vRet.w ;
	//vLispsmLookat.z = vRet.z/vRet.w ;

	//vLispsmLookat = vLispsmPos+vLispsmDir ;
	//D3DXMatrixLookAtLH(&matLView, &vLispsmPos, &vLispsmLookat, &vLispsmUp) ;
	//vLispsmDir = vLispsmLookat-vLispsmPos ;
	//D3DXVec3Normalize(&vLispsmDir, &vLispsmDir) ;
    
    //vLispsmPos = vLispsmLookat-(vLispsmDir*fDistfromBBoxCentertoCamera) ;

	v1 = -vLispsmUp ;
	vLispsmUp = vLispsmDir ;
	vLispsmDir = v1 ;

	BuildViewMatrix(&matLView, &vLispsmPos, &vLispsmDir, &vLispsmUp) ;

	m_matLView = matLView ;

	//D3DXVec3Cross(&vRightDir, &v1, &v2) ;//calculate right vector

	//vUpDir = -(*pvLightDir) ;
	//D3DXVec3Cross(&vZDir, &vRightDir, &vUpDir) ;

}
void CSecretShadowMap::_CalculateUpVector(D3DXVECTOR3 *pvUp, D3DXVECTOR3 *pvCameraDir, D3DXVECTOR3 *pvLightDir)
{
	D3DXVECTOR3 vRightDir ;
	D3DXVec3Cross(&vRightDir, pvCameraDir, pvLightDir) ;
	D3DXVec3Cross(pvUp, pvLightDir, &vRightDir) ;
    D3DXVec3Normalize(pvUp, pvUp) ;
}
void CSecretShadowMap::_CalculateBoundingPoint(D3DXVECTOR3 *pvMin, D3DXVECTOR3 *pvMax, D3DXMATRIX *pmat, CSecretFrustum *pcFrustum)
{
	D3DXVECTOR4 vRet ;
	D3DXVECTOR3 vPos ;

	vPos.x = pcFrustum->m_pvBoundingPositions[0].x ;
	vPos.y = pcFrustum->m_pvBoundingPositions[0].y ;
	vPos.z = pcFrustum->m_pvBoundingPositions[0].z ;

	D3DXVec3Transform(&vRet, &vPos, pmat) ;
	vPos.x = vRet.x/vRet.w ;
	vPos.y = vRet.y/vRet.w ;
	vPos.z = vRet.z/vRet.w ;
	(*pvMin) = (*pvMax) = vPos ;

	for(int i=1 ; i<pcFrustum->m_nNumBoundingPosition ; i++)
	{
		vPos.x = pcFrustum->m_pvBoundingPositions[i].x ;
		vPos.y = pcFrustum->m_pvBoundingPositions[i].y ;
		vPos.z = pcFrustum->m_pvBoundingPositions[i].z ;

		D3DXVec3Transform(&vRet, &vPos, pmat) ;
		vPos.x = vRet.x/vRet.w ;
		vPos.y = vRet.y/vRet.w ;
		vPos.z = vRet.z/vRet.w ;

		if(pvMin->x > vPos.x) pvMin->x = vPos.x ;
		if(pvMin->y > vPos.y) pvMin->y = vPos.y ;
		if(pvMin->z > vPos.z) pvMin->z = vPos.z ;

		if(pvMax->x < vPos.x) pvMax->x = vPos.x ;
		if(pvMax->y < vPos.y) pvMax->y = vPos.y ;
		if(pvMax->z < vPos.z) pvMax->z = vPos.z ;
	}
}
//void CSecretShadowMap::BuildLightViewMatrix(D3DXMATRIX *pmatLView, D3DXMATRIX *pmatView, D3DXVECTOR3 *pvLightDir, CSecretFrustum *pcFrustum)
//{
//}
void CSecretShadowMap::BuildViewMatrix(D3DXMATRIX *pmatView, D3DXVECTOR3 *pvPos, D3DXVECTOR3 *pvDir, D3DXVECTOR3 *pvUp)
{
	D3DXVECTOR3 vAxisZ, vAxisY, vAxisX ;

	D3DXVec3Cross(&vAxisX, pvUp, pvDir) ;
	D3DXVec3Normalize(&vAxisX, &vAxisX) ;

	D3DXVec3Cross(&vAxisY, pvDir, &vAxisX) ;
	D3DXVec3Normalize(&vAxisY, &vAxisY) ;
	D3DXVec3Normalize(&vAxisZ, pvDir) ;

	pmatView->_11 = vAxisX.x ;
	pmatView->_12 = vAxisY.x ;
	pmatView->_13 = vAxisZ.x ;
	pmatView->_14 = 0.0f ;

	pmatView->_21 = vAxisX.y ;
	pmatView->_22 = vAxisY.y ;
	pmatView->_23 = vAxisZ.y ;
	pmatView->_24 = 0.0f ;

	pmatView->_31 = vAxisX.z ;
	pmatView->_32 = vAxisY.z ;
	pmatView->_33 = vAxisZ.z ;
	pmatView->_34 = 0.0f ;

	pmatView->_41 = -D3DXVec3Dot(&vAxisX, pvPos) ;
	pmatView->_42 = -D3DXVec3Dot(&vAxisY, pvPos) ;
	pmatView->_43 = -D3DXVec3Dot(&vAxisZ, pvPos) ;
	pmatView->_44 = 1.0f ;
}

CSecretShadowMapEffect::CSecretShadowMapEffect()
{
	m_pDecl = NULL ;
	m_pEffect = NULL ;
	m_hTechnique = NULL ;
}
CSecretShadowMapEffect::~CSecretShadowMapEffect()
{
	Release() ;
}
void CSecretShadowMapEffect::Release()
{
	SAFE_RELEASE(m_pEffect) ;
	SAFE_RELEASE(m_pDecl) ;
}
HRESULT CSecretShadowMapEffect::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName)
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