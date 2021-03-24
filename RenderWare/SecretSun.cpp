#include "SecretSun.h"
#include "SecretSkyDome.h"

CSecretSun::CSecretSun()
{
	m_pd3dDevice = NULL ;
	m_pVB = NULL ;
	m_pIB = NULL ;
	m_pcEffect = NULL ;
	m_nNumVertex = m_nNumIndex = 0 ;
}
CSecretSun::~CSecretSun()
{
	release() ;
}
void CSecretSun::release()
{
	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pIB) ;
}
bool CSecretSun::initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretSkyDomeEffect *pcEffect)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcEffect = pcEffect ;

	return true ;
}
void CSecretSun::render()
{
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SSun_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pIB) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                       Shader Setting                                           //
	////////////////////////////////////////////////////////////////////////////////////////////////////
	SEffectEssentialElements *psEffectElements = m_pcEffect->m_psEssentialElements ;
	D3DXVECTOR3 a ;
	D3DXVECTOR4 vSunToCamera ;
	D3DXMATRIX mat ;

	m_pcEffect->m_pEffect->BeginPass(1) ;

	m_pd3dDevice->SetFVF(D3DFVF_SUNVERTEX) ;

	m_pcEffect->m_pEffect->SetVector(m_pcEffect->m_hvCameraPos, &psEffectElements->vCameraPos) ;

	a.x = psEffectElements->vCameraPos.x - m_vPosition.x ;
	a.y = psEffectElements->vCameraPos.y - m_vPosition.y ;
	a.z = psEffectElements->vCameraPos.z - m_vPosition.z ;
	D3DXVec3Normalize(&a, &a) ;
	vSunToCamera.x = a.x ;
	vSunToCamera.y = a.y ;
	vSunToCamera.z = a.z ;
	vSunToCamera.w = 0.0f;
	m_pcEffect->m_pEffect->SetVector(m_pcEffect->m_hvSunToCamera, &vSunToCamera) ;

	mat = psEffectElements->matWorld * psEffectElements->matView * psEffectElements->matProj ;
	m_pcEffect->m_pEffect->SetMatrix(m_pcEffect->m_hmatWVP, &mat) ;

	m_pcEffect->m_pEffect->SetTexture("SunMap", m_pTex) ;

	m_pcEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2) ;

	m_pcEffect->m_pEffect->EndPass() ;
}
bool CSecretSun::setPosition(D3DXVECTOR4 vSunDir, float width, float height, float dist)
{
	SSun_Vertex sVertices[4], *psVertices ;
	SSun_Index sIndices[2], *psIndices ;

	m_vPosition.x = vSunDir.x * dist ;
	m_vPosition.y = vSunDir.y * dist ;
	m_vPosition.z = vSunDir.z * dist ;

	m_nNumVertex = 4 ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(4*sizeof(SSun_Vertex), 0, D3DFVF_SUNVERTEX, D3DPOOL_MANAGED, &m_pVB, NULL)))
		return false ;

	if(FAILED(m_pVB->Lock(0, m_nNumVertex*sizeof(SSun_Vertex), (void **)&psVertices, 0)))
		return false ;

	psVertices[0].pos = psVertices[1].pos = psVertices[2].pos = psVertices[3].pos = m_vPosition ;

	psVertices[0].t.x = 0.0f ;
	psVertices[0].t.y = 0.0f ;

	psVertices[1].t.x = 1.0f ;
	psVertices[1].t.y = 0.0f ;

	psVertices[2].t.x = 0.0f ;
	psVertices[2].t.y = 1.0f ;

	psVertices[3].t.x = 1.0f ;
	psVertices[3].t.y = 1.0f ;

	//upper-left
	psVertices[0].rightFactor = -width/2.0f ;
	psVertices[0].upFactor = height/2.0f ;

	//upper-right
	psVertices[1].rightFactor = width/2.0f ;
	psVertices[1].upFactor = height/2.0f ;

	//lower-left
	psVertices[2].rightFactor = -width/2.0f ;
	psVertices[2].upFactor = -height/2.0f ;

	//lower-right
	psVertices[3].rightFactor = width/2.0f ;
	psVertices[3].upFactor = -height/2.0f ;

	m_pVB->Unlock() ;

	if(FAILED(m_pd3dDevice->CreateIndexBuffer(2*sizeof(SSun_Index), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB, NULL)))
		return false ;

	if(FAILED(m_pIB->Lock(0, 2*sizeof(SSun_Index), (void **)&psIndices, 0)))
		return false ;

	psIndices[0].anIndex[0] = 0 ;
	psIndices[0].anIndex[1] = 1 ;
	psIndices[0].anIndex[2] = 2 ;

	psIndices[1].anIndex[0] = 2 ;
	psIndices[1].anIndex[1] = 1 ;
	psIndices[1].anIndex[2] = 3 ;

	m_pIB->Unlock() ;

	return true ;
}
void CSecretSun::setTexture(LPDIRECT3DTEXTURE9 pTex)
{
	m_pTex = pTex ;
}