#include "SecretRigidMeshBump.h"
#include <assert.h>
#include "Matrix.h"
#include "MathOrdinary.h"
#include "SecretShadowMesh.h"

D3DVERTEXELEMENT9 decl_RigidBump[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//법선에 대한 설정
	{0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},//접선벡터 설정
	{0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정
	D3DDECL_END()
} ;


CSecretRigidMeshBump::CSecretRigidMeshBump(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) : CSecretRigidMesh(lEigenIndex, pd3dDevice)
{
	m_psBumpVertices = NULL ;
	//TRACE("CSecretRigidMeshBump construction EigenIndex=%d\r\n", lEigenIndex) ;
}

CSecretRigidMeshBump::~CSecretRigidMeshBump()
{
	SAFE_DELETEARRAY(m_psBumpVertices) ;
	//TRACE("CSecretRigidMeshBump destruction\r\n") ;
}

HRESULT CSecretRigidMeshBump::Initialize(SMesh *pMesh)
{
	int i, n ;

	//TRACE("RigidMesh Init NodeName=%s, ParentNodeName=%s\r\n", pMesh->szMeshName, pMesh->szParentName) ;

	//Overriding Parent Initialize
	CSecretMesh::Initialize(pMesh) ;

	m_pVertexElement = decl_RigidBump ;

	///////////////////////////////////////////////////
	//Create Vertices
	///////////////////////////////////////////////////
	m_lNumVertex = pMesh->nNumVertex ;

	m_psBumpVertices = new SRigidMeshBump_Vertex[m_lNumVertex] ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SRigidMeshBump_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_RIGIDMESHBUMPVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddVBSize(m_lNumVertex*sizeof(SRigidMeshBump_Vertex)) ;

	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SRigidMeshBump_Vertex), &pVertices, 0)))
		return E_FAIL ;

	SRigidMeshBump_Vertex *psVertices = (SRigidMeshBump_Vertex *)pVertices ;
	for(i=0 ; i<(int)m_lNumVertex ; i++)
	{
		psVertices->pos = D3DXVECTOR3(pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z)  ;
		psVertices->normal = D3DXVECTOR3(pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z)  ;
		psVertices->tangent = D3DXVECTOR3(pMesh->psVertex[i].tangent.x, pMesh->psVertex[i].tangent.y, pMesh->psVertex[i].tangent.z)  ;
		psVertices->t = D3DXVECTOR2(pMesh->psVertex[i].tex.u, pMesh->psVertex[i].tex.v) ;

		//TRACE("[%d] pos(%07.3f %07.3f %07.3f) tex(%07.3f %07.3f) \r\n",
		//	i, psVertices->pos.x, psVertices->pos.y, psVertices->pos.z,
		//	psVertices->t.x, psVertices->t.y) ;
		//TRACE("{%3.3ff, %3.3ff, %3.3ff, %3.3ff, %3.3ff, %3.3ff},\r\n",
		//	pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z,
		//	pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z) ;

        m_psBumpVertices[i] = *psVertices ;

		psVertices++ ;
	}

	m_pVB->Unlock() ;

    ///////////////////////////////////////////////////
	//Create Indices
	///////////////////////////////////////////////////
	m_lNumTriangle = pMesh->nNumTriangle ;

	m_psIndices = new SRigidMesh_Index[m_lNumTriangle] ;

	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SRigidMesh_Index), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVI, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddIBSize(m_lNumTriangle*sizeof(SRigidMesh_Index)) ;

	void *pIndices ;
	if(FAILED(m_pVI->Lock(0, m_lNumTriangle*sizeof(SRigidMesh_Index), (void **)&pIndices, 0)))
		return E_FAIL ;

	bool bExist ;
	int nCountMaterialUsed=0, nPrevMaterialID=pMesh->psTriangle[0].nMaterialID ;
	int nCountTriangle=0 ;

	typedef struct
	{
        int nNumTriangle ;
		int nMaterialID ;
	} SFaceSet ;

	SFaceSet asFaceSet[MAXNUM_MATERIAL] ;//SubMaterial 갯수가 255개 이하라고 생각한다.

    SRigidMesh_Index *psIndices = (SRigidMesh_Index *)pIndices ;
	for(i=0 ; i<(int)m_lNumTriangle ; i++)
	{
		psIndices->anIndex[0] = pMesh->psTriangle[i].anVertexIndex[0] ;
		psIndices->anIndex[1] = pMesh->psTriangle[i].anVertexIndex[1] ;
		psIndices->anIndex[2] = pMesh->psTriangle[i].anVertexIndex[2] ;

		m_psIndices[i] = *psIndices ;

		//############################################################//
		//SubMaterial을 사용하는 경우 *MESH_MTLID 의 번호별로 분류작업을 한다.
		//############################################################//
		if(pMesh->psTriangle[i].nMaterialID != nPrevMaterialID)
		{
			bExist = false ;
			for(n=0 ; n<nCountMaterialUsed ; n++)
			{
				if(asFaceSet[n].nMaterialID == nPrevMaterialID)//이전에 MaterialID와 같은 것이 있다면
				{
                    asFaceSet[n].nNumTriangle += nCountTriangle ;
					bExist = true ;
				}
			}

			if(!bExist)
			{
				asFaceSet[nCountMaterialUsed].nNumTriangle = nCountTriangle ;
				asFaceSet[nCountMaterialUsed].nMaterialID = nPrevMaterialID ;
				nCountMaterialUsed++ ;
			}
			nCountTriangle = 0 ;
		}

		nCountTriangle++ ;
        nPrevMaterialID = pMesh->psTriangle[i].nMaterialID ;
		//############################################################//

//		TRACE("[%d] index[0]=%d index[1]=%d index[2]=%d\r\n", i, psIndices->anIndex[0], psIndices->anIndex[1], psIndices->anIndex[2]) ;
//		TRACE("{%2d, %2d, %2d},\r\n", psIndices->anIndex[0], psIndices->anIndex[1], psIndices->anIndex[2]) ;
		psIndices++ ;
	}

	//############################################################//
    //마지막 Set을 저장해야 된다.
	//############################################################//
	bExist = false ;
	for(n=0 ; n<nCountMaterialUsed ; n++)
	{
		if(asFaceSet[n].nMaterialID == pMesh->psTriangle[m_lNumTriangle-1].nMaterialID)//이전 MaterialID와 같은 것이 있다면
		{
			asFaceSet[n].nNumTriangle += nCountTriangle ;
			bExist = true ;
		}
	}
	if(!bExist)
	{
		asFaceSet[nCountMaterialUsed].nNumTriangle = nCountTriangle ;
		asFaceSet[nCountMaterialUsed++].nMaterialID = pMesh->psTriangle[m_lNumTriangle-1].nMaterialID ;
	}
	//############################################################//
	m_pVI->Unlock() ;

	if(pMesh->nAttr & SECRETNODEATTR_USESUBMATERIAL)
	{
		m_psIndexSubSet = new SRigidMesh_IndexSubSet[nCountMaterialUsed] ;
		for(n=0 ; n<nCountMaterialUsed ; n++)
		{
			m_psIndexSubSet[n].nMaterialID = asFaceSet[n].nMaterialID ;
			m_psIndexSubSet[n].nNumTriangle = 0 ;
			m_psIndexSubSet[n].psIndices = new SRigidMesh_Index[asFaceSet[n].nNumTriangle] ;
		}

		for(i=0 ; i<(int)m_lNumTriangle ; i++)
		{
			for(n=0 ; n<nCountMaterialUsed ; n++)
			{
				if(m_psIndexSubSet[n].nMaterialID == pMesh->psTriangle[i].nMaterialID)
				{
					m_psIndexSubSet[n].psIndices[m_psIndexSubSet[n].nNumTriangle++] = m_psIndices[i] ;
					//TRACE("set IndexSet IndexSet[%d] MaterialID=%d NumTriangle=%d\r\n", n, m_psIndexSubSet[n].nMaterialID, m_psIndexSubSet[n].nNumTriangle) ;
				}

			}
		}

		m_nNumSubSet = nCountMaterialUsed ;
	}

	InitShadowData() ;

	CSecretMesh::SetNormalVectors(pMesh) ;

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

