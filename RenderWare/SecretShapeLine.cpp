#include "SecretShapeLine.h"
#include "def.h"
#include "ASEData.h"
#include "SecretTerrain.h"

CSecretShapeLine::CSecretShapeLine() : CSecretShape(0)
{
	m_nAttr = m_nNumPosition = m_nNumVertex = m_nNumIndex = 0 ;
	m_nMaxNumPosition = m_nMaxNumVertex = m_nMaxNumIndex = 0 ;
	m_dwColor = 0xff0000ff ;
	m_pvPositions = m_psOriginPositions = NULL ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_fLineWidth = 0.1f ;
}
CSecretShapeLine::CSecretShapeLine(int nKind) : CSecretShape(nKind)
{
	m_nAttr = m_nNumPosition = m_nNumVertex = m_nNumIndex = 0 ;
	m_nMaxNumPosition = m_nMaxNumVertex = m_nMaxNumIndex = 0 ;
	m_dwColor = 0xff0000ff ;
	m_pvPositions = m_psOriginPositions = NULL ;
	m_psVertices = NULL ;
	m_psIndices = NULL ;
	m_fLineWidth = 0.1f ;
}

CSecretShapeLine::~CSecretShapeLine()
{
}
void CSecretShapeLine::Release()
{
	SAFE_DELETEARRAY(m_pvPositions) ;
	SAFE_DELETEARRAY(m_psOriginPositions) ;
	SAFE_DELETEARRAY(m_psVertices) ;
	SAFE_DELETEARRAY(m_psIndices) ;
}
bool CSecretShapeLine::Initialize(SShapeLine *psShapeLine, LPDIRECT3DDEVICE9 pd3dDevice, float fLineWidth)
{
	int i ;
	D3DXVECTOR4 vRet ;
	D3DXVECTOR3 vTotal(0, 0, 0) ;
	m_pd3dDevice = pd3dDevice ;
	if(psShapeLine->bClosed)
		AddAttr(CSecretShape::ATTR_CLOSED) ;

	if(m_nAttr & ATTR_CLOSED)
	{
		m_nMaxNumPosition = m_nNumPosition = psShapeLine->nNumPos+1 ;
		assert(m_nNumPosition > 1) ;

		m_pvPositions = new D3DXVECTOR3[m_nNumPosition] ;
		m_psOriginPositions = new D3DXVECTOR3[m_nNumPosition] ;
		m_nMaxNumVertex = m_nNumVertex = (m_nNumPosition*4)-4 ;
		m_psVertices = new SShapeLine_Vertex[m_nNumVertex] ;
		m_nMaxNumIndex = m_nNumIndex = (m_nNumPosition-1)*2 ;
		m_psIndices = new SShapeLine_Index[m_nNumIndex] ;

		for(i=0 ; i<(m_nNumPosition-1) ; i++)
		{
			m_pvPositions[i].x = psShapeLine->psPos[i].x ;
			m_pvPositions[i].y = psShapeLine->psPos[i].y ;
			m_pvPositions[i].z = psShapeLine->psPos[i].z ;

            //D3DXVec3Transform(&vRet, &m_pvPositions[i], &m_matLocal) ;
			//m_pvPositions[i].x = vRet.x ;
			//m_pvPositions[i].y = vRet.y ;
			//m_pvPositions[i].z = vRet.z ;

			m_psOriginPositions[i] = m_pvPositions[i] ;

			vTotal.x += m_pvPositions[i].x ;
			vTotal.y += m_pvPositions[i].y ;
			vTotal.z += m_pvPositions[i].z ;
		}
		m_pvPositions[i] = m_pvPositions[0] ;
	}
	else
	{
		m_nMaxNumPosition = m_nNumPosition = psShapeLine->nNumPos ;
		assert(m_nNumPosition > 1) ;

		m_pvPositions = new D3DXVECTOR3[m_nNumPosition] ;
		m_psOriginPositions = new D3DXVECTOR3[m_nNumPosition] ;
		//m_nNumVertex = m_nNumPosition*2 ;
		m_nMaxNumVertex = m_nNumVertex = (m_nNumPosition*4)-4 ;
		m_psVertices = new SShapeLine_Vertex[m_nNumVertex] ;
		m_nMaxNumIndex = m_nNumIndex = (m_nNumPosition-1)*2 ;
		m_psIndices = new SShapeLine_Index[m_nNumIndex] ;

		for(i=0 ; i<m_nNumPosition ; i++)
		{
			m_pvPositions[i].x = psShapeLine->psPos[i].x ;
			m_pvPositions[i].y = psShapeLine->psPos[i].y ;
			m_pvPositions[i].z = psShapeLine->psPos[i].z ;

            //D3DXVec3Transform(&vRet, &m_pvPositions[i], &m_matLocal) ;
			//m_pvPositions[i].x = vRet.x ;
			//m_pvPositions[i].y = vRet.y ;
			//m_pvPositions[i].z = vRet.z ;

			m_psOriginPositions[i] = m_pvPositions[i] ;

			vTotal.x += m_pvPositions[i].x ;
			vTotal.y += m_pvPositions[i].y ;
			vTotal.z += m_pvPositions[i].z ;
		}
	}

	m_vCenter.x = vTotal.x/(float)m_nNumPosition ;
	m_vCenter.y = vTotal.y/(float)m_nNumPosition ;
	m_vCenter.z = vTotal.z/(float)m_nNumPosition ;

	AddAttr(CSecretShape::ATTR_RENDER) ;

	m_fLineWidth = fLineWidth ;

	//m_nNumPosition = 4 ;
	//m_nNumVertex = (m_nNumPosition*4)-4 ;
	//m_nNumIndex = (m_nNumPosition-1)*2 ;

	//m_pvPositions[0] = D3DXVECTOR3(0, 6, 0) ;
	//m_pvPositions[1] = D3DXVECTOR3(1, 6, 0) ;
	//m_pvPositions[2] = D3DXVECTOR3(2.5f, 7, 1) ;
	//m_pvPositions[3] = D3DXVECTOR3(1, 8, 2) ;

	return true ;
}
void CSecretShapeLine::Process(D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj)
{
	if(m_nNumPosition <= 1)
		return ;

	D3DXMATRIX matVP = (*pmatView) * (*pmatProj) ;

	float fWidth ;
	if(m_nAttr & CSecretShape::ATTR_SAMESIZE)
	{
		D3DXVECTOR4 r ;
		D3DXVec3Transform(&r, &m_vCenter, &matVP) ;
		fWidth = r.w*m_fLineWidth/1000.0f ;
		if(fWidth <= 0.1f)
			fWidth = 0.1f ;
		//g_cDebugMsg.SetDebugMsg(3, "r.w = %g", r.w) ;
	}
	else
		fWidth = m_fLineWidth ;

    //_BuildQuadStrip(m_pvPositions, m_nNumPosition, pvCamera, pvUp, &matVP, m_psVertices, 0xff0000ff) ;
	_BuildQuadList(m_pvPositions, m_nNumPosition, pvCamera, pvUp, &matVP, m_psVertices, m_psIndices, fWidth, m_dwColor) ;
}
void CSecretShapeLine::Render()
{
	if(m_nNumPosition <= 1)
		return ;
	//m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME) ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	DWORD dwZEnable ;
	m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	//m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, (m_nNumPosition*2)-2, m_psVertices, sizeof(m_psVertices[0])) ;
	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_nNumVertex, m_nNumIndex, m_psIndices, D3DFMT_INDEX16, m_psVertices, sizeof(m_psVertices[0])) ;

	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;

	//m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID) ;
}
void CSecretShapeLine::SetLocalTM(D3DXMATRIX *pmat)
{
	m_matLocal = (*pmat) ;
}
void CSecretShapeLine::_BuildQuadStrip(D3DXVECTOR3 *pvPos, int nNumPos, D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatVP, SShapeLine_Vertex *psVertices, float fWidth, DWORD color)
{
	int i, nStrip=0 ;
	D3DXVECTOR3 N, B, T, v ;
	D3DXVECTOR4 v4 ;

	//first point
	N = (*pvCamera)-(*pvPos) ;
	D3DXVec3Normalize(&N, &N) ;

	B = (*(pvPos+1))-(*pvPos) ;
	D3DXVec3Normalize(&B, &B) ;

	D3DXVec3Cross(&T, &B, &N) ;
	D3DXVec3Normalize(&T, &T) ;
	if(float_eq(T.x, 0.0f) && float_eq(T.y, 0.0f) && float_eq(T.z, 0.0f))// opposite direction between camera dir and tangent dir
        T = *pvUp ;

	//D3DXVec3Transform(&v4, pvPos, pmatVP) ;
	//fWidth = v4.w*m_fLineWidth/1000.0f ;//  v4.w/m_fLineWidth ;

	psVertices->pos = (*pvPos) - fWidth*T ;
	psVertices->color = color ;
	psVertices++ ;
	nStrip++ ;

	psVertices->pos = (*pvPos) + fWidth*T ;
	psVertices->color = color ;
	psVertices++ ;
	nStrip++ ;

	pvPos++ ;

	for(i=1 ; i<(nNumPos-1) ; i++, pvPos++)
	{
		N = (*pvCamera)-(*pvPos) ;
		D3DXVec3Normalize(&N, &N) ;

		B = (*(pvPos+1))-(*(pvPos-1)) ;
		//B = (*(pvPos+1))-(*pvPos) ;
		D3DXVec3Normalize(&B, &B) ;

		D3DXVec3Cross(&T, &B, &N) ;
		D3DXVec3Normalize(&T, &T) ;
		if(float_eq(T.x, 0.0f) && float_eq(T.y, 0.0f) && float_eq(T.z, 0.0f))// opposite direction between camera dir and tangent dir
			T = *pvUp ;

		D3DXVec3Transform(&v4, pvPos, pmatVP) ;
		fWidth = v4.w*m_fLineWidth/1000.0f ;//  v4.w/m_fLineWidth ;

		psVertices->pos = (*pvPos) - fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nStrip++ ;

		psVertices->pos = (*pvPos) + fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nStrip++ ;
	}

	//end point
	N = (*pvCamera)-(*pvPos) ;
	D3DXVec3Normalize(&N, &N) ;

	B = (*pvPos)-(*(pvPos-1)) ;
	D3DXVec3Normalize(&B, &B) ;

	D3DXVec3Cross(&T, &B, &N) ;
	D3DXVec3Normalize(&T, &T) ;
	if(float_eq(T.x, 0.0f) && float_eq(T.y, 0.0f) && float_eq(T.z, 0.0f))// opposite direction between camera dir and tangent dir
        T = *pvUp ;

	D3DXVec3Transform(&v4, pvPos, pmatVP) ;
	fWidth = v4.w*m_fLineWidth/1000.0f ;//  v4.w/m_fLineWidth ;

	psVertices->pos = (*pvPos) - fWidth*T ;
	psVertices->color = color ;
	psVertices++ ;
	nStrip++ ;

	psVertices->pos = (*pvPos) + fWidth*T ;
	psVertices->color = color ;
	psVertices++ ;
	nStrip++ ;

	//TRACE("Strip count %d\r\n", nStrip) ;
}
void CSecretShapeLine::_BuildQuadList(D3DXVECTOR3 *pvPos, int nNumPos, D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatVP, SShapeLine_Vertex *psVertices, SShapeLine_Index *psIndices, float fWidth, DWORD color)
{
	int i, nVertexCount=0, nIndexCount=0 ;
	float ftemp ;
	D3DXVECTOR3 N, B, T, v ;
	D3DXVECTOR4 v4 ;

	for(i=0 ; i<(nNumPos-1) ; i++, pvPos++)
	{
		B = (*(pvPos+1))-(*pvPos) ;
		ftemp = D3DXVec3Length(&B)/2.0f ;
		D3DXVec3Normalize(&B, &B) ;

		v = (*pvPos) + ftemp*B ;
		N = (*pvCamera)-v ;
		D3DXVec3Normalize(&N, &N) ;

		D3DXVec3Cross(&T, &B, &N) ;
		D3DXVec3Normalize(&T, &T) ;
		if(float_eq(T.x, 0.0f) && float_eq(T.y, 0.0f) && float_eq(T.z, 0.0f))// opposite direction between camera dir and tangent dir
			T = *pvUp ;

		//D3DXVec3Transform(&v4, &v, pmatVP) ;
		//fWidth = v4.w*m_fLineWidth/1000.0f ;//  v4.w/m_fLineWidth ;

		psVertices->pos = (*pvPos) - fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psVertices->pos = (*pvPos) + fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psVertices->pos = (*(pvPos+1)) - fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psVertices->pos = (*(pvPos+1)) + fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psIndices->wIndex[0] = nVertexCount-4 ;
		psIndices->wIndex[1] = nVertexCount-3 ;
		psIndices->wIndex[2] = nVertexCount-1 ;
		psIndices++ ;
		nIndexCount++ ;

        psIndices->wIndex[0] = nVertexCount-1 ;
		psIndices->wIndex[1] = nVertexCount-2 ;
		psIndices->wIndex[2] = nVertexCount-4 ;
		psIndices++ ;
		nIndexCount++ ;
	}

	m_nNumVertex = nVertexCount ;
	m_nNumIndex = nIndexCount ;
	//TRACE("vertex=%d index=%d\r\n", nVertexCount, nIndexCount) ;
}
void CSecretShapeLine::OnTerrain(CSecretTerrain *pcTerrain)
{
	for(int i=0 ; i<m_nNumPosition ; i++)
		m_pvPositions[i].y = pcTerrain->m_sHeightMap.GetHeightMap(m_pvPositions[i].x, m_pvPositions[i].z)+0.1f ;

	AddAttr(CSecretShape::ATTR_ONTERRAIN) ;
}