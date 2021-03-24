#include "SecretBoneMesh.h"

//정점 오브젝트 생성
D3DVERTEXELEMENT9 decl_Bone[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//법선에 대한 설정
	D3DDECL_END()
} ;

CSecretBoneMesh::CSecretBoneMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) : CSecretMesh(lEigenIndex, pd3dDevice)
{
    m_bWireFrame = true ;
    
	TRACE("CSecretBoneMesh construction EigenIndex=%d\r\n", lEigenIndex) ;
}

CSecretBoneMesh::~CSecretBoneMesh()
{
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;

	TRACE("CSecretBoneMesh destruction\r\n") ;
}

HRESULT CSecretBoneMesh::Initialize(SMesh *pMesh)
{
	int i ;

	TRACE("BoneMesh Init NodeName=%s, ParentNodeName=%s\r\n", pMesh->szMeshName, pMesh->szParentName) ;

	CSecretMesh::Initialize(pMesh) ;

	m_pVertexElement = decl_Bone ;

	///////////////////////////////////////////////////
	//Create Vertices
	///////////////////////////////////////////////////
	m_lNumVertex = pMesh->nNumVertex ;

	m_psVertices = new SBoneMesh_Vertex[m_lNumVertex] ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SBoneMesh_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_BONEMESHVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return E_FAIL ;

	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SBoneMesh_Vertex), &pVertices, 0)))
		return E_FAIL ;

	SBoneMesh_Vertex *psVertices = (SBoneMesh_Vertex *)pVertices ;
	for(i=0 ; i<(int)m_lNumVertex ; i++)
	{
		psVertices->pos = D3DXVECTOR3(pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z)  ;
		psVertices->normal = D3DXVECTOR3(pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z)  ;
//		TRACE("[%d] pos  x=%.3f y=%.3f, z=%.3f\r\n", i, psVertices->pos.x, psVertices->pos.y, psVertices->pos.z) ;

		psVertices++ ;
	}

	m_pVB->Unlock() ;

    ///////////////////////////////////////////////////
	//Create Indices
	///////////////////////////////////////////////////
	m_lNumTriangle = pMesh->nNumTriangle ;

	m_psIndices = new SBoneMesh_Index[m_lNumTriangle] ;

	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SBoneMesh_Index), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVI, NULL)))
		return E_FAIL ;

	void *pIndices ;
	if(FAILED(m_pVI->Lock(0, m_lNumTriangle*sizeof(SBoneMesh_Index), (void **)&pIndices, 0)))
		return E_FAIL ;

    SBoneMesh_Index *psIndices = (SBoneMesh_Index *)pIndices ;
	for(i=0 ; i<(int)m_lNumTriangle ; i++)
	{
		psIndices->anIndex[0] = pMesh->psTriangle[i].anVertexIndex[0] ;
		psIndices->anIndex[1] = pMesh->psTriangle[i].anVertexIndex[1] ;
		psIndices->anIndex[2] = pMesh->psTriangle[i].anVertexIndex[2] ;

//		TRACE("[%d] index[0]=%d index[1]=%d index[2]=%d\r\n", i, psIndices->anIndex[0], psIndices->anIndex[1], psIndices->anIndex[2]) ;
		psIndices++ ;
	}

	m_pVI->Unlock() ;

	/////////////////////////////////////////////////////////////////////////
	//Create AniTrack
	/////////////////////////////////////////////////////////////////////////
	m_bAnimaion = true ;
	sprintf(m_cAniTrack.m_szNodeName, "%s", m_szNodeName) ;
	sprintf(m_cAniTrack.m_szParentNodeName, "%s", m_szParentNodeName) ;

	if(pMesh->nNumMeshKeyPos)
		m_cAniTrack.InitKeyPos(pMesh->apMeshKeyPos, pMesh->nNumMeshKeyPos) ;

	if(pMesh->nNumMeshKeyRot)
		m_cAniTrack.InitKeyRot(pMesh->apMeshKeyRot, pMesh->nNumMeshKeyRot) ;

	if(pMesh->nNumMeshKeyScale)
		m_cAniTrack.InitKeyScale(pMesh->apMeshKeyScale, pMesh->nNumMeshKeyScale) ;
	///////////////////////////////////////////////////////////////////////////

    return S_OK ;
}

void CSecretBoneMesh::Render()
{
	if(m_lNumTriangle <= 0)
		return ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetFVF(D3DFVF_BONEMESHVERTEX) ;
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SBoneMesh_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

}