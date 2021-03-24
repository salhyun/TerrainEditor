#include "SecretBillboardEffect.h"
#include "def.h"
#include "ASEData.h"
#include "SecretD3DEffect.h"
#include "SecretTextureContainer.h"

CSecretBillboardEffect::CSecretBillboardEffect()
{
	m_pVB = NULL ;
	m_pIB = NULL ;
	m_pVTex = NULL ;
	m_pcEffect = NULL ;
}
CSecretBillboardEffect::~CSecretBillboardEffect()
{
	release() ;
}
void CSecretBillboardEffect::release()
{
	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pIB) ;
	SAFE_RELEASE(m_pVTex) ;
}

bool CSecretBillboardEffect::initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName, float fOffsetY_inPixel)
{
	char str[256] ;
	RemoveExt(str, pszFileName) ;
	sprintf_s(str, "%s.dds", str) ;

	DDSURFACEDESC2 desc2 ;
	ZeroMemory(&desc2, sizeof(DDSURFACEDESC2)) ;
	FILE *pf = fopen(str, "r+b") ;
	if(pf)
	{
		DWORD dwMagicNumber ;
		fread(&dwMagicNumber, 4, 1, pf) ;
		fread(&desc2, sizeof(DDSURFACEDESC2), 1, pf) ;

		fclose(pf) ;
	}


	int i, n ;
	CASEData data ;
	if(!data.ImportRMD(pszFileName))
	{
		TRACE("cannot load %s", pszFileName) ;
		assert(false && "Cannot load RMD file") ;
		return false ;
	}
	assert(data.m_nNumMesh == 1) ;//메쉬는 하나

	m_pd3dDevice = pd3dDevice ;

	SMesh *psMesh = data.m_apMesh[0] ;

	m_nNumVertex = psMesh->nNumVertex ;
	m_nNumIndex = psMesh->nNumTriangle ;

	memcpy(&m_matWorld, &psMesh->smatLocal, sizeof(D3DXMATRIX)) ;

	D3DXMATRIX matS ;
	D3DXMatrixScaling(&matS, 0.1f, 0.1f, 0.1f) ;
	m_matWorld *= matS ;

	//###################//
	//  Create Vertices  //
	//###################//
	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_nNumVertex*sizeof(SBillboardEffect_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_BILLBOARDEFFECTVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return false ;

	SBillboardEffect_Vertex *psVertices ;
	if(FAILED(m_pVB->Lock(0, m_nNumVertex*sizeof(SBillboardEffect_Vertex), (void **)&psVertices, 0)))
		return false ;

	D3DXVECTOR3 vMin, vMax ;
	vMin.x = vMin.y = vMin.z = FLT_MAX ;
	vMax.x = vMax.y = vMax.z = FLT_MIN ;
	D3DXVECTOR2 tMin, tMax ;
	tMin.x = tMin.y = FLT_MAX ;
	tMax.x = tMax.y = FLT_MIN ;

	D3DXVECTOR4 vRet ;
	for(i=0 ; i<psMesh->nNumVertex ; i++)
	{
		psVertices[i].pos.x = psMesh->psVertex[i].pos.x ;
		psVertices[i].pos.y = psMesh->psVertex[i].pos.y ;
		psVertices[i].pos.z = psMesh->psVertex[i].pos.z ;

		D3DXVec3Transform(&vRet, &psVertices[i].pos, &m_matWorld) ;
		psVertices[i].pos.x = vRet.x ;
		psVertices[i].pos.y = vRet.y ;
		psVertices[i].pos.z = vRet.z ;

		if(vMin.x > psVertices[i].pos.x)	vMin.x = psVertices[i].pos.x ;
		if(vMin.y > psVertices[i].pos.y)	vMin.y = psVertices[i].pos.y ;
		if(vMin.z > psVertices[i].pos.z)	vMin.z = psVertices[i].pos.z ;

		if(vMax.x < psVertices[i].pos.x)	vMax.x = psVertices[i].pos.x ;
		if(vMax.y < psVertices[i].pos.y)	vMax.y = psVertices[i].pos.y ;
		if(vMax.z < psVertices[i].pos.z)	vMax.z = psVertices[i].pos.z ;

		psVertices[i].t.x = psMesh->psVertex[i].tex.u ;
		psVertices[i].t.y = psMesh->psVertex[i].tex.v ;
		//TRACE("[%d] tex(%g, %g)\r\n", i, psVertices[i].t.x, psVertices[i].t.y) ;

		if(tMin.x > psVertices[i].t.x)	tMin.x = psVertices[i].t.x ;
		if(tMin.y > psVertices[i].t.y)	tMin.y = psVertices[i].t.y ;

		if(tMax.x < psVertices[i].t.x)	tMax.x = psVertices[i].t.x ;
		if(tMax.y < psVertices[i].t.y)	tMax.y = psVertices[i].t.y ;

		psVertices[i].pos.x = psVertices[i].pos.y = psVertices[i].pos.z = 0.0f ;
	}

	psVertices[0].t.x = tMin.x ;
	psVertices[0].t.y = tMin.y ;

	psVertices[1].t.x = tMax.x ;
	psVertices[1].t.y = tMin.y ;

	psVertices[2].t.x = tMin.x ;
	psVertices[2].t.y = tMax.y ;

	psVertices[3].t.x = tMax.x ;
	psVertices[3].t.y = tMax.y ;

	float fWidth = vMax.x - vMin.x ;
	assert(fWidth > 0.0f) ;

	float fHeight = vMax.y - vMin.y ;
	if(fHeight <= 0.0f)
		fHeight = vMax.z = vMin.z ;
	assert(fHeight > 0.0f) ;

	float div_upfactor=0.0f ;
	if(desc2.dwHeight > 0)
		div_upfactor = fOffsetY_inPixel * (fHeight/desc2.dwHeight) ;

	//upper-left
	psVertices[0].rightFactor = -fWidth/2.0f ;
	psVertices[0].upFactor = fHeight - div_upfactor ;

	//upper-right
	psVertices[1].rightFactor = fWidth/2.0f ;
	psVertices[1].upFactor = fHeight - div_upfactor ;

	//lower-left
	psVertices[2].rightFactor = -fWidth/2.0f ;
	psVertices[2].upFactor = -div_upfactor ;

	//lower-right
	psVertices[3].rightFactor = fWidth/2.0f ;
	psVertices[3].upFactor = -div_upfactor ;

	m_pVB->Unlock() ;

	//###################//
	//  Create Indices   //
	//###################//
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(2*sizeof(SBillboardEffect_Index), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB, NULL)))
		return false ;

    SBillboardEffect_Index *psIndices ;
	if(FAILED(m_pIB->Lock(0, 2*sizeof(SBillboardEffect_Index), (void **)&psIndices, 0)))
		return false ;

	psIndices[0].anIndex[0] = 0 ;
	psIndices[0].anIndex[1] = 1 ;
	psIndices[0].anIndex[2] = 2 ;

	psIndices[1].anIndex[0] = 2 ;
	psIndices[1].anIndex[1] = 1 ;
	psIndices[1].anIndex[2] = 3 ;

	m_pIB->Unlock() ;

	assert(data.m_nNumMaterial == 1) ;

	m_fElapsedTime = 0.0f ;

	return true ;
}

