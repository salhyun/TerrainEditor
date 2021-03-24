#include "SecretSkyDome.h"
#include "def.h"

D3DVERTEXELEMENT9 decl_SkyDome[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정
	D3DDECL_END()
} ;

CSecretSkyDome::CSecretSkyDome()
{
	m_pVB = NULL ;
	m_pIB = NULL ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_pTex = NULL ;
	m_lNumVertex = m_lNumTriangle = 0 ;
	m_fWorldScale = 1.0f ;
	m_vTexAni = D3DXVECTOR4(0, 0, 0, 1) ;
	m_vSkydomPos.x = 0.0f ;
	m_vSkydomPos.y = -242.0f ;
	m_vSkydomPos.z = 0.0f ;
}

CSecretSkyDome::~CSecretSkyDome()
{
	Release() ;
}

void CSecretSkyDome::Release()
{
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;
	//SAFE_RELEASE(m_pTex) ;
	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pIB) ;
}

bool CSecretSkyDome::Initialize(CASEData *pcASEData, LPDIRECT3DDEVICE9 pd3dDevice)
{
	int i, n ;

	m_pd3dDevice = pd3dDevice ;

	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
	{
		m_lNumVertex += pcASEData->m_apMesh[i]->nNumVertex ;
		m_lNumTriangle += pcASEData->m_apMesh[i]->nNumTriangle ;
	}

	m_psVertices = new SSkyDomeVertex[m_lNumVertex] ;
	m_psIndices = new SSkyDomeIndex[m_lNumTriangle] ;

	Vector3 pos ;
	Matrix4 matLocal ;

	float fLowestY = 2048.0f, fHighestY = 0.0f ;

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

			if(fLowestY > pos.y)
				fLowestY = pos.y ;

			if(fHighestY < pos.y)
				fHighestY = pos.y ;
			
			m_psVertices[m_lNumVertex].t.x = pcASEData->m_apMesh[i]->psVertex[n].tex.u ;
			m_psVertices[m_lNumVertex].t.y = pcASEData->m_apMesh[i]->psVertex[n].tex.v ;
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
	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SSkyDomeVertex), D3DUSAGE_WRITEONLY, D3DFVF_SkyDomeVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SSkyDomeVertex), 0, D3DFVF_SkyDomeVERTEX, D3DPOOL_MANAGED, &m_pVB, NULL)))
		return false ;
	else
	{
		g_cDebugMsg.AddVBSize(m_lNumVertex*sizeof(SSkyDomeVertex)) ;

		void *pv ;
		SSkyDomeVertex *psVertices ;

		if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SSkyDomeVertex), (void **)&pv, 0)))
			return false ;

		psVertices = (SSkyDomeVertex *)pv ;

		for(i=0 ; i<(int)m_lNumVertex ; i++, psVertices++)
		{
			psVertices->pos.x = m_psVertices[i].pos.x ;
			psVertices->pos.y = m_psVertices[i].pos.y ;
			psVertices->pos.z = m_psVertices[i].pos.z ;

			psVertices->t.x = m_psVertices[i].t.x ;
			psVertices->t.y = m_psVertices[i].t.y ;

			//TRACE("Vertex SkyDome[%02d]pos(%07.03f %07.03f %07.03f) tex(%07.03f %07.03f)\r\n",
			//	i, psVertices->pos.x, psVertices->pos.y, psVertices->pos.z, psVertices->t.x, psVertices->t.y) ;
		}
		m_pVB->Unlock() ;
	}

	//############################################//
	//              Create Indices                //
	//############################################//
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SSkyDomeIndex), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SSkyDomeIndex), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB, NULL)))
		return false ;

	g_cDebugMsg.AddIBSize(m_lNumTriangle*sizeof(SSkyDomeIndex)) ;

	void *pIndices ;
	if(FAILED(m_pIB->Lock(0, m_lNumTriangle*sizeof(SSkyDomeIndex), (void **)&pIndices, 0)))
		return false ;
	else
	{
		SSkyDomeIndex *psIndices = (SSkyDomeIndex *)pIndices ;
		for(i=0 ; i<(int)m_lNumTriangle ; i++, psIndices++)
		{
			psIndices->anIndex[0] = m_psIndices[i].anIndex[0] ;
			psIndices->anIndex[1] = m_psIndices[i].anIndex[1] ;
			psIndices->anIndex[2] = m_psIndices[i].anIndex[2] ;
			//TRACE("Index SkyDome[%03d] (%03d %03d %03d)\r\n", i, psIndices->anIndex[0], psIndices->anIndex[1], psIndices->anIndex[2]) ;
		}
		m_pIB->Unlock() ;
	}

	m_fWorldScale = 1.0f ;

	m_vTexAni = D3DXVECTOR4(64.0f, fHighestY, 1.0f/(fHighestY-fLowestY), 1) ;

	m_bWireFrame = false ;
    return true ;
}

