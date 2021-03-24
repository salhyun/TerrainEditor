#include "SecretSod.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "LightScatteringData.h"
#include "SecretTextureContainer.h"
#include "SecretTerrain.h"
#include "PerlinNoise.h"
#include "SecretToolBrush.h"
#include "SecretShadowMap.h"

int sorting_sods(const void *pv1, const void *pv2)//increasing order 오름차순 (큰숫자 -> 적은숫자)
{
	SClump *pNode1 = (SClump *)pv1 ;
	SClump *pNode2 = (SClump *)pv2 ;

	float dist = pNode1->fDistfromCamera - pNode2->fDistfromCamera ;
    if(float_less(dist, 0.0f))
		return 1 ;
	else if(float_greater(dist, 0.0f))
		return -1 ;
	else
		return 0 ;
}

D3DVERTEXELEMENT9 decl_Sod[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//노말
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정
	{0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},//빌보드의 크기정보
	{0, 40, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},//여분의 데이타
	D3DDECL_END()
} ;

SClumpMark::SClumpMark()
{
	bEnable = false ;
	lWidth=lHeight=lTotalSize=0 ;
	pdwMark=NULL ;
}
SClumpMark::~SClumpMark()
{
}
void SClumpMark::Release()
{
	SAFE_DELETEARRAY(pdwMark) ;
	bEnable = false ;
}
bool SClumpMark::initialize(unsigned long width, unsigned long height)
{
	lWidth = width ;
	lHeight = height ;
	unsigned long r = ((width*height)%32) ? 1 : 0 ;
	lTotalSize = (width*height)/32 + r ;
	pdwMark = new DWORD[lTotalSize] ;
	ZeroMemory(pdwMark, sizeof(DWORD)*lTotalSize) ;
	TRACE("ClumpMark %gMbytes allocated!! lTotalSize in bytes=%d\r\n", (lTotalSize*4)/1024.0f/1024.0f, lTotalSize*sizeof(DWORD)) ;
	bEnable = true ;
	return true ;
}
void SClumpMark::SetMark(unsigned long x, unsigned long z, bool enable)
{
    unsigned long pos = z*lWidth + x ;
	unsigned long q = pos/32 ;
	unsigned long r = 31-(pos%32) ;

	if(enable)
		pdwMark[q] |= (0x00000001<<r) ;
	else
        pdwMark[q] &= ~(0x00000001<<r) ;
}
bool SClumpMark::GetMark(unsigned long x, unsigned long z)
{
	unsigned long pos = z*lWidth + x ;
	unsigned long q = pos/32 ;
	unsigned long r = 31-(pos%32) ;

    if(pdwMark[q] & (0x00000001<<r))
		return true ;

	return false ;
}
void SClumpMark::operator =(SClumpMark sMark)
{
	this->initialize(sMark.lWidth, sMark.lHeight) ;
	memcpy(this->pdwMark, sMark.pdwMark, sizeof(DWORD)*sMark.lTotalSize) ;
}

//#########################################
//SClumpKind
//#########################################
void SClumpKind::set(Vector2 vStartPos, float tex_width, float tex_height, float xfactor, float yfactor, char *pszName)
{
	//dist : pixel = 4cm : 23pixel
	float ratio = 0.04f/(23.0f*yfactor) ;
	fWidth = ratio*tex_width ;
	fHeight = ratio*tex_height ;

	//fWidth = width ;
	//fHeight = height ;

	avTex[0].set(vStartPos.x, vStartPos.y) ;
	avTex[1].set(vStartPos.x+tex_width, vStartPos.y) ;
	avTex[2].set(vStartPos.x, vStartPos.y+tex_height) ;
	avTex[3].set(vStartPos.x+tex_width, vStartPos.y+tex_height) ;
	sprintf(szName, "%s", pszName) ;
}

