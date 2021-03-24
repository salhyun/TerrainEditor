#include "MathOrdinary.h"
#include "SecretWater.h"
#include "SecretTextureContainer.h"
#include "SecretMirror.h"
#include "SecretPicking.h"
#include "SecretTerrain.h"
#include "SecretSky.h"
#include "SecretCloud.h"
#include "SecretgizmoSystem.h"
#include "D3DMathLib.h"
#include "SecretShadowMap.h"
#include "SecretSceneManager.h"

D3DVERTEXELEMENT9 decl_Water[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//법선에 대한 설정
	{0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},//접선벡터 설정
	{0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정
	D3DDECL_END()
} ;

//##########################################################################
//SWaterKind
//##########################################################################
void SWaterKind::set(char *pstr, int pass)
{
    sprintf(szKindName, "%s", pstr) ;
	nPass = pass ;
}

SWaterSurface::SWaterSurface()
{
	bEnable = false ;
	nWidth = 0 ;
	nHeight = 0 ;
	nDelta = 0 ;
	vCenterPos.set(0, 0, 0) ;
	lNumVertex = lNumTriangle = 0 ;
	psVertices = NULL ;
	psTriangles = NULL ;
}

SWaterSurface::~SWaterSurface()
{
}
void SWaterSurface::Release()
{
	if(lNumVertex)
		SAFE_DELETEARRAY(psVertices) ;
	if(lNumTriangle)
		SAFE_DELETEARRAY(psTriangles) ;
}
void SWaterSurface::Initialize(int width, int height, int delta, Vector3 pos)
{
	bEnable = true ;
	nWidth = width ;
	nHeight = height ;
	nDelta = delta ;
    vCenterPos = pos ;

	lNumVertex = (nWidth/nDelta+1)*(nHeight/nDelta+1) ;
	lNumTriangle = (nWidth/nDelta)*(nHeight/nDelta)*2 ;

	psVertices = new SMeshVertex[lNumVertex] ;
    psTriangles = new SMeshTriangle[lNumTriangle] ;

	BuildVertexData() ;
}

void SWaterSurface::BuildVertexData()
{
	int x, z ;
	float fXOffset, fZOffset ;
	Vector3 vlt, vrb ;
	vlt.set(vCenterPos.x-(float)(nWidth/2), vCenterPos.y, vCenterPos.z+(float)(nHeight/2)) ;
	vrb.set(vCenterPos.x+(float)(nWidth/2), vCenterPos.y, vCenterPos.z-(float)(nHeight/2)) ;

	//fixed size of the texture coordinate 64
	float fFixedSize = 1.0f/64.0f ;

	SMeshVertex *pv=psVertices ;
	fZOffset=vlt.z ;
    for(z=0 ; z<=nHeight ; z += nDelta, fZOffset -= (float)nDelta)
	{
		fXOffset=vlt.x ;
		for(x=0 ; x<=nWidth ; x += nDelta, fXOffset += (float)nDelta)
		{
			pv->pos.set(fXOffset, vCenterPos.y, fZOffset) ;
			pv->normal.set(0, 1, 0) ;

			//pv->tex.set(x*1.0f/nWidth, z*1.0f/nHeight) ;
			pv->tex.u = x*fFixedSize ;
			pv->tex.v = z*fFixedSize ;

			//TRACE("pos(%06.3f %06.3f %06.3f), tex(%06.3f %06.3f)\r\n", enumVector(pv->pos), pv->tex.u, pv->tex.v) ;
			pv++ ;
		}
	}

	TRACE("\r\n") ;

	WORD wIndex[4] ;
	SMeshTriangle *ptri = psTriangles ;
	for(z=0 ; z<nHeight/nDelta ; z++)
	{
		for(x=0 ; x<nWidth/nDelta ; x++)
		{
			wIndex[0] = ((z)*(nWidth/nDelta+1))+(x) ;
			wIndex[1] = ((z)*(nWidth/nDelta+1))+(x+1) ;
			wIndex[2] = ((z+1)*(nWidth/nDelta+1))+(x) ;
			wIndex[3] = ((z+1)*(nWidth/nDelta+1))+(x+1) ;

			//TRACE("wIndex[0]=%03d, wIndex[1]=%03d wIndex[2]=%03d wIndex[3]=%03d\r\n", wIndex[0], wIndex[1], wIndex[2], wIndex[3]) ;

			ptri->anVertexIndex[0] = wIndex[0] ;
			ptri->anVertexIndex[1] = wIndex[1] ;
			ptri->anVertexIndex[2] = wIndex[2] ;
			ptri++ ;
			ptri->anVertexIndex[0] = wIndex[2] ;
			ptri->anVertexIndex[1] = wIndex[1] ;
			ptri->anVertexIndex[2] = wIndex[3] ;
			ptri++ ;
		}
	}

	MakeTangent(psVertices, lNumTriangle, psTriangles) ;
}

void SWaterSurface::BuildVertexData(SWaterVertex *psVertices, SWaterIndex *psIndices)
{
	Vector3 vlt, vrb ;
	vlt.set(vCenterPos.x-(float)(nWidth/2), vCenterPos.y-(float)(nWidth/2), vCenterPos.z) ;
	vrb.set(vCenterPos.x+(float)(nWidth/2), vCenterPos.y+(float)(nWidth/2), vCenterPos.z) ;

	int x, y ;
	float fXOffset, fYOffset ;

    SWaterVertex *pv=psVertices ;
	fYOffset=vlt.y ;
	for(y=0 ; y<=nHeight ; y += nDelta, fYOffset += (float)nDelta)
	{
		fXOffset=vlt.x ;
		for(x=0 ; x<=nWidth ; x += nDelta, fXOffset += (float)nDelta)
		{
			pv->pos = D3DXVECTOR3(fXOffset, fYOffset, vCenterPos.z) ;
			pv->normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f) ;
			pv->t = D3DXVECTOR2(x*1.0f/nWidth, y*1.0f/nHeight) ;

			//TRACE("pos(%06.3f %06.3f %06.3f), tex(%06.3f %06.3f)\r\n", enumVector(pv->pos), pv->t.x, pv->t.y) ;

			pv++ ;
		}
	}
	//TRACE("\r\n") ;

	WORD wIndex[4] ;

	SWaterIndex *pi=psIndices ;
    for(y=0 ; y<nHeight/nDelta ; y++)
	{
		for(x=0 ; x<nWidth/nDelta ; x++)
		{
			wIndex[0] = ((y)*(nWidth/nDelta+1))+(x) ;
			wIndex[1] = ((y)*(nWidth/nDelta+1))+(x+1) ;
			wIndex[2] = ((y+1)*(nWidth/nDelta+1))+(x) ;
			wIndex[3] = ((y+1)*(nWidth/nDelta+1))+(x+1) ;

			//TRACE("wIndex[0]=%03d, wIndex[1]=%03d wIndex[2]=%03d wIndex[3]=%03d\r\n", wIndex[0], wIndex[1], wIndex[2], wIndex[3]) ;

			pi->anIndex[0] = wIndex[0] ;
			pi->anIndex[1] = wIndex[1] ;
			pi->anIndex[2] = wIndex[2] ;
			pi++ ;
			pi->anIndex[0] = wIndex[0] ;
			pi->anIndex[1] = wIndex[2] ;
			pi->anIndex[2] = wIndex[3] ;
			pi++ ;
		}
	}
}

