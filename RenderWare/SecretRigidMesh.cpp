#include "SecretRigidMesh.h"
#include <assert.h>
#include "Matrix.h"
#include "MathOrdinary.h"
#include "Quaternion.h"
#include "GeoLib.h"
#include "SecretShadowMesh.h"
#include "OBB.h"
#include "coldet.h"

//정점 오브젝트 생성
D3DVERTEXELEMENT9 decl_Rigid[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//법선에 대한 설정
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정
	{0, 32, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},//AmbientDegree
	D3DDECL_END()
} ;

CSecretRigidMesh::CSecretRigidMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) : CSecretMesh(lEigenIndex, pd3dDevice)
{
    m_bWireFrame = false ;

	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_psIndexSubSet = NULL ;
	m_ppSubSetVI = NULL ;
	m_pcShadowMesh = NULL ;
	m_nNumSubSet = 0 ;

	m_psBoundingPolyhedron = NULL ;
	m_pCollisionModel = NULL ;

	//TRACE("CSecretRigidMesh construction EigenIndex=%d\r\n", lEigenIndex) ;
}

CSecretRigidMesh::~CSecretRigidMesh()
{
	int i ;

	ReleaseCollisionModel() ;

	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;

	for(i=0 ; i<m_nNumSubSet ; i++)
	{
		SAFE_DELETEARRAY(m_psIndexSubSet[i].psIndices) ;
		SAFE_RELEASE(m_ppSubSetVI[i]) ;
	}
	SAFE_DELETEARRAY(m_psIndexSubSet) ;
	SAFE_DELETEARRAY(m_ppSubSetVI) ;

	SAFE_DELETE(m_pcShadowMesh) ;

	//TRACE("CSecretRigidMesh destruction\r\n") ;
}

HRESULT CSecretRigidMesh::RefreashVertexBuffer()
{
	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SRigidMesh_Vertex), &pVertices, 0)))
		return E_FAIL ;

	SRigidMesh_Vertex *psVertices = (SRigidMesh_Vertex *)pVertices ;
	for(int i=0 ; i<(int)m_lNumVertex ; i++)
	{
		(*psVertices) = m_psVertices[i] ;
		psVertices++ ;
	}

	m_pVB->Unlock() ;

	return S_OK ;
}

HRESULT CSecretRigidMesh::Initialize(SMesh *pMesh)
{
	int i, n ;

	//TRACE("RigidMesh Init NodeName=%s, ParentNodeName=%s\r\n", pMesh->szMeshName, pMesh->szParentName) ;

	//Overriding Parent Initialize
	CSecretMesh::Initialize(pMesh) ;

	SetVertexElement(decl_Rigid) ;

	//m_pcShadowMesh = new CSecretShadowMesh(m_pd3dDevice) ;

	///////////////////////////////////////////////////
	//Create Vertices
	///////////////////////////////////////////////////
	assert(pMesh->nNumVertex > 0) ;
	m_lNumVertex = pMesh->nNumVertex ;

	m_psVertices = new SRigidMesh_Vertex[m_lNumVertex] ;

	//if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SRigidMesh_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_RIGIDMESHVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SRigidMesh_Vertex), 0, D3DFVF_RIGIDMESHVERTEX, D3DPOOL_MANAGED, &m_pVB, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddVBSize(m_lNumVertex*sizeof(SRigidMesh_Vertex)) ;

	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SRigidMesh_Vertex), &pVertices, 0)))
		return E_FAIL ;

	SRigidMesh_Vertex *psVertices = (SRigidMesh_Vertex *)pVertices ;
	for(i=0 ; i<(int)m_lNumVertex ; i++)
	{
		psVertices->pos = D3DXVECTOR3(pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z)  ;
		psVertices->normal = D3DXVECTOR3(pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z)  ;
		psVertices->t = D3DXVECTOR2(pMesh->psVertex[i].tex.u, pMesh->psVertex[i].tex.v) ;
		psVertices->ambient = pMesh->psVertex[i].fAmbientDegree ;

		//TRACE("[%02d] pos(%+07.03f %+07.03f %+07.03f)\r\n", i, psVertices->pos.x, psVertices->pos.y, psVertices->pos.z) ;
		//TRACE("{%3.3ff, %3.3ff, %3.3ff, %3.3ff, %3.3ff, %3.3ff},\r\n",
		//	pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z,
		//	pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z) ;

        m_psVertices[i] = *psVertices ;

		psVertices++ ;
	}

	m_pVB->Unlock() ;

    ///////////////////////////////////////////////////
	//Create Indices
	///////////////////////////////////////////////////
	assert(pMesh->nNumTriangle > 0) ;
	m_lNumTriangle = pMesh->nNumTriangle ;

	m_psIndices = new SRigidMesh_Index[m_lNumTriangle] ;

	//if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SRigidMesh_Index), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVI, NULL)))
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SRigidMesh_Index), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pVI, NULL)))
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

		//TRACE("[%02d] index(%02d %02d %02d)\r\n", i, psIndices->anIndex[0], psIndices->anIndex[1], psIndices->anIndex[2]) ;
		//TRACE("{%2d, %2d, %2d},\r\n", psIndices->anIndex[0], psIndices->anIndex[1], psIndices->anIndex[2]) ;
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
		g_cDebugMsg.SubtractIBSize(m_lNumTriangle*sizeof(SRigidMesh_Index)) ;
        SAFE_RELEASE(m_pVI) ;

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