//#########################################
//CSecretSod
//#########################################
CSecretSod::CSecretSod(int nNumTile) :
MAXNUMCLUMP(nNumTile),
MAXNUMVERTEX(nNumTile*4),
MAXNUMINDEX(nNumTile*2)
{
	m_pd3dDevice = NULL ;
	m_pVB = NULL ;
	m_pIB = NULL ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_nNumVertex = m_nNumIndex = 0 ;
	m_nNumClumpKind = 0 ;
	m_nNumClump = 0 ;
	m_psClumps = NULL ;
	m_bWireFrame = false ;
	m_fRadius = 1 ;
	m_pcTerrain = NULL ;
	m_vCenter.set(0, 0, -75) ;
	m_pcTexContainer = NULL ;
	m_fXInterval = m_fZInterval = 0.5f ;
	m_pcBrushTool = NULL ;
	//m_nBoardWidth = m_nBoardHeight = 0 ;
	//m_ppsSodMark = NULL ;

	for(int i=0 ; i<MAXNUM_CLUMPKIND ; i++)
		m_apsClumpKind[i] = NULL ;

}
CSecretSod::~CSecretSod()
{
}
void CSecretSod::Release()
{
	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pIB) ;
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;
	SAFE_DELETEARRAY(m_psClumps) ;

	SAFE_RELEASE(m_pTexNoise) ;

	m_sClumpMark.Release() ;
	//if(m_ppsSodMark)
	//{
	//	for(int n=0 ; n<m_nBoardHeight ; n++)
	//		SAFE_DELETEARRAY(m_ppsSodMark[n]) ;
	//	SAFE_DELETEARRAY(m_ppsSodMark) ;
	//}
}
bool CSecretSod::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretTerrain *pcTerrain, CSecretTextureContainer *pcTexContainer, CSecretBrushTool *pcBrushTool, CSecretSodEffect *pcSodEffect)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcTerrain = pcTerrain ;
	m_pcTexContainer = pcTexContainer ;
	m_pcBrushTool = pcBrushTool ;
	m_pcSodEffect = pcSodEffect ;

	m_pTexClump = m_pcTexContainer->FindTexture("sods") ;

	_CreateNoiseTexture(&m_pTexNoise) ;

    m_psClumps = new SClump[MAXNUMCLUMP] ;

    m_psVertices = new SSod_Vertex[MAXNUMVERTEX] ;
	if(FAILED(m_pd3dDevice->CreateVertexBuffer(MAXNUMVERTEX*sizeof(SSod_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_SODVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return false ;

	m_psIndices = new SSod_Index[MAXNUMINDEX] ;
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(MAXNUMINDEX*sizeof(SSod_Index), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_pIB, NULL)))
		return false ;

	srand((unsigned)time(NULL)) ;

	m_nVertexCount = m_nIndexCount = m_nNumVertex = m_nNumIndex = 0 ;

	return true ;
}
void CSecretSod::GenerateClumps(Vector3 *pvCameraPos)
{
	if(!m_nNumClump)
		return ;

	int i ;
	SClump *psClumps = m_psClumps ;
	m_nVertexCount = m_nIndexCount = 0 ;
	psClumps = m_psClumps ;
	for(i=0 ; i<m_nNumClump ; i++, psClumps++)
	{
		if(m_pcTerrain->m_pcFrustum->IsIn(psClumps->vPos))
			_AddClump(psClumps, pvCameraPos) ;
	}
}
void CSecretSod::copyVertices()
{
	m_nNumVertex = m_nVertexCount ;
	m_nNumIndex = m_nIndexCount ;

	SSod_Vertex *psVertices ;
	m_pVB->Lock(0, sizeof(SSod_Vertex)*m_nNumVertex, (void **)&psVertices, 0) ;
	memcpy(psVertices, m_psVertices, sizeof(SSod_Vertex)*m_nNumVertex) ;
	m_pVB->Unlock() ;

	SSod_Index *psIndices ;
	m_pIB->Lock(0, sizeof(SSod_Index)*m_nNumIndex, (void **)&psIndices, 0) ;
	memcpy(psIndices, m_psIndices, sizeof(SSod_Index)*m_nNumIndex) ;
	m_pIB->Unlock() ;
}
void CSecretSod::Process(int nPlanting)
{
	if(m_nNumClump >= MAXNUMCLUMP)
		return ;

	SClump sClump ;
	float x1, x2, z1, z2, x, z ;

	Vector3 vCenter = m_pcBrushTool->GetCenterPos() ;
	float fRadius = m_pcBrushTool->GetRadius() ;

	x1 = vCenter.x-fRadius ;
	x2 = vCenter.x+fRadius ;
	z1 = vCenter.z+fRadius ;
	z2 = vCenter.z-fRadius ;
	x1 += (m_fXInterval-fmod(x1, m_fXInterval)) ;
	x2 += (m_fXInterval-fmod(x2, m_fXInterval)) ;
	z1 += (m_fZInterval-fmod(z1, m_fZInterval)) ;
	z2 += (m_fZInterval-fmod(z2, m_fZInterval)) ;

	int nx1, nx2, nz1, nz2, nx, nz ;

	nz1 = (int)((z1+m_pcBrushTool->m_pcTerrain->m_sHeightMap.lHeight*m_pcBrushTool->m_pcTerrain->m_sHeightMap.nTileSize/2.0f)/m_fZInterval+0.5f) ;
	nz2 = (int)((z2+m_pcBrushTool->m_pcTerrain->m_sHeightMap.lHeight*m_pcBrushTool->m_pcTerrain->m_sHeightMap.nTileSize/2.0f)/m_fZInterval+0.5f) ;
	nx1 = (int)((x1+m_pcBrushTool->m_pcTerrain->m_sHeightMap.lWidth*m_pcBrushTool->m_pcTerrain->m_sHeightMap.nTileSize/2.0f)/m_fXInterval+0.5f) ;
	nx2 = (int)((x2+m_pcBrushTool->m_pcTerrain->m_sHeightMap.lWidth*m_pcBrushTool->m_pcTerrain->m_sHeightMap.nTileSize/2.0f)/m_fXInterval+0.5f) ;

	float a, b, l, fx ;
    for(nz=nz1 ; nz>=nz2 ; nz--)
	{
        z = (float)nz*m_fZInterval ;
		z -= (m_pcBrushTool->m_pcTerrain->m_sHeightMap.lHeight*m_pcBrushTool->m_pcTerrain->m_sHeightMap.nTileSize/2.0f) ;
		for(nx=nx1 ; nx<=nx2 ; nx++)
		{
			if(m_nNumClump >= MAXNUMCLUMP)
				return ;

			x = (float)nx*m_fXInterval ;
			x -= (m_pcBrushTool->m_pcTerrain->m_sHeightMap.lWidth*m_pcBrushTool->m_pcTerrain->m_sHeightMap.nTileSize/2.0f) ;

			a = vCenter.x-x ;
			b = vCenter.z-z ;
			l = sqrtf(a*a+b*b) ;
			if(float_less_eq(l, fRadius))
			{
				if(nPlanting == CSecretSod::PLAINTING)
				{
					//if(!m_ppsSodMark[nz][nx].bEnable)
					if(!m_sClumpMark.GetMark(nx, nz))
					{
						if(nz%2)// odd number
							fx = x+(m_fXInterval*0.25f) ;
						else
							fx = x ;
						//fx += (m_fXInterval/(float)(1024-(rand()%2048))) ;

						m_psClumps[m_nNumClump].vPos.x = fx ;
						m_psClumps[m_nNumClump].vPos.y = m_pcBrushTool->m_pcTerrain->m_sHeightMap.GetHeightMap(fx, z) ;
						m_psClumps[m_nNumClump].vPos.z = z ;
						m_psClumps[m_nNumClump].nKind = rand()%m_nNumClumpKind ;

						int r=256 ;
						float temp, div=512.0f ;
						Vector3 vRand ;
						temp = (float)(rand()%r) ;
						vRand.x = temp/div ;
						temp = (float)(rand()%r) ;
						vRand.y = 0 ;// temp/div ;
						temp = (float)(rand()%r) ;
						vRand.z = temp/div ;
						m_psClumps[m_nNumClump].vCrumple = vRand ;

						//m_ppsSodMark[nz][nx].set(true) ;
						m_sClumpMark.SetMark(nx, nz, true) ;

						m_nNumClump++ ;
					}
				}
				else if(nPlanting == CSecretSod::REMOVE)
				{
					m_sClumpMark.SetMark(nx, nz, false) ;
				}
			}
		}
	}
	if(nPlanting == CSecretSod::REMOVE)//지울때는 모든 버퍼를 새로 갱신
        RefreshClumps() ;
}
void CSecretSod::Render(CSecretShadowMap *pcShadowMap)
{
	if(!m_nNumVertex)
		return ;

	m_pd3dDevice->SetFVF(D3DFVF_SODVERTEX) ;

	m_pcSodEffect->m_pEffect->SetTechnique(m_pcSodEffect->m_hTechnique) ;
	m_pcSodEffect->m_pEffect->Begin(NULL, 0) ;

	m_pcSodEffect->m_pEffect->BeginPass(0) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	DWORD dwAlphaTest, dwZWrite, dwAlphaBlend, dwSrcBlend, dwDestBlend, dwAlphaRef, dwAlphaFunc, dwZFunc ;
	m_pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTest) ;
	m_pd3dDevice->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWrite) ;
	m_pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend) ;

	m_pd3dDevice->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend) ;
	m_pd3dDevice->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend) ;

	m_pd3dDevice->GetRenderState(D3DRS_ALPHAREF, &dwAlphaRef) ;
	m_pd3dDevice->GetRenderState(D3DRS_ALPHAFUNC, &dwAlphaFunc) ;

	m_pd3dDevice->GetRenderState(D3DRS_ZFUNC, &dwZFunc) ;

	/*
	//one pass rendering
	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x08) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SSod_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pIB) ;

	m_pcSodEffect->m_pEffect->SetTexture("DecalMap", m_pTexClump) ;
	m_pcSodEffect->m_pEffect->SetTexture("NoiseMap", m_pTexNoise) ;
	m_pcSodEffect->m_pEffect->SetVector(m_pcSodEffect->m_hvCameraPos, &m_pcSodEffect->m_psEssentialElements->vCameraPos) ;

	D3DXVECTOR4 v ;
	v.x = m_pcSodEffect->m_psEssentialElements->matView._12 ;
	v.y = m_pcSodEffect->m_psEssentialElements->matView._22 ;
	v.z = m_pcSodEffect->m_psEssentialElements->matView._32 ;
	v.w = 0 ;
	m_pcSodEffect->m_pEffect->SetVector(m_pcSodEffect->m_hvAxisY, &v) ;

	v.x = m_pcSodEffect->m_psEssentialElements->matView._11 ;
	v.y = m_pcSodEffect->m_psEssentialElements->matView._21 ;
	v.z = m_pcSodEffect->m_psEssentialElements->matView._31 ;
	v.w = 0 ;
	m_pcSodEffect->m_pEffect->SetVector(m_pcSodEffect->m_hvAxisX, &v) ;

	D3DXMATRIX matWVP = m_pcSodEffect->m_psEssentialElements->matView * m_pcSodEffect->m_psEssentialElements->matProj ;
	m_pcSodEffect->m_pEffect->SetMatrix(m_pcSodEffect->m_hmatWVP, &matWVP) ;

	m_pcSodEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nNumVertex, 0, m_nNumIndex) ;
	*/

	//two pass rendering
	//first
	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x000000bf) ;//cut off greater that 0.75
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE) ;
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO) ;

	//DWORD dwColor = 0x02ffffff ;
	//m_pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, dwColor) ;

 //   m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1) ;
	//m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE) ;

 //   m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE) ;
	//m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE) ;
	//m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR) ;

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SSod_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pIB) ;

	//투영변환후에 텍스쳐공간으로 변환하기 위한 행렬, Offset값으로 오차를 조정한다.
	float fOffsetX = 0.5f+(0.5f/(float)pcShadowMap->GetRenderTarget()->lWidth) ;
	float fOffsetY = 0.5f+(0.5f/(float)pcShadowMap->GetRenderTarget()->lHeight) ;

	D3DXMATRIX matTexBias(0.5f,	    0.0f,	   0.0f,	   0.0f,
						  0.0f,    -0.5f,      0.0f,       0.0f,
						  0.0f,     0.0f,      0.0f,       0.0f,
						  fOffsetX, fOffsetY,  0.0f,       1.0f) ;

	SRenderTarget *psRenderTarget = pcShadowMap->GetRenderTarget() ;
	D3DXMATRIX matWLP, matWLPT ;
	matWLP = (*pcShadowMap->GetmatLVP()) ;
	m_pcSodEffect->m_pEffect->SetMatrix(m_pcSodEffect->m_hmatWLP, &matWLP) ;
	matWLPT = (*pcShadowMap->GetmatLVP()) * matTexBias ;
	m_pcSodEffect->m_pEffect->SetMatrix(m_pcSodEffect->m_hmatWLPT, &matWLPT) ;
	m_pcSodEffect->m_pEffect->SetTexture("SrcShadowMap", pcShadowMap->GetTexSoft()) ;

	m_pcSodEffect->m_pEffect->SetTexture("DecalMap", m_pTexClump) ;
	m_pcSodEffect->m_pEffect->SetTexture("NoiseMap", m_pTexNoise) ;
	m_pcSodEffect->m_pEffect->SetVector(m_pcSodEffect->m_hvCameraPos, &m_pcSodEffect->m_psEssentialElements->vCameraPos) ;

	D3DXVECTOR4 v ;
	v.x = m_pcSodEffect->m_psEssentialElements->matView._12 ;
	v.y = m_pcSodEffect->m_psEssentialElements->matView._22 ;
	v.z = m_pcSodEffect->m_psEssentialElements->matView._32 ;
	v.w = 0 ;

	Vector3 up1(0, 1, 0), up2(v.x, v.y, v.z) ;
	if(IncludedAngle(up1, up2) >= deg_rad(45))
	{
		Vector3 up3 = GetNewVectorfromTwoVector(up1, up2, 45) ;
		v.x = up3.x ;
		v.y = up3.y ;
		v.z = up3.z ;
		v.w = 0 ;
	}
	m_pcSodEffect->m_pEffect->SetVector(m_pcSodEffect->m_hvAxisY, &v) ;

	v.x = m_pcSodEffect->m_psEssentialElements->matView._11 ;
	v.y = m_pcSodEffect->m_psEssentialElements->matView._21 ;
	v.z = m_pcSodEffect->m_psEssentialElements->matView._31 ;
	v.w = 0 ;
	m_pcSodEffect->m_pEffect->SetVector(m_pcSodEffect->m_hvAxisX, &v) ;

	D3DXMATRIX matWVP = m_pcSodEffect->m_psEssentialElements->matView * m_pcSodEffect->m_psEssentialElements->matProj ;
	m_pcSodEffect->m_pEffect->SetMatrix(m_pcSodEffect->m_hmatWVP, &matWVP) ;

	m_pcSodEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nNumVertex, 0, m_nNumIndex) ;

	//second
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) ;
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) ;

	m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE) ;
	m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS) ;

	m_pcSodEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nNumVertex, 0, m_nNumIndex) ;

	m_pcSodEffect->m_pEffect->EndPass() ;

	m_pcSodEffect->m_pEffect->End() ;


	m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, dwZFunc) ;

	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, dwAlphaRef) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, dwAlphaFunc) ;

	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend) ;
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, dwDestBlend) ;

	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTest) ;
	m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, dwZWrite) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend) ;

}
void CSecretSod::_AddClump(SClump *psClump, Vector3 *pvCameraPos)
{
	if( ((m_nVertexCount+4) >= MAXNUMVERTEX) || ((m_nIndexCount+2) >= MAXNUMINDEX) )
		return ;

	SClumpKind *psClumpKind = m_apsClumpKind[psClump->nKind] ;

	//버텍스 4개씩 할당된다.

	float alpha=1.0f ;
	Vector3 vNormal ;
	//Vector3 vCamera(m_pcSodEffect->m_psEssentialElements->vCameraPos.x, m_pcSodEffect->m_psEssentialElements->vCameraPos.y, m_pcSodEffect->m_psEssentialElements->vCameraPos.z) ;
	float a = (psClump->vPos-(*pvCameraPos)).Magnitude() ;
	if(a > 5.0f)
	{
		alpha = 1.0f-(a/50.0f) ;
		if(float_less_eq(alpha, 0.0f))
			return ;
	}

	m_psVertices[m_nVertexCount].reserved = D3DXVECTOR4(psClump->vCrumple.x, psClump->vCrumple.y, psClump->vCrumple.z, alpha) ;
	m_psVertices[m_nVertexCount+1].reserved = m_psVertices[m_nVertexCount].reserved ;
	m_psVertices[m_nVertexCount+2].reserved = m_psVertices[m_nVertexCount].reserved ;
	m_psVertices[m_nVertexCount+3].reserved = m_psVertices[m_nVertexCount].reserved ;

	//vNormal = m_pcTerrain->m_sHeightMap.GetNormal(psClump->vPos.x, psClump->vPos.z) ;

	//left-top
	m_psVertices[m_nVertexCount].pos = D3DXVECTOR3(psClump->vPos.x, psClump->vPos.y, psClump->vPos.z) ;
	m_psVertices[m_nVertexCount].tex = D3DXVECTOR2(psClumpKind->avTex[LT].x, psClumpKind->avTex[LT].y) ;
	m_psVertices[m_nVertexCount].factor = D3DXVECTOR2(-psClumpKind->fWidth/2.0f, psClumpKind->fHeight) ;

	//right-top
	m_psVertices[m_nVertexCount+1].pos = D3DXVECTOR3(psClump->vPos.x, psClump->vPos.y, psClump->vPos.z) ;
	m_psVertices[m_nVertexCount+1].tex = D3DXVECTOR2(psClumpKind->avTex[RT].x, psClumpKind->avTex[RT].y) ;
	m_psVertices[m_nVertexCount+1].factor = D3DXVECTOR2(psClumpKind->fWidth/2.0f, psClumpKind->fHeight) ;

	//left-bottom
	m_psVertices[m_nVertexCount+2].pos = D3DXVECTOR3(psClump->vPos.x, psClump->vPos.y, psClump->vPos.z) ;
	m_psVertices[m_nVertexCount+2].tex = D3DXVECTOR2(psClumpKind->avTex[LB].x, psClumpKind->avTex[LB].y) ;
	m_psVertices[m_nVertexCount+2].factor = D3DXVECTOR2(-psClumpKind->fWidth/2.0f, 0) ;

	//right-bottom
	m_psVertices[m_nVertexCount+3].pos = D3DXVECTOR3(psClump->vPos.x, psClump->vPos.y, psClump->vPos.z) ;
	m_psVertices[m_nVertexCount+3].tex = D3DXVECTOR2(psClumpKind->avTex[RB].x, psClumpKind->avTex[RB].y) ;
	m_psVertices[m_nVertexCount+3].factor = D3DXVECTOR2(psClumpKind->fWidth/2.0f, 0) ;

	m_psIndices[m_nIndexCount].anIndex[0] = m_nVertexCount ;
	m_psIndices[m_nIndexCount].anIndex[1] = m_nVertexCount+1 ;
	m_psIndices[m_nIndexCount].anIndex[2] = m_nVertexCount+2 ;
	m_nIndexCount++ ;

	m_psIndices[m_nIndexCount].anIndex[0] = m_nVertexCount+2 ;
	m_psIndices[m_nIndexCount].anIndex[1] = m_nVertexCount+1 ;
	m_psIndices[m_nIndexCount].anIndex[2] = m_nVertexCount+3 ;
	m_nIndexCount++ ;

	m_nVertexCount += 4 ;
}
void CSecretSod::_DistributionClumpPos(Vector2 *pvPos)
{
	static int nMapSize = 256 ;
	static CPerlinNoise cPerlin(1.0f/2.0f, 8, nMapSize) ;
	static int count=0 ;

	int x, y ;

	while(true)
	{
		x = count%nMapSize ;
		y = count/nMapSize ;
		count++ ;
		float value = cPerlin.PerlinNoise2D((float)x, (float)y) ;

		if(value > 0.35f)
		{
			pvPos->x = m_vCenter.x + (x*2.0f/nMapSize) ;
			pvPos->y = m_vCenter.z + (y*2.0f/nMapSize) ;
            break ;            
		}
	}
}
bool CSecretSod::SaveNoiseDataToFile(char *pszFileName, float fPersistence, int nNumOctaves, int nResolution)
{
	FILE *pf=NULL ;
	SSodNoiseInfo info ;
	CPerlinNoise cPerlin(fPersistence, nNumOctaves, nResolution) ;

	info.nKind = CPerlinNoise::DIMENSION_TWO ;
	info.fPersistence = fPersistence ;
	info.nNumOctaves = nNumOctaves ;
	info.nResolution = nResolution ;
	GetFileNameFromPath(info.szName, pszFileName) ;

	pf = fopen(pszFileName, "wb+") ;
	fwrite(&info, sizeof(SSodNoiseInfo), 1, pf) ;

	float fValue ;
	BYTE byValue ;
    int x, y ;
	for(y=0 ; y<nResolution ; y++)
	{
		for(x=0 ; x<nResolution ; x++)
		{
			fValue = cPerlin.PerlinNoise2D((float)x, (float)y) ;
            byValue = (BYTE)((fValue+1.0f)*255.0f/2.0f) ;
			fwrite(&byValue, sizeof(BYTE), 1, pf) ;
		}
	}

    fclose(pf) ;
	return true ;
}
bool CSecretSod::_CreateNoiseTexture(LPDIRECT3DTEXTURE9 *ppTex)
{
	int x, y ;
	int nMapSize = 256 ;
	CPerlinNoise cPerlin ;
	cPerlin.Initialize(1.0f/4.0f, 8, nMapSize) ;

	float value ;
	int count=0 ;
	BYTE *pbyBuffer = new BYTE[nMapSize*nMapSize] ;
	for(y=0 ; y<nMapSize ; y++)
	{
		for(x=0 ; x<nMapSize ; x++, count++)
		{
			value = cPerlin.PerlinNoise2D((float)x, (float)y) ;
			//pbyBuffer[count] = ((value+1.0f)*256.0f/2.0f) ;
			pbyBuffer[count] = (BYTE)((value+1.0f)*64.0f/2.0f) ;
			pbyBuffer[count] += 191 ;
		}
	}

	if(FAILED(D3DXCreateTexture(m_pd3dDevice, nMapSize, nMapSize, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8, D3DPOOL_MANAGED, ppTex)))
		return false ;

	FillAlphaTexture((*ppTex), pbyBuffer) ;
	(*ppTex)->GenerateMipSubLevels() ;

    SAFE_DELETEARRAY(pbyBuffer) ;
    return true ;
}