void CSecretBillboardEffect::loadVolumeTexture(char *pszFileName, UINT nDepth)
{
	//볼륨텍스쳐를 로드한다
	char str[256] ;
	RemoveExt(str, pszFileName) ;
	sprintf_s(str, "%s.dds", str) ;

	D3DXCreateVolumeTextureFromFileEx(
		m_pd3dDevice,
		str,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_FROM_FILE,
		D3DX_FROM_FILE,
		0,
		D3DFMT_FROM_FILE,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		NULL,
		NULL,
		&m_pVTex) ;

	D3DVOLUME_DESC desc ;
	int nLevelCount = m_pVTex->GetLevelCount() ;
	m_pVTex->GetLevelDesc(0, &desc) ;
	TRACE("%s desc width=%d height=%d depth=%d mipmap=%d\r\n", str, desc.Width, desc.Height, desc.Depth, nLevelCount) ;
}

void CSecretBillboardEffect::process(float time)
{
	m_fElapsedTime += time ;
	if(m_fElapsedTime > 1.0f)
		m_fElapsedTime = (m_fElapsedTime - 1.0f) ;
}
void CSecretBillboardEffect::render(float fElapsedTime, D3DXMATRIX *pmatWorld)
{
	m_pd3dDevice->SetFVF(D3DFVF_BILLBOARDEFFECTVERTEX) ;

	m_pcEffect->m_pEffect->SetTechnique(m_pcEffect->m_hTechnique) ;
	m_pcEffect->m_pEffect->Begin(NULL, 0) ;
	m_pcEffect->m_pEffect->BeginPass(0) ;

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SBillboardEffect_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pIB) ;

	D3DXMATRIX matWVP = (*pmatWorld) * m_pcEffect->m_psEssentialElements->matView * m_pcEffect->m_psEssentialElements->matProj ;
	m_pcEffect->m_pEffect->SetMatrix(m_pcEffect->m_hmatWVP, &matWVP) ;

	D3DXVECTOR4 vLookat(-m_pcEffect->m_psEssentialElements->matView._13, -m_pcEffect->m_psEssentialElements->matView._23, -m_pcEffect->m_psEssentialElements->matView._33, 0) ;
	m_pcEffect->m_pEffect->SetVector(m_pcEffect->m_hvLookat, &vLookat) ;

	m_pcEffect->m_pEffect->SetFloat(m_pcEffect->m_hfElapsedTime, fElapsedTime) ;

	m_pcEffect->m_pEffect->SetTexture("VolBillboardMap", m_pVTex) ;

	m_pcEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nNumVertex, 0, 2) ;

	m_pcEffect->m_pEffect->EndPass() ;
	m_pcEffect->m_pEffect->End() ;
}
void CSecretBillboardEffect::setEffect(CSecretBillboardEffectEffect *pcEffect)
{
	m_pcEffect = pcEffect ;
}

//################################################
// CSecretBillboardEffectEffect
//################################################

CSecretBillboardEffectEffect::CSecretBillboardEffectEffect()
{
	m_pDecl = NULL ;
	m_pEffect = NULL ;
	m_hTechnique = NULL ;
}
CSecretBillboardEffectEffect::~CSecretBillboardEffectEffect()
{
	Release() ;
}
void CSecretBillboardEffectEffect::Release()
{
	SAFE_RELEASE(m_pEffect) ;
	SAFE_RELEASE(m_pDecl) ;
}
HRESULT CSecretBillboardEffectEffect::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName)
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

	SetD3DXHANDLE(&m_hmatWVP, "matWVP") ;
	SetD3DXHANDLE(&m_hvLookat, "vLookat") ;
	SetD3DXHANDLE(&m_hfElapsedTime, "fElapsedTime") ;

	m_psEssentialElements = psEssentialElements ;

	return S_OK ;
}
bool CSecretBillboardEffectEffect::SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName)
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