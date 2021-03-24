#include "SecretShadowMesh.h"
#include "def.h"

CSecretShadowMesh::CSecretShadowMesh(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_pVB = NULL ;
	m_pVI = NULL ;
	m_pd3dDevice = pd3dDevice ;
	m_lNumVertex = m_lNumIndex = 0 ;
	m_nNumSubset = 0 ;
	m_psIndexSubset = NULL ;
	m_ppSubsetIB = NULL ;
}

CSecretShadowMesh::~CSecretShadowMesh()
{
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;
	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pVI) ;

	SAFE_DELETEARRAY(m_psIndexSubset) ;
    for(int i=0 ; i<m_nNumSubset ; i++)
		SAFE_RELEASE(m_ppSubsetIB[i]) ;
    SAFE_DELETEARRAY(m_ppSubsetIB) ;
}

HRESULT CSecretShadowMesh::SetVertexData(SShadowMesh_Vertex *psSrcVertices, UINT lNumVertex)
{
	UINT i ;
	m_lNumVertex = lNumVertex ;
    m_psVertices = new SShadowMesh_Vertex[m_lNumVertex] ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SShadowMesh_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_SHADOWMESHVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddVBSize(m_lNumVertex*sizeof(SShadowMesh_Vertex)) ;

	void *pv ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SShadowMesh_Vertex), &pv, 0)))
		return E_FAIL ;

	SShadowMesh_Vertex *psVertices = (SShadowMesh_Vertex *)pv ;
	for(i=0 ; i<m_lNumVertex ; i++)
	{
		psVertices->pos = psSrcVertices[i].pos ;
		psVertices->normal = psSrcVertices[i].normal ;
		psVertices++ ;

		m_psVertices[i] = psSrcVertices[i] ;
	}

	m_pVB->Unlock() ;

	return S_OK ;
}

HRESULT CSecretShadowMesh::SetIndexData(SShadowMesh_Index *psSrcIndices, UINT lNumIndex)
{
    UINT i ;
	m_lNumIndex = lNumIndex ;
    m_psIndices = new SShadowMesh_Index[m_lNumIndex] ;

	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumIndex*sizeof(SShadowMesh_Index), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVI, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddIBSize(m_lNumIndex*sizeof(SShadowMesh_Index)) ;

	void *pv ;
	if(FAILED(m_pVI->Lock(0, m_lNumIndex*sizeof(SShadowMesh_Index), &pv, 0)))
		return E_FAIL ;

	SShadowMesh_Index *psIndices = (SShadowMesh_Index *)pv ;
	for(i=0 ; i<m_lNumIndex ; i++)
	{
		psIndices->anIndex[0] = psSrcIndices[i].anIndex[0] ;
		psIndices->anIndex[1] = psSrcIndices[i].anIndex[1] ;
		psIndices->anIndex[2] = psSrcIndices[i].anIndex[2] ;
		psIndices++ ;

		m_psIndices[i] = psSrcIndices[i] ;
	}

	m_pVI->Unlock() ;

	return S_OK ;
}
void CSecretShadowMesh::SetSubsetIndexData(SShadowMesh_IndexSubSet *psIndexSubset, int nNumSubset)
{
}
void CSecretShadowMesh::Render()
{
	m_pd3dDevice->SetFVF(D3DFVF_SHADOWMESHVERTEX) ;
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SShadowMesh_Vertex)) ;
	
	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumIndex) ;

	//m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}