//##########################################################################
//CSecretWater
//##########################################################################

CSecretWater::CSecretWater()
{
    m_bWireFrame = false ;

	m_pVB = NULL ;
	m_pIB = NULL ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;

	m_pTexReflect = m_pTexRefraction = m_pTexRipple1Normal = m_pTexDetail = NULL ;
	m_pVolTexRipple1Normal = NULL ;
	m_pcMirror = NULL ;
	m_nStatus = 0 ;
	m_pcGizmo = NULL ;
	m_pcWaterManager = NULL ;
	m_psWaterKind = NULL ;
	//m_fElapsedTime = -431602080.0f ;

	//m_avBumpMove[0].set(-431602080.0f, -431602080.0f) ;
	m_avBumpMove[0].set(0, 0) ;
	m_avBumpMove[1]=m_avBumpMove[2]=m_avBumpMove[3]=m_avBumpMove[0] ;

	//m_avBumpMoveScalar[0].set(0.02f, 0.06f) ;
	//m_avBumpMoveScalar[1].set(0.02f, 0.04f) ;
	//m_avBumpMoveScalar[2].set(0.06f, 0.02f) ;
	//m_avBumpMoveScalar[3].set(0.04f, 0.02f) ;

	m_avBumpMoveScalar[0].set(0.2f, 0.6f) ;
	m_avBumpMoveScalar[1].set(0.2f, 0.4f) ;
	m_avBumpMoveScalar[2].set(0.6f, 0.2f) ;
	m_avBumpMoveScalar[3].set(0.4f, 0.2f) ;

	//m_avBumpMoveScalar[0] *= 0.25f ;
	//m_avBumpMoveScalar[1] *= 0.25f ;
	//m_avBumpMoveScalar[2] *= 0.25f ;
	//m_avBumpMoveScalar[3] *= 0.25f ;
	//m_avBumpMoveScalar[0] *= 0.09f ;
	//m_avBumpMoveScalar[1] *= 0.09f ;
	//m_avBumpMoveScalar[2] *= 0.09f ;
	//m_avBumpMoveScalar[3] *= 0.09f ;

	float speed=0.09f ;
	m_avBumpMoveScalar[0] *= speed ;
	m_avBumpMoveScalar[1] *= speed ;
	m_avBumpMoveScalar[2] *= speed ;
	m_avBumpMoveScalar[3] *= speed ;

	m_fTimeSinceLastFrame = 0 ;
	m_vTilingBumpTex = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f) ;

	D3DXMatrixIdentity(&m_matOriginScale) ;
	D3DXMatrixIdentity(&m_matOriginRot) ;
	D3DXMatrixIdentity(&m_matOriginTrans) ;
	D3DXMatrixIdentity(&m_matTransform) ;

	m_nNumReflectedTRObject = 0 ;
	m_bItHasNaverRendered = true ;

	//TRACE("CSecretWater construction\r\n") ;
}