void CSecretSkyDome::SetTexture(LPDIRECT3DTEXTURE9 pTex)
{
	m_pTex = pTex ;
}

void CSecretSkyDome::Process(float fTimeSinceLastFrame)
{
	//float vel = 0.00138125f*2.0f ;
	float vel = 0.00138125f*0.8f ;

	m_vTexAni.x -= (fTimeSinceLastFrame*vel) ;

	if(m_vTexAni.x <= -64.0f)
		m_vTexAni.x = 64.0f ;
}

void CSecretSkyDome::Render(CSecretSkyDomeEffect *pcSkyDomeEffect)
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
	SEffectEssentialElements *psEffectElements = pcSkyDomeEffect->m_psEssentialElements ;
	D3DXVECTOR4 vCameraPos, vSunDir, vSunColor ;
	D3DXMATRIX mat ;

	pcSkyDomeEffect->m_pEffect->BeginPass(0) ;

	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_SkyDome, &pcSkyDomeEffect->m_pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretSkyDomeDome::Render()") ;

	//D3DXMatrixInverse(&mat, NULL, &psEffectElements->matWorld) ;
    D3DXMatrixIdentity(&mat) ;
	mat._41 = m_vSkydomPos.x ;
	mat._42 = m_vSkydomPos.y ;
	mat._43 = m_vSkydomPos.z ;

	//mat._41 = m_vSkydomPos.x ;
	//mat._42 = m_vSkydomPos.y + 250.0f ;
	//mat._43 = m_vSkydomPos.z ;

	//mat._11 = mat._22 = mat._33 = 0.1f ;
	//mat._41 = psEffectElements->vCameraPos.x ;
	//mat._42 = psEffectElements->vCameraPos.y ;//-20.0f ;
	//mat._43 = psEffectElements->vCameraPos.z ;

	//D3DXVec4Transform(&vCameraPos, &psEffectElements->vCameraPos, &mat) ;
	//vCameraPos = D3DXVECTOR4(0, 0, 0, 1) ;
	pcSkyDomeEffect->m_pEffect->SetVector(pcSkyDomeEffect->m_hvCameraPos, &psEffectElements->vCameraPos) ;

	mat = mat * psEffectElements->matWorld * psEffectElements->matView * psEffectElements->matProj ;
	pcSkyDomeEffect->m_pEffect->SetMatrix(pcSkyDomeEffect->m_hmatWVP, &mat) ;

	vSunColor = D3DXVECTOR4(psEffectElements->vSunColor.x, psEffectElements->vSunColor.y, psEffectElements->vSunColor.z, psEffectElements->vSunColor.w) ;
	pcSkyDomeEffect->m_pEffect->SetVector(pcSkyDomeEffect->m_hvSunColor, &vSunColor) ;

	vSunDir = D3DXVECTOR4(psEffectElements->vSunDir.x, psEffectElements->vSunDir.y, psEffectElements->vSunDir.z, psEffectElements->vSunDir.w) ;
	pcSkyDomeEffect->m_pEffect->SetVector(pcSkyDomeEffect->m_hvSunDir, &vSunDir) ;

	pcSkyDomeEffect->m_pEffect->SetVector(pcSkyDomeEffect->m_hvTexAni, &m_vTexAni) ;

	pcSkyDomeEffect->m_pEffect->SetTexture("SkyDomeMap", m_pTex) ;

	//m_pd3dDevice->SetFVF(D3DFVF_SkyDomeVERTEX) ;
	m_pd3dDevice->SetVertexDeclaration(pcSkyDomeEffect->m_pDecl) ;
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SSkyDomeVertex)) ;
	m_pd3dDevice->SetIndices(m_pIB) ;

	pcSkyDomeEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;

	pcSkyDomeEffect->m_pEffect->EndPass() ;

	pcSkyDomeEffect->m_pDecl->Release() ;
	pcSkyDomeEffect->m_pDecl = NULL ;
	
	//m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	//m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, dwd3dCull) ;
	//m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) ;
}

void CSecretSkyDome::SetWireFrame(bool bEnable)
{
	m_bWireFrame = bEnable ;
}

void CSecretSkyDome::AttachPlanet(Vector3 vSunDir)
{
	geo::SLine line(Vector3(0, 0, 0), vSunDir, 8000.0f) ;

	//for(i=0 ; i<m_lNumTriangle ; i++)
}

//CSecretSkyDomeDomeEffect
CSecretSkyDomeEffect::CSecretSkyDomeEffect()
{
}

CSecretSkyDomeEffect::~CSecretSkyDomeEffect()
{
}

HRESULT CSecretSkyDomeEffect::Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName)
{
	CSecretD3DEffect::Initialze(pd3dDevice, psEssentialElements, pszFileName) ;

	SetD3DXHANDLE(&m_hvTexAni, "vTexAni") ;
	SetD3DXHANDLE(&m_hvSunToCamera, "vSunToCamera") ;

	return S_OK ;
}