void CSecretSod::SetSodBoard(float fXInterval, float fZInterval)
{
	SClumpMark sOldMark ;

	if(m_sClumpMark.bEnable)
	{
		sOldMark = m_sClumpMark ;
		m_sClumpMark.Release() ;
	}

	m_fXInterval = fXInterval ;
	m_fZInterval = fZInterval ;


	float ftemp ;
	unsigned long lWidth, lHeight ;

	ftemp = m_pcTerrain->m_sHeightMap.lWidth*m_pcTerrain->m_sHeightMap.nTileSize/m_fXInterval ;
	lWidth = (unsigned long)ftemp ;

	ftemp = m_pcTerrain->m_sHeightMap.lHeight*m_pcTerrain->m_sHeightMap.nTileSize/m_fZInterval ;
	lHeight = (unsigned long)ftemp ;

	//m_sClumpMark.initialize((unsigned long)(m_pcTerrain->m_sHeightMap.lWidth*m_pcTerrain->m_sHeightMap.nTileSize/m_fXInterval), (unsigned long)(m_pcTerrain->m_sHeightMap.lHeight*m_pcTerrain->m_sHeightMap.nTileSize/m_fZInterval)) ;
	m_sClumpMark.initialize(lWidth, lHeight) ;
	assert(m_sClumpMark.pdwMark != NULL) ;

	if(sOldMark.bEnable)
	{
		float fXFactor = (float)sOldMark.lWidth/(float)m_sClumpMark.lWidth ;
		float fZFactor = (float)sOldMark.lHeight/(float)m_sClumpMark.lHeight ;
		unsigned long lOldZ, lOldX ;

		for(unsigned long z=0 ; z<m_sClumpMark.lHeight ; z++)
		{
			lOldZ = (unsigned long)(z*fZFactor+0.5f) ;
			if(lOldZ >= sOldMark.lHeight)
				lOldZ = sOldMark.lHeight-1 ;

			for(unsigned long x=0 ; x<m_sClumpMark.lWidth ; x++)
			{
				lOldX = (unsigned long)(x*fXFactor+0.5f) ;
				if(lOldX >= sOldMark.lWidth)
					lOldX = sOldMark.lWidth-1 ;

				if(sOldMark.GetMark(lOldX, lOldZ))
					int xxx=0 ;

				m_sClumpMark.SetMark(x, z, sOldMark.GetMark(lOldX, lOldZ)) ;
			}
		}
		sOldMark.Release() ;
	}
}
void CSecretSod::RefreshClumps()
{
	SClump sClump ;
	float fx, fz ;
	unsigned long nx, nz ;
    m_nNumClump = 0 ;
	m_nNumVertex = m_nNumIndex = 0 ;
	for(nz=0 ; nz<m_sClumpMark.lHeight ; nz++)
	{
		for(nx=0 ; nx<m_sClumpMark.lWidth ; nx++)
		{
			//if(m_ppsSodMark[nz][nx].bEnable)
			if(m_sClumpMark.GetMark(nx, nz))
			{
				fx = (float)nx*m_fXInterval ;
				fz = (float)nz*m_fZInterval ;
				fx -= (m_pcBrushTool->m_pcTerrain->m_sHeightMap.lWidth*m_pcBrushTool->m_pcTerrain->m_sHeightMap.nTileSize/2.0f) ;
				fz -= (m_pcBrushTool->m_pcTerrain->m_sHeightMap.lHeight*m_pcBrushTool->m_pcTerrain->m_sHeightMap.nTileSize/2.0f) ;

				if(nz%2)// odd number
					fx += (m_fXInterval*0.25f) ;
				//fx += (m_fXInterval/(float)(1024-(rand()%2048))) ;

				m_psClumps[m_nNumClump].vPos.x = fx ;
				m_psClumps[m_nNumClump].vPos.y = m_pcBrushTool->m_pcTerrain->m_sHeightMap.GetHeightMap(fx, fz) ;
				m_psClumps[m_nNumClump].vPos.z = fz ;
				m_psClumps[m_nNumClump].nKind = rand()%m_nNumClumpKind ;
				m_nNumClump++ ;
			}
		}
	}
}
SClumpKind *CSecretSod::GetClumpKind(int n)
{
	if(n > m_nNumClumpKind)
		return NULL ;
	return m_apsClumpKind[n] ;
}
SClump *CSecretSod::getClumps()
{
	return m_psClumps ;
}
int CSecretSod::getNumClump()
{
	return m_nNumClump ;
}