CSecretWater::~CSecretWater()
{
}
void CSecretWater::Release()
{
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;

	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pIB) ;

	SAFE_DELETE(m_pcMirror) ;
	m_sWaterSurface.Release() ;

	//TRACE("CSecretRigidMesh destruction\r\n") ;
}
HRESULT CSecretWater::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszName, int nWidth, int nHeight, CSecretTerrain *pcTerrain, CSecretPicking *pcPicking, CSecretGizmoSystem *pcGizmo, CSecretWaterEffect *pcWaterEffect, SWaterKind *psWaterKind)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcTerrain = pcTerrain ;
	m_pcPicking = pcPicking ;
	m_pcGizmo = pcGizmo ;
	m_pcWaterEffect = pcWaterEffect ;
	m_psWaterKind = psWaterKind ;
	sprintf(m_szName, "%s", pszName) ;

	//m_sWaterSurface.BuildVertexData(m_psVertices, m_psIndices) ;

    Matrix4 matRot ;
    m_pcMirror = new CSecretMirror(Vector3(0, 0, 0), Vector3(0, 1, 0), (float)(nWidth*1), (float)(nHeight*1), matRot, pd3dDevice, true, true) ;
	m_pcMirror->SetRenderMethod(CSecretMirror::RENDER_BEHINDMIRROR) ;

	m_sWaterSurface.Initialize(nWidth, nHeight, 2, Vector3(0, 0, 0)) ;

	m_lNumVertex = m_sWaterSurface.lNumVertex ;
	m_lNumTriangle = m_sWaterSurface.lNumTriangle ;

	m_nWidth = nWidth ;
	m_nHeight = nHeight ;
	m_fSphereRadius = sqrt((float)(nWidth*nWidth + nHeight*nHeight)) ;

	//m_vTilingBumpTex.y = (float)nWidth/32.0f ;
	m_vTilingBumpTex.y = (float)128.0f/16.0f ;
    
	//m_psVertices = new SWaterVertex[m_lNumVertex] ;
    //m_psIndices = new SWaterIndex[m_lNumTriangle] ;
    
	int i ;

	//Create VertexData
	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SWaterVertex), D3DUSAGE_WRITEONLY, D3DFVF_WATERVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return E_FAIL ;

	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SWaterVertex), &pVertices, 0)))
		return E_FAIL ;

	SWaterVertex *psVertices = (SWaterVertex *)pVertices ;
	for(i=0 ; i<(int)m_lNumVertex ; i++)
	{
		COPYVECTOR3(psVertices->pos, m_sWaterSurface.psVertices[i].pos) ;
		COPYVECTOR3(psVertices->normal, m_sWaterSurface.psVertices[i].normal) ;
		COPYVECTOR3(psVertices->tangent, m_sWaterSurface.psVertices[i].tangent) ;
		psVertices->t.x = m_sWaterSurface.psVertices[i].tex.u ;
		psVertices->t.y = m_sWaterSurface.psVertices[i].tex.v ;

		psVertices++ ;
	}

	m_pVB->Unlock() ;

	///////////////////////////////////////////////////
	//Create Indices
	///////////////////////////////////////////////////
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SWaterIndex), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL)))
		return E_FAIL ;

	void *pIndices ;
	if(FAILED(m_pIB->Lock(0, m_lNumTriangle*sizeof(SWaterIndex), (void **)&pIndices, 0)))
		return E_FAIL ;

    SWaterIndex *psIndices = (SWaterIndex *)pIndices ;
	for(i=0 ; i<(int)m_lNumTriangle ; i++)
	{
		psIndices->anIndex[0] = m_sWaterSurface.psTriangles[i].anVertexIndex[0] ;
		psIndices->anIndex[1] = m_sWaterSurface.psTriangles[i].anVertexIndex[1] ;
		psIndices->anIndex[2] = m_sWaterSurface.psTriangles[i].anVertexIndex[2] ;

		//psIndices->anIndex[0] = m_psIndices[i].anIndex[0] ;
		//psIndices->anIndex[1] = m_psIndices[i].anIndex[1] ;
		//psIndices->anIndex[2] = m_psIndices[i].anIndex[2] ;

//		TRACE("[%d] index[0]=%d index[1]=%d index[2]=%d\r\n", i, psIndices->anIndex[0], psIndices->anIndex[1], psIndices->anIndex[2]) ;
		psIndices++ ;
	}
	m_pIB->Unlock() ;

	m_bItHasNaverRendered = true ;

	return true ;
}
void CSecretWater::Process(bool bMouseLButtonEvent, bool bMouseLButtonDirect)
{
	if(m_nStatus == CSecretWater::PICKING)
	{
        if(bMouseLButtonEvent)
			SetStatus(CSecretWater::SELECTED) ;

		m_pcMirror->SetMirrorPos(m_pcPicking->GetCurVertex()->pos) ;
		m_matTransform._41 = m_pcPicking->GetCurVertex()->pos.x ;
		m_matTransform._42 = m_pcPicking->GetCurVertex()->pos.y ;
		m_matTransform._43 = m_pcPicking->GetCurVertex()->pos.z ;
	}
	else if(m_nStatus == CSecretWater::SELECTED)
	{
		Vector3 vCameraPos ;
		vCameraPos.set(m_pcWaterEffect->m_psEssentialElements->vCameraPos.x, m_pcWaterEffect->m_psEssentialElements->vCameraPos.y, m_pcWaterEffect->m_psEssentialElements->vCameraPos.z) ;
		m_pcGizmo->Process(&vCameraPos, &m_pcPicking->m_sLine, &m_pcWaterEffect->m_psEssentialElements->matView, &m_pcWaterEffect->m_psEssentialElements->matProj, bMouseLButtonDirect) ;

		if(m_pcGizmo->m_sScrape.GetStatus() == SGizmoScrape::SCRAPE_START)
		{
			D3DXMatrixDecomposeTranslation(&m_matOriginTrans, &m_matTransform) ;
			D3DXMatrixDecomposeRotation(&m_matOriginRot, &m_matTransform) ;
			D3DXMatrixDecomposeScaling(&m_matOriginScale, &m_matTransform) ;
		}
		else if(m_pcGizmo->m_sScrape.GetStatus() == SGizmoScrape::SCRAPE_SCRAPING)
		{
			if(m_pcGizmo->m_sGizmoMoveGeometry.dwSelectedSubset == GIZMOSUBSET_AXISX)
			{
				Vector3 vPos = m_pcMirror->GetCenterPos() ;
				vPos.x += m_pcGizmo->m_sScrape.vAmount.x ;
				m_pcMirror->SetMirrorPos(vPos) ;
				m_matTransform._41 = vPos.x ;
			}
			else if(m_pcGizmo->m_sGizmoMoveGeometry.dwSelectedSubset == GIZMOSUBSET_AXISY)
			{
				Vector3 vPos = m_pcMirror->GetCenterPos() ;
				vPos.y += m_pcGizmo->m_sScrape.vAmount.y ;
				m_pcMirror->SetMirrorPos(vPos) ;
				m_matTransform._42 = vPos.y ;
			}
			else if(m_pcGizmo->m_sGizmoMoveGeometry.dwSelectedSubset == GIZMOSUBSET_AXISZ)
			{
				Vector3 vPos = m_pcMirror->GetCenterPos() ;
				vPos.z += m_pcGizmo->m_sScrape.vAmount.z ;
				m_pcMirror->SetMirrorPos(vPos) ;
				m_matTransform._43 = vPos.z ;
			}
			else if(m_pcGizmo->m_sGizmoRotateGeometry.dwSelectedSubset == GIZMOSUBSET_ROTATEBYAXISX
				|| m_pcGizmo->m_sGizmoRotateGeometry.dwSelectedSubset == GIZMOSUBSET_ROTATEBYAXISY
				|| m_pcGizmo->m_sGizmoRotateGeometry.dwSelectedSubset == GIZMOSUBSET_ROTATEBYAXISZ)
			{
				m_matTransform = m_matOriginScale * m_matOriginRot * m_pcGizmo->m_sGizmoRotateGeometry.matRotate * m_matOriginTrans ;
			}
		}
	}
}
void CSecretWater::ProcessBump()
{
	for(int i=0 ; i<4 ; i++)
		m_avBumpMove[i] += (m_avBumpMoveScalar[i]*m_fTimeSinceLastFrame) ;
}
void CSecretWater::Render()
{
	HRESULT hr ;
	if(m_lNumTriangle <= 0)
		return ;

	//m_pcMirror->Render() ;

	m_pcWaterEffect->m_pEffect->SetTechnique(m_pcWaterEffect->m_hTechnique) ;
	m_pcWaterEffect->m_pEffect->Begin(NULL, 0) ;
	
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pcWaterEffect->m_pEffect->BeginPass(m_psWaterKind->nPass) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//set shader//
	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Water, &m_pcWaterEffect->m_pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretWater::Render()") ;

	D3DXMATRIX mat ;
	D3DXVECTOR4 v ;

	v.x = v.y = v.z = 0.7f  ;
	v.w = 1.0f ;
	m_pcWaterEffect->m_pEffect->SetVector(m_pcWaterEffect->m_hvDiffuse, &v) ;

	//m_pcWaterEffect->m_pEffect->SetVector(m_pcWaterEffect->m_hvCameraPos, &m_pcWaterEffect->m_psEssentialElements->vCameraPos) ;
	m_pcWaterEffect->m_pEffect->SetVector(m_pcWaterEffect->m_hvCameraPos, &m_vCamera) ;

	m_pcWaterEffect->m_pEffect->SetVector(m_pcWaterEffect->m_hvSunDir, &m_pcWaterEffect->m_psEssentialElements->vSunDir) ;

    D3DXMatrixIdentity(&mat) ;
	mat._41 = m_pcMirror->GetCenterPos().x ;
	mat._42 = m_pcMirror->GetCenterPos().y ;
	mat._43 = m_pcMirror->GetCenterPos().z ;

	//matWorld
	//m_pcWaterEffect->m_pEffect->SetMatrix(m_pcWaterEffect->m_hmatWorld, &mat) ;
	m_pcWaterEffect->m_pEffect->SetMatrix(m_pcWaterEffect->m_hmatWorld, &m_matTransform) ;

	//mat = mat * m_pcWaterEffect->m_psEssentialElements->matView * m_pcWaterEffect->m_psEssentialElements->matProj ;
	mat = m_matTransform * m_pcWaterEffect->m_psEssentialElements->matView * m_pcWaterEffect->m_psEssentialElements->matProj ;
	m_pcWaterEffect->m_pEffect->SetMatrix(m_pcWaterEffect->m_hmatWVP, &mat) ;

	mat = m_matTransform * m_matLODView * m_pcWaterEffect->m_psEssentialElements->matProj ;
	m_pcWaterEffect->m_pEffect->SetMatrix(m_pcWaterEffect->m_hmatLODWVP, &mat) ;

	m_pcWaterEffect->m_pEffect->SetFloat(m_pcWaterEffect->m_hfProjOffset, m_pcWaterEffect->m_fProjOffset) ;

    //BumpMove
    D3DXVECTOR4 avBumpMove[2] ;

	avBumpMove[0].x = m_avBumpMove[0].x ;
	avBumpMove[0].y = m_avBumpMove[0].y ;
	avBumpMove[0].z = m_avBumpMove[1].x ;
	avBumpMove[0].w = m_avBumpMove[1].y ;
	avBumpMove[1].x = m_avBumpMove[2].x ;
	avBumpMove[1].y = m_avBumpMove[2].y ;
	avBumpMove[1].z = m_avBumpMove[3].x ;
	avBumpMove[1].w = m_avBumpMove[3].y ;

	m_pcWaterEffect->m_pEffect->SetVectorArray(m_pcWaterEffect->m_havBumpMove, avBumpMove, 2) ;

	//TilingBumpTex
	m_pcWaterEffect->m_pEffect->SetVector(m_pcWaterEffect->m_hvTilingBumpTex, &m_vTilingBumpTex) ;

	//ElapsedTime
	float fElapsedTime = m_psParam->fElapsedTime/2.0f ;
	m_pcWaterEffect->m_pEffect->SetFloat(m_pcWaterEffect->m_hfElapsedTime, fElapsedTime) ;

	//ScreenPos Offset
	m_pcWaterEffect->m_pEffect->SetFloat(m_pcWaterEffect->m_hfScreenPosOffset, m_psParam->fScreenPosOffset) ;

	//SunStrength
	m_pcWaterEffect->m_pEffect->SetFloat(m_pcWaterEffect->m_hfSunStrength, m_psParam->fSunStrength) ;

	//SunShininess
	m_pcWaterEffect->m_pEffect->SetFloat(m_pcWaterEffect->m_hfSunShininess, m_psParam->fSunShininess) ;

	m_pcWaterEffect->m_pEffect->SetTexture("DetailMap", m_pTexDetail) ;
	m_pcWaterEffect->m_pEffect->SetTexture("Ripple1NormalMap", m_pTexRipple1Normal) ;
	m_pcWaterEffect->m_pEffect->SetTexture("VolRipple1NormalMap", m_pcWaterManager->m_pVolTexRipple1Normal) ;
	m_pcWaterEffect->m_pEffect->SetTexture("ReflectMap", m_pcMirror->GetTexReflect()) ;
	m_pcWaterEffect->m_pEffect->SetTexture("UnderWaterMap", m_pcMirror->GetTexBehind()) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	m_pd3dDevice->SetVertexDeclaration(m_pcWaterEffect->m_pDecl) ;
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SWaterVertex)) ;
	m_pd3dDevice->SetIndices(m_pIB) ;

	m_pcWaterEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;
	g_cDebugMsg.AddTriangles(m_lNumTriangle) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	m_pcWaterEffect->m_pEffect->EndPass() ;

	m_pcWaterEffect->m_pDecl->Release() ;
	m_pcWaterEffect->m_pDecl = NULL ;

	m_pcWaterEffect->m_pEffect->End() ;

	if(m_bItHasNaverRendered)
		m_bItHasNaverRendered = false ;
}
void CSecretWater::SetStatus(int nStatus)
{
	//previous work

	m_nStatus = nStatus ;

	if(m_nStatus == CSecretWater::PICKING)
	{
	}
	else if(m_nStatus == CSecretWater::SELECTED)
	{
		m_pcGizmo->SetEnable(true) ;
		//m_pcGizmo->SetType(CSecretGizmoSystem::GIZMOTYPE::MOVE) ;

        D3DXMATRIX mat ;
		D3DXMatrixIdentity(&mat) ;
		mat._41 = m_pcMirror->GetCenterPos().x ;
		mat._42 = m_pcMirror->GetCenterPos().y ;
		mat._43 = m_pcMirror->GetCenterPos().z ;
        m_pcGizmo->SetmatWorld(&mat) ;
		Vector3 vCameraPos ;
		vCameraPos.set(m_pcWaterEffect->m_psEssentialElements->vCameraPos.x, m_pcWaterEffect->m_psEssentialElements->vCameraPos.y, m_pcWaterEffect->m_psEssentialElements->vCameraPos.z) ;
		m_pcGizmo->Process(&vCameraPos, &m_pcPicking->m_sLine, &m_pcWaterEffect->m_psEssentialElements->matView, &m_pcWaterEffect->m_psEssentialElements->matProj, false) ;
	}
}
int CSecretWater::GetStatus()
{
	return m_nStatus ;
}
D3DXMATRIX *CSecretWater::GetTransform()
{
	return &m_matTransform ;
}
void CSecretWater::SetTransform(D3DXMATRIX *pmatTransform)
{
	m_matTransform = *pmatTransform ;
	m_pcMirror->SetMirrorPos(Vector3(m_matTransform._41, m_matTransform._42, m_matTransform._43)) ;
}
void CSecretWater::ResetReflectedTRObject()
{
	m_nNumReflectedTRObject = 0 ;

	float fHalfWidth = m_sWaterSurface.nWidth * 0.5f ;
	float fHalfHeight = m_sWaterSurface.nHeight * 0.5f ;

	m_vBoundingPos[0].set(-fHalfWidth, 0, fHalfHeight) ;//left-top
	m_vBoundingPos[1].set(fHalfWidth, 0, fHalfHeight) ;//right-top
	m_vBoundingPos[2].set(-fHalfWidth, 0, -fHalfHeight) ;//left-bottom
	m_vBoundingPos[3].set(fHalfWidth, 0, -fHalfHeight) ;//right-bottom

	Matrix4 mat = MatrixConvert(m_matTransform) ;

	m_vBoundingPos[0] *= mat ;
	m_vBoundingPos[1] *= mat ;
	m_vBoundingPos[2] *= mat ;
	m_vBoundingPos[3] *= mat ;

	m_sCollisionSphere.set(Vector3(m_matTransform._41, m_matTransform._42, m_matTransform._43), m_fSphereRadius) ;

	m_asCollisionTriangles[0].set(m_vBoundingPos[0], m_vBoundingPos[1], m_vBoundingPos[2]) ;
	m_asCollisionTriangles[1].set(m_vBoundingPos[2], m_vBoundingPos[1], m_vBoundingPos[3]) ;

	m_sCollisionPlane.set(m_sCollisionSphere.vPos, Vector3(m_matTransform._12, m_matTransform._22, m_matTransform._32)) ;
}
bool CSecretWater::Intersect(geo::SLine *psLine, Vector3 &vIntersect, bool bLineTest)
{
	if(bLineTest)
	{
		if(IntersectLinetoTriangle(*psLine, m_asCollisionTriangles[0], vIntersect, true, true, FLT_EPSILON) == geo::INTERSECT_POINT)
			return true ;

		if(IntersectLinetoTriangle(*psLine, m_asCollisionTriangles[1], vIntersect, true, true, FLT_EPSILON) == geo::INTERSECT_POINT)
			return true ;
	}
	else
	{
		if(isInWaterRect(psLine->GetEndPos()))
		{
			if(m_asCollisionTriangles[0].sPlane.ToPosition(psLine->GetEndPos()) < 0.0f)
				return true ;

			if(IntersectLinetoTriangle(*psLine, m_asCollisionTriangles[0], vIntersect, true, true, FLT_EPSILON) == geo::INTERSECT_POINT)
				return true ;

			if(IntersectLinetoTriangle(*psLine, m_asCollisionTriangles[1], vIntersect, true, true, FLT_EPSILON) == geo::INTERSECT_POINT)
				return true ;
		}
	}
	return false ;
}
bool CSecretWater::IsUnderWater(D3DXVECTOR3 *p)
{
	D3DXVECTOR4 vResult ;
	D3DXVec3Transform(&vResult, p, &m_matTransform) ;

	if(float_abs(vResult.x) > (m_sWaterSurface.nWidth*0.5f) || float_abs(vResult.z) > (m_sWaterSurface.nHeight*0.5f))
	//if((vPos - m_sCollisionSphere.vPos).Magnitude() <= m_sCollisionSphere.fRadius)
	{
		if(m_sCollisionPlane.Classify(Vector3(p->x, p->y, p->z)) == geo::SPlane::SPLANE_BEHIND)
			return true ;
	}
	return false ;
}
bool CSecretWater::isInWaterRect(Vector3 p)
{
	geo::SPlane *psPlane = &m_asCollisionTriangles[0].sPlane ;
	p = psPlane->GetOnPos(p) ;

	Vector3 a, b, c ;

	a = p - m_vBoundingPos[0] ;
	b = m_vBoundingPos[1] - m_vBoundingPos[0] ;
	c = ProjectionVector(a, b) ;
	if(c.IsZero() || c.Magnitude() > b.Magnitude())
		return false ;

	a = p - m_vBoundingPos[1] ;
	b = m_vBoundingPos[3] - m_vBoundingPos[1] ;
	c = ProjectionVector(a, b) ;
	if(c.IsZero() || c.Magnitude() > b.Magnitude())
		return false ;

	a = p - m_vBoundingPos[3] ;
	b = m_vBoundingPos[2] - m_vBoundingPos[3] ;
	c = ProjectionVector(a, b) ;
	if(c.IsZero() || c.Magnitude() > b.Magnitude())
		return false ;

	a = p - m_vBoundingPos[2] ;
	b = m_vBoundingPos[0] - m_vBoundingPos[2] ;
	c = ProjectionVector(a, b) ;
	if(c.IsZero() || c.Magnitude() > b.Magnitude())
		return false ;

	return true ;
}
void CSecretWater::getWaterPlane(geo::SPlane *psPlane)
{
	*psPlane = m_asCollisionTriangles[0].sPlane ;
}

