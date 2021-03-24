#include "SecretCloud.h"
#include "def.h"

D3DVERTEXELEMENT9 decl_Cloud[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	D3DDECL_END()
} ;

CSecretCloud::CSecretCloud()
{
	m_pVB = NULL ;
	m_pIB = NULL ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_pTexLayer = NULL ;
	m_lNumVertex = m_lNumTriangle = 0 ;
	m_fWorldScale = 1.0f ;
}

CSecretCloud::~CSecretCloud()
{
	Release() ;
}

void CSecretCloud::Release()
{
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;
	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pIB) ;
	SAFE_RELEASE(m_pTexLayer) ;
}

bool CSecretCloud::Initialize(CASEData *pcASEData, LPDIRECT3DDEVICE9 pd3dDevice)
{
	int i, n ;

	m_pd3dDevice = pd3dDevice ;

	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
	{
		m_lNumVertex += pcASEData->m_apMesh[i]->nNumVertex ;
		m_lNumTriangle += pcASEData->m_apMesh[i]->nNumTriangle ;
	}

	m_psVertices = new SCloudVertex[m_lNumVertex] ;
	m_psIndices = new SCloudIndex[m_lNumTriangle] ;

	Vector3 pos ;
	Matrix4 matLocal ;

	m_lNumVertex = m_lNumTriangle = 0 ;
	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
	{
		memcpy(&matLocal, &pcASEData->m_apMesh[i]->smatLocal, sizeof(Matrix4)) ;
		//matLocal.m42 = 50.0f ;
		for(n=0 ; n<pcASEData->m_apMesh[i]->nNumVertex ; n++, m_lNumVertex++)
		{
			pos.set(pcASEData->m_apMesh[i]->psVertex[n].pos.x, pcASEData->m_apMesh[i]->psVertex[n].pos.y, pcASEData->m_apMesh[i]->psVertex[n].pos.z) ;
			pos *= matLocal ;

			m_psVertices[m_lNumVertex].pos.x = pos.x ;
			m_psVertices[m_lNumVertex].pos.y = pos.y ;
			m_psVertices[m_lNumVertex].pos.z = pos.z ;
		}

		for(n=0 ; n<pcASEData->m_apMesh[i]->nNumTriangle ; n++, m_lNumTriangle++)
		{
			m_psIndices[m_lNumTriangle].anIndex[0] = pcASEData->m_apMesh[i]->psTriangle[n].anVertexIndex[0] ;
			m_psIndices[m_lNumTriangle].anIndex[1] = pcASEData->m_apMesh[i]->psTriangle[n].anVertexIndex[1] ;
			m_psIndices[m_lNumTriangle].anIndex[2] = pcASEData->m_apMesh[i]->psTriangle[n].anVertexIndex[2] ;
		}

	}

    //############################################//
	//              Create Vertices               //
	//############################################//

	//float fMaxHeight, fMinHeight ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SCloudVertex), D3DUSAGE_WRITEONLY, D3DFVF_CLOUDVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SCloudVertex), 0, D3DFVF_CLOUDVERTEX, D3DPOOL_MANAGED, &m_pVB, NULL)))
		return false ;
	else
	{
		g_cDebugMsg.AddVBSize(m_lNumVertex*sizeof(SCloudVertex)) ;

		void *pv ;
		SCloudVertex *psVertices ;

		if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SCloudVertex), (void **)&pv, 0)))
			return false ;

		psVertices = (SCloudVertex *)pv ;

		//fMaxHeight = fMinHeight = psVertices->pos.y ;

		for(i=0 ; i<(int)m_lNumVertex ; i++, psVertices++)
		{
			psVertices->pos.x = m_psVertices[i].pos.x ;
			psVertices->pos.y = m_psVertices[i].pos.y ;
			psVertices->pos.z = m_psVertices[i].pos.z ;

			//if(fMaxHeight < psVertices->pos.y)
			//	fMaxHeight = psVertices->pos.y ;

			//if(fMinHeight > psVertices->pos.y)
			//	fMinHeight = psVertices->pos.y ;

			//TRACE("Cloud[%02d]pos(%07.03f %07.03f %07.03f)\r\n", i, psVertices->pos.x, psVertices->pos.y, psVertices->pos.z) ;
		}
		m_pVB->Unlock() ;
	}

	//############################################//
	//              Create Indices                //
	//############################################//
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SCloudIndex), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SCloudIndex), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB, NULL)))
		return false ;

	g_cDebugMsg.AddIBSize(m_lNumTriangle*sizeof(SCloudIndex)) ;

	void *pIndices ;
	if(FAILED(m_pIB->Lock(0, m_lNumTriangle*sizeof(SCloudIndex), (void **)&pIndices, 0)))
		return false ;
	else
	{
		SCloudIndex *psIndices = (SCloudIndex *)pIndices ;
		for(i=0 ; i<(int)m_lNumTriangle ; i++, psIndices++)
		{
			psIndices->anIndex[0] = m_psIndices[i].anIndex[0] ;
			psIndices->anIndex[1] = m_psIndices[i].anIndex[1] ;
			psIndices->anIndex[2] = m_psIndices[i].anIndex[2] ;
		}
		m_pIB->Unlock() ;
	}

	if(FAILED(D3DXCreateTextureFromFile(m_pd3dDevice, "../../Media/SkyMap/clouds.dds", &m_pTexLayer)))
		return false ;

	m_vCloudMove = D3DXVECTOR4(1.0f, 1.0f, 0.0f, 0.0f) ;
	m_bWireFrame = false ;

    return true ;
}

