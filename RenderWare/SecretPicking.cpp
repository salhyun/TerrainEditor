#include "SecretPicking.h"
#include "D3DEnvironment.h"
#include "SecretTerrain.h"
#include "SecretQuadTree.h"

//지형타일이 2m간격일 경우 4096
//지형타일이 1m간격일 경우 8192
//CSecretPicking::CSecretPicking(int nTerrainTileSize) : MAXNUM_INTERSECTEDTRIANGLE(8192/nTerrainTileSize)
CSecretPicking::CSecretPicking() : MAXNUM_INTERSECTEDTRIANGLE(1024)
{
	m_pEnvironment = NULL ;
	m_psIntersectedTriangles = NULL ;
	m_psTestedTriangle = NULL ;
}

CSecretPicking::~CSecretPicking()
{
	Release() ;
}

void CSecretPicking::Release()
{
	SAFE_DELETEARRAY(m_psIntersectedTriangles) ;
	SAFE_DELETEARRAY(m_psTestedTriangle) ;
}

bool CSecretPicking::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CD3DEnvironment *pEnvironment)
{
    m_pEnvironment = pEnvironment ;
	m_pd3dDevice = pd3dDevice ;

	m_psIntersectedTriangles = new geo::STriangle[MAXNUM_INTERSECTEDTRIANGLE] ;
	m_psTestedTriangle = new geo::STriangle[MAXNUM_INTERSECTEDTRIANGLE*2] ;

	return true ;
}

void CSecretPicking::SetLine(Vector2 *pvPos, Matrix4 &matV, Matrix4 &matP)
{
	//static bool first=false ;

	//if(!first)
	//	first = true ;
	//else
	//	return ;

	m_vScreenPos = *pvPos ;
    
	Vector2 vPos ;
	Vector3 v1, v2, v3 ;

	vPos.x = (pvPos->x*2/m_pEnvironment->m_lScreenWidth) - 1 ;
	vPos.y = ((m_pEnvironment->m_lScreenHeight-pvPos->y)*2/m_pEnvironment->m_lScreenHeight) - 1 ;

	v1.set(vPos.x, vPos.y, 0) ;
	v2.set(vPos.x, vPos.y, 1) ;

	Matrix4 matInvVP = (matV*matP).Inverse() ;
    v1 *= matInvVP ;
	v2 *= matInvVP ;

	v1.x = float_round(v1.x) ;
	v1.y = float_round(v1.y) ;
	v1.z = float_round(v1.z) ;

	v2.x = float_round(v2.x) ;
	v2.y = float_round(v2.y) ;
	v2.z = float_round(v2.z) ;

	v3 = v2-v1 ;

	m_sLine.set(v1, v3.Normalize(), v3.Magnitude()) ;
	m_bIntersect = false ;
}

