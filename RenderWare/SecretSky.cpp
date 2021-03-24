#include "SecretSky.h"
#include "def.h"

D3DVERTEXELEMENT9 decl_Sky[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	D3DDECL_END()
} ;

void CSecretSkyLightScatteringData::CalculateShaderParameterSkyOptimized()
{
	m_shaderParams.vSumBeta1Beta2 *= 0.1f ;
	m_shaderParams.vBetaD1 *= 0.1f ;
	m_shaderParams.vBetaD2 *= 0.1f ;
	m_shaderParams.vRcpSumBeta1Beta2 *= 15.0f ;
	m_shaderParams.vTermMultipliers.x *= 0.1f;
	m_shaderParams.vTermMultipliers.y *= 0.1f;
}

CSecretSky::CSecretSky()
{
	m_pVB = NULL ;
	m_pIB = NULL ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_pcAtm = NULL ;
	m_lNumVertex = m_lNumTriangle = 0 ;
	m_fWorldScale = 1.0f ;
}

CSecretSky::~CSecretSky()
{
	Release() ;
}

void CSecretSky::Release()
{
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;
	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pIB) ;
	SAFE_DELETE(m_pcAtm) ;
}

bool CSecretSky::Initialize(CASEData *pcASEData, LPDIRECT3DDEVICE9 pd3dDevice)
{
	int i, n ;

	m_pd3dDevice = pd3dDevice ;

	for(i=0 ; i<pcASEData->m_nNumMesh ; i++)
	{
		m_lNumVertex += pcASEData->m_apMesh[i]->nNumVertex ;
		m_lNumTriangle += pcASEData->m_apMesh[i]->nNumTriangle ;
	}

	m_psVertices = new SSkyVertex[m_lNumVertex] ;
	m_psIndices = new SSkyIndex[m_lNumTriangle] ;

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
	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SSkyVertex), D3DUSAGE_WRITEONLY, D3DFVF_SKYVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SSkyVertex), 0, D3DFVF_SKYVERTEX, D3DPOOL_MANAGED, &m_pVB, NULL)))
		return false ;
	else
	{
		g_cDebugMsg.AddVBSize(m_lNumVertex*sizeof(SSkyVertex)) ;

		void *pv ;
		SSkyVertex *psVertices ;

		if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SSkyVertex), (void **)&pv, 0)))
			return false ;

		psVertices = (SSkyVertex *)pv ;

		for(i=0 ; i<(int)m_lNumVertex ; i++, psVertices++)
		{
			psVertices->pos.x = m_psVertices[i].pos.x ;
			psVertices->pos.y = m_psVertices[i].pos.y ;
			psVertices->pos.z = m_psVertices[i].pos.z ;

			//TRACE("Sky[%02d]pos(%07.03f %07.03f %07.03f)\r\n", count++, psVertices->pos.x, psVertices->pos.y, psVertices->pos.z) ;
		}
		m_pVB->Unlock() ;
	}

	//############################################//
	//              Create Indices                //
	//############################################//
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SSkyIndex), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SSkyIndex), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB, NULL)))
		return false ;

	g_cDebugMsg.AddIBSize(m_lNumTriangle*sizeof(SSkyIndex)) ;

	void *pIndices ;
	if(FAILED(m_pIB->Lock(0, m_lNumTriangle*sizeof(SSkyIndex), (void **)&pIndices, 0)))
		return false ;
	else
	{
		SSkyIndex *psIndices = (SSkyIndex *)pIndices ;
		for(i=0 ; i<(int)m_lNumTriangle ; i++, psIndices++)
		{
			psIndices->anIndex[0] = m_psIndices[i].anIndex[0] ;
			psIndices->anIndex[1] = m_psIndices[i].anIndex[1] ;
			psIndices->anIndex[2] = m_psIndices[i].anIndex[2] ;
		}
		m_pIB->Unlock() ;
	}

	m_pcAtm = new CSecretSkyLightScatteringData ;
	m_pcAtm->CalculateShaderParameterSkyOptimized() ;

	m_bWireFrame = false ;

    return true ;
}

void CSecretSky::Render(CSecretSkyEffect *pcSkyEffect)
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
	SEffectEssentialElements *psEffectElements = pcSkyEffect->m_psEssentialElements ;
	D3DXVECTOR4 vCameraPos, vSunDir, vSunColor ;
	D3DXMATRIX mat ;

	pcSkyEffect->m_pEffect->BeginPass(0) ;

	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Sky, &pcSkyEffect->m_pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretSkyDome::Render()") ;

	//D3DXMatrixInverse(&mat, NULL, &psEffectElements->matWorld) ;
    D3DXMatrixIdentity(&mat) ;
	mat._11 = mat._22 = mat._33 = m_fWorldScale ;
	mat._41 = psEffectElements->vCameraPos.x ;
	mat._42 = psEffectElements->vCameraPos.y ;//-20.0f ;
	mat._43 = psEffectElements->vCameraPos.z ;

	//D3DXVec4Transform(&vCameraPos, &psEffectElements->vCameraPos, &mat) ;
	//vCameraPos = D3DXVECTOR4(0, 0, 0, 1) ;
	pcSkyEffect->m_pEffect->SetVector(pcSkyEffect->m_hvCameraPos, &psEffectElements->vCameraPos) ;

	mat = mat * psEffectElements->matWorld * psEffectElements->matView * psEffectElements->matProj ;
	pcSkyEffect->m_pEffect->SetMatrix(pcSkyEffect->m_hmatWVP, &mat) ;

	vSunColor = D3DXVECTOR4(psEffectElements->vSunColor.x, psEffectElements->vSunColor.y, psEffectElements->vSunColor.z, psEffectElements->vSunColor.w) ;
	pcSkyEffect->m_pEffect->SetVector(pcSkyEffect->m_hvSunColor, &vSunColor) ;

	vSunDir = D3DXVECTOR4(psEffectElements->vSunDir.x, psEffectElements->vSunDir.y, psEffectElements->vSunDir.z, psEffectElements->vSunDir.w) ;
	pcSkyEffect->m_pEffect->SetVector(pcSkyEffect->m_hvSunDir, &vSunDir) ;

	pcSkyEffect->m_pEffect->SetValue(pcSkyEffect->m_hAtm, &m_pcAtm->m_shaderParams, sizeof(sLightScatteringShaderParams)) ;

	//m_pd3dDevice->SetFVF(D3DFVF_SKYVERTEX) ;
	m_pd3dDevice->SetVertexDeclaration(pcSkyEffect->m_pDecl) ;
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SSkyVertex)) ;
	m_pd3dDevice->SetIndices(m_pIB) ;

	pcSkyEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;

	pcSkyEffect->m_pEffect->EndPass() ;

	pcSkyEffect->m_pDecl->Release() ;
	pcSkyEffect->m_pDecl = NULL ;
	
	//m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	//m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, dwd3dCull) ;
	//m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) ;
}

void CSecretSky::SetWireFrame(bool bEnable)
{
	m_bWireFrame = bEnable ;
}

//CSecretSkyDomeEffect
CSecretSkyEffect::CSecretSkyEffect()
{
}

CSecretSkyEffect::~CSecretSkyEffect()
{
}

HRESULT CSecretSkyEffect::Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName)
{
	CSecretD3DEffect::Initialze(pd3dDevice, psEssentialElements, pszFileName) ;

	SetD3DXHANDLE(&m_hAtm, "atm") ;

	return S_OK ;
}