void CSecretWater::SelectReflectedTRObject(STrueRenderingObject *psTRObject)
{
	//if(!strstr("00_soo_s1_7m_10412", psTRObject->asCStrTreeItem[0].str))
	//	return ;
	if(m_nNumReflectedTRObject >= MAXNUM_REFLECTEDOBJECT)
		return ;

	if((psTRObject->matWorld._41 >= m_sWaterSurface.vMin.x) && (psTRObject->matWorld._41 <= m_sWaterSurface.vMax.x)
		&& (psTRObject->matWorld._43 >= m_sWaterSurface.vMin.z) && (psTRObject->matWorld._43 <= m_sWaterSurface.vMax.z))
	{
		m_apsReflectedTRObjects[m_nNumReflectedTRObject++] = psTRObject ;
	}
}
int CSecretWater::GetNumReflectedTRObject()
{
	return m_nNumReflectedTRObject ;
}
STrueRenderingObject **CSecretWater::GetReflectedTRObject()
{
	return m_apsReflectedTRObjects ;
}
bool CSecretWater::IsInFrustum()
{
	if(m_pcTerrain->m_pcFrustum->IsIn(m_pcMirror->GetCenterPos()))//중심점이 프로스텀뷰안인지 검사
		return true ;

	int i ;
	float fWidth = (float)m_nWidth/2.0f, fHeight = (float)m_nHeight/2.0f ;

	Vector3 avPos[4] ;
	Vector3 vAxisX(m_matTransform._11, m_matTransform._21, m_matTransform._31), vAxisZ(m_matTransform._13, m_matTransform._23, m_matTransform._33) ;
	Vector3 vCenter(m_matTransform._41, m_matTransform._42, m_matTransform._43) ;

	avPos[0] = vCenter - (vAxisX*fWidth) + (vAxisZ*fHeight) ;//left-top
	avPos[1] = vCenter + (vAxisX*fWidth) + (vAxisZ*fHeight) ;//right-top
	avPos[2] = vCenter + (vAxisX*fWidth) - (vAxisZ*fHeight) ;//right-bottom
	avPos[3] = vCenter - (vAxisX*fWidth) - (vAxisZ*fHeight) ;//left-bottom

	m_sWaterSurface.vMin.set(FLT_MAX, FLT_MAX, FLT_MAX) ;
	m_sWaterSurface.vMax.set(FLT_MIN, FLT_MIN, FLT_MIN) ;
	for(i=0 ; i<4 ; i++)
	{
		if(avPos[i].x < m_sWaterSurface.vMin.x)
			m_sWaterSurface.vMin.x = avPos[i].x ;
		if(avPos[i].y < m_sWaterSurface.vMin.y)
			m_sWaterSurface.vMin.y = avPos[i].y ;
		if(avPos[i].z < m_sWaterSurface.vMin.z)
			m_sWaterSurface.vMin.z = avPos[i].z ;

		if(avPos[i].x > m_sWaterSurface.vMax.x)
			m_sWaterSurface.vMax.x = avPos[i].x ;
		if(avPos[i].y > m_sWaterSurface.vMax.y)
			m_sWaterSurface.vMax.y = avPos[i].y ;
		if(avPos[i].z > m_sWaterSurface.vMax.z)
			m_sWaterSurface.vMax.z = avPos[i].z ;
	}

	if( (m_sWaterSurface.vMin.x < m_vCamera.x) && (m_sWaterSurface.vMax.x > m_vCamera.x)
		&& (m_sWaterSurface.vMin.z < m_vCamera.z) && (m_sWaterSurface.vMax.z > m_vCamera.z) )//카메라가 물위에 있을경우
		return true ;

	for(i=0 ; i<4 ; i++)
	{
		if(m_pcTerrain->m_pcFrustum->IsIn(avPos[i]))
			return true ;
	}
	return false ;
}