bool CSecretPicking::IntersectedQuadTreeTile(geo::SAACube *psCube, TERRAINVERTEX *pVertices, TERRAININDEX *pIndices, int nTriangles)
{
	if(IntersectLineToCube(&m_sLine, psCube))
	{
		int i ;
		geo::STriangle tri ;
		TERRAINVERTEX *pVertex1, *pVertex2, *pVertex3 ;
		Vector3 v ;

		for(i=0 ; i<nTriangles ; i++)
		{
			pVertex1 = &pVertices[pIndices[i].i1] ;
			pVertex2 = &pVertices[pIndices[i].i2] ;
			pVertex3 = &pVertices[pIndices[i].i3] ;

			tri.avVertex[0].set(Vector3(pVertex1->pos.x, pVertex1->pos.y, pVertex1->pos.z),
								Vector3(pVertex1->normal.x, pVertex1->normal.y, pVertex1->normal.z),
								Vector2(pVertex1->t.x, pVertex1->t.y)) ;

			tri.avVertex[1].set(Vector3(pVertex2->pos.x, pVertex2->pos.y, pVertex2->pos.z),
								Vector3(pVertex2->normal.x, pVertex2->normal.y, pVertex2->normal.z),
								Vector2(pVertex2->t.x, pVertex2->t.y)) ;

			tri.avVertex[2].set(Vector3(pVertex3->pos.x, pVertex3->pos.y, pVertex3->pos.z),
								Vector3(pVertex3->normal.x, pVertex3->normal.y, pVertex3->normal.z),
								Vector2(pVertex3->t.x, pVertex3->t.y)) ;

			tri.sPlane.MakePlane(tri.avVertex[0].vPos, tri.avVertex[1].vPos, tri.avVertex[2].vPos) ;
								

			if(IntersectLinetoTriangle(m_sLine, tri, v) == geo::INTERSECT_POINT)
			{
				m_sPrevVertex = m_sCurVertex ;
				m_sCurVertex.pos = (tri.avVertex[0].vPos*v.x) + (tri.avVertex[1].vPos*v.y) + (tri.avVertex[2].vPos*v.z) ;
				m_sCurVertex.normal = ((tri.avVertex[0].vNormal*v.x) + (tri.avVertex[1].vNormal*v.y) + (tri.avVertex[2].vNormal*v.z)).Normalize() ;
				m_sCurVertex.tex = (tri.avVertex[0].vTex*v.x) + (tri.avVertex[1].vTex*v.y) + (tri.avVertex[2].vTex*v.z) ;
				m_bIntersect = true ;
				return true ;
			}
		}
	}
	return false ;
}

bool CSecretPicking::IntersectedQuadTreeTile(geo::SAACube *psCube, int nNumLevel, int nPatchType, int tl, float fBestHeight, CSecretTileSubTree *pcSubTree)
{
	if(IntersectLineToSphere(&m_sLine, psCube->vCenter, psCube->fWidth))
	//if(IntersectLineToCube(&m_sLine, psCube))
	{
		int nTriangles = pcSubTree->GetIntersectLine(&m_sLine, m_pcTerrain->m_sHeightMap.pVertices, m_psIntersectedTriangles, 0, nNumLevel, nPatchType, tl, fBestHeight) ;
		assert(nTriangles <= MAXNUM_INTERSECTEDTRIANGLE) ;
		Vector3 v ;
		geo::STriangle *psTriangles = m_psIntersectedTriangles ;

		for(int i=0 ; i<nTriangles ; i++, psTriangles++)
		{
			if(IntersectLinetoTriangle(m_sLine, *psTriangles, v) == geo::INTERSECT_POINT)
			{
				m_sPrevVertex = m_sCurVertex ;
				m_sCurVertex.pos = (psTriangles->avVertex[0].vPos*v.x) + (psTriangles->avVertex[1].vPos*v.y) + (psTriangles->avVertex[2].vPos*v.z) ;
				m_sCurVertex.normal = ((psTriangles->avVertex[0].vNormal*v.x) + (psTriangles->avVertex[1].vNormal*v.y) + (psTriangles->avVertex[2].vNormal*v.z)).Normalize() ;
				m_sCurVertex.tex = (psTriangles->avVertex[0].vTex*v.x) + (psTriangles->avVertex[1].vTex*v.y) + (psTriangles->avVertex[2].vTex*v.z) ;
				m_bIntersect = true ;
				return true ;
			}
		}
	}
	return false ;
}