HRESULT CSecretRigidMeshBump::InitShadowData()
{
	UINT i ;
    m_pcShadowMesh = new CSecretShadowMesh(m_pd3dDevice) ;
    SShadowMesh_Vertex *psShadowVertices = new SShadowMesh_Vertex[m_lNumVertex] ;

	for(i=0 ; i<m_lNumVertex ; i++)
		psShadowVertices[i].pos = m_psBumpVertices[i].pos ;

	m_pcShadowMesh->SetVertexData(psShadowVertices, m_lNumVertex) ;
	delete []psShadowVertices ;

	SShadowMesh_Index *psShadowIndices = new SShadowMesh_Index[m_lNumTriangle] ;

	for(i=0 ; i<m_lNumTriangle ; i++)
	{
		psShadowIndices[i].anIndex[0] = m_psIndices[i].anIndex[0] ;
		psShadowIndices[i].anIndex[1] = m_psIndices[i].anIndex[1] ;
		psShadowIndices[i].anIndex[2] = m_psIndices[i].anIndex[2] ;
	}

	m_pcShadowMesh->SetIndexData(psShadowIndices, m_lNumTriangle) ;
	delete []psShadowIndices ;

	return S_OK ;
}

void CSecretRigidMeshBump::Render()
{
	if(m_lNumTriangle <= 0)
		return ;
	
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SRigidMeshBump_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;
	g_cDebugMsg.AddTriangles(m_lNumTriangle) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}