#include <assert.h>
#include "Matrix.h"
#include "MathOrdinary.h"
#include "SecretRigidMeshAmbO.h"

D3DVERTEXELEMENT9 decl_RigidAmbO[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//������ ���� ����
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//������ ���� ����
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//�ؽ��� ��ǥ�� ���� ����
	{0, 32, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},//AmbientDegree
	D3DDECL_END()
} ;

CSecretRigidMeshAmbO::CSecretRigidMeshAmbO(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) : CSecretRigidMesh(lEigenIndex, pd3dDevice)
{
	m_psAmbOVertices = NULL ;
	//TRACE("CSecretRigidMeshAmbO construction EigenIndex=%d\r\n", lEigenIndex) ;
}

CSecretRigidMeshAmbO::~CSecretRigidMeshAmbO()
{
	SAFE_DELETEARRAY(m_psAmbOVertices) ;
	//TRACE("CSecretRigidMeshAmbO destruction\r\n") ;
}

HRESULT CSecretRigidMeshAmbO::Initialize(SMesh *pMesh)
{
	int i, n ;

	//TRACE("RigidMesh Init NodeName=%s, ParentNodeName=%s\r\n", pMesh->szMeshName, pMesh->szParentName) ;

	//Overriding Parent Initialize
	CSecretMesh::Initialize(pMesh) ;

	m_pVertexElement = decl_RigidAmbO ;

	///////////////////////////////////////////////////
	//Create Vertices
	///////////////////////////////////////////////////
	m_lNumVertex = pMesh->nNumVertex ;

	m_psAmbOVertices = new SRigidMeshAmbO_Vertex[m_lNumVertex] ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SRigidMeshAmbO_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_RIGIDMESHAMBOVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddVBSize(m_lNumVertex*sizeof(SRigidMeshAmbO_Vertex)) ;

	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SRigidMeshAmbO_Vertex), &pVertices, 0)))
		return E_FAIL ;

	SRigidMeshAmbO_Vertex *psVertices = (SRigidMeshAmbO_Vertex *)pVertices ;
	for(i=0 ; i<(int)m_lNumVertex ; i++)
	{
		psVertices->pos = D3DXVECTOR3(pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z)  ;
		psVertices->normal = D3DXVECTOR3(pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z)  ;
		//psVertices->ambient =D3DXVECTOR3(1, 1, 1) - D3DXVECTOR3(pMesh->psVertex[i].fAmbientDegree, pMesh->psVertex[i].fAmbientDegree, pMesh->psVertex[i].fAmbientDegree) ;// D3DXVECTOR3(1.0f, 1.0f, 1.0f) ;
		//psVertices->ambient = D3DXVECTOR3(pMesh->psVertex[i].fAmbientDegree, pMesh->psVertex[i].fAmbientDegree, pMesh->psVertex[i].fAmbientDegree) ;// D3DXVECTOR3(1.0f, 1.0f, 1.0f) ;
		psVertices->t = D3DXVECTOR2(pMesh->psVertex[i].tex.u, pMesh->psVertex[i].tex.v) ;
		psVertices->ambient = pMesh->psVertex[i].fAmbientDegree ;

//		TRACE("[%d] pos  x=%.3f y=%.3f, z=%.3f\r\n", i, psVertices->pos.x, psVertices->pos.y, psVertices->pos.z) ;
		//TRACE("{%3.3ff, %3.3ff, %3.3ff, %3.3ff, %3.3ff, %3.3ff},\r\n",
		//	pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z,
		//	pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z) ;

        m_psAmbOVertices[i] = *psVertices ;

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

	SFaceSet asFaceSet[MAXNUM_MATERIAL] ;//SubMaterial ������ 255�� ���϶�� �����Ѵ�.

    SRigidMesh_Index *psIndices = (SRigidMesh_Index *)pIndices ;
	for(i=0 ; i<(int)m_lNumTriangle ; i++)
	{
		psIndices->anIndex[0] = pMesh->psTriangle[i].anVertexIndex[0] ;
		psIndices->anIndex[1] = pMesh->psTriangle[i].anVertexIndex[1] ;
		psIndices->anIndex[2] = pMesh->psTriangle[i].anVertexIndex[2] ;

		m_psIndices[i] = *psIndices ;

		//############################################################//
		//SubMaterial�� ����ϴ� ��� *MESH_MTLID �� ��ȣ���� �з��۾��� �Ѵ�.
		//############################################################//
		if(pMesh->psTriangle[i].nMaterialID != nPrevMaterialID)
		{
			bExist = false ;
			for(n=0 ; n<nCountMaterialUsed ; n++)
			{
				if(asFaceSet[n].nMaterialID == nPrevMaterialID)//������ MaterialID�� ���� ���� �ִٸ�
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
    //������ Set�� �����ؾ� �ȴ�.
	//############################################################//
	bExist = false ;
	for(n=0 ; n<nCountMaterialUsed ; n++)
	{
		if(asFaceSet[n].nMaterialID == pMesh->psTriangle[m_lNumTriangle-1].nMaterialID)//���� MaterialID�� ���� ���� �ִٸ�
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

		m_ppSubSetVI = new LPDIRECT3DINDEXBUFFER9[m_nNumSubSet] ;
		for(i=0 ; i<m_nNumSubSet ; i++)
		{
			//if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_psIndexSubSet[i].nNumTriangle*sizeof(SRigidMesh_Index), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_ppSubSetVI[i], NULL)))
			if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_psIndexSubSet[i].nNumTriangle*sizeof(SRigidMesh_Index), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_ppSubSetVI[i], NULL)))
				return E_FAIL ;

			g_cDebugMsg.AddIBSize(m_psIndexSubSet[i].nNumTriangle*sizeof(SRigidMesh_Index)) ;

			if(FAILED(m_ppSubSetVI[i]->Lock(0, m_psIndexSubSet[i].nNumTriangle*sizeof(SRigidMesh_Index), &pIndices, 0)))
				return E_FAIL ;

            psIndices = (SRigidMesh_Index *)pIndices ;

			for(n=0 ; n<m_psIndexSubSet[i].nNumTriangle ; n++)
			{
				psIndices->anIndex[0] = m_psIndexSubSet[i].psIndices[n].anIndex[0] ;
				psIndices->anIndex[1] = m_psIndexSubSet[i].psIndices[n].anIndex[1] ;
				psIndices->anIndex[2] = m_psIndexSubSet[i].psIndices[n].anIndex[2] ;
				psIndices++ ;
			}

			m_ppSubSetVI[i]->Unlock() ;
		}
	}

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

void CSecretRigidMeshAmbO::PutAmbientDegree()
{
	for(int i=0 ; i<(int)m_lNumVertex ; i++)
		m_psAmbOVertices[i].ambient = m_psVertexDisk[i].fAmbientDegree ;
		//m_psAmbOVertices[i].ambient = D3DXVECTOR3(m_psVertexDisk[i].fAmbientDegree, m_psVertexDisk[i].fAmbientDegree, m_psVertexDisk[i].fAmbientDegree) ;
}

void CSecretRigidMeshAmbO::Render()
{
	if(m_lNumTriangle <= 0)
		return ;
	
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SRigidMeshAmbO_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;
	g_cDebugMsg.AddTriangles(m_lNumTriangle) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}