//bool CSecretPicking::IsPickingPoint(geo::STriangle &psTriangle)
//{
//	geo::SLine sLine = m_sLine ;
//	sLine.s.y = sLine.v.y = 0 ;
//	int nTriangles = m_pcTerrain->m_pcZQuadTree->GetIntersectedTriangle(&m_sLine, m_pcTerrain->m_sHeightMap.pVertices, m_ppsIntersectedTriangles) ;
//
//    int i ;
//	Vector3 v ;
//	for(i=0 ; i<nTriangles ; i++)
//	{
//		if(IntersectLinetoTriangle(m_sLine, *m_ppsIntersectedTriangles[i], v) == geo::INTERSECT_POINT)
//		{
//			psTriangle = *m_ppsIntersectedTriangles[i] ;
//
//			m_sPrevVertex = m_sCurVertex ;
//			m_sCurVertex.pos = (psTriangle.avVertex[0].vPos*v.x) + (psTriangle.avVertex[1].vPos*v.y) + (psTriangle.avVertex[2].vPos*v.z) ;
//			m_sCurVertex.normal = ((psTriangle.avVertex[0].vNormal*v.x) + (psTriangle.avVertex[1].vNormal*v.y) + (psTriangle.avVertex[2].vNormal*v.z)).Normalize() ;
//			m_sCurVertex.tex = (psTriangle.avVertex[0].vTex*v.x) + (psTriangle.avVertex[1].vTex*v.y) + (psTriangle.avVertex[2].vTex*v.z) ;
//			return true ;
//		}
//	}
//    return false ;
//}

void CSecretPicking::Render()
{
	typedef struct
	{
		D3DXVECTOR3 pos ;
		unsigned long color ;
	} PICKING_VERTEX ;

	unsigned short index[2] ;
	PICKING_VERTEX vt[2] ;
	Vector3 v ;

	vt[0].pos = D3DXVECTOR3(m_sLine.s.x, m_sLine.s.y, m_sLine.s.z) ;
	vt[0].color = 0xffff0000 ;

	v = m_sLine.GetEndPos() ;
	vt[1].pos = D3DXVECTOR3(v.x, v.y, v.z) ;
	vt[1].color = 0xffff0000 ;

    index[0] = 0 ;
	index[1] = 1 ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

    m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(PICKING_VERTEX)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 2, 1, index, D3DFMT_INDEX16, vt, sizeof(vt[0])) ;

//#ifdef _DEBUG
//    //draw tested triangles
//	static int c ;
//	static PICKING_VERTEX tested_vt[4096] ;
//
//	c=0 ;
//	//for(int i=0 ; i<m_nNumTestedTriangle ; i++)
//	if(m_nNumTestedTriangle > 0)
//	{
//		int i ;
//		for(i=0 ; i<m_nNumTestedTriangle ; i++)
//		{
//			memcpy(&tested_vt[c].pos, &m_psTestedTriangle[i].avVertex[0].vPos, sizeof(Vector3)) ;
//			tested_vt[c].color = 0xffff0000 ;
//			c++ ;
//
//			memcpy(&tested_vt[c].pos, &m_psTestedTriangle[i].avVertex[1].vPos, sizeof(Vector3)) ;
//			tested_vt[c].color = 0xffff0000 ;
//			c++ ;
//
//			memcpy(&tested_vt[c].pos, &m_psTestedTriangle[i].avVertex[1].vPos, sizeof(Vector3)) ;
//			tested_vt[c].color = 0xffff0000 ;
//			c++ ;
//
//			memcpy(&tested_vt[c].pos, &m_psTestedTriangle[i].avVertex[2].vPos, sizeof(Vector3)) ;
//			tested_vt[c].color = 0xffff0000 ;
//			c++ ;
//
//			memcpy(&tested_vt[c].pos, &m_psTestedTriangle[i].avVertex[2].vPos, sizeof(Vector3)) ;
//			tested_vt[c].color = 0xffff0000 ;
//			c++ ;
//
//			memcpy(&tested_vt[c].pos, &m_psTestedTriangle[i].avVertex[0].vPos, sizeof(Vector3)) ;
//			tested_vt[c].color = 0xffff0000 ;
//			c++ ;
//		}
//
//		DWORD dwZEnable ;
//		m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
//        
//		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;
//		m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, i*3, tested_vt, sizeof(PICKING_VERTEX)) ;
//
//		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;
//	}
//#endif //_DEBUG

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;
}