//		ExportHeightMap(17, 17, "map17x17.bmp") ;
//		ExportHeightMap(16, 16, "map16x16.bmp") ;
//		ExportHeightMap(15, 15, "map15x15.bmp") ;
//		ExportHeightMap(33, 33, "map33x33.bmp") ;
	}

	//InitShadowData() ;

	CSecretMesh::SetNormalVectors(pMesh) ;

	//if(!strcmp(pMesh->szMeshName, "holecup_collision"))
	//	_ExportHolecupPolygons() ;

	//CSecretMesh::SetVertexDisk(pMesh) ;

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

    //SAFE_DELETEARRAY(m_psIndices) ;
	//SAFE_DELETEARRAY(m_psVertices) ;

    return S_OK ;
}
HRESULT CSecretRigidMesh::InitShadowData()
{
	int i, n ;
    m_pcShadowMesh = new CSecretShadowMesh(m_pd3dDevice) ;
    SShadowMesh_Vertex *psShadowVertices = new SShadowMesh_Vertex[m_lNumVertex] ;

	for(i=0 ; i<(int)m_lNumVertex ; i++)
		psShadowVertices[i].pos = m_psVertices[i].pos ;

	m_pcShadowMesh->SetVertexData(psShadowVertices, m_lNumVertex) ;

	SAFE_DELETEARRAY(psShadowVertices) ;

	SShadowMesh_Index *psShadowIndices=NULL ;
	int nNumIndex=0 ;
	if(m_lAttr & SECRETNODEATTR_USESUBMATERIAL)
	{
		m_pcShadowMesh->m_nNumSubset = m_nNumSubSet ;
		m_pcShadowMesh->m_psIndexSubset = new SShadowMesh_IndexSubSet[m_pcShadowMesh->m_nNumSubset] ;
		m_pcShadowMesh->m_ppSubsetIB = new LPDIRECT3DINDEXBUFFER9[m_pcShadowMesh->m_nNumSubset] ;

		for(i=0 ; i<m_nNumSubSet ; i++)
		{
            SRigidMesh_Index *psIndices ;            
			m_pcShadowMesh->m_psIndexSubset[i].set(m_psIndexSubSet[i].nMaterialID, m_psIndexSubSet[i].nNumTriangle) ;

			if(FAILED(m_ppSubSetVI[i]->Lock(0, m_psIndexSubSet[i].nNumTriangle*sizeof(SRigidMesh_Index), (void **)&psIndices, 0)))
				return E_FAIL ;

			for(n=0 ; n<m_psIndexSubSet[i].nNumTriangle ; n++, psIndices++)
			{
				m_pcShadowMesh->m_psIndexSubset[i].psIndices[n].anIndex[0] = psIndices[n].anIndex[0] ;
				m_pcShadowMesh->m_psIndexSubset[i].psIndices[n].anIndex[1] = psIndices[n].anIndex[1] ;
				m_pcShadowMesh->m_psIndexSubset[i].psIndices[n].anIndex[2] = psIndices[n].anIndex[2] ;
			}
			m_ppSubSetVI[i]->Unlock() ;            
		}
	}
	else
	{
		nNumIndex = m_lNumTriangle ;
		psShadowIndices = new SShadowMesh_Index[nNumIndex] ;
		for(i=0 ; i<nNumIndex ; i++)
		{
			psShadowIndices[i].anIndex[0] = m_psIndices[i].anIndex[0] ;
			psShadowIndices[i].anIndex[1] = m_psIndices[i].anIndex[1] ;
			psShadowIndices[i].anIndex[2] = m_psIndices[i].anIndex[2] ;
		}
	}

	m_pcShadowMesh->SetIndexData(psShadowIndices, nNumIndex) ;

	SAFE_DELETEARRAY(psShadowIndices) ;

	return S_OK ;
}

void CSecretRigidMesh::Render()
{
	if(m_lNumTriangle <= 0)
		return ;
	
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SRigidMesh_Vertex)) ;
	
	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;
	g_cDebugMsg.AddTriangles(m_lNumTriangle) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}

void CSecretRigidMesh::RenderShadow()
{
	if(m_lNumTriangle <= 0)
		return ;
	
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetStreamSource(0, m_pcShadowMesh->m_pVB, 0, sizeof(SShadowMesh_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pcShadowMesh->m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_pcShadowMesh->m_lNumIndex) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}

void CSecretRigidMesh::PreRenderSubSet()
{
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SRigidMesh_Vertex)) ;
	m_pd3dDevice->SetFVF(D3DFVF_RIGIDMESHVERTEX) ;
}

void CSecretRigidMesh::RenderSubSet(int nCount)
{
	if(m_psIndexSubSet[nCount].nNumTriangle <= 0)
		return ;

	/*
	void *pIndices ;
	if(FAILED(m_pVI->Lock(0, m_psIndexSubSet[nCount].nNumTriangle*sizeof(SRigidMesh_Index), (void **)&pIndices, 0)))
		assert(false && "m_pVI->Lock is failed") ;

	memcpy(pIndices, m_psIndexSubSet[nCount].psIndices, m_psIndexSubSet[nCount].nNumTriangle*sizeof(SRigidMesh_Index)) ;

	m_pVI->Unlock() ;

	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_psIndexSubSet[nCount].nNumTriangle) ;
	*/

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetFVF(D3DFVF_RIGIDMESHVERTEX) ;
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SRigidMesh_Vertex)) ;
	
	m_pd3dDevice->SetIndices(m_ppSubSetVI[nCount]) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_psIndexSubSet[nCount].nNumTriangle) ;
	g_cDebugMsg.AddTriangles(m_psIndexSubSet[nCount].nNumTriangle) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	//g_cDebugMsg.AddTriangles(m_psIndexSubSet[nCount].nNumTriangle) ;
}

void CSecretRigidMesh::PostRenderSubSet()
{
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}
void CSecretRigidMesh::SetMeshDiffuse(char *pszObjectName)
{
	if( strstr(pszObjectName, "104_yan") )
		m_vDiffuse = D3DXVECTOR4(0.6f, 0.6f, 0.6f, 0.6f) ;
}

#include "SecretShadowMap.h"

