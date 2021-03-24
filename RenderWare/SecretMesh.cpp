#include "SecretMesh.h"
#include "SecretMath.h"
#include "GeoLib.h"

CSecretMesh::CSecretMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) : CSecretNode(lEigenIndex)
{
	m_lNumVertex = 0 ;
	m_lNumTriangle = 0 ;

	m_pd3dDevice = pd3dDevice ;
	m_pVB = NULL ;
	m_pVI = NULL ;

	m_psNormalVector = NULL ;
	m_psNormalVertex = NULL ;
	m_psVertexDisk = NULL ;

	m_psBoundingPolyhedron = NULL ;
	m_pCollisionModel = NULL ;

	m_vDiffuse = D3DXVECTOR4(0.7f, 0.7f, 0.7f, 1.0f) ;
	m_vOriginDiffuse = m_vDiffuse ;
	m_fAlphaBlendingByForce = 1.0f ;
	//TRACE("SecretMesh construction EigenIndex=%d\r\n", lEigenIndex) ;
}
CSecretMesh::~CSecretMesh()
{
	Release() ;
	//TRACE("CSecretMesh destruction\r\n") ;
}

HRESULT CSecretMesh::Initialize(SMesh *pMesh)
{
	m_matLocal = D3DXMATRIX((D3DXMATRIX &)pMesh->smatLocal) ;
	m_matChild = D3DXMATRIX((D3DXMATRIX &)pMesh->smatChild) ;
	D3DXMatrixIdentity(&m_matTM) ;
    D3DXMatrixIdentity(&m_matSkin) ;

	m_nMaterialID = pMesh->nMaterialIndex ;
	m_nParentID = pMesh->nMeshParentIndex ;

	sprintf(m_szNodeName, "%s", pMesh->szMeshName) ;
	
	if(pMesh->bParent)
	{
		sprintf(m_szParentNodeName, "%s", pMesh->szParentName) ;
		m_bParent = true ;
	}

	Vector3 vCenter ;
	float w, h, d ;

	vCenter.x = pMesh->vMin.x+((pMesh->vMax.x-pMesh->vMin.x)*0.5f) ;
	vCenter.y = pMesh->vMin.y+((pMesh->vMax.y-pMesh->vMin.y)*0.5f) ;
	vCenter.z = pMesh->vMin.z+((pMesh->vMax.z-pMesh->vMin.z)*0.5f) ;
	w = (pMesh->vMax.x-pMesh->vMin.x) ;
	h = (pMesh->vMax.y-pMesh->vMin.y) ;
	d = (pMesh->vMax.z-pMesh->vMin.z) ;
	m_sAACube.set(vCenter, w, h, d) ;

	m_lAttr = pMesh->nAttr ;

	//메쉬 종류 설정
	if(pMesh->MeshKind == SMESHKIND_BONE)
		m_NodeKind = SECRETNODEKIND_BONE ;
	else if(pMesh->MeshKind == SMESHKIND_RIGID || pMesh->MeshKind == SMESHKIND_XBILLBOARD)
		m_NodeKind = SECRETNODEKIND_RIGID ;
	else if(pMesh->MeshKind == SMESHKIND_DUMMY)
		m_NodeKind = SECRETNODEKIND_DUMMY ;
	else if(pMesh->MeshKind == SMESHKIND_SKINNING)
		m_NodeKind = SECRETNODEKIND_SKINNING ;
	else if(pMesh->MeshKind == SMESHKIND_BILLBOARD)
		m_NodeKind = SECRETNODEKIND_BILLBOARD ;

	return S_OK ;
}

void CSecretMesh::SetNormalVectors(SMesh *pMesh)
{
	m_lNumNormalVector = pMesh->nNumTriangle ;
	m_psNormalVector = new SNormalVector[m_lNumNormalVector] ;
	m_psNormalVertex = new SNormalVertex[m_lNumNormalVector] ;

	int i, n, nIndex ;
	Vector3 avPos[3], vNormal ;
	for(i=0 ; i<pMesh->nNumTriangle ; i++)
	{
		for(n=0 ; n<3 ; n++)
		{
			nIndex = pMesh->psTriangle[i].anVertexIndex[n] ;
			avPos[n] = Vector3(pMesh->psVertex[nIndex].pos.x, pMesh->psVertex[nIndex].pos.y, pMesh->psVertex[nIndex].pos.z) ;
		}

		m_psNormalVector[i].vStart = GetMidPoint(avPos[0], avPos[1], avPos[2]) ;
		m_psNormalVector[i].vNormal = Vector3(pMesh->psTriangle[i].sFaceNormal.x, pMesh->psTriangle[i].sFaceNormal.y, pMesh->psTriangle[i].sFaceNormal.z) ;

		m_psNormalVertex[i].vStart = m_psNormalVector[i].vStart ;
		m_psNormalVertex[i].vEnd = m_psNormalVector[i].vStart+m_psNormalVector[i].vNormal ;
	}
}