////////////////////////////////////////////////////////////
//CSecretSodEffect
////////////////////////////////////////////////////////////
CSecretSodEffect::CSecretSodEffect()
{
	m_pDecl = NULL ;
	m_pEffect = NULL ;
	m_hTechnique = NULL ;
}
CSecretSodEffect::~CSecretSodEffect()
{
	Release() ;
}
void CSecretSodEffect::Release()
{
	SAFE_RELEASE(m_pEffect) ;
	SAFE_RELEASE(m_pDecl) ;
}
HRESULT CSecretSodEffect::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName)
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
	SetD3DXHANDLE(&m_hvAxisX, "vAxisX") ;
	SetD3DXHANDLE(&m_hvAxisY, "vAxisY") ;
	SetD3DXHANDLE(&m_hvCameraPos, "vCameraPos") ;
	SetD3DXHANDLE(&m_hmatWLP, "matWLP") ;
	SetD3DXHANDLE(&m_hmatWLPT, "matWLPT") ;
	return S_OK ;
}
bool CSecretSodEffect::SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName)
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

//////////////////////////////////////////////////////////////////////////////////
//CSecretSodManager
//////////////////////////////////////////////////////////////////////////////////

DWORD GenerateClumpProc(CSecretSodManager *pcSodManager)
{
	while(pcSodManager->m_nGenerateClumpThreadStatus != CSecretSodManager::GCSTA_TERMINATE)
	{
		pcSodManager->GenerateClumps() ;

		pcSodManager->m_nGenerateClumpThreadStatus = CSecretSodManager::GCSTA_GENERATED ;
		SuspendThread(pcSodManager->m_hGenerateClumpThread) ;
	}

	CloseHandle(pcSodManager->m_hGenerateClumpThread) ;
	pcSodManager->m_hGenerateClumpThread = NULL ;
	TRACE("end GenerateClumpProc\r\n") ;
	return 0 ;
}