HRESULT CSecretRigidMesh::RenderPerspectiveShadowMap(dxMatrix4 &dxmatWorld, dxMatrix4 &dxmatView, CSecretIdShadow *pcIdShadow)
{
	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SRigidMesh_Vertex), &pVertices, 0)))
		return E_FAIL ;

	SRigidMesh_Vertex *psVertices = (SRigidMesh_Vertex *)pVertices ;
	UINT i ;
    Vector3 pos ;
	Matrix4 matWorld, matView, matInvView, matWV, matInvWorld, matLocalView, matPerspective ;


	float fnear=0.1f, ffar=100.0f ;
	matPerspective.m33 = (fnear+ffar)/(ffar-fnear) ;
	matPerspective.m34 = (2.0f*fnear*ffar)/(ffar-fnear) ;
	matPerspective.m43 = -1 ;

	D3DXMATRIX matProj ;
	float fAspectRatio = 800.0f/600.0f ;
	D3DXMatrixPerspectiveFovLH(&matProj, 3.141592f/4.f, fAspectRatio, 0.1f, 100.f) ;

	matWorld = MatrixConvert(dxmatWorld) ;
	matView = MatrixConvert(dxmatView) ;
	matInvView = matView.Inverse() ;
	matInvWorld = matWorld.Inverse() ;
	matWV = matWorld*matView ;
	matLocalView = matView ;

	float ffactor=2.0f, fscale ;

	for(i=0 ; i<m_lNumVertex ; i++)
	{
		pos.set(m_psVertices[i].pos.x, m_psVertices[i].pos.y, m_psVertices[i].pos.z) ;

		pos *= matWV ;

		pos -= Vector3(matView.m41, matView.m42, 0) ;

		fscale = ffactor/log(pos.z) ;
		pos.x *= fscale ;
		pos.y *= fscale ;

		pos += Vector3(matView.m41, matView.m42, 0) ;

		//pos *= (matWV*matPerspective) ;

		//pos *= (matInvView*matInvWorld) ;
		pos *= (matInvView);//*matWorld) ;

		psVertices->pos = dxVector3(pos.x, pos.y, pos.z) ;

		psVertices++ ;
	}

	m_pVB->Unlock() ;

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SRigidMesh_Vertex)) ;
	
	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;
	g_cDebugMsg.AddTriangles(m_lNumTriangle) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SRigidMesh_Vertex), &pVertices, 0)))
		return E_FAIL ;

	psVertices = (SRigidMesh_Vertex *)pVertices ;
	for(i=0 ; i<m_lNumVertex ; i++)
	{
		psVertices->pos = dxVector3(m_psVertices[i].pos.x, m_psVertices[i].pos.y, m_psVertices[i].pos.z) ;
		psVertices++ ;
	}

	m_pVB->Unlock() ;
    return S_OK ;
}

void CSecretRigidMesh::RenderNormalVector(Matrix4 &matWorld)
{
	//Matrix4 matLocal = MatrixConvert(GetmatLocal()) ;

	//for(UINT i=0 ; i<m_lNumNormalVector ; i++)
	//{
	//	m_psNormalVertex[i].vStart = m_psNormalVector[i].vStart*matLocal ;
	//	m_psNormalVertex[i].vEnd = (m_psNormalVector[i].vStart+m_psNormalVector[i].vNormal)*matLocal ;
	//}

	//D3DMATERIAL9 mtrl ;
	//ZeroMemory(&mtrl, sizeof(D3DMATERIAL9)) ;
	//mtrl.Diffuse.r = mtrl.Diffuse.g = mtrl.Diffuse.b = 1 ;
	//mtrl.Ambient.r = mtrl.Ambient.g = mtrl.Ambient.b = 1 ;
	//m_pd3dDevice->SetMaterial(&mtrl) ;

	//m_pd3dDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1) ;
	//m_pd3dDevice->SetRenderState(D3DRS_AMBIENT, TRUE) ;

	typedef struct
	{
		float x, y, z ;
		DWORD color ;
	} VERTEX ;

	VERTEX vertex[2] ;

	vertex[0].color = 0xffffff00 ;
	vertex[1].color = 0xffff00ff ;

	Vector3 vStart, vNormal ;
	for(UINT i=0 ; i<m_lNumNormalVector ; i++)
	{
		vStart = m_psNormalVector[i].vStart * matWorld ;
		vertex[0].x = vStart.x ;
		vertex[0].y = vStart.y ;
		vertex[0].z = vStart.z ;

		//m_psNormalVector[i].vStart = m_psNormalVector[i].vStart * matWorld ;
		//vertex[0].x = m_psNormalVector[i].vStart.x ;
		//vertex[0].y = m_psNormalVector[i].vStart.y ;
		//vertex[0].z = m_psNormalVector[i].vStart.z ;

		vNormal.x = m_psNormalVector[i].vNormal.x*matWorld.m11 + m_psNormalVector[i].vNormal.y*matWorld.m21 + m_psNormalVector[i].vNormal.z*matWorld.m31 ;
		vNormal.y =	m_psNormalVector[i].vNormal.x*matWorld.m12 + m_psNormalVector[i].vNormal.y*matWorld.m22 + m_psNormalVector[i].vNormal.z*matWorld.m32 ;
		vNormal.z = m_psNormalVector[i].vNormal.x*matWorld.m13 + m_psNormalVector[i].vNormal.y*matWorld.m23 + m_psNormalVector[i].vNormal.z*matWorld.m33 ;

		vNormal *= (0.1f/vNormal.Magnitude()) ;
		//m_psNormalVertex[i].vEnd = m_psNormalVertex[i].vStart+vNormal ;
		m_psNormalVertex[i].vEnd = vStart+vNormal ;

		vertex[1].x = m_psNormalVertex[i].vEnd.x ;
		vertex[1].y = m_psNormalVertex[i].vEnd.y ;
		vertex[1].z = m_psNormalVertex[i].vEnd.z ;

		m_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE) ;
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertex, sizeof(vertex[0])) ;
	}
}

