#include "SecretSkinnedMeshVS.h"


//정점 오브젝트 생성
D3DVERTEXELEMENT9 decl_SkinnedVS[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//법선에 대한 설정
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정 tu, tv 범프맵핑일 경우 사용
	{0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},//Blend Weight
	{0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},//Blend Indices
	D3DDECL_END()
} ;

int compare_Weight_greaterVS(const void *pv1, const void *pv2)
{
	SBoneWeight *psBoneWeight1 = (SBoneWeight *)pv1 ;
	SBoneWeight *psBoneWeight2 = (SBoneWeight *)pv2 ;

	float fValue = (psBoneWeight2->fWeight - psBoneWeight1->fWeight)*512.0f ;

    return (int)fValue ;
}

CSecretSkinnedMeshVS::CSecretSkinnedMeshVS(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) : CSecretMesh(lEigenIndex, pd3dDevice)
{
	m_bWireFrame = false ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;
    
	TRACE("CSecretSkinnedMeshVS construction EigenIndex=%d\r\n", lEigenIndex) ;
}

CSecretSkinnedMeshVS::~CSecretSkinnedMeshVS()
{
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;

	TRACE("CSecretSkinnedMeshVS destruction\r\n") ;
}

HRESULT CSecretSkinnedMeshVS::Initialize(SMesh *pMesh)
{
	int i, n ;
	int nNumIndex ;
	DWORD dwShift=0 ;

	TRACE("SkinnedMesh Init NodeName=%s, ParentNodeName=%s\r\n", pMesh->szMeshName, pMesh->szParentName) ;

	SBoneWeight asBoneWeight[12] ;

	//Overriding Parent Initialize
	CSecretMesh::Initialize(pMesh) ;

	m_pVertexElement = decl_SkinnedVS ;

//	D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE] ;
//	D3DXDeclaratorFromFVF(D3DFVF_SKINNEDMESHVERTEX_VS, decl) ;

	///////////////////////////////////////////////////
	//Create Vertices
	///////////////////////////////////////////////////
	m_lNumVertex = pMesh->nNumVertex ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SSkinnedMeshVS_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_SKINNEDMESHVERTEX_VS, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddVBSize(m_lNumVertex*sizeof(SSkinnedMeshVS_Vertex)) ;

	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SSkinnedMeshVS_Vertex), &pVertices, 0)))
		return E_FAIL ;

	SSkinnedMeshVS_Vertex *psVertices = (SSkinnedMeshVS_Vertex *)pVertices ;
    for(i=0 ; i<(int)m_lNumVertex ; i++)
	{
		psVertices->pos = D3DXVECTOR3(pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z) ;
		psVertices->normal = D3DXVECTOR3(pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z)  ;
		psVertices->t = D3DXVECTOR2(pMesh->psVertex[i].tex.u, pMesh->psVertex[i].tex.v) ;

		memset(asBoneWeight, 0, sizeof(SBoneWeight)*4) ;

        //본가중치를 큰순서데로 정렬한다
		for(n=0 ; n<pMesh->psVertex[i].nNumSkin ; n++)
		{
			asBoneWeight[n].nIndex = pMesh->psVertex[i].apsSkin[n]->nBoneIndex ;
			asBoneWeight[n].fWeight = pMesh->psVertex[i].apsSkin[n]->fWeight ;
		}

		qsort(asBoneWeight, pMesh->psVertex[i].nNumSkin, sizeof(SBoneWeight), compare_Weight_greaterVS) ;

		if(pMesh->psVertex[i].nNumSkin > 3)
			nNumIndex = 4 ;
		else
			nNumIndex = pMesh->psVertex[i].nNumSkin ;

		psVertices->blend.x = asBoneWeight[0].fWeight ;
		psVertices->blend.y = asBoneWeight[1].fWeight ;
		psVertices->blend.z = asBoneWeight[2].fWeight ;
		psVertices->blend.w = 1.0f - (asBoneWeight[0].fWeight + asBoneWeight[1].fWeight + asBoneWeight[2].fWeight) ;

		psVertices->indices.x = (float)asBoneWeight[0].nIndex ;
		psVertices->indices.y = (float)asBoneWeight[1].nIndex ;
		psVertices->indices.z = (float)asBoneWeight[2].nIndex ;
		psVertices->indices.w = (float)asBoneWeight[3].nIndex ;

		psVertices++ ;
	}

	m_pVB->Unlock() ;

    ///////////////////////////////////////////////////
	//Create Indices
	///////////////////////////////////////////////////
	m_lNumTriangle = pMesh->nNumTriangle ;

	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SSkinnedMeshVS_Index), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVI, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddIBSize(m_lNumTriangle*sizeof(SSkinnedMeshVS_Index)) ;

	void *pIndices ;
	if(FAILED(m_pVI->Lock(0, m_lNumTriangle*sizeof(SSkinnedMeshVS_Index), (void **)&pIndices, 0)))
		return E_FAIL ;

    SSkinnedMeshVS_Index *psIndices = (SSkinnedMeshVS_Index *)pIndices ;
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

void CSecretSkinnedMeshVS::Render()
{
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	//m_pd3dDevice->SetFVF(D3DFVF_SKINNEDMESHVERTEX_VS) ;
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SSkinnedMeshVS_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;
	g_cDebugMsg.AddTriangles(m_lNumTriangle) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

}