CSecretSodManager::CSecretSodManager() :
MAXNUM_SOD(64)
{
	m_pd3dDevice = NULL ;
	m_pcTexContainer = NULL ;
	m_pcBrushTool = NULL ;
	m_pcSodEffect = NULL ;
	m_pcCurrentSod = NULL ;
	m_nNumClumpKind = 0 ;
	m_pcShadowMap = NULL ;

	m_nGenerateClumpThreadStatus = CGSTA_WAITING ;
	m_hGenerateClumpThread = NULL ;
	m_dwGenerateClumpThreadID = (DWORD)-1 ;

	_InitClumpKind() ;
}
CSecretSodManager::~CSecretSodManager()
{
	Release() ;
}
void CSecretSodManager::Release()
{
	if(m_nAttr & ATTR_USINGTHREAD)
	{
		m_nGenerateClumpThreadStatus = GCSTA_TERMINATE ;
		int nSuspendCount ;
		do {
			nSuspendCount = ResumeThread(m_hGenerateClumpThread) ;
		} while(nSuspendCount<0) ;

		while(m_hGenerateClumpThread) {} ;

		OutputStringforRelease("GenerateClumpThread is Ternimated in CSecretSodManager::Release()\r\n") ;
	}

	for(int i=0 ; i<m_cSods.nCurPos ; i++)
		m_cSods.GetAt(i)->Release() ;
	m_cSods.Reset() ;

	SAFE_DELETE(m_pcTexContainer) ;
	SAFE_DELETE(m_pcBrushTool) ;
	SAFE_DELETE(m_pcSodEffect) ;
	SAFE_DELETEARRAY(m_psClumpKind) ;
}
bool CSecretSodManager::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, CSecretPicking *pcPicking, int nAttr)
{
	m_nAttr = nAttr ;
	m_pd3dDevice = pd3dDevice ;
	m_cSods.Initialize(MAXNUM_SOD) ;

	m_pcTexContainer = new CSecretTextureContainer() ;
	m_pcTexContainer->Initialize("../../Media/map/sod/sod.txr", pd3dDevice) ;

	m_pcBrushTool = new CSecretBrushTool() ;
	m_pcBrushTool->Initialize(m_pd3dDevice, pcPicking) ;

	m_pcSodEffect = new CSecretSodEffect() ;
	m_pcSodEffect->Initialize(m_pd3dDevice, "Shader/Sod.fx") ;
	m_pcSodEffect->m_psEssentialElements = psEssentialElements ;

	if(m_nAttr & ATTR_USINGTHREAD)
	{
		m_hGenerateClumpThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GenerateClumpProc, this, CREATE_SUSPENDED, &m_dwGenerateClumpThreadID) ;
		m_nGenerateClumpThreadStatus = CGSTA_WAITING ;
	}

	return true ;
}
void CSecretSodManager::GenerateClumps()
{
	for(int i=0 ; i<m_cSods.nCurPos ; i++)
		m_cSods.GetAt(i)->GenerateClumps(&m_vCameraPos) ;
}
void CSecretSodManager::Process(int nPlanting, Matrix4 *pmatView, D3DXMATRIX *pmatProj)
{
	m_pcBrushTool->Process(pmatView, pmatProj) ;

	if(!(nPlanting == CSecretSod::IDLE))
	{
		if(m_pcCurrentSod)
		{
			m_pcCurrentSod->Process(nPlanting) ;
		}
	}

	if(m_nAttr & ATTR_USINGTHREAD)
	{
		if(m_cSods.nCurPos > 0)
		{
			if(m_nGenerateClumpThreadStatus == CGSTA_WAITING)
			{
				m_vCameraPos.set(m_pcSodEffect->m_psEssentialElements->vCameraPos.x, m_pcSodEffect->m_psEssentialElements->vCameraPos.y, m_pcSodEffect->m_psEssentialElements->vCameraPos.z) ;

				int nSuspendCount ;
				do {
					nSuspendCount = ResumeThread(m_hGenerateClumpThread) ;
				} while(nSuspendCount > 0) ;
				m_nGenerateClumpThreadStatus = GCSTA_GENERATING ;
			}
			else if(m_nGenerateClumpThreadStatus == GCSTA_GENERATED)
			{
				for(int i=0 ; i<m_cSods.nCurPos ; i++)
					m_cSods.GetAt(i)->copyVertices() ;

				m_nGenerateClumpThreadStatus = CGSTA_WAITING ;
			}
		}
	}
	else
	{
		m_vCameraPos.set(m_pcSodEffect->m_psEssentialElements->vCameraPos.x, m_pcSodEffect->m_psEssentialElements->vCameraPos.y, m_pcSodEffect->m_psEssentialElements->vCameraPos.z) ;

		for(int i=0 ; i<m_cSods.nCurPos ; i++)
		{
			m_cSods.GetAt(i)->GenerateClumps(&m_vCameraPos) ;
			m_cSods.GetAt(i)->copyVertices() ;
		}
	}
}
void CSecretSodManager::CreateSod(char *pszName, float fXInterval, float fZInterval, data::SCString *psString, int nNum)
{
	CSecretSod cSod ;
	sprintf(cSod.m_szName, "%s", pszName) ;
	cSod.Initialize(m_pd3dDevice, m_pcBrushTool->m_pcTerrain, m_pcTexContainer, m_pcBrushTool, m_pcSodEffect) ;

    for(int i=0 ; i<nNum ; i++)
	{
		cSod.m_apsClumpKind[i] = _FindClumpKind(psString[i].str) ;
		assert(cSod.m_apsClumpKind[i]) ;
	}
	cSod.m_nNumClumpKind = nNum ;

	cSod.SetSodBoard(fXInterval, fZInterval) ;

	m_cSods.Insert(&cSod) ;
}
void CSecretSodManager::SelectSod(int nNum)
{
	m_pcCurrentSod = m_cSods.GetAt(nNum) ;
}
void CSecretSodManager::DeleteSod(int nNum)
{
	CSecretSod *pcSod = m_cSods.Delete(nNum) ;
	if(pcSod)
	{
		pcSod->Release() ;
		if(pcSod == m_pcCurrentSod)
			m_pcCurrentSod = NULL ;
	}
}
void CSecretSodManager::Render()
{
	if(m_bEnable)
		m_pcBrushTool->Render() ;

	for(int i=0 ; i<m_cSods.nCurPos ; i++)
		m_cSods.GetAt(i)->Render(m_pcShadowMap) ;
}
SClumpKind *CSecretSodManager::_FindClumpKind(char *pszName)
{
	for(int i=0 ; i<m_nNumClumpKind ; i++)
	{
		if(strcmp(m_psClumpKind[i].szName, pszName) == 0)
            return &m_psClumpKind[i] ;
	}
	return NULL ;
}
int CSecretSodManager::GetIndexofClumpKind(SClumpKind *psClumpKind)
{
	for(int i=0 ; i<m_nNumClumpKind ; i++)
	{
		if(strcmp(m_psClumpKind[i].szName, psClumpKind->szName) == 0)
			return i ;
	}
    return -1 ;
}
void CSecretSodManager::SetTerrain(CSecretTerrain *pcTerrain)
{
	m_pcBrushTool->SetTerrain(pcTerrain) ;
}
void CSecretSodManager::SetShadowMap(CSecretShadowMap *pcShadowMap)
{
	m_pcShadowMap = pcShadowMap ;
}
void CSecretSodManager::SetClumpKind(CSecretSod *pcSod, data::SCString *psString, int nNum)
{
	if(!pcSod)
		return ;

    for(int i=0 ; i<nNum ; i++)
	{
		pcSod->m_apsClumpKind[i] = _FindClumpKind(psString[i].str) ;
		assert(pcSod->m_apsClumpKind[i]) ;
	}
	pcSod->m_nNumClumpKind = nNum ;    
}
void CSecretSodManager::ResetSods()
{
	for(int i=0 ; i<m_cSods.nCurPos ; i++)
		m_cSods.GetAt(i)->Release() ;
	m_cSods.Reset() ;
	m_pcCurrentSod = NULL ;
}
CSecretSod *CSecretSodManager::getSod(char *pszName)
{
	CSecretSod *pSod=NULL ;
	for(int i=0 ; i<m_cSods.nCurPos ; i++)
	{
		pSod = m_cSods.GetAt(i) ;
		if(!strcmp(pSod->m_szName, pszName))
			return pSod ;
	}
	return NULL ;
}