void CSecretRigidMesh::RenderBoundingVolume(Matrix4 &matWorld)
{
	typedef struct
	{
		float x, y, z ;
		DWORD color ;
	} VERTEX ;

	if(!m_psBoundingPolyhedron)
		return ;

	//geo::SSphere *psSphere = new geo::SSphere() ;
	//psSphere->set(vCenter, longest) ;
	//m_psBoundingPolyhedron = (geo::SPolyhedron *)psSphere ;

	if(m_psBoundingPolyhedron->nKind == geo::SPHERE)
	{

	}
	else if(m_psBoundingPolyhedron->nKind == geo::CUBE)
	{
		geo::SCube *psCube = (geo::SCube *)m_psBoundingPolyhedron ;
		VERTEX vt[24] ;
		Vector3 p ;

		int i, c=0 ;
		for(i=0 ; i<4 ; i++)//윗면
		{
			p = psCube->vVertices[i] * matWorld ;

			vt[c].x = p.x ;
			vt[c].y = p.y ;
			vt[c].z = p.z ;
			vt[c].color = 0xffffff00 ;
			c++ ;

			int ii = ((i+1) < 4) ? (i+1) : 0 ;

			p = psCube->vVertices[ii] * matWorld ;

			vt[c].x = p.x ;
			vt[c].y = p.y ;
			vt[c].z = p.z ;
			vt[c].color = 0xffffff00 ;
			c++ ;
		}

		for(i=4 ; i<8 ; i++)//아래면
		{
			p = psCube->vVertices[i] * matWorld ;

			vt[c].x = p.x ;
			vt[c].y = p.y ;
			vt[c].z = p.z ;
			vt[c].color = 0xffffff00 ;
			c++ ;

			int ii = ((i+1) < 8) ? (i+1) : 4 ;

			p = psCube->vVertices[ii] * matWorld ;

			vt[c].x = p.x ;
			vt[c].y = p.y ;
			vt[c].z = p.z ;
			vt[c].color = 0xffffff00 ;
			c++ ;
		}

		for(i=0 ; i<4 ; i++)//기둥
		{
			p = psCube->vVertices[i] * matWorld ;

			vt[c].x = p.x ;
			vt[c].y = p.y ;
			vt[c].z = p.z ;
			vt[c].color = 0xffffff00 ;
			c++ ;

			p = psCube->vVertices[i+4] * matWorld ;

			vt[c].x = p.x ;
			vt[c].y = p.y ;
			vt[c].z = p.z ;
			vt[c].color = 0xffffff00 ;
			c++ ;
		}

		m_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE) ;
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 12, vt, sizeof(vt[0])) ;
	}
}

void CSecretRigidMesh::RenderVertexDisk(Matrix4 &matWorld)
{
	typedef struct
	{
		float x, y, z ;
		DWORD color ;
	} VERTEX ;

	typedef struct
	{
		WORD index[3] ;
	} INDEX ;

	int i, n ;
	VERTEX vertex[16+1] ;
	float ftheta = 2.0f*3.141592f/16.0f ;

	vertex[0].x = vertex[0].y = vertex[0].z = 0 ;
	vertex[0].color = 0xffff00ff ;

	for(i=0 ; i<16 ; i++)
	{
		vertex[i+1].x = cosf(ftheta*i) ;
		vertex[i+1].z = sinf(ftheta*i) ;
		vertex[i+1].y = 0 ;
		vertex[i+1].color = 0xffff00ff ;
	}

	INDEX index[] = 
	{
		0, 2, 1,
		0, 3, 2,
		0, 4, 3,
		0, 5, 4,
		0, 6, 5,
		0, 7, 6,
		0, 8, 7,
		0, 9, 8,
		0,10, 9,
		0,11,10,
		0,12,11,
		0,13,12,
		0,14,13,
		0,15,14,
		0,16,15,
		0, 1,16
	} ;

	VERTEX vt[17] ;
	Vector3 v1, v2, vNormal, vAxis ;
	Quaternion q ;
	float fIncludeAngle ;

    Matrix4 matRot, matScale, matTranslate, matTransform ;

	matRot = matWorld ;
	matRot.m41 = matRot.m42 = matRot.m43 = 0 ;

	for(n=0 ; n<(int)m_lNumVertex ; n++)
	{
		vNormal = m_psVertexDisk[n].vNormal * matRot ;
		if(!VECTOR_EQ(vNormal, Vector3(0, 1, 0)))
		{
			if(VECTOR_EQ(vNormal, Vector3(0, -1, 0)))
			{
				matTransform.Identity() ;
				matTransform.m22 = -1 ;

				matTransform.Identity() ;
				matTransform.SetRotatebyAxisX(180) ;
			}
			else
			{
				fIncludeAngle = IncludedAngle(Vector3(0, 1, 0), vNormal) ;
				vAxis = (Vector3(0, 1, 0)).cross(vNormal) ;
				q.AxisToQuat(vAxis, fIncludeAngle) ;
				q.GetMatrix(matTransform) ;
			}
		}
		else
			matTransform.Identity() ;

		matScale.Identity() ;
		matScale.m11 = matScale.m22 = matScale.m33 = m_psVertexDisk[n].fRadius ;

		matTranslate.Identity() ;
		matTranslate.SetTranslation(m_psVertexDisk[n].vPos.x+matWorld.m41, m_psVertexDisk[n].vPos.y+matWorld.m42, m_psVertexDisk[n].vPos.z+matWorld.m43) ;

		matTransform = matScale * matTransform * matTranslate ;

		for(i=0 ; i<17 ; i++)
		{
			v1.set(vt[i].x, vt[i].y, vt[i].z) ;
			v2.set(vertex[i].x, vertex[i].y, vertex[i].z) ;
			v1 = v2*matTransform ;

			vt[i].x = v1.x ;
			vt[i].y = v1.y ;
			vt[i].z = v1.z ;
			vt[i].color = 0x80ffffff ;
		}

		m_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE) ;
		m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 17, 16, index, D3DFMT_INDEX16, vt, sizeof(vertex[0])) ;
	}
}

int CSecretRigidMesh::GetSubMaterialID(int nCount)
{
	return m_psIndexSubSet[nCount].nMaterialID ;
}