void CSecretPicking::ResetTilesMustbeTested()
{
	STerrainTileDataMustbeTested *psTiles=m_asTilesMustbeTested ;
	for(int i=0 ; i<256 ; i++, psTiles++)
	{
		psTiles->nNumIndex = 0 ;
		psTiles->pcTileSubTree = NULL ;
		psTiles->psIndices = NULL ;
	}
	m_nNumTiles = 0 ;
	m_nNumTestedTriangle = 0 ;
}

void CSecretPicking::InsertTilesMustbeTested(TERRAININDEX *psIndices, int nNum, Vector3 *pvCenter)
{
	STerrainTileDataMustbeTested *psTiles = &m_asTilesMustbeTested[m_nNumTiles++] ;
	psTiles->nNumIndex = nNum ;
	psTiles->psIndices = psIndices ;
	psTiles->vCenter = (*pvCenter) ;
}
void CSecretPicking::InsertTilesMustbeTested(CSecretQuadTree *pcQuadTree, CSecretTileSubTree *pcTileSubTree, int nNumLevel, int nPatchType, int tl, float fBestHeight, Vector3 *pvCenter)
{
	STerrainTileDataMustbeTested *psTiles = &m_asTilesMustbeTested[m_nNumTiles++] ;
	psTiles->nNumLevel = nNumLevel ;
	psTiles->nPatchType = nPatchType ;
	psTiles->tl = tl ;
	psTiles->fBestHeight = fBestHeight ;
	psTiles->pcQuadTree = pcQuadTree ;
	psTiles->pcTileSubTree = pcTileSubTree ;
	psTiles->vCenter = (*pvCenter) ;
}

//increasing order 오름차순
int CompareSubTiles(const void *pv1, const void *pv2)
{
	STerrainTileDataMustbeTested *pTile1 = (STerrainTileDataMustbeTested *)pv1 ;
	STerrainTileDataMustbeTested *pTile2 = (STerrainTileDataMustbeTested *)pv2 ;

	float dist = pTile1->fDistfromCamera - pTile2->fDistfromCamera ;
    if(float_less(dist, 0.0f))//타일1이 타일2보다 작은경우
		return -1 ;
	else if(float_greater(dist, 0.0f))//타일1이 타일2보다 큰경우
		return 1 ;
	else
		return 0 ;
}