void CSecretSodManager::_InitClumpKind()
{
	m_nNumClumpKind = 51 ;
	m_psClumpKind = new SClumpKind[m_nNumClumpKind] ;

	int count=0 ;
	float xfactor, yfactor, xshift, yshift, size=2048.0f, x, y ;
	xfactor = 1.0f/size ;
    yfactor = 1.0f/size ;
	//dist : pixel = 4cm : 23pixel

	//GREEN ROUGH
	xshift = 128.0f*xfactor ;
	yshift = 128.0f*yfactor ;
    x = y = 0.0f ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*0.0f), 128.0f*xfactor, 119.0f*yfactor, xfactor, yfactor, "G_rough_5cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*0.0f), 128.0f*xfactor, 119.0f*yfactor, xfactor, yfactor, "G_rough_7cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*0.0f), 128.0f*xfactor, 119.0f*yfactor, xfactor, yfactor, "G_rough_8cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*3.0f, y+yshift*0.0f), 128.0f*xfactor, 120.0f*yfactor, xfactor, yfactor, "G_rough_10cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*4.0f, y+yshift*0.0f), 128.0f*xfactor, 120.0f*yfactor, xfactor, yfactor, "G_rough_12cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*5.0f, y+yshift*0.0f), 128.0f*xfactor, 121.0f*yfactor, xfactor, yfactor, "G_rough_14cm") ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*1.0f), 128.0f*xfactor, 121.0f*yfactor, xfactor, yfactor, "G_rough_16cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*1.0f), 128.0f*xfactor, 122.0f*yfactor, xfactor, yfactor, "G_rough_18cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*1.0f), 128.0f*xfactor, 122.0f*yfactor, xfactor, yfactor, "G_rough_21cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*3.0f, y+yshift*1.0f), 128.0f*xfactor, 123.0f*yfactor, xfactor, yfactor, "G_rough_25cm") ;

    xshift = 256.0f*xfactor ;
	yshift = 160.0f*yfactor ;
	x = 0.0f ;
	y = 256.0f*yfactor ;

    m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*0.0f), 256.0f*xfactor, 153.0f*yfactor, xfactor, yfactor, "G_rough_10cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*0.0f), 256.0f*xfactor, 152.0f*yfactor, xfactor, yfactor, "G_rough_12cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*0.0f), 256.0f*xfactor, 154.0f*yfactor, xfactor, yfactor, "G_rough_14cmx2") ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*1.0f), 256.0f*xfactor, 154.0f*yfactor, xfactor, yfactor, "G_rough_16cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*1.0f), 256.0f*xfactor, 154.0f*yfactor, xfactor, yfactor, "G_rough_18cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*1.0f), 256.0f*xfactor, 156.0f*yfactor, xfactor, yfactor, "G_rough_21cmx2") ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*2.0f), 256.0f*xfactor, 158.0f*yfactor, xfactor, yfactor, "G_rough_25cmx2") ;

	//OLIVE ROUGH
	xshift = 128.0f*xfactor ;
	yshift = 128.0f*yfactor ;
    x = 768.0f*xfactor ;
	y = 0.0f ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*0.0f), 128.0f*xfactor, 119.0f*yfactor, xfactor, yfactor, "O_rough_5cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*0.0f), 128.0f*xfactor, 119.0f*yfactor, xfactor, yfactor, "O_rough_7cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*0.0f), 128.0f*xfactor, 119.0f*yfactor, xfactor, yfactor, "O_rough_8cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*3.0f, y+yshift*0.0f), 128.0f*xfactor, 120.0f*yfactor, xfactor, yfactor, "O_rough_10cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*4.0f, y+yshift*0.0f), 128.0f*xfactor, 120.0f*yfactor, xfactor, yfactor, "O_rough_12cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*5.0f, y+yshift*0.0f), 128.0f*xfactor, 121.0f*yfactor, xfactor, yfactor, "O_rough_14cm") ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*1.0f), 128.0f*xfactor, 121.0f*yfactor, xfactor, yfactor, "O_rough_16cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*1.0f), 128.0f*xfactor, 122.0f*yfactor, xfactor, yfactor, "O_rough_18cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*1.0f), 128.0f*xfactor, 122.0f*yfactor, xfactor, yfactor, "O_rough_21cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*3.0f, y+yshift*1.0f), 128.0f*xfactor, 123.0f*yfactor, xfactor, yfactor, "O_rough_25cm") ;

    xshift = 256.0f*xfactor ;
	yshift = 160.0f*yfactor ;
	x = 768.0f*xfactor ;
	y = 256.0f*yfactor ;

    m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*0.0f), 256.0f*xfactor, 153.0f*yfactor, xfactor, yfactor, "O_rough_10cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*0.0f), 256.0f*xfactor, 152.0f*yfactor, xfactor, yfactor, "O_rough_12cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*0.0f), 256.0f*xfactor, 154.0f*yfactor, xfactor, yfactor, "O_rough_14cmx2") ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*1.0f), 256.0f*xfactor, 154.0f*yfactor, xfactor, yfactor, "O_rough_16cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*1.0f), 256.0f*xfactor, 154.0f*yfactor, xfactor, yfactor, "O_rough_18cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*1.0f), 256.0f*xfactor, 156.0f*yfactor, xfactor, yfactor, "O_rough_21cmx2") ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*2.0f), 256.0f*xfactor, 158.0f*yfactor, xfactor, yfactor, "O_rough_25cmx2") ;

	//BLUE ROUGH
	xshift = 128.0f*xfactor ;
	yshift = 128.0f*yfactor ;
	x = 0.0f ;
    y = 768.0f*yfactor ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*0.0f), 128.0f*xfactor, 119.0f*yfactor, xfactor, yfactor, "B_rough_5cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*0.0f), 128.0f*xfactor, 119.0f*yfactor, xfactor, yfactor, "B_rough_7cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*0.0f), 128.0f*xfactor, 119.0f*yfactor, xfactor, yfactor, "B_rough_8cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*3.0f, y+yshift*0.0f), 128.0f*xfactor, 120.0f*yfactor, xfactor, yfactor, "B_rough_10cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*4.0f, y+yshift*0.0f), 128.0f*xfactor, 120.0f*yfactor, xfactor, yfactor, "B_rough_12cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*5.0f, y+yshift*0.0f), 128.0f*xfactor, 121.0f*yfactor, xfactor, yfactor, "B_rough_14cm") ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*1.0f), 128.0f*xfactor, 121.0f*yfactor, xfactor, yfactor, "B_rough_16cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*1.0f), 128.0f*xfactor, 122.0f*yfactor, xfactor, yfactor, "B_rough_18cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*1.0f), 128.0f*xfactor, 122.0f*yfactor, xfactor, yfactor, "B_rough_21cm") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*3.0f, y+yshift*1.0f), 128.0f*xfactor, 123.0f*yfactor, xfactor, yfactor, "B_rough_25cm") ;

    xshift = 256.0f*xfactor ;
	yshift = 160.0f*yfactor ;
	x = 0.0f ;
	y = (768.0f+256.0f)*yfactor ;

    m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*0.0f), 256.0f*xfactor, 153.0f*yfactor, xfactor, yfactor, "B_rough_10cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*0.0f), 256.0f*xfactor, 152.0f*yfactor, xfactor, yfactor, "B_rough_12cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*0.0f), 256.0f*xfactor, 154.0f*yfactor, xfactor, yfactor, "B_rough_14cmx2") ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*1.0f), 256.0f*xfactor, 154.0f*yfactor, xfactor, yfactor, "B_rough_16cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*1.0f, y+yshift*1.0f), 256.0f*xfactor, 154.0f*yfactor, xfactor, yfactor, "B_rough_18cmx2") ;
	m_psClumpKind[count++].set(Vector2(x+xshift*2.0f, y+yshift*1.0f), 256.0f*xfactor, 156.0f*yfactor, xfactor, yfactor, "B_rough_21cmx2") ;

	m_psClumpKind[count++].set(Vector2(x+xshift*0.0f, y+yshift*2.0f), 256.0f*xfactor, 158.0f*yfactor, xfactor, yfactor, "B_rough_25cmx2") ;

	assert(m_nNumClumpKind == count) ;
}