void CSecretRigidMesh::GetFragmentInfo(float fMinX, float fMinZ, float fWidth, float fHeight, STerrain_FragmentInfo *psFragmentInfo)
{
	int i, n ;
	geo::STriangle *psTriangle ;
	int nCount=0 ;
	float u, v ;
	STerrain_FragmentInfo sFragmentInfo ;

	psTriangle = new geo::STriangle[m_lNumTriangle] ;

	for(i=0 ; i<(int)m_lNumTriangle ; i++)
	{
		for(n=0 ; n<3 ; n++)
		{
			//psTriangle[i].avPos[n] = Vector3(m_psVertices[m_psIndices[i].anIndex[n]].pos.x,
			//	m_psVertices[m_psIndices[i].anIndex[n]].pos.y,
			//	m_psVertices[m_psIndices[i].anIndex[n]].pos.z) ;
			psTriangle[i].avVertex[n].vPos = Vector3(m_psVertices[m_psIndices[i].anIndex[n]].pos.x,
				m_psVertices[m_psIndices[i].anIndex[n]].pos.y,
				m_psVertices[m_psIndices[i].anIndex[n]].pos.z) ;

			//D3DXVec3Transform(&v4Pos, &m_psVertices[m_psIndices[i].anIndex[n]].pos, this->GetmatLocal()) ;
			//psTriangle[i].avPos[n] = D3DXVECTOR3(v4Pos.x, v4Pos.y, v4Pos.z) ;
			
			//psTriangle[i].avNormal[n] = Vector3(m_psVertices[m_psIndices[i].anIndex[n]].normal.x,
			//	m_psVertices[m_psIndices[i].anIndex[n]].normal.y,
			//	m_psVertices[m_psIndices[i].anIndex[n]].normal.z) ;
			psTriangle[i].avVertex[n].vNormal = Vector3(m_psVertices[m_psIndices[i].anIndex[n]].normal.x,
				m_psVertices[m_psIndices[i].anIndex[n]].normal.y,
				m_psVertices[m_psIndices[i].anIndex[n]].normal.z) ;

			//psTriangle[i].avTex[n] = Vector3(m_psVertices[m_psIndices[i].anIndex[n]].t.x,
			//	m_psVertices[m_psIndices[i].anIndex[n]].t.y, 0.0f) ;
			psTriangle[i].avVertex[n].vTex = Vector2(m_psVertices[m_psIndices[i].anIndex[n]].t.x,
				m_psVertices[m_psIndices[i].anIndex[n]].t.y) ;
		}
	}

	TRACE("\r\n") ;
	TRACE("##HeightValue START##\r\n") ;

	nCount=0 ;
	int nTraceCount=0 ;
	for( ; fMinZ<(fMinZ+fHeight) ; fMinZ += 1.0f)
	{
		for( ; fMinX<(fMinX+fWidth) ; fMinX += 1.0f)
		{
			memset(&sFragmentInfo, 0, sizeof(STerrain_FragmentInfo)) ;
			for(i=0 ; i<(int)m_lNumTriangle ; i++)
			{
				if(IntesectXZProjTriangleToPoint(psTriangle[i], Vector3(fMinX, 0.0f, fMinZ), u, v))//u = P1, v = P2, w = P0
				{
					//Height Value Interpolation
					//sFragmentInfo.fElevation = (1-u-v)*psTriangle[i].avPos[0].y + u*psTriangle[i].avPos[1].y + v*psTriangle[i].avPos[2].y ;
					sFragmentInfo.fElevation =
						(1-u-v)*psTriangle[i].avVertex[0].vPos.y + u*psTriangle[i].avVertex[1].vPos.y + v*psTriangle[i].avVertex[2].vPos.y ;

					//u of texture Coordinate Interpolation
					//sFragmentInfo.u = (1-u-v)*psTriangle[i].avTex[0].x + u*psTriangle[i].avTex[1].x + v*psTriangle[i].avTex[2].x ;
					sFragmentInfo.u =
						(1-u-v)*psTriangle[i].avVertex[0].vTex.x + u*psTriangle[i].avVertex[1].vTex.x + v*psTriangle[i].avVertex[2].vTex.x ;
					//v of texture Coordinate Interpolation
					//sFragmentInfo.v = (1-u-v)*psTriangle[i].avTex[0].y + u*psTriangle[i].avTex[1].y + v*psTriangle[i].avTex[2].y ;
					sFragmentInfo.v =
						(1-u-v)*psTriangle[i].avVertex[0].vTex.y + u*psTriangle[i].avVertex[1].vTex.y + v*psTriangle[i].avVertex[2].vTex.y ;
					break ;
				}
			}
			psFragmentInfo[nCount++] = sFragmentInfo ;
		}
	}

	TRACE("##HeightValue END##\r\n") ;
	TRACE("\r\n") ;
}