bool CSecretPicking::IntersectTiles(TERRAINVERTEX *pVertices)
{
	if(m_nNumTiles == 0)
		return false ;

	int n ;
	STerrainTileDataMustbeTested *psTiles = m_asTilesMustbeTested ;

	for(n=0 ; n<m_nNumTiles ; n++, psTiles++)
		psTiles->fDistfromCamera = (psTiles->vCenter-m_sLine.s).Magnitude() ;

	qsort(m_asTilesMustbeTested, m_nNumTiles, sizeof(STerrainTileDataMustbeTested), CompareSubTiles) ;

	psTiles = m_asTilesMustbeTested ;
	for(n=0 ; n<m_nNumTiles ; n++, psTiles++)
	{
		if(psTiles->nNumIndex)//레벨이 0 이나 1 인 경우의 타일을 검색
		{
			int i ;
			geo::STriangle tri ;
			TERRAINVERTEX *pVertex1, *pVertex2, *pVertex3 ;
			Vector3 v ;

			for(i=0 ; i<psTiles->nNumIndex ; i++)
			{
				pVertex1 = &pVertices[psTiles->psIndices[i].i1] ;
				pVertex2 = &pVertices[psTiles->psIndices[i].i2] ;
				pVertex3 = &pVertices[psTiles->psIndices[i].i3] ;

				tri.avVertex[0].set(Vector3(pVertex1->pos.x, pVertex1->pos.y, pVertex1->pos.z),
					Vector3(pVertex1->normal.x, pVertex1->normal.y, pVertex1->normal.z),
					Vector2(pVertex1->t.x, pVertex1->t.y)) ;

				tri.avVertex[1].set(Vector3(pVertex2->pos.x, pVertex2->pos.y, pVertex2->pos.z),
					Vector3(pVertex2->normal.x, pVertex2->normal.y, pVertex2->normal.z),
					Vector2(pVertex2->t.x, pVertex2->t.y)) ;

				tri.avVertex[2].set(Vector3(pVertex3->pos.x, pVertex3->pos.y, pVertex3->pos.z),
					Vector3(pVertex3->normal.x, pVertex3->normal.y, pVertex3->normal.z),
					Vector2(pVertex3->t.x, pVertex3->t.y)) ;

				tri.sPlane.MakePlane(tri.avVertex[0].vPos, tri.avVertex[1].vPos, tri.avVertex[2].vPos) ;

                m_psTestedTriangle[m_nNumTestedTriangle] = tri ;
                m_nNumTestedTriangle++ ;

				if(IntersectLinetoTriangle(m_sLine, tri, v) == geo::INTERSECT_POINT)
				{
					m_sPrevVertex = m_sCurVertex ;
					m_sCurVertex.pos = (tri.avVertex[0].vPos*v.x) + (tri.avVertex[1].vPos*v.y) + (tri.avVertex[2].vPos*v.z) ;
					m_sCurVertex.normal = ((tri.avVertex[0].vNormal*v.x) + (tri.avVertex[1].vNormal*v.y) + (tri.avVertex[2].vNormal*v.z)).Normalize() ;
					m_sCurVertex.tex = (tri.avVertex[0].vTex*v.x) + (tri.avVertex[1].vTex*v.y) + (tri.avVertex[2].vTex*v.z) ;
					m_bIntersect = true ;
					return true ;
				}
			}
		}
		else if(psTiles->pcTileSubTree)//1보다 큰 레벨의 타일을 검색
		{
			int nTriangles = psTiles->pcTileSubTree->GetIntersectLine(&m_sLine, pVertices, m_psIntersectedTriangles, 0, psTiles->nNumLevel, psTiles->nPatchType, psTiles->tl, psTiles->fBestHeight) ;

			if(bIntersectTri_TileSubTree)
			{
				geo::STriangle *psTriangles = m_psIntersectedTriangles ;
				Vector3 v = vIntersectTriPos_TileSubTree ;
				m_sPrevVertex = m_sCurVertex ;
				m_sCurVertex.pos = (psTriangles->avVertex[0].vPos*v.x) + (psTriangles->avVertex[1].vPos*v.y) + (psTriangles->avVertex[2].vPos*v.z) ;
				m_sCurVertex.normal = ((psTriangles->avVertex[0].vNormal*v.x) + (psTriangles->avVertex[1].vNormal*v.y) + (psTriangles->avVertex[2].vNormal*v.z)).Normalize() ;
				m_sCurVertex.tex = (psTriangles->avVertex[0].vTex*v.x) + (psTriangles->avVertex[1].vTex*v.y) + (psTriangles->avVertex[2].vTex*v.z) ;
				m_bIntersect = true ;
				return true ;
			}

			//assert(nTriangles <= MAXNUM_INTERSECTEDTRIANGLE) ;
			//Vector3 v ;
			//geo::STriangle *psTriangles = m_psIntersectedTriangles ;

			//for(int i=0 ; i<nTriangles ; i++, psTriangles++)
			//{
			//	m_psTestedTriangle[m_nNumTestedTriangle] = *psTriangles ;
			//	m_nNumTestedTriangle++ ;
			//	if(IntersectLinetoTriangle(m_sLine, *psTriangles, v) == geo::INTERSECT_POINT)
			//	{
			//		m_sPrevVertex = m_sCurVertex ;
			//		m_sCurVertex.pos = (psTriangles->avVertex[0].vPos*v.x) + (psTriangles->avVertex[1].vPos*v.y) + (psTriangles->avVertex[2].vPos*v.z) ;
			//		m_sCurVertex.normal = ((psTriangles->avVertex[0].vNormal*v.x) + (psTriangles->avVertex[1].vNormal*v.y) + (psTriangles->avVertex[2].vNormal*v.z)).Normalize() ;
			//		m_sCurVertex.tex = (psTriangles->avVertex[0].vTex*v.x) + (psTriangles->avVertex[1].vTex*v.y) + (psTriangles->avVertex[2].vTex*v.z) ;
			//		m_bIntersect = true ;
			//		return true ;
			//	}
			//}
		}
	}
	return false ;
}

