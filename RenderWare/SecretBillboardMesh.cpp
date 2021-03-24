#include "SecretBillboardMesh.h"

//정점 오브젝트 생성
D3DVERTEXELEMENT9 decl_Billboard[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정
	{0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},//텍스쳐 좌표에 대한 설정
	D3DDECL_END()
} ;

CSecretBillboardMesh::CSecretBillboardMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) : CSecretMesh(lEigenIndex, pd3dDevice)
{
	m_bWireFrame = false ;

	m_psVertices = NULL ;
	m_psIndices = NULL ;
}

CSecretBillboardMesh::~CSecretBillboardMesh()
{
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;
}

HRESULT CSecretBillboardMesh::Initialize(SMesh *pMesh)
{
    int i ;
	int origin_count=0 ;

	//TRACE("BillboardMesh Init NodeName=%s, ParentNodeName=%s\r\n", pMesh->szMeshName, pMesh->szParentName) ;

	//Overriding Parent Initialize
	CSecretMesh::Initialize(pMesh) ;

	m_vDiffuse = D3DXVECTOR4(1.4f, 1.4f, 1.4f, 1.0f) ;

	SetVertexElement(decl_Billboard) ;

	Vector3 *pvPos = new Vector3[pMesh->nNumVertex] ;
	Vector2 *pvTex = new Vector2[pMesh->nNumVertex] ;
	Vector3 vMin, vMax ;
	Vector2 tMin(1, 1), tMax(0, 0) ;

	Matrix4 matLocal ;
	memcpy(&matLocal, &pMesh->smatLocal, sizeof(Matrix4)) ;

    vMin = vMax = Vector3(pMesh->psVertex[0].pos.x, pMesh->psVertex[0].pos.y, pMesh->psVertex[0].pos.z) ;
	vMin *= matLocal ;
	vMax *= matLocal ;
	for(i=0 ; i<(int)pMesh->nNumVertex ; i++)
	{
		pvPos[i] = Vector3(pMesh->psVertex[i].pos.x, pMesh->psVertex[i].pos.y, pMesh->psVertex[i].pos.z) ;
		pvPos[i] *= matLocal ;
		pvTex[i] = Vector2(pMesh->psVertex[i].tex.u, pMesh->psVertex[i].tex.v) ;

		if(float_eq(pvTex[i].x, 0.0f) && float_eq(pvTex[i].y, 0.0f))// 4개보다 많은 텍스쳐좌표가 있을경우
			origin_count++ ;

		if(vMin.x > pvPos[i].x) vMin.x = pvPos[i].x ;
		if(vMin.y > pvPos[i].y) vMin.y = pvPos[i].y ;
		if(vMin.z > pvPos[i].z) vMin.z = pvPos[i].z ;

		if(vMax.x < pvPos[i].x) vMax.x = pvPos[i].x ;
		if(vMax.y < pvPos[i].y) vMax.y = pvPos[i].y ;
		if(vMax.z < pvPos[i].z) vMax.z = pvPos[i].z ;

		if(tMin.x > pvTex[i].x)
			tMin.x = pvTex[i].x ;
		if(tMin.y > pvTex[i].y)
			tMin.y = pvTex[i].y ;

		if(tMax.x < pvTex[i].x)
			tMax.x = pvTex[i].x ;
		if(tMax.y < pvTex[i].y)
			tMax.y = pvTex[i].y ;
	}

	m_fWidth = (vMax.x-vMin.x) ;
	assert(float_greater(m_fWidth, 0.0f)) ;

	m_fHeight = (vMax.z-vMin.z) ;
	if(float_eq(m_fHeight, 0.0f))
	{
		m_fHeight = (vMax.y-vMin.y) ;
		assert(float_greater(m_fHeight, 0.0f)) ;
	}

	m_vTexMin = tMin ;
	m_vTexMax = tMax ;

	//m_vCenterPos.set(0, m_fHeight/2.0f, 0) ;
	m_vBillboardOffset.set(0, vMin.y, 0) ;

	//###################//
	//  Create Vertices  //
	//###################//
    m_psVertices = new SBillboardMesh_Vertex[4] ;

	//if(FAILED(m_pd3dDevice->CreateVertexBuffer(4*sizeof(SBillboardMesh_Vertex), D3DUSAGE_WRITEONLY, D3DFVF_BILLBOARDMESHVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL)))
	if(FAILED(m_pd3dDevice->CreateVertexBuffer(4*sizeof(SBillboardMesh_Vertex), 0, D3DFVF_BILLBOARDMESHVERTEX, D3DPOOL_MANAGED, &m_pVB, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddVBSize(4*sizeof(SBillboardMesh_Vertex)) ;

    void *pVertices ;	
	if(FAILED(m_pVB->Lock(0, m_lNumVertex*sizeof(SBillboardMesh_Vertex), (void **)&pVertices, 0)))
		return E_FAIL ;

	SBillboardMesh_Vertex *psVertices = (SBillboardMesh_Vertex *)pVertices ;

	psVertices[0].pos = psVertices[1].pos = psVertices[2].pos = psVertices[3].pos = D3DXVECTOR3(m_vBillboardOffset.x, m_vBillboardOffset.y, m_vBillboardOffset.z) ;

	psVertices[0].t = D3DXVECTOR2(tMin.x, tMin.y) ;
	psVertices[1].t = D3DXVECTOR2(tMax.x, tMin.y) ;
	psVertices[2].t = D3DXVECTOR2(tMin.x, tMax.y) ;
	psVertices[3].t = D3DXVECTOR2(tMax.x, tMax.y) ;

	////upper-left
	//psVertices[0].rightFactor = -m_fWidth/2.0f ;
	//psVertices[0].upFactor = m_fHeight/2.0f ;

	////upper-right
	//psVertices[1].rightFactor = m_fWidth/2.0f ;
	//psVertices[1].upFactor = m_fHeight/2.0f ;

	////lower-left
	//psVertices[2].rightFactor = -m_fWidth/2.0f ;
	//psVertices[2].upFactor = -m_fHeight/2.0f ;

	////lower-right
	//psVertices[3].rightFactor = m_fWidth/2.0f ;
	//psVertices[3].upFactor = -m_fHeight/2.0f ;

	float div_upfactor=1.25f ;

	div_upfactor = 0.0f ;

	//upper-left
	psVertices[0].rightFactor = -m_fWidth/2.0f ;
	psVertices[0].upFactor = m_fHeight - div_upfactor ;

	//upper-right
	psVertices[1].rightFactor = m_fWidth/2.0f ;
	psVertices[1].upFactor = m_fHeight - div_upfactor ;

	//lower-left
	psVertices[2].rightFactor = -m_fWidth/2.0f ;
	psVertices[2].upFactor = -div_upfactor ;

	//lower-right
	psVertices[3].rightFactor = m_fWidth/2.0f ;
	psVertices[3].upFactor = -div_upfactor ;

	m_psVertices[0] = psVertices[0] ;
	m_psVertices[1] = psVertices[1] ;
	m_psVertices[2] = psVertices[2] ;
	m_psVertices[3] = psVertices[3] ;

	m_pVB->Unlock() ;

	SAFE_DELETEARRAY(pvPos) ;
	SAFE_DELETEARRAY(pvTex) ;

	//###################//
	//  Create Indices   //
	//###################//
	m_psIndices = new SBillboardMesh_Index[2] ;

	//if(FAILED(m_pd3dDevice->CreateIndexBuffer(2*sizeof(SBillboardMesh_Index), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVI, NULL)))
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(2*sizeof(SBillboardMesh_Index), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pVI, NULL)))
		return E_FAIL ;

	g_cDebugMsg.AddIBSize(2*sizeof(SBillboardMesh_Index)) ;

    void *pIndices ;	
	if(FAILED(m_pVI->Lock(0, 2*sizeof(SBillboardMesh_Index), (void **)&pIndices, 0)))
		return E_FAIL ;

	SBillboardMesh_Index *psIndices = (SBillboardMesh_Index *)pIndices ;

	psIndices[0].anIndex[0] = 0 ;
	psIndices[0].anIndex[1] = 1 ;
	psIndices[0].anIndex[2] = 2 ;

	psIndices[1].anIndex[0] = 2 ;
	psIndices[1].anIndex[1] = 1 ;
	psIndices[1].anIndex[2] = 3 ;

	m_psIndices[0] = psIndices[0] ;
	m_psIndices[1] = psIndices[1] ;

	m_pVI->Unlock() ;

	return S_OK ;
}
void CSecretBillboardMesh::Render()
{
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SBillboardMesh_Vertex)) ;
	
	m_pd3dDevice->SetIndices(m_pVI) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2) ;
	g_cDebugMsg.AddTriangles(4) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}
void CSecretBillboardMesh::SetBillboardDiffuse(char *pszObjectName)
{
	if( strstr(pszObjectName, "104_yan") )
		m_vDiffuse = D3DXVECTOR4(0.7f, 0.7f, 0.7f, 1.0f) ;

	if( strstr(pszObjectName, "10_gaa_o") )
		m_vDiffuse = D3DXVECTOR4(1.1f, 1.1f, 1.1f, 1.0f) ;
	if( strstr(pszObjectName, "10_gaa_t") )
		m_vDiffuse = D3DXVECTOR4(1.1f, 1.1f, 1.1f, 1.0f) ;

	if( strstr(pszObjectName, "66_soo") )
		m_vDiffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f) ;
		//m_vDiffuse = D3DXVECTOR4(2.1f, 2.1f, 2.1f, 1.0f) ;
}