void CSecretRigidMesh::_BuildBoundingSphere()
{
	int i ;

	Vector3 vAccum(0, 0, 0), vCenter(0, 0, 0) ;
	for(i=0 ; i<m_lNumVertex ; i++)
		vAccum += Vector3(m_psVertices[i].pos.x, m_psVertices[i].pos.y, m_psVertices[i].pos.z) ;

	vCenter = vAccum*(1.0f/(float)m_lNumVertex) ;

	Vector3 pos(Vector3(m_psVertices[0].pos.x, m_psVertices[0].pos.y, m_psVertices[0].pos.z)) ;
	float longest = (pos-vCenter).Magnitude(), d ;
	for(i=1 ; i<m_lNumVertex ; i++)
	{
		pos.set(m_psVertices[i].pos.x, m_psVertices[i].pos.y, m_psVertices[i].pos.z) ;

		d = (pos-vCenter).Magnitude() ;
		if(longest < d)
			longest = d ;
	}

	geo::SSphere *psSphere = new geo::SSphere() ;
	psSphere->set(vCenter, longest) ;
	m_psBoundingPolyhedron = (geo::SPolyhedron *)psSphere ;
}
void CSecretRigidMesh::BuildCollisionModel()
{
	_BuildBoundingCube() ;
	_BuildCollisionModel() ;
}
void CSecretRigidMesh::ReleaseCollisionModel()
{
	SAFE_DELETE(m_pCollisionModel) ;

	if(m_psBoundingPolyhedron)
	{
		if(m_psBoundingPolyhedron->nKind == geo::SPHERE)
		{
			geo::SSphere *p = (geo::SSphere *)m_psBoundingPolyhedron ;
			delete p ;
			m_psBoundingPolyhedron = NULL ;
		}
		else if(m_psBoundingPolyhedron->nKind == geo::CUBE)
		{
			geo::SCube *p = (geo::SCube *)m_psBoundingPolyhedron ;
			delete p ;
			m_psBoundingPolyhedron = NULL ;
		}
		else if(m_psBoundingPolyhedron->nKind == geo::CYLINDER)
		{
			geo::SCylinder *p = (geo::SCylinder *)m_psBoundingPolyhedron ;
			delete p ;
			m_psBoundingPolyhedron = NULL ;
		}
	}
}
void CSecretRigidMesh::_BuildBoundingCube()
{
	if(m_psBoundingPolyhedron)
		return ;

	int i, n ;
	int nNumPos=0 ;
	Vector3 *pvPos=NULL ;
	if(m_nNumSubSet > 0)
	{
		//SRigidMesh_IndexSubSet *m_psIndexSubSet
		for(i=0 ; i<m_nNumSubSet ; i++)
			nNumPos += m_psIndexSubSet[i].nNumTriangle ;

		pvPos = new Vector3[nNumPos] ;

		int c=0 ; 
		D3DXVECTOR3 pos[3] ;
		SRigidMesh_IndexSubSet *psSubset ;
		for(i=0 ; i<m_nNumSubSet ; i++)
		{
			psSubset = &m_psIndexSubSet[i] ;
			for(n=0 ; n<psSubset->nNumTriangle ; n++)
			{
				pos[0] = m_psVertices[psSubset->psIndices[n].anIndex[0]].pos ;
				pos[1] = m_psVertices[psSubset->psIndices[n].anIndex[1]].pos ;
				pos[2] = m_psVertices[psSubset->psIndices[n].anIndex[2]].pos ;

				pvPos[c++].set( (pos[0].x+pos[1].x+pos[2].x)/3.0f, (pos[0].y+pos[1].y+pos[2].y)/3.0f, (pos[0].z+pos[1].z+pos[2].z)/3.0f ) ;
			}
		}
	}
	else
	{
		nNumPos = m_lNumTriangle ;
		pvPos = new Vector3[nNumPos] ;

		int c=0 ;
		D3DXVECTOR3 pos[3] ;
		for(i=0 ; i<m_lNumTriangle ; i++)
		{
			pos[0] = m_psVertices[m_psIndices[i].anIndex[0]].pos ;
			pos[1] = m_psVertices[m_psIndices[i].anIndex[1]].pos ;
			pos[2] = m_psVertices[m_psIndices[i].anIndex[2]].pos ;

			pvPos[c++].set( (pos[0].x+pos[1].x+pos[2].x)/3.0f, (pos[0].y+pos[1].y+pos[2].y)/3.0f, (pos[0].z+pos[1].z+pos[2].z)/3.0f ) ;
		}
	}

	Vector3 vCenter, vBasis[3], min, max ;

	ComputeBoundingBox(pvPos, nNumPos, vCenter, vBasis, min, max) ;

	geo::SCube *psCube = new geo::SCube() ;
	psCube->set(vCenter, vBasis, min, max) ;

	m_psBoundingPolyhedron = (geo::SPolyhedron *)psCube ;

	delete []pvPos ;
}
void CSecretRigidMesh::_BuildCollisionModel()
{
	if(m_pCollisionModel)
		return ;

	int i, n ;
	D3DXVECTOR4 pos[3] ;

	m_pCollisionModel = newCollisionModel3D(false) ;

	D3DXMATRIX *pmatLocal = GetmatLocal() ;

	if(m_nNumSubSet > 0)
	{
		SRigidMesh_IndexSubSet *psSubset ;
		for(i=0 ; i<m_nNumSubSet ; i++)
		{
			psSubset = &m_psIndexSubSet[i] ;
			for(n=0 ; n<psSubset->nNumTriangle ; n++)
			{
				D3DXVec3Transform(&pos[0], &m_psVertices[psSubset->psIndices[n].anIndex[0]].pos, pmatLocal) ;
				D3DXVec3Transform(&pos[1], &m_psVertices[psSubset->psIndices[n].anIndex[1]].pos, pmatLocal) ;
				D3DXVec3Transform(&pos[2], &m_psVertices[psSubset->psIndices[n].anIndex[2]].pos, pmatLocal) ;

				//pos[0] = m_psVertices[psSubset->psIndices[n].anIndex[0]].pos ;
				//pos[1] = m_psVertices[psSubset->psIndices[n].anIndex[1]].pos ;
				//pos[2] = m_psVertices[psSubset->psIndices[n].anIndex[2]].pos ;

				m_pCollisionModel->addTriangle(pos[0].x, pos[0].y, pos[0].z, pos[1].x, pos[1].y, pos[1].z, pos[2].x, pos[2].y, pos[2].z) ;
			}
		}
	}
	else
	{
		for(i=0 ; i<m_lNumTriangle ; i++)
		{
			D3DXVec3Transform(&pos[0], &m_psVertices[m_psIndices[i].anIndex[0]].pos, pmatLocal) ;
			D3DXVec3Transform(&pos[1], &m_psVertices[m_psIndices[i].anIndex[1]].pos, pmatLocal) ;
			D3DXVec3Transform(&pos[2], &m_psVertices[m_psIndices[i].anIndex[2]].pos, pmatLocal) ;

			//pos[0] = m_psVertices[m_psIndices[i].anIndex[0]].pos ;
			//pos[1] = m_psVertices[m_psIndices[i].anIndex[1]].pos ;
			//pos[2] = m_psVertices[m_psIndices[i].anIndex[2]].pos ;

			m_pCollisionModel->addTriangle(pos[0].x, pos[0].y, pos[0].z, pos[1].x, pos[1].y, pos[1].z, pos[2].x, pos[2].y, pos[2].z) ;
		}
	}

	m_pCollisionModel->finalize() ;

	TRACE("collision model triangle gathering finished.\r\n") ;
}
void CSecretRigidMesh::_ExportHolecupPolygons()
{
	int i, n ;
	D3DXVECTOR4 pos[3] ;
	D3DXMATRIX *pmatLocal = GetmatLocal() ;
	geo::STriangle sTriangle ;
	geo::STriangle sTriangles[512] ;
	geo::SCylinder sCylinders[512] ;
	geo::SSphere sSpheres[512] ;
	int nNumCylinder=0, nNumSphere=0, nTotalCylinderCount=0, nTotalSphereCount=0, nNumTriangle=0 ;
	float fBallRadius=0.02f ;

	for(i=0 ; i<m_lNumTriangle ; i++)
	{
		D3DXVec3Transform(&pos[0], &m_psVertices[m_psIndices[i].anIndex[0]].pos, pmatLocal) ;
		D3DXVec3Transform(&pos[1], &m_psVertices[m_psIndices[i].anIndex[1]].pos, pmatLocal) ;
		D3DXVec3Transform(&pos[2], &m_psVertices[m_psIndices[i].anIndex[2]].pos, pmatLocal) ;

		sTriangle.set(Vector3(pos[0].x, pos[0].y, pos[0].z), Vector3(pos[1].x, pos[1].y, pos[1].z), Vector3(pos[2].x, pos[2].y, pos[2].z)) ;

		for(n=0 ; n<3 ; n++)
		{
			bool bExistCylinder=false, bExistSphere=false ;
			int nStart = n ;
			int nEnd = ((n+1)<3) ? n+1 : 0 ;

			for(int cylinder_count=0 ; cylinder_count<nNumCylinder ; cylinder_count++)
			{
				if( (vector_eq(sTriangle.avVertex[nStart].vPos, sCylinders[cylinder_count].vStart) && vector_eq(sTriangle.avVertex[nEnd].vPos, sCylinders[cylinder_count].vEnd))
					|| (vector_eq(sTriangle.avVertex[nEnd].vPos, sCylinders[cylinder_count].vStart) && vector_eq(sTriangle.avVertex[nStart].vPos, sCylinders[cylinder_count].vEnd)) )
				{
					bExistCylinder=true ;
					break ;
				}
			}
			if(!bExistCylinder)
			{
				sCylinders[nNumCylinder++].set(sTriangle.avVertex[nStart].vPos, sTriangle.avVertex[nEnd].vPos, fBallRadius) ;
				assert(nNumCylinder < 512) ;
			}

			nTotalSphereCount++ ;
			for(int sphere_count=0 ; sphere_count<nNumSphere ; sphere_count++)
			{
				if(vector_eq(sTriangle.avVertex[n].vPos, sSpheres[sphere_count].vPos))
				{
					bExistSphere = true ;
					break ;
				}
			}
			if(!bExistSphere)
			{
				sSpheres[nNumSphere++].set(sTriangle.avVertex[n].vPos, fBallRadius) ;
				assert(nNumSphere < 512) ;
			}
		}
		sTriangles[nNumTriangle++] = sTriangle ;
	}

	TRACE("EXPORT HOLECUP COLLIDING POLYGONS\r\n") ;
	TRACE("numTriangle=%d numCylinder=%d numSphere=%d\r\n", nNumTriangle, nNumCylinder, nNumSphere) ;

	FILE *pfile = fopen("Holecup.col", "w+b") ;
	if(pfile)
	{
		fwrite(&nNumTriangle, 4, 1, pfile) ;
		for(i=0 ; i<nNumTriangle ; i++)
			fwrite(&sTriangles[i], sizeof(geo::STriangle), 1, pfile) ;

		fwrite(&nNumCylinder, 4, 1, pfile) ;
		for(i=0 ; i<nNumCylinder ; i++)
			fwrite(&sCylinders[i], sizeof(geo::SCylinder), 1, pfile) ;

		fwrite(&nNumSphere, 4, 1, pfile) ;
		for(i=0 ; i<nNumSphere ; i++)
			fwrite(&sSpheres[i], sizeof(geo::SSphere), 1, pfile) ;

		fclose(pfile) ;
	}
}