/////////////////////////////////////////////////////////////////////
// CSecretBrushTool
/////////////////////////////////////////////////////////////////////
#include "SecretTerrain.h"

CSecretBrushTool::CSecretBrushTool()
{
	m_pd3dDevice = NULL ;
	m_pcPicking = NULL ;
	m_pLine = NULL ;
	m_pvBoundaryPos1 = NULL ;
	m_pvBoundaryPos2 = NULL ;
	m_fRadius = 0.5f ;
}
CSecretBrushTool::~CSecretBrushTool()
{
	Release() ;
}
void CSecretBrushTool::Release()
{
	SAFE_RELEASE(m_pLine) ;
	SAFE_DELETEARRAY(m_pvBoundaryPos1) ;
	SAFE_DELETEARRAY(m_pvBoundaryPos2) ;
}
void CSecretBrushTool::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretPicking *pcPicking)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcPicking = pcPicking ;

	m_nNumBoundary = 16 ;

	m_pvBoundaryPos1 = new Vector3[m_nNumBoundary] ;
	m_pvBoundaryPos2 = new D3DXVECTOR3[m_nNumBoundary+1] ;
	D3DXCreateLine(m_pd3dDevice, &m_pLine) ;
	m_pLine->SetWidth(3) ;
	m_pLine->SetAntialias(TRUE) ;
}
void CSecretBrushTool::Process(Matrix4 *pmatView, D3DXMATRIX *pmatProj)
{
	m_pmatView = pmatView ;
    m_pmatProj = pmatProj ;

	static bool bFirst=false ;
	if(m_pcPicking->m_bIntersect)
	{
		m_vtPrevPicking = m_vtCurPicking ;
		memcpy(&m_vtCurPicking, m_pcPicking->GetCurVertex(), sizeof(SPickingVertex)) ;

		if(!bFirst)
		{
			m_vtPrevPicking = m_vtCurPicking ;
			bFirst = true ;
		}
		m_vCenter = m_vtCurPicking.pos ;
	}

	int i ;
	float add, theta ;
	add = deg_rad(360.0f/m_nNumBoundary) ;
	theta = 0 ;
	Vector3 v ;

	for(i=0 ; i<m_nNumBoundary ; i++, theta += add)
	{
		v.x = m_vCenter.x + cosf(theta)*m_fRadius ;
		v.z = m_vCenter.z + sinf(theta)*m_fRadius ;
		v.y = m_pcTerrain->m_sHeightMap.GetHeightMap(v.x, v.z) ;

		v  *= (*m_pmatView) ;
		m_pvBoundaryPos2[i] = D3DXVECTOR3(v.x, v.y, v.z) ;
	}
}
void CSecretBrushTool::Render()
{
	int i ;
	static D3DXVECTOR3 avPos[33] ;

	for(i=0 ; i<m_nNumBoundary ; i++)
		avPos[i] = m_pvBoundaryPos2[i] ;

	avPos[i] = avPos[0] ;

	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;

	m_pLine->Begin() ;
	m_pLine->DrawTransform(avPos, m_nNumBoundary+1, m_pmatProj, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)) ;
	m_pLine->End() ;

	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) ;
}