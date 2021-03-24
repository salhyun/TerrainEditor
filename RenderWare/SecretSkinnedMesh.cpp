#include "SecretSkinnedMesh.h"

using namespace std ;

//정점 오브젝트 생성
D3DVERTEXELEMENT9 decl_Skinned[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//법선에 대한 설정
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정 tu, tv 범프맵핑일 경우 사용
	D3DDECL_END()
} ;

int compare_Weight_greaterSW(const void *pv1, const void *pv2)
{
	SBoneWeight *psBoneWeight1 = (SBoneWeight *)pv1 ;
	SBoneWeight *psBoneWeight2 = (SBoneWeight *)pv2 ;

	float fValue = (psBoneWeight2->fWeight - psBoneWeight1->fWeight)*512.0f ;

    return (int)fValue ;
}

CSecretSkinnedMesh::CSecretSkinnedMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) : CSecretMesh(lEigenIndex, pd3dDevice)
{
	m_bWireFrame = false ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_psSkinData = NULL ;

	TRACE("CSecretSkinnedMesh construction EigenIndex=%d\r\n", lEigenIndex) ;
}

CSecretSkinnedMesh::~CSecretSkinnedMesh()
{
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;
    SAFE_DELETEARRAY(m_psSkinData) ;

	TRACE("CSecretSkinnedMesh destruction\r\n") ;
}

HRESULT CSecretSkinnedMesh::Initialize(SMesh *pMesh)
{
	int i, n ;

	TRACE("SkinnedMesh Init NodeName=%s, ParentNodeName=%s\r\n", pMesh->szMeshName, pMesh->szParentName) ;

	//Overriding Parent Initialize
	CSecretMesh::Initialize(pMesh) ;

	m_pVertexElement = decl_Skinned ;

	SBoneWeight asBoneWeight[MAXNUM_WEIGHTVALUE] ;

	///////////////////////////////////////////////////
	//Create Vertices
	///////////////////////////////////////////////////
	m_lNumVertex = pMesh->nNumVertex ;
	m_psSkinData = new SSkinData[m_lNumVertex] ;

	m_lNumNormalVector = m_lNumVertex ;
	m_psNormalVector = new SNormalVector[m_lNumNormalVector] ;
	m_psNormalVertex = new SNormalVertex[m_lNumNormalVector] ;
	
	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*sizeof(SSkinnedMesh_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_SKINNEDMESHVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddVBSize(m_lNumVertex*sizeof(SSkinnedMesh_Vertex)) ;

	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SSkinnedMesh_Vertex), &pVertices, 0)))
		return E_FAIL ;

	SSkinnedMesh_Vertex *psVertices = (SSkinnedMesh_Vertex *)pVertices ;
	for(i=0 ; i<(int)m_lNumVertex ; i++)
	{
		psVertices->pos = D3DXVECTOR3(pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z)  ;
		psVertices->normal = D3DXVECTOR3(pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z)  ;
		psVertices->t = D3DXVECTOR2(pMesh->psVertex[i].tex.u, pMesh->psVertex[i].tex.v) ;
//		TRACE("[%d] pos  x=%.3f y=%.3f, z=%.3f\r\n", i, psVertices->pos.x, psVertices->pos.y, psVertices->pos.z) ;

		m_psSkinData[i].vPos = psVertices->pos ;
		m_psSkinData[i].vNormal = psVertices->normal ;

		//본가중치를 큰순서데로 정렬한다
		for(n=0 ; n<pMesh->psVertex[i].nNumSkin ; n++)
		{
			asBoneWeight[n].nIndex = pMesh->psVertex[i].apsSkin[n]->nBoneIndex ;
			asBoneWeight[n].fWeight = pMesh->psVertex[i].apsSkin[n]->fWeight ;
		}

		qsort(asBoneWeight, pMesh->psVertex[i].nNumSkin, sizeof(SBoneWeight), compare_Weight_greaterSW) ;

		if(pMesh->psVertex[i].nNumSkin > 3)
			m_psSkinData[i].nNumIndex = 4 ;
		else
			m_psSkinData[i].nNumIndex = pMesh->psVertex[i].nNumSkin ;

		m_psSkinData[i].anBoneIndex.resize(m_psSkinData[i].nNumIndex) ;
		m_psSkinData[i].afWeight.resize(m_psSkinData[i].nNumIndex) ;

        for(n=0 ; n<m_psSkinData[i].nNumIndex ; n++)
		{
			m_psSkinData[i].anBoneIndex[n] = asBoneWeight[n].nIndex ;
			m_psSkinData[i].afWeight[n] = asBoneWeight[n].fWeight ;
		}
        //3개까지만 저장하고 나머지 하나는 끼워맞춰 넣는다.
		if(pMesh->psVertex[i].nNumSkin > 3)
		{
			m_psSkinData[i].anBoneIndex[3] = asBoneWeight[3].nIndex ;
			m_psSkinData[i].afWeight[3] = 1.0f - (asBoneWeight[0].fWeight + asBoneWeight[1].fWeight + asBoneWeight[2].fWeight) ;
		}

		psVertices++ ;
	}

	m_pVB->Unlock() ;

    ///////////////////////////////////////////////////
	//Create Indices
	///////////////////////////////////////////////////
	m_lNumTriangle = pMesh->nNumTriangle ;

	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumTriangle*sizeof(SSkinnedMesh_Index), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVI, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddIBSize(m_lNumTriangle*sizeof(SSkinnedMesh_Index)) ;

	void *pIndices ;
	if(FAILED(m_pVI->Lock(0, m_lNumTriangle*sizeof(SSkinnedMesh_Index), (void **)&pIndices, 0)))
		return E_FAIL ;

    SSkinnedMesh_Index *psIndices = (SSkinnedMesh_Index *)pIndices ;
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