//####################################################
//SLeafAni_in_RigidMesh
//####################################################

#include "PerlinNoise.h"

SLeafAni_in_RigidMesh::SLeafAni_in_RigidMesh()
{
    ZeroMemory(afNoise, sizeof(float)*16) ;
	fCurTheta=fLimitedTheta=fSpeed=0.0f ;
	D3DXMatrixIdentity(&matAni) ;
	bEnable=false ;
}
void SLeafAni_in_RigidMesh::Reset(float limited_theta, float speed, float initial_theta)
{
	bEnable = true ;
    fCurTheta = deg_rad(initial_theta) ;
    fLimitedTheta = deg_rad(limited_theta) ;
	fSpeed = speed ;
	D3DXMatrixIdentity(&matAni) ;

	CPerlinNoise noise(1.0f/4.0f, 8, 32) ;
	float value ;
	for(int i=0 ; i<16 ; i++)
	{
		value = noise.PerlinNoise1D((float)i) ;//range [-1, +1]
		value /= 2.0f ; //change from[-1, +1] to[-0.5, +0.5]
		afNoise[i] = value ;
	}
}
void SLeafAni_in_RigidMesh::Process(float fTimeSinceLastFrame)
{
	//fTimeSinceLastFrame = 0.005f ;
	float delta = (fTimeSinceLastFrame*fSpeed) ;

    float x = float_abs(fmod(fCurTheta, fLimitedTheta)*15.0f/fLimitedTheta) ;
	if(float_less(x, 0) || float_greater(x, 16.0f))
		int sucker=0 ;

	float qq = afNoise[(int)floorf(x)] ;
	float qqq = afNoise[(int)ceilf(x)] ;

    x = LinerInterpolation(afNoise[(int)floorf(x)], afNoise[(int)ceilf(x)], x-floorf(x)) ;
	x = float_abs(x) ;
	x *= fTimeSinceLastFrame ;

	xuck = x ;
	curdelta = delta ;

	fCurTheta += (delta) ;
	if(float_abs(fCurTheta) > fLimitedTheta)
	{
		if(float_positive(delta))//양수로 진행하고 있을때
		{
			float remnant = -fmod(fCurTheta, fLimitedTheta)*2.0f ;// (fLimitedTheta - fCurTheta)*2.0f ;
			fCurTheta = fCurTheta + remnant ;
		}
		else//음수로 진행하고 있을때
		{
			float remnant = fmod(fCurTheta, fLimitedTheta)*2.0f ;// (fCurTheta + fLimitedTheta)*2.0f ;
			fCurTheta = fCurTheta - remnant ;            
		}
		fSpeed = -fSpeed ;
	}
	D3DXMatrixRotationZ(&matAni, fCurTheta) ;
}