//void CSecretWater::SetTextures(UINT lNumTexDecale, char **ppszTexDecale, UINT lNumTexNormal, char **ppszTexNormal)
//{
//	UINT i ;
//	for(i=0 ; i<m_lNumTexDecale; i++)
//		sprintf(m_aszTexDecale[i], "%s", ppszTexDecale[i]) ;
//
//	for(i=0 ; i<m_lNumTexNormal; i++)
//		sprintf(m_aszTexNormal[i], "%s", ppszTexNormal[i]) ;
//}

//##########################################################################
//SWaterEffectParameter
//##########################################################################

SWaterEffectParameter::SWaterEffectParameter()
{
	fElapsedTime = 0.0f ;
	fScreenPosOffset = 0.1f ;
	fSunStrength = 5270.0f ;
	fSunShininess = 700.0f ;
}

//##########################################################################
//CSecretWaterManager
//##########################################################################

CSecretWaterManager::CSecretWaterManager() : MAXNUM_WATER(4)
{
	m_pd3dDevice=NULL ;
	m_pcTexContainer=NULL ;
	m_pcCurrentWater=NULL ;
	m_pcPicking=NULL ;
	m_pcGizmo = NULL ;
	m_pVolTexRipple1Normal = NULL ;

	m_pcTerrain = NULL ;	
	m_psEffect = NULL ;
	m_pcTerrainEffect = NULL ;
	m_pcSkyEffect = NULL ;
	m_pcSky = NULL ;
	m_pcCloudEffect = NULL ;
	m_pcCloud = NULL ;
	m_pcRenderingGroup = NULL ;
	m_pcCurrentWater = NULL ;

	m_nNumWaterKind = 0 ;
	m_psWaterKind = NULL ;
	m_nStatus = CSecretWaterManager::IDLING ;
}
CSecretWaterManager::~CSecretWaterManager()
{
	Release() ;
}
void CSecretWaterManager::Release()
{
	for(int i=0 ; i<m_cWater.nCurPos ; i++)
		m_cWater.GetAt(i)->Release() ;
	m_cWater.Reset() ;

	SAFE_DELETE(m_pcTexContainer) ;
	m_pcCurrentWater = NULL ;
	SAFE_DELETE(m_pcGizmo) ;

	SAFE_RELEASE(m_pVolTexRipple1Normal) ;

	SAFE_DELETEARRAY(m_psWaterKind) ;
}
void CSecretWaterManager::_InitKind()
{
    m_nNumWaterKind = 2 ;
	m_psWaterKind = new SWaterKind[m_nNumWaterKind] ;

	m_psWaterKind[0].set("River", CSecretWaterManager::RIVER) ;
	m_psWaterKind[1].set("Lake", CSecretWaterManager::LAKE) ;
}
bool CSecretWaterManager::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretWaterEffect *pcWaterEffect)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcWaterEffect = pcWaterEffect ;

	m_cWater.Initialize(MAXNUM_WATER) ;

	_InitKind() ;

	m_pcTexContainer = new CSecretTextureContainer() ;
	m_pcTexContainer->Initialize("../../Media/map/Water/Water.txr", pd3dDevice) ;

    if(FAILED(D3DXCreateVolumeTextureFromFile(m_pd3dDevice, "../../Media/map/Water/WaterBumps4_NM.dds", &m_pVolTexRipple1Normal)))
	{
		assert(false && "cannot load WaterBump4_NM.dds") ;
		return false ;
	}

	m_pcGizmo = new CSecretGizmoSystem() ;
	m_pcGizmo->Initialize(m_pd3dDevice) ;
	m_pcGizmo->SetEnable(false) ;

	return true ;
}
void CSecretWaterManager::Process(bool bMouseLButtonEvent, bool bMouseLButtonDirect)
{
	m_sParam.fElapsedTime += m_fTimeSinceLastFrame ;

	Vector3 p ;
	CSecretWater *pcWater ;
	CSecretMirror *pcMirror ;
	int nNumReflectedTRObject ;
	for(int i=0 ; i<m_cWater.nCurPos ; i++)
	{
		pcWater = m_cWater.GetAt(i) ;
		pcWater->m_fTimeSinceLastFrame = m_fTimeSinceLastFrame ;
		pcWater->m_vCamera = m_pcTerrainEffect->m_psEssentialElements->vCameraPos ;

		//render mirror to render_target
		pcMirror = pcWater->GetMirror() ;
		p.set(m_pcTerrainEffect->m_psEssentialElements->vCameraPos.x, m_pcTerrainEffect->m_psEssentialElements->vCameraPos.y, m_pcTerrainEffect->m_psEssentialElements->vCameraPos.z) ;


		if(!pcWater->IsInFrustum())
		{
			pcWater->m_bRenderingNow = false ;
			continue ;
		}

		if(((p-pcMirror->GetCenterPos()).Magnitude() < 200.0f) && !pcWater->m_bItHasNaverRendered)
		{
			pcWater->m_bRenderingNow = true ;
			nNumReflectedTRObject = pcWater->GetNumReflectedTRObject() ;
		}
		else
		{
			pcWater->m_bRenderingNow = false ;
			nNumReflectedTRObject = 0 ;
		}

		pcWater->m_matLODView = m_pcTerrainEffect->m_psEssentialElements->matView ;

		if(m_nStatus == CSecretWaterManager::PROCESSING)
			pcWater->Process(bMouseLButtonEvent, bMouseLButtonDirect) ;

		pcWater->ProcessBump() ;

		STrueRenderingObject **psTRObjects = pcWater->GetReflectedTRObject() ;

		pcMirror->SetRenderMethod(CSecretMirror::RENDER_MIRROR) ;
		pcMirror->Process(Vector3(m_pcTerrainEffect->m_psEssentialElements->vCameraPos.x, m_pcTerrainEffect->m_psEssentialElements->vCameraPos.y, m_pcTerrainEffect->m_psEssentialElements->vCameraPos.z)) ;
		pcMirror->RenderVirtualCamera(
			pcMirror->GetmatReflectView(),
			pcMirror->GetmatReflectProj(),
			pcMirror->GetTexReflect(),
			m_psEffect,
			m_pcTerrainEffect,
			m_pcTerrain,
			//m_pcSkyEffect,
			//m_pcSky,
			//m_pcCloudEffect,
			//m_pcCloud,
			m_pcSkyDomeEffect,
			m_pcSkyTop,
			m_pcSkyLower,
			m_pcRenderingGroup,
			&psTRObjects,
			nNumReflectedTRObject) ;

		pcMirror->SetRenderMethod(CSecretMirror::RENDER_BEHINDMIRROR, true) ;

		//if((p.y-pcMirror->GetCenterPos().y) < 15.0f)
		//	p.y = pcMirror->GetCenterPos().y+15.0f ;

		pcMirror->Process(p) ;
		//pcMirror->Process(Vector3(m_pcTerrainEffect->m_psEssentialElements->vCameraPos.x, pcMirror->GetCenterPos().y+45.0f, m_pcTerrainEffect->m_psEssentialElements->vCameraPos.z)) ;
		pcMirror->RenderVirtualCamera(
			pcMirror->GetmatReflectView(),
			pcMirror->GetmatReflectProj(),
			pcMirror->GetTexBehind(),
			m_psEffect,
			m_pcTerrainEffect,
			m_pcTerrain,
			//m_pcSkyEffect,
			//m_pcSky,
			//m_pcCloudEffect,
			//m_pcCloud,
			m_pcSkyDomeEffect,
			m_pcSkyTop,
			m_pcSkyLower,
			m_pcRenderingGroup,//) ;
			&psTRObjects,
			nNumReflectedTRObject) ;
	}
}
void CSecretWaterManager::Render()
{
	for(int i=0 ; i<m_cWater.nCurPos ; i++)
	{
		m_cWater.GetAt(i)->Render() ;
		//m_cWater.GetAt(i)->GetMirror()->Render() ;
	}

    if(m_nStatus == CSecretWaterManager::PROCESSING)
		m_pcGizmo->Render() ;
}
void CSecretWaterManager::CreateWater(char *pszName, int nWidth, int nHeight, char *pszWaterKind)
{
	int nWaterKind = FindWaterKind(pszWaterKind) ;
	if(nWaterKind < 0)
		return ;

	nWidth -= (nWidth%2) ;
	nHeight -= (nHeight%2) ;

	CSecretWater cWater ;
	cWater.Initialize(m_pd3dDevice, pszName, nWidth, nHeight, m_pcTerrain, m_pcPicking, m_pcGizmo, m_pcWaterEffect, &m_psWaterKind[nWaterKind]) ;
	cWater.m_pTexRipple1Normal = m_pcTexContainer->FindTexture("WaterBumps3_NM") ;
	cWater.m_pTexDetail = m_pcTexContainer->FindTexture("WaterDetails") ;
	cWater.m_psParam = &m_sParam ;
	cWater.m_pcWaterManager = this ;

	cWater.m_vCamera = m_psEffect->vCameraPos ;
	cWater.m_matLODView = m_psEffect->matView ;

	m_cWater.Insert(&cWater) ;
}
CSecretWater *CSecretWaterManager::SelectWater(int nIndex)
{
	m_pcCurrentWater = m_cWater.GetAt(nIndex) ;
	return m_pcCurrentWater ;
}
void CSecretWaterManager::DeleteWater(int nIndex)
{
	CSecretWater *pcWater = m_cWater.Delete(nIndex) ;
	if(pcWater)
	{
		pcWater->Release() ;
		if(pcWater == m_pcCurrentWater)
            m_pcCurrentWater = NULL ;

		m_pcGizmo->SetEnable(false) ;
	}
}
int CSecretWaterManager::FindWaterKind(char *psz)
{
	for(int i=0 ; i<m_nNumWaterKind ; i++)
	{
		if(!strcmp(m_psWaterKind[i].szKindName, psz))
			return i ;
	}
	return -1 ;
}
void CSecretWaterManager::ImportWater(char *pszName, D3DXMATRIX matTransform, int nWidth, int nHeight, char *pszWaterKind)
{
	CreateWater(pszName, nWidth, nHeight, pszWaterKind) ;
	CSecretWater *pcWater = m_cWater.GetAt(m_cWater.nCurPos-1) ;
	//pcWater->GetMirror()->SetMirrorPos(vPos) ;
	pcWater->SetTransform(&matTransform) ;
	pcWater->ResetReflectedTRObject() ;
	pcWater->SetStatus(CSecretWater::UNSELECTED) ;
}
CSecretWater *CSecretWaterManager::GetWater(int nIndex)
{
	return m_cWater.GetAt(nIndex) ;
}
int CSecretWaterManager::GetNumWater()
{
	return m_cWater.nCurPos ;
}
void CSecretWaterManager::ResetWater()
{
	for(int i=0 ; i<m_cWater.nCurPos ; i++)
		m_cWater.GetAt(i)->Release() ;
	m_cWater.Reset() ;
	m_pcCurrentWater = NULL ;
	m_pcGizmo->SetEnable(false) ;
}
void CSecretWaterManager::SetStatus(int nStatus)
{
	m_nStatus = nStatus ;
}
int CSecretWaterManager::GetStatus()
{
	return m_nStatus ;
}