HRESULT CSecretSkinnedMesh::SetMatrixPaletteSW(D3DXMATRIX *pmatSkin, D3DXMATRIX *pmatDummy, char **ppszBoneName)
{
    int i, n ;
	int nBoneIndex ;
	float fWeight ;
	D3DXMATRIX matTemp, matSkin ;
	D3DXVECTOR3 vtemp, vPos, vNormal ;
	char *pszBoneName ;

	void *pVertices ;
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SSkinnedMesh_Vertex), &pVertices, 0)))
		return E_FAIL ;

	SSkinnedMesh_Vertex *psVertices = (SSkinnedMesh_Vertex *)pVertices ;
	for(i=0 ; i<(int)m_lNumVertex ; i++)
	{
		psVertices->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f) ;
		vNormal = D3DXVECTOR3(0.0f, 0.0f, 0.0f) ;
		D3DXMatrixIdentity(&matTemp) ;

		for(n=0 ; n<m_psSkinData[i].nNumIndex ; n++)
		{
			nBoneIndex = m_psSkinData[i].anBoneIndex[n] ;
			fWeight = (float)m_psSkinData[i].afWeight[n] ;
			pszBoneName = ppszBoneName[nBoneIndex] ;
			matSkin = pmatSkin[nBoneIndex] ;

            //matSkin = m_matTM * pmatSkin[nBoneIndex] ;
			matSkin = m_matLocal * pmatSkin[nBoneIndex] ;

			D3DXVec3TransformCoord(&vPos, &m_psSkinData[i].vPos, &matSkin) ;
			psVertices->pos += (vPos * fWeight) ;

			//노말벡터를 다시 계산 해야 된다.
			vtemp.x = m_psSkinData[i].vNormal.x*matSkin._11 + m_psSkinData[i].vNormal.y*matSkin._21 + m_psSkinData[i].vNormal.z*matSkin._31 ;
			vtemp.y = m_psSkinData[i].vNormal.x*matSkin._12 + m_psSkinData[i].vNormal.y*matSkin._22 + m_psSkinData[i].vNormal.z*matSkin._32 ;
			vtemp.z = m_psSkinData[i].vNormal.x*matSkin._13 + m_psSkinData[i].vNormal.y*matSkin._23 + m_psSkinData[i].vNormal.z*matSkin._33 ;

			vNormal += (vtemp*fWeight) ;
		}

		D3DXVec3Normalize(&vNormal, &vNormal) ;
		psVertices->normal = vNormal ;

		m_psNormalVector[i].vStart = Vector3(psVertices->pos.x, psVertices->pos.y, psVertices->pos.z)  ;
		m_psNormalVector[i].vNormal = Vector3(vNormal.x, vNormal.y, vNormal.z) ;
		//m_psNormalVector[i].vNormal = Vector3(m_psSkinData[i].vNormal.x, m_psSkinData[i].vNormal.y, m_psSkinData[i].vNormal.z) ;

		psVertices++ ;
	}
	m_pVB->Unlock() ;

	return S_OK ;
}

void CSecretSkinnedMesh::Render()
{
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	//m_pd3dDevice->SetFVF(D3DFVF_SKINNEDMESHVERTEX) ;
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SSkinnedMesh_Vertex)) ;
	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_lNumVertex, 0, m_lNumTriangle) ;
	g_cDebugMsg.AddTriangles(m_lNumTriangle) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}

#include "Matrix.h"

void CSecretSkinnedMesh::RenderNormalVector(Matrix4 &matWorld)
{
	typedef struct
	{
		float x, y, z ;
		DWORD color ;
	} VERTEX ;

	VERTEX vertex[2] ;

	vertex[0].color = vertex[1].color = 0xffffffff ;

	Vector3 vNormal ;
	for(UINT i=0 ; i<m_lNumNormalVector ; i++)
	{
		m_psNormalVertex[i].vStart = m_psNormalVector[i].vStart * matWorld ;
		vertex[0].x = m_psNormalVertex[i].vStart.x ;
		vertex[0].y = m_psNormalVertex[i].vStart.y ;
		vertex[0].z = m_psNormalVertex[i].vStart.z ;

		vNormal.x = m_psNormalVector[i].vNormal.x*matWorld.m11 + m_psNormalVector[i].vNormal.y*matWorld.m21 + m_psNormalVector[i].vNormal.z*matWorld.m31 ;
		vNormal.y =	m_psNormalVector[i].vNormal.x*matWorld.m12 + m_psNormalVector[i].vNormal.y*matWorld.m22 + m_psNormalVector[i].vNormal.z*matWorld.m32 ;
		vNormal.z = m_psNormalVector[i].vNormal.x*matWorld.m13 + m_psNormalVector[i].vNormal.y*matWorld.m23 + m_psNormalVector[i].vNormal.z*matWorld.m33 ;

		vNormal *= (0.1f/vNormal.Magnitude()) ;
		m_psNormalVertex[i].vEnd = m_psNormalVertex[i].vStart+vNormal ;

		vertex[1].x = m_psNormalVertex[i].vEnd.x ;
		vertex[1].y = m_psNormalVertex[i].vEnd.y ;
		vertex[1].z = m_psNormalVertex[i].vEnd.z ;

		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertex, sizeof(vertex[0])) ;
	}
}