void CSecretCloud::Process(float fTimeSinceLastFrame)
{
	float velocity = fTimeSinceLastFrame*8.0f ;
	m_vCloudMove.x += (velocity*0.00025f) ;
	m_vCloudMove.y += (velocity*0.00035f);
	m_vCloudMove.z += (velocity*0.0004f);
	m_vCloudMove.w += (velocity*0.0005f);
}

void CSecretCloud::Render(CSecretCloudEffect *pcCloudEffect)
{
	HRESULT hr ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	//DWORD dwd3dCull ;
	//m_pd3dDevice->GetRenderState(D3DRS_CULLMODE, &dwd3dCull) ;
	//m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE) ;
    //m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                       Shader Setting                                           //
	////////////////////////////////////////////////////////////////////////////////////////////////////
	SEffectEssentialElements *psEffectElements = pcCloudEffect->m_psEssentialElements ;
	D3DXVECTOR4 vCameraPos, vSunDir, vSunColor ;
	D3DXMATRIX mat ;

	pcCloudEffect->m_pEffect->BeginPass(0) ;

	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Cloud, &pcCloudEffect->m_pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretCloudDome::Render()") ;

	//D3DXMatrixInverse(&mat, NULL, &psEffectElements->matWorld) ;
    D3DXMatrixIdentity(&mat) ;
	mat._11 = mat._22 = mat._33 = m_fWorldScale ;
	mat._41 = psEffectElements->vCameraPos.x ;
	mat._42 = psEffectElements->vCameraPos.y ;
	mat._43 = psEffectElements->vCameraPos.z ;

	//D3DXVec4Transform(&vCameraPos, &psEffectElements->vCameraPos, &mat) ;
	//vCameraPos = D3DXVECTOR4(0, 0, 0, 1) ;
	pcCloudEffect->m_pEffect->SetVector(pcCloudEffect->m_hvCameraPos, &psEffectElements->vCameraPos) ;

	mat = mat * psEffectElements->matWorld * psEffectElements->matView * psEffectElements->matProj ;
	pcCloudEffect->m_pEffect->SetMatrix(pcCloudEffect->m_hmatWVP, &mat) ;

	vSunColor = D3DXVECTOR4(psEffectElements->vSunColor.x, psEffectElements->vSunColor.y, psEffectElements->vSunColor.z, psEffectElements->vSunColor.w) ;
	pcCloudEffect->m_pEffect->SetVector(pcCloudEffect->m_hvSunColor, &vSunColor) ;

	vSunDir = D3DXVECTOR4(psEffectElements->vSunDir.x, psEffectElements->vSunDir.y, psEffectElements->vSunDir.z, psEffectElements->vSunDir.w) ;
	pcCloudEffect->m_pEffect->SetVector(pcCloudEffect->m_hvSunDir, &vSunDir) ;

	pcCloudEffect->m_pEffect->SetVector(pcCloudEffect->m_hvCloudMove, &m_vCloudMove) ;

	pcCloudEffect->m_pEffect->SetTexture("CloudLayerMap", m_pTexLayer) ;

	//m_pd3dDevice->SetFVF(D3DFVF_CLOUDVERTEX) ;
	m_pd3dDevice->SetVertexDeclaration(pcCloudEffect->m_pDecl) ;
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SCloudVertex)) ;
	m_pd3dDevice->SetIndices(m_pIB) ;

	pcCloudEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;

	pcCloudEffect->m_pEffect->EndPass() ;

	pcCloudEffect->m_pDecl->Release() ;
	pcCloudEffect->m_pDecl = NULL ;
	
	//m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	//m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, dwd3dCull) ;
	//m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) ;
}

void CSecretCloud::SetWireFrame(bool bEnable)
{
	m_bWireFrame = bEnable ;
}

//CSecretCloudEffect
CSecretCloudEffect::CSecretCloudEffect()
{
}

CSecretCloudEffect::~CSecretCloudEffect()
{
}

HRESULT CSecretCloudEffect::Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName)
{
	CSecretD3DEffect::Initialze(pd3dDevice, psEssentialElements, pszFileName) ;

	SetD3DXHANDLE(&m_hvCloudMove, "vCloudMove") ;

	return S_OK ;
}