void CSecretWaterManager::ResetReflectedTRObject()
{
	for(int i=0 ; i<m_cWater.nCurPos ; i++)
		m_cWater.GetAt(i)->ResetReflectedTRObject() ;
}
void CSecretWaterManager::SelectReflectedTRObject(STrueRenderingObject *psTRObject)
{
	for(int i=0 ; i<m_cWater.nCurPos ; i++)
		m_cWater.GetAt(i)->SelectReflectedTRObject(psTRObject) ;
}
int CSecretWaterManager::Intersect(geo::SLine *psLine, Vector3 &vIntersect, bool bLineTest)
{
	for(int i=0 ; i<m_cWater.nCurPos ; i++)
	{
		if(m_cWater.GetAt(i)->Intersect(psLine, vIntersect, bLineTest))
			return i ;
	}
	return -1 ;
}
int CSecretWaterManager::UnderWater(Vector3 &vPos)
{
	D3DXVECTOR3 pos(vPos.x, vPos.y, vPos.z) ;

	for(int i=0 ; i<m_cWater.nCurPos ; i++)
	{
		if(m_cWater.GetAt(i)->IsUnderWater(&pos))
			return i ;
	}
	return -1 ;
}
int CSecretWaterManager::isInWaterRect(Vector3 &vPos)
{
	for(int i=0 ; i<m_cWater.nCurPos ; i++)
	{
		if(m_cWater.GetAt(i)->isInWaterRect(vPos))
			return i ;
	}
	return -1 ;
}

//########################################################
//CSecretWaterEffect
//########################################################

CSecretWaterEffect::CSecretWaterEffect()
{
	m_fProjOffset = 0.5f/1024.0f ;
}

CSecretWaterEffect::~CSecretWaterEffect()
{
}

HRESULT CSecretWaterEffect::Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName)
{
	CSecretD3DEffect::Initialze(pd3dDevice, psEssentialElements, pszFileName) ;

	SetD3DXHANDLE(&m_hmatWorld, "matWorld") ;
    SetD3DXHANDLE(&m_havBumpMove, "avBumpMove") ;
	SetD3DXHANDLE(&m_hvTilingBumpTex, "vTilingBumpTex") ;
	SetD3DXHANDLE(&m_hfElapsedTime, "fElapsedTime") ;
	SetD3DXHANDLE(&m_hfProjOffset, "fProjOffset") ;
	SetD3DXHANDLE(&m_hfScreenPosOffset, "fScreenPosOffset") ;
	SetD3DXHANDLE(&m_hfSunStrength, "fSunStrength") ;
	SetD3DXHANDLE(&m_hfSunShininess, "fSunShininess") ;
	SetD3DXHANDLE(&m_hmatLODWVP, "matLODWVP") ;

	return S_OK ;
}