/*
void CSecretRigidMesh::ExportHeightMap(int nWidth, int nHeight)
{
	int i, n, x, z ;
	BYTE *pbyData ;
	geo::STriangle *psTriangle ;
	int nCount=0 ;
	unsigned long lDataSize, lBitCount=24 ;
	float u, v ;
	RGBQUAD rgb ;
	D3DXVECTOR4 v4Pos ;
	D3DXVECTOR2 vTex ;

	pbyData = new BYTE[nWidth*nHeight*(lBitCount/8)] ;

	psTriangle = new geo::STriangle[m_lNumTriangle] ;

	for(i=0 ; i<(int)m_lNumTriangle ; i++)
	{
		for(n=0 ; n<3 ; n++)
		{
			//psTriangle[i].avPos[n] =  m_psVertices[m_psIndices[i].anIndex[n]].pos ;
			D3DXVec3Transform(&v4Pos, &m_psVertices[m_psIndices[i].anIndex[n]].pos, this->GetmatLocal()) ;
			psTriangle[i].avPos[n] = D3DXVECTOR3(v4Pos.x, v4Pos.y, v4Pos.z) ;
			
			psTriangle[i].avNormal[n] = m_psVertices[m_psIndices[i].anIndex[n]].normal ;
			psTriangle[i].avTex[n] = m_psVertices[m_psIndices[i].anIndex[n]].t ;
		}
	}

	float fMaxY=m_psVertices[0].pos.y, fMinY=m_psVertices[0].pos.y ;
	for(i=0 ; i<(int)m_lNumVertex ; i++)
	{
		if(fMaxY < m_psVertices[i].pos.y)
			fMaxY = m_psVertices[i].pos.y ;
		if(fMinY > m_psVertices[i].pos.y)
			fMinY = m_psVertices[i].pos.y ;
	}

	v4Pos = D3DXVECTOR4(0, fMaxY, 0, 1) ;
	D3DXVec4Transform(&v4Pos, &v4Pos, GetmatLocal()) ;
	fMaxY = v4Pos.y ;

	v4Pos = D3DXVECTOR4(0, fMinY, 0, 1) ;
	D3DXVec4Transform(&v4Pos, &v4Pos, GetmatLocal()) ;
	fMinY = v4Pos.y ;

	TRACE("\r\n") ;
	TRACE("##HeightValue START##\r\n") ;

	nCount=0 ;
	int nTraceCount=0 ;
	for(z=0 ; z<nHeight ; z++)
	{
		for(x=0 ; x<nWidth ; x++)
		{
			memset(&rgb, 0, sizeof(RGBQUAD)) ;
			for(i=0 ; i<(int)m_lNumTriangle ; i++)
			{
				if(IntesectXZProjTriangleToPoint(psTriangle[i], D3DXVECTOR3((float)x, 0, (float)z), u, v))//u = P1, v = P2, w = P0
				{
					//Height Value Interpolation
					float fInterpolation = (1-u-v)*psTriangle[i].avPos[0].y + u*psTriangle[i].avPos[1].y + v*psTriangle[i].avPos[2].y ;

					fInterpolation += (0-fMinY) ;//Translation
					fInterpolation /= fabs(fMaxY-fMinY) ;//Scale
					fInterpolation = fInterpolation*255 ;

					rgb.rgbRed = (BYTE)((fInterpolation)+0.5f) ;
					rgb.rgbGreen = (BYTE)((fInterpolation)+0.5f) ;
					rgb.rgbBlue = (BYTE)((fInterpolation)+0.5f) ;

					//u of texture Coordinate Interpolation
					vTex.x = (1-u-v)*psTriangle[i].avTex[0].x + u*psTriangle[i].avTex[1].x + v*psTriangle[i].avTex[2].x ;

					//v of texture Coordinate Interpolation
					vTex.y = (1-u-v)*psTriangle[i].avTex[0].y + u*psTriangle[i].avTex[1].y + v*psTriangle[i].avTex[2].y ;

					TRACE("[%2d]InTriangle x=%3d y=%3d height=%2.3f u=%2.3f v=%2.3f\r\n", nTraceCount++, x, z, fInterpolation, vTex.x, vTex.y) ;
					break ;
				}
			}

			pbyData[nCount++] = rgb.rgbRed ;
			pbyData[nCount++] = rgb.rgbGreen ;
			pbyData[nCount++] = rgb.rgbBlue ;
		}
	}

	TRACE("##HeightValue END##\r\n") ;
	TRACE("\r\n") ;

	//BITMAPFILEHEADER bmfh ;
	//BITMAPINFOHEADER bmih ;

	//memset(&bmfh, 0, sizeof(BITMAPFILEHEADER)) ;
	//memset(&bmih, 0, sizeof(BITMAPINFOHEADER)) ;

	//bmih.biSize = sizeof(BITMAPINFOHEADER) ;
	//bmih.biWidth = nWidth ;
	//bmih.biHeight = nHeight ;
	//bmih.biPlanes = 1 ;
	//bmih.biBitCount = (WORD)lBitCount ;
	//bmih.biCompression = BI_RGB ;
	//bmih.biSizeImage = 0 ;
	//bmih.biXPelsPerMeter = 2834 ;
	//bmih.biYPelsPerMeter = 2834 ;

	//int lExtra=0, lPitch = nWidth*(lBitCount/8) ;
	//if(lPitch%4)
	//{
	//	lExtra = 4-(lPitch%4) ;
	//}

	//BYTE *pbyBuf = new BYTE[lPitch] ;
	//DWORD dwBuf=0 ;

	//lDataSize = bmih.biWidth*bmih.biHeight*(bmih.biBitCount/8) + (lExtra*bmih.biHeight) ;
	//bmfh.bfType = ((WORD) ('M' << 8) | 'B') ;
	//bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + lDataSize ;
	//bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) ;

	//FILE *pf=NULL ;
	//pf = fopen(pszFileName, "wb+") ;
	//if(pf)
	//{
	//	fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, pf) ;
	//	fwrite(&bmih, sizeof(BITMAPINFOHEADER), 1, pf) ;

	//	for(i=0 ; i<nHeight ; i++)
	//	{
	//		memcpy(pbyBuf, &pbyData[i*lPitch], lPitch) ;
	//		fwrite(pbyBuf, lPitch, 1, pf) ;

	//		if(lExtra)
	//			fwrite(&dwBuf, lExtra, 1, pf) ;
	//	}
	//}
	//fclose(pf) ;
	//pf=NULL ;

	//SAFE_DELETEARRAY(pbyBuf) ;

	SAFE_DELETEARRAY(pbyData) ;
	SAFE_DELETEARRAY(psTriangle) ;
}
*/