void CSecretMesh::Animation(float fFrame, D3DXMATRIX *pmatParentTM)
{
	if(GetAttr() & SECRETNODEATTR_ANIMATION)//Bone, Bip...
	{
		D3DXMATRIX *pmatAni ;
		pmatAni = m_cAniTrack.Animation(fFrame, this) ;

		//에니메이션인 경우는 에니메이션되는 행렬만 계산된다 matTM에다가
		m_matTM = (*pmatAni) * (*pmatParentTM) ;
	}
	else//에니가 아닌경우는 로칼행렬이 계산된다 Rigid, Skinning...
	{
		m_matTM = m_matChild * (*pmatParentTM) ;
	}

	//로칼역행렬과 애니행렬을 미리 계산 해둔다. 소프트웨어 스키닝을 위해서
	if(GetNodeKind() == SECRETNODEKIND_BONE)
	{
		D3DXMatrixInverse(&m_matSkin, 0, &m_matLocal) ;
		m_matSkin = m_matSkin * m_matTM ;
	}
}

void CSecretMesh::SetWireFrame(bool bEnable)
{
	m_bWireFrame = bEnable ;
}

float Corresponded(SMeshVertex *psVertex, Vector3 *pv)
{
	for(int i=0 ; i<3 ; i++)
	{
		if(VECTOR_EQ(psVertex->pos, pv[i]))
		{
			Vector3 pos(psVertex->pos.x, psVertex->pos.y, psVertex->pos.z) ;
            Vector3 center = GetMidPoint(pv[0], pv[1], pv[2]) ;
			return (pos-center).Magnitude() ;
		}
	}
	return 0 ;
}

float ComputeDiskAreaPerVertex(SMeshVertex *psVertex, SMesh *psMesh)
{
	int n, i, nCorresponded=0 ;
	float fRadius, fSumRadius=0 ;
	Vector3 avPos[3] ;

	for(i=0 ; i<psMesh->nNumTriangle ; i++)
	{
		for(n=0 ; n<3 ; n++)
		{
			avPos[n].set(psMesh->psVertex[psMesh->psTriangle[i].anVertexIndex[n]].pos.x,
						 psMesh->psVertex[psMesh->psTriangle[i].anVertexIndex[n]].pos.y,
						 psMesh->psVertex[psMesh->psTriangle[i].anVertexIndex[n]].pos.z);
		}

		fRadius = Corresponded(psVertex, avPos) ;//현재의 버텍스가 삼각형의 꼭지점에 포함되어 있다면
		if(fRadius > (0+g_fEpsilon))
		{
			nCorresponded++ ;
			fSumRadius += fRadius ;
		}
	}
    assert(nCorresponded) ;//부합되는 버텍스가 없으면 안된다.
	return fSumRadius/(float)nCorresponded ;//평균 반지름을 구한다
}

void CSecretMesh::SetVertexDisk(SMesh *pMesh)
{
	m_psVertexDisk = new SVertexDisk[pMesh->nNumVertex] ;

	for(int i=0 ; i<pMesh->nNumVertex ; i++)
	{
		m_psVertexDisk[i].vPos.set(pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z) ;
		m_psVertexDisk[i].vNormal.set(pMesh->psVertex[i].normal.x, pMesh->psVertex[i].normal.y, pMesh->psVertex[i].normal.z) ;
		m_psVertexDisk[i].fRadius = ComputeDiskAreaPerVertex(&pMesh->psVertex[i], pMesh) ;
		m_psVertexDisk[i].nIndexOfVertex = i ;
	}
}

void CSecretMesh::SetVertexElement(D3DVERTEXELEMENT9 *pVertexElement)
{
    m_pVertexElement = pVertexElement ;
}

void CSecretMesh::Release()
{
	if(m_pVB)
		m_pVB->Release() ;
	if(m_pVI)
		m_pVI->Release() ;

	if(m_psNormalVector)
		delete []m_psNormalVector ;
	if(m_psNormalVertex)
		delete []m_psNormalVertex ;
    if(m_psVertexDisk)
		delete []m_psVertexDisk ;
}

Vector3 CSecretMesh::GetPos()
{
	Vector3 v(m_matTM._41, m_matTM._42, m_matTM._43) ;
	return v ;
}

CollisionModel3D *CSecretMesh::GetCollisionModel()
{
	return m_pCollisionModel ;
}