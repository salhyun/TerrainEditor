#include "SecretTilePatchs.h"
#include "MathOrdinary.h"
#include "SecretTerrain.h"
#include "SecretToolBrush.h"
#include "SecretSceneManager.h"
#include "SecretMeshObject.h"
#include "SecretTerrainModifier.h"
#include "SecretSod.h"

float g_fQuadTreeLODRatio = QUADTREE_LODRATIO ;

int nMaxDepthLevel ;
int nMapPitch ;

DWORD dwTotalSize=0 ;
CSecretTile *pcTile=NULL ;
CSecretTileSubTree *pcTileSubTree=NULL ;
int nTerrainTileSize=1 ;

//#######################################################################//
//                        CSecretQuadTree                                //
//#######################################################################//

CSecretQuadTree::CSecretQuadTree(CSecretQuadTree *pcParent)
{
	int i ;
	for(i=0 ; i<4 ; i++)
	{
		m_pcChild[i] = NULL ;
		m_pcNeighbor[i] = NULL ;
	}
	m_pcParent = pcParent ;
	m_nDetailLevel = 0 ;
	m_bLastDepthLevel = false ;
	m_bCulled = FALSE;
	m_fRadius = 0.0f;
	//m_sTRObjects.resize(256) ;

	if(pcParent)
		m_nTreeLevel = pcParent->m_nTreeLevel+1 ;
	else
        m_nTreeLevel = 0 ;
}

CSecretQuadTree::CSecretQuadTree(int cx, int cy)
{
	int i ;
	for(i=0 ; i<4 ; i++)
	{
		m_pcChild[i] = NULL ;
		m_pcNeighbor[i] = NULL ;
	}
	m_pcParent = NULL ;
	m_nDetailLevel = 0 ;

	m_nCorner[CORNER_TL]	= 0;
	m_nCorner[CORNER_TR]	= cx - 1;
	m_nCorner[CORNER_BL]	= cx * ( cy - 1 );
	m_nCorner[CORNER_BR]	= cx * cy - 1;
	m_nCenter				= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] + 
								m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 4;

	m_vCenter.set(0, 0, 0) ;

	nMapPitch = m_nCorner[CORNER_TR] ;
	m_bLastDepthLevel = false ;
	m_bCulled = FALSE;
	m_fRadius = 0.0f;
	//m_sTRObjects.resize(256) ;
}

CSecretQuadTree::~CSecretQuadTree()
{
	_Destroy() ;
}

// 메모리에서 쿼드트리를 삭제한다.
void	CSecretQuadTree::_Destroy()
{
	// 자식 노드들을 소멸 시킨다.
	for( int i = 0 ; i < 4 ; i++ )
	{
		if(m_pcChild[i])
			delete m_pcChild[i] ;
	}
}

bool CSecretQuadTree::_SetCorners( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR )
{
	m_nCorner[CORNER_TL] = nCornerTL;
	m_nCorner[CORNER_TR] = nCornerTR;
	m_nCorner[CORNER_BL] = nCornerBL;
	m_nCorner[CORNER_BR] = nCornerBR;
	m_nCenter			 = ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] + 
							 m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 4;
	return true ;
}

CSecretQuadTree *CSecretQuadTree::_AddChild(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR)
{
    CSecretQuadTree *pcChild = new CSecretQuadTree(this) ;
	pcChild->_SetCorners( nCornerTL, nCornerTR, nCornerBL, nCornerBR );
	return pcChild ;
}

bool CSecretQuadTree::_SubDivide(TERRAINVERTEX *pVertices)
{
	int		nTopEdgeCenter;
	int		nBottomEdgeCenter;
	int		nLeftEdgeCenter;
	int		nRightEdgeCenter;
	int		nCentralPoint;

	// 상단변 가운데
	nTopEdgeCenter		= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] ) / 2;
	// 하단변 가운데 
	nBottomEdgeCenter	= ( m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 2;
	// 좌측변 가운데
	nLeftEdgeCenter		= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_BL] ) / 2;
	// 우측변 가운데
	nRightEdgeCenter	= ( m_nCorner[CORNER_TR] + m_nCorner[CORNER_BR] ) / 2;
	// 한가운데
	nCentralPoint		= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] + 
							m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 4;

	// 더이상 분할이 불가능한가? 그렇다면 SubDivide() 종료
	if( (m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL]) <= QUADTREE_MINSIZE )
	{
		m_bLastDepthLevel = true ;

		//현재 구역에서 가장 높은 y값을 구해낸다.
        int nPos = m_nCorner[CORNER_TL] ;
		float fMaxHeight, fMinHeight, fHeight ;
		fMaxHeight = fMinHeight = pVertices[nPos].pos.y ;
		for(int y=0 ; y<((m_nCorner[CORNER_BL]-m_nCorner[CORNER_TL])/nMapPitch) ; y++, nPos += (nMapPitch-QUADTREE_MINSIZE))
		{
			for(int x=0 ; x<QUADTREE_MINSIZE ; x++, nPos++)
			{
				if(fMaxHeight < pVertices[nPos].pos.y)
					fMaxHeight = pVertices[nPos].pos.y ;
				if(fMinHeight > pVertices[nPos].pos.y)
					fMinHeight = pVertices[nPos].pos.y ;
			}
		}
		fMinHeight -= (fMaxHeight-fMinHeight)/2.0f ;
		fMaxHeight += (fMaxHeight-fMinHeight)/2.0f ;

		m_fBestHeight = fMaxHeight ;

        fHeight = fMaxHeight-fMinHeight ;
        if(float_less_eq(fHeight, 1.0f))
			fHeight = 1.0f ;

		m_vCenter = VectorConvert((pVertices+m_nCenter)->pos) ;
		m_vCenter.y = (fMinHeight+fMaxHeight)/2.0f ;

		float width = pVertices[m_nCorner[CORNER_TR]].pos.x - pVertices[m_nCorner[CORNER_TL]].pos.x ;
		float depth = pVertices[m_nCorner[CORNER_TL]].pos.z - pVertices[m_nCorner[CORNER_BL]].pos.z ;

		m_sCube.set(m_vCenter, width, fHeight, depth) ;
        //Vector3 vPos = VectorConvert((pVertices+m_nCenter)->pos) ;
		//m_sCube.set(vPos, QUADTREE_MINSIZE, height*2.0f, QUADTREE_MINSIZE) ;

		//float l = (QUADTREE_MINSIZE/2) ;
		//l = l*l ;
		//m_fRadius = sqrtf(l+l) ;
		float l = (pVertices[m_nCenter].pos.x-pVertices[m_nCorner[CORNER_TL]].pos.x) ;
		l = l*l ;
		m_fRadius = sqrtf(l+l) ;

		return false ;
	}

	// 4개의 자식노드 추가
	m_pcChild[CORNER_TL] = _AddChild( m_nCorner[CORNER_TL], nTopEdgeCenter, nLeftEdgeCenter, nCentralPoint );
	m_pcChild[CORNER_TR] = _AddChild( nTopEdgeCenter, m_nCorner[CORNER_TR], nCentralPoint, nRightEdgeCenter );
	m_pcChild[CORNER_BL] = _AddChild( nLeftEdgeCenter, nCentralPoint, m_nCorner[CORNER_BL], nBottomEdgeCenter );
	m_pcChild[CORNER_BR] = _AddChild( nCentralPoint, nRightEdgeCenter, nBottomEdgeCenter, m_nCorner[CORNER_BR] );
	
	return true ;
}

bool CSecretQuadTree::_Build(TERRAINVERTEX *pVertices, float fMaxHeight, float fMinHeight)
{
	if( _SubDivide(pVertices) )
	{
		// 좌측상단과, 우측 하단의 거리를 구한다.
		D3DXVECTOR3 v = *((D3DXVECTOR3*)(pVertices+m_nCorner[CORNER_TL])) - *((D3DXVECTOR3*)(pVertices+m_nCorner[CORNER_BR]));
		// v의 거리값이 이 노드를 감싸는 경계구의 지름이므로, 
		// 2로 나누어 반지름을 구한다.
		m_fRadius	  = D3DXVec3Length( &v ) / 2.0f;

		float l = (pVertices[m_nCenter].pos.x-pVertices[m_nCorner[CORNER_TL]].pos.x) ;
		l = l*l ;
		m_fRadius = sqrtf(l+l) ;

		//_SetCube(pVertices, fBestHeight) ;
		_SetCube(pVertices, (fMaxHeight-fMinHeight), (fMaxHeight+fMinHeight)/2.0f) ;

		m_pcChild[CORNER_TL]->_Build( pVertices, fMaxHeight, fMinHeight );
		m_pcChild[CORNER_TR]->_Build( pVertices, fMaxHeight, fMinHeight );
		m_pcChild[CORNER_BL]->_Build( pVertices, fMaxHeight, fMinHeight );
		m_pcChild[CORNER_BR]->_Build( pVertices, fMaxHeight, fMinHeight );
	}

	return true ;
}

bool CSecretQuadTree::Build(TERRAINVERTEX *pVertices, float fMaxHeight, float fMinHeight, int nTileSize)
{
	nMaxDepthLevel = (int)(log((float)QUADTREE_MINSIZE)/log(2.0f))+1 ;
	nTerrainTileSize = nTileSize ;

	pcTile = new CSecretTile[nMaxDepthLevel] ;

	int tl, tr, bl, br ;
	tl = 0 ;
	tr = QUADTREE_MINSIZE ;
	bl = (QUADTREE_MINSIZE)*(nMapPitch+1) ;
	br = bl+QUADTREE_MINSIZE ;

	pcTile[0].GernerateZeroLevelIndices(tl, tr, bl, br) ;
	pcTile[1].GernerateOneLevelIndices(tl, tr, bl, br) ;
	for(int i=2 ; i<nMaxDepthLevel ; i++)
		pcTile[i].DivideTile(tl, tr, bl, br, i-1) ;

	//쿼드트리가 전체가 이 하나의 TileSubTree를 사용한다. 그래서 요기에서 생성하고
	//최하위 노드의 트리에서는 타일패치타입에 따라서 TileSubTree의 인덱스를 받아와서 자신의 위치값을 더해주기만 하면 된다.
	pcTileSubTree = new CSecretTileSubTree(tl, tr, bl, br) ;
	pcTileSubTree->Build(pVertices, fMaxHeight, pcTile, nMaxDepthLevel) ;

	_SetCube(pVertices, (fMaxHeight-fMinHeight), (fMaxHeight+fMinHeight)/2.0f) ;

	_Build(pVertices, fMaxHeight, fMinHeight) ;
	_BuildNeighborNode(this, pVertices, m_nCorner[CORNER_TR]+1, (m_nCorner[CORNER_BR]+1)/(m_nCorner[CORNER_TR]+1)) ;
	return true ;
}

void CSecretQuadTree::DeleteTiles()
{
	SAFE_DELETE(pcTileSubTree) ;
	SAFE_DELETEARRAY(pcTile) ;
}

void CSecretQuadTree::_SetCube(TERRAINVERTEX *pVertices, float height, float fMidHeight)
{
	m_vCenter.set(pVertices[m_nCenter].pos.x, pVertices[m_nCenter].pos.y, pVertices[m_nCenter].pos.z) ;
	m_vCenter.y = fMidHeight ;
	//Vector3 vCenter(pVertices[m_nCenter].pos.x, pVertices[m_nCenter].pos.y, pVertices[m_nCenter].pos.z) ;
	//float size = fabs(pVertices[m_nCorner[CORNER_TR]].pos.x-pVertices[m_nCorner[CORNER_TL]].pos.x) ;

	float width = pVertices[m_nCorner[CORNER_TR]].pos.x - pVertices[m_nCorner[CORNER_TL]].pos.x ;
	float depth = pVertices[m_nCorner[CORNER_TL]].pos.z - pVertices[m_nCorner[CORNER_BL]].pos.z ;
	m_sCube.set(m_vCenter, width, height, depth) ;
    //m_sCube.set(vCenter, size, height*2.0f, size) ;
}

int CSecretQuadTree::_IsInFrustum(TERRAINVERTEX *pVertices, CSecretFrustum *pcFrustum)
{
	int i ;
	bool bIn[4] ;
	D3DXVECTOR3 vPos ;

	vPos = pVertices[m_nCenter].pos ;
	if(!pcFrustum->IsInSphere(vPos, m_fRadius))
		return FRUSTUM_OUT ;

	for(i=0 ; i<4 ; i++)
	{
		vPos = pVertices[m_nCorner[i]].pos ;
		bIn[i] = pcFrustum->IsIn(vPos) ;
	}

	if( (bIn[0]+bIn[1]+bIn[2]+bIn[3]) == 4 )
		return FRUSTUM_COMPLETELY_IN ;

    return FRUSTUM_PARTIALLY_IN ;
}

//int CSecretQuadTree::FrustumCull(TERRAINVERTEX *pVertices, CSecretFrustum *pcFrustum, Vector3 *pvCamera, TERRAININDEX *pIndices, int nNumIndex, std::vector<STrueRenderingObject *> *psTRObjects)
int CSecretQuadTree::FrustumCull(TERRAINVERTEX *pVertices, CSecretFrustum *pcFrustum, Vector3 *pvCamera, TERRAININDEX *pIndices, int nNumIndex, data::SKeepingArray<TERRAININDEX> *psExceptionalIndices)
{
	int nResult = _IsInFrustum(pVertices, pcFrustum) ;

	if(nResult == FRUSTUM_OUT)//완전 벗어나면 바로 리턴
	{
		m_bCulled = true ;
		return nNumIndex ;
	}
	else
	{
		m_bCulled = false ;
	}

	m_bIntersectPos = false ;
	m_bIntersectLine = false ;

	g_cDebugMsg.SetDebugMsg(7, "TotalSize=%d", dwTotalSize/1048576) ;

	CSecretTerrainModifier *pcTerrainModifier = pcFrustum->m_pcTerrainModifier ;

	//if(m_pcTile)//트리 가장 마지막부분 타일의 인덱스를 가지고 있는 부분
	if(m_bLastDepthLevel)
	{
		int nLevel = _GetLODLevel(pvCamera, pVertices) ;

        int i ;
		DWORD dwPatchType=0 ;
		for(i=0 ; i<4 ; i++)
		{
			if(m_pcNeighbor[i])
			{
				if(!m_pcNeighbor[i]->_IsVisible(nLevel, pvCamera, pVertices))
					dwPatchType |= (0x01<<i) ;
			}
		}
		int nNum = pcTile[nLevel].GetIndices(dwPatchType, &pIndices[nNumIndex], m_nCorner[CORNER_TL], psExceptionalIndices) ;
		nNumIndex += nNum ;

		if(pcFrustum->m_pcToolBrush)
		{
			//픽킹 라인과 지형과 교차
			//if(!pcFrustum->m_pcToolBrush->m_cPicking.m_bIntersect)// && m_pcParent->m_bIntersectLine)
			if(m_pcParent->m_bIntersectLine)
			{
				if(nLevel < 2)
				{
					//if(pcFrustum->m_pcToolBrush->m_cPicking.IntersectedQuadTreeTile(&m_sCube, pVertices, &pIndices[nNumIndex], nNum))
					//	m_bIntersectLine = true ;

					Vector3 vPos(pVertices[m_nCenter].pos.x, pVertices[m_nCenter].pos.y, pVertices[m_nCenter].pos.z) ;
					if(IntersectLineToSphere(&pcFrustum->m_pcToolBrush->m_pcPicking->m_sLine, vPos, m_fRadius))
					{
						pcFrustum->m_pcToolBrush->m_pcPicking->InsertTilesMustbeTested(&pIndices[nNumIndex], nNum, &vPos) ;
						m_bIntersectLine = true ;
					}
				}
				else
				{
					//if(pcFrustum->m_pcToolBrush->m_cPicking.IntersectedQuadTreeTile(&m_sCube, nLevel, dwPatchType, m_nCorner[CORNER_TL], m_fBestHeight, pcTileSubTree))
					//	m_bIntersectLine = true ;

					Vector3 vPos(pVertices[m_nCenter].pos.x, pVertices[m_nCenter].pos.y, pVertices[m_nCenter].pos.z) ;
					//if(IntersectLineToSphere(&pcFrustum->m_pcToolBrush->m_pcPicking->m_sLine, vPos, m_fRadius))
					if(IntersectLineToCube(&pcFrustum->m_pcToolBrush->m_pcPicking->m_sLine, &m_sCube))
					{
						pcFrustum->m_pcToolBrush->m_pcPicking->InsertTilesMustbeTested(this, pcTileSubTree, nLevel, dwPatchType, m_nCorner[CORNER_TL], m_fBestHeight, &vPos) ;
						m_bIntersectLine = true ;
					}
				}
			}
			else
				m_bIntersectLine = false ;
			//툴브러쉬 지형과 교차
			//if(!pcFrustum->m_pcToolBrush->m_bIntersectedAll && pcFrustum->m_pcToolBrush->IsVisible())
			//{
			//	if(nLevel < 2)
			//	{
			//		if(pcFrustum->m_pcToolBrush->IntersectedQuadTreeTile(m_vCenter, m_fRadius, pVertices, &pIndices[nNumIndex], nNum))
			//			m_bIntersectPos = true ;
			//	}
			//	else
			//	{
			//		if(pcFrustum->m_pcToolBrush->IntersectedQuadTreeTile(m_vCenter, m_fRadius, nLevel, dwPatchType, m_nCorner[CORNER_TL], pcTileSubTree))
			//			m_bIntersectPos = true ;
			//	}
			//}
			//else
			//	m_bIntersectPos = false ;

			//필요한것
			//현재타일이 범위안에 들어갔다면 타일서브트리(TileSubTree)로 검색해서 삼각형들을 찾아낸다.
			//그리고 그렇게 모아놓은 삼각형을 가지고 삼각형교차검사를 해서 점들을 뽑아낸다.

			//if(psTRObjects)//현재타일위에 있는 오브젝트를 검색한다.
			//{
			//	int bSame=false ;
			//	std::list<STrueRenderingObject *>::iterator it_tile ;
			//	std::vector<STrueRenderingObject *>::iterator it_recv ;
			//	for(it_tile=m_sTRObjects.begin(); it_tile!=m_sTRObjects.end(); it_tile++)
			//	{
			//		if((*it_tile)->IsEnable() && !(*it_tile)->IsInFrustum())
			//		{
			//			(*it_tile)->SetInFrustum(true) ;
			//			(*it_tile)->fDistfromCamera = ((*pvCamera)-Vector3((*it_tile)->matWorld._41, (*it_tile)->matWorld._42, (*it_tile)->matWorld._43)).Magnitude() ;
			//			psTRObjects->push_back((*it_tile)) ;
			//		}
			//	}
			//}
		}
		//if(ppsTRObjects)//현재타일위에 있는 오브젝트를 검색한다.
		{
			int nPosCount, nCurNum, nTotalNum ;
			Matrix4 matWorld ;
			Vector3 vPos ;
			float fDistFromCamera ;
            STrueRenderingObject *psTRObject ;
			std::list<STrueRenderingObject *>::iterator it_tile ;
			for(it_tile=m_sTRObjects.begin(); it_tile!=m_sTRObjects.end(); it_tile++)
			{
				if((*it_tile)->IsEnable() && !(*it_tile)->IsInFrustum())
				{
					fDistFromCamera = ((*pvCamera)-Vector3((*it_tile)->matWorld._41, (*it_tile)->matWorld._42, (*it_tile)->matWorld._43)).Magnitude() ;
					//if( (fDistFromCamera > 200.0f) && ((*it_tile)->apcObject[0]->m_fObjectHeight < 10.0f) )//1m이하 오브젝트, 카메라와의 200m이하 거리
					//	continue ;

					if(pcFrustum->IsInSphere((*it_tile)->cBBox.m_vCenter, (*it_tile)->cBBox.m_fRadius) != FRUSTUM_OUT)
					{
						(*it_tile)->SetInFrustum(true) ;
						//(*it_tile)->fDistfromCamera = ((*pvCamera)-Vector3((*it_tile)->matWorld._41, (*it_tile)->matWorld._42, (*it_tile)->matWorld._43)).Magnitude() ;
						if((*it_tile)->IsFlag(STrueRenderingObject::FLAG_TWOBILLBOARD))
							(*it_tile)->SetDistance(((*pvCamera)-Vector3((*it_tile)->matWorld._41, (*it_tile)->matWorld._42, (*it_tile)->matWorld._43)).Magnitude(), pvCamera) ;
						else
							(*it_tile)->SetDistance(((*pvCamera)-Vector3((*it_tile)->matWorld._41, (*it_tile)->matWorld._42, (*it_tile)->matWorld._43)).Magnitude()) ;

						pcFrustum->m_apsTRObjects[pcFrustum->m_nNumTRObject++] = (*it_tile) ;

						psTRObject = (*it_tile) ;

						if(pcFrustum->m_nMaxNumBoundingPosition && psTRObject->IsFlag(STrueRenderingObject::FLAG_SHADOWENABLE))
						{
							matWorld = MatrixConvert(psTRObject->matWorld) ;

							//TRACE("getting bounding positions\r\n") ;

							nCurNum = psTRObject->apcObject[0]->GetBoundingPosfromAllMesh(&pcFrustum->m_pvBoundingPositions[pcFrustum->m_nNumBoundingPosition]) ;

							//TRACE("finish bounding positions\r\n") ;

                            nTotalNum = pcFrustum->m_nNumBoundingPosition+nCurNum ;
							if(nTotalNum < pcFrustum->m_nMaxNumBoundingPosition)
							{
								for(nPosCount=pcFrustum->m_nNumBoundingPosition ; nPosCount<nTotalNum ; nPosCount++)
									pcFrustum->m_pvBoundingPositions[nPosCount] *= matWorld ;

								pcFrustum->m_nNumBoundingPosition += nCurNum ;
							}

							//for(int nPosCount=0 ; nPosCount<8 ; nPosCount++)
							//	pcFrustum->InsertBoundPosition(&psTRObject->cBBox.m_avPos[nPosCount]) ;
						}
					}
				}
			}
		}
	}
	else
	{
		if(pcFrustum->m_pcToolBrush)
		{
			if(!pcFrustum->m_pcToolBrush->m_pcPicking->m_bIntersect)
			{
				if(!m_pcParent)
				{
					Vector3 vPos(pVertices[m_nCenter].pos.x, pVertices[m_nCenter].pos.y, pVertices[m_nCenter].pos.z) ;
					if(IntersectLineToSphere(&pcFrustum->m_pcToolBrush->m_pcPicking->m_sLine, vPos, m_fRadius))
						//if(IntersectLineToCube(&pcFrustum->m_pcToolBrush->m_cPicking.m_sLine, &m_sCube))				
						m_bIntersectLine = true ;
					else
						m_bIntersectLine = false ;
				}
				else
				{
					if(m_pcParent->m_bIntersectLine)
					{
						Vector3 vPos(pVertices[m_nCenter].pos.x, pVertices[m_nCenter].pos.y, pVertices[m_nCenter].pos.z) ;
						//if(IntersectLineToCube(&pcFrustum->m_pcToolBrush->m_cPicking.m_sLine, &m_sCube))
						if(IntersectLineToSphere(&pcFrustum->m_pcToolBrush->m_pcPicking->m_sLine, vPos, m_fRadius))
						{
							m_bIntersectLine = true ;
						}
						else
							m_bIntersectLine = false ;
					}
				}
			}
			//툴브러쉬 지형과 테스트
			//if(!pcFrustum->m_pcToolBrush->m_bIntersectedAll && pcFrustum->m_pcToolBrush->IsVisible())
			//{
			//	float x, z, l ;
			//	x = (m_vCenter.x-pcFrustum->m_pcToolBrush->m_vCenterPos.x) ;
			//	z = (m_vCenter.z-pcFrustum->m_pcToolBrush->m_vCenterPos.z) ;
			//	l = sqrtf((x*x)+(z*z)) ;

			//	if(!m_pcParent)
			//	{
			//		if(l <= (pcFrustum->m_pcToolBrush->m_fRadius+m_fRadius))
			//		//if((m_vCenter-pcFrustum->m_pcToolBrush->m_vCenterPos).Magnitude() <= (pcFrustum->m_pcToolBrush->m_fRadius+m_fRadius))
			//			m_bIntersectPos = true ;
			//		else
			//			m_bIntersectPos = false ;
			//	}
			//	else
			//	{
			//		if(m_pcParent->m_bIntersectPos)
			//		{
			//			if(l <= (pcFrustum->m_pcToolBrush->m_fRadius+m_fRadius))
			//			//if((m_vCenter-pcFrustum->m_pcToolBrush->m_vCenterPos).Magnitude() <= (pcFrustum->m_pcToolBrush->m_fRadius+m_fRadius))
			//				m_bIntersectPos = true ;
			//			else
			//				m_bIntersectPos = false ;
			//		}
			//	}
			//}
		}
	}

    if(m_pcChild[CORNER_TL]) nNumIndex = m_pcChild[CORNER_TL]->FrustumCull(pVertices, pcFrustum, pvCamera, pIndices, nNumIndex, psExceptionalIndices) ;
	if(m_pcChild[CORNER_TR]) nNumIndex = m_pcChild[CORNER_TR]->FrustumCull(pVertices, pcFrustum, pvCamera, pIndices, nNumIndex, psExceptionalIndices) ;
	if(m_pcChild[CORNER_BL]) nNumIndex = m_pcChild[CORNER_BL]->FrustumCull(pVertices, pcFrustum, pvCamera, pIndices, nNumIndex, psExceptionalIndices) ;
	if(m_pcChild[CORNER_BR]) nNumIndex = m_pcChild[CORNER_BR]->FrustumCull(pVertices, pcFrustum, pvCamera, pIndices, nNumIndex, psExceptionalIndices) ;

	return nNumIndex ;
}

bool CSecretQuadTree::_IsVisible(int nLevel, Vector3 *pvCamera, TERRAINVERTEX *pVertices)
{
    if(nLevel > _GetLODLevel(pvCamera, pVertices))
		return false ;

	return true ;
}

int CSecretQuadTree::_GetLODLevel(Vector3 *pvCamera, TERRAINVERTEX *pVertices)
{
	//static const float fLODRatio = 0.025f ;//간격이 1일때 사용
	//static const float fLODRatio = 0.03125f ;//간격이 2일때 사용

	//Vector3 vCenter(pVertices[m_nCenter].pos.x, pVertices[m_nCenter].pos.y, pVertices[m_nCenter].pos.z) ;
	//float fdist = ((*pvCamera)-vCenter).Magnitude()*fLODRatio ;

	float fdist = ((*pvCamera)-m_vCenter).Magnitude()*g_fQuadTreeLODRatio ;
    float fLevel = m_fRadius/fdist ;

	//if(float_greater(fLevel, 3.0f) || float_eq(fLevel, 3.0f))
	//	fLevel = 3.0f ;
	//else if(float_less(fLevel, 0.0f) || float_eq(fLevel, 0.0f))
	//	fLevel = 0.0f ;

    if(fLevel >= (float)(nMaxDepthLevel-1))
		fLevel = (float)(nMaxDepthLevel-1) ;
	else if(fLevel <= 0)
		fLevel = 0.0f ;

	return (int)(fLevel);//+0.5f) ;
}

int CSecretQuadTree::_GetNeighborNodeIndex(int nEdge, int nPitch, int nHeight, int *pnEdge)
{
	int nCenter, nGap, nEdgeHeight ;
	int anEdge[4] ;

	anEdge[0] = pnEdge[0] ;
	anEdge[1] = pnEdge[1] ;
	anEdge[2] = pnEdge[2] ;
	anEdge[3] = pnEdge[3] ;
	nGap = anEdge[1]-anEdge[0] ;
	nEdgeHeight = (anEdge[2]-anEdge[0])/nPitch ;

	switch(nEdge)
	{
	case NEIGHBOR_UP :
		pnEdge[0] = anEdge[0]-(nEdgeHeight*nPitch) ;
		pnEdge[1] = anEdge[1]-(nEdgeHeight*nPitch) ;
		pnEdge[2] = anEdge[0] ;
		pnEdge[3] = anEdge[1] ;

		break ;

	case NEIGHBOR_DOWN :
		pnEdge[0] = anEdge[2] ;
		pnEdge[1] = anEdge[3] ;
		pnEdge[2] = anEdge[2]+(nEdgeHeight*nPitch) ;
		pnEdge[3] = anEdge[3]+(nEdgeHeight*nPitch) ;

		break ;

	case NEIGHBOR_LEFT :
		pnEdge[0] = anEdge[0]-nGap ;
		pnEdge[1] = anEdge[0] ;
		pnEdge[2] = anEdge[2]-nGap ;
		pnEdge[3] = anEdge[2] ;

		break ;

	case NEIGHBOR_RIGHT :
		pnEdge[0] = anEdge[1] ;
		pnEdge[1] = anEdge[1]+nGap ;
		pnEdge[2] = anEdge[3] ;
		pnEdge[3] = anEdge[3]+nGap ;

		break ;
	}

	nCenter = (pnEdge[0]+pnEdge[1]+pnEdge[2]+pnEdge[3])/4 ;

	//전체맵안에 포함되어있는지 판단
	//if((nCenter >= 0) && (nCenter <= (nPitch*nPitch-1)))
	if((nCenter >= 0) && (nCenter <= (nPitch*nHeight-1)))
		return nCenter ;

	return -1 ;
}

CSecretQuadTree *CSecretQuadTree::_FindNode(TERRAINVERTEX *pVertices, int nTL, int nTR, int nBL, int nBR)
{
	//현재 노드와 일치하면 리턴한다.
	if((m_nCorner[0] == nTL) && (m_nCorner[1] == nTR) && (m_nCorner[2] == nBL) && (m_nCorner[3] == nBR))
		return this ;

	if(m_pcChild[0])//자식노드가 있다면
	{
		//RECT rt ;
		//POINT pt ;
		geo::SPoint point ;
		geo::SRect rect ;
		int i, nCenter ;

		nCenter = (nTL+nTR+nBL+nBR)/4 ;

		point.x = pVertices[nCenter].pos.x ;// (int)pHeightMap[nCenter].pos.x ;
		point.y = pVertices[nCenter].pos.z ;//(int)pHeightMap[nCenter].pos.z ;

		//4개의 자식노드를 검색한다.
		for(i=0 ; i<4 ; i++)
		{
			rect.set(pVertices[m_pcChild[i]->m_nCorner[CORNER_TL]].pos.x,
				pVertices[m_pcChild[i]->m_nCorner[CORNER_BR]].pos.x,
				pVertices[m_pcChild[i]->m_nCorner[CORNER_TL]].pos.z,				
				pVertices[m_pcChild[i]->m_nCorner[CORNER_BR]].pos.z) ;

			//현재의 Center위치값이 Rect안에 있는 점이라면 자식노드를 따라서 재귀호출을 계속한다.
			if((point.x >= rect.left) && (point.x <= rect.right) && (point.y <= rect.top) && (point.y >= rect.bottom))
				return m_pcChild[i]->_FindNode(pVertices, nTL, nTR, nBL, nBR) ;

		}
	}
	return NULL ;
}

void CSecretQuadTree::_BuildNeighborNode(CSecretQuadTree *pRoot, TERRAINVERTEX *pVertices, int nPitch, int nHeight)
{
	int i, nCenter ;
	int anEdge[4] ;

    for(i=0 ; i<4 ; i++)
	{
		//현재의 4귀퉁이를 모두구하고
		anEdge[0] = m_nCorner[0] ;
		anEdge[1] = m_nCorner[1] ;
		anEdge[2] = m_nCorner[2] ;
		anEdge[3] = m_nCorner[3] ;

		//4귀퉁이를 가지고 위[0], 아래[1], 왼쪽[2], 오른쪽[3] 의 이웃노드를 구한다.
        nCenter = _GetNeighborNodeIndex(i, nPitch, nHeight, anEdge) ;

		if(nCenter >= 0)//이웃노드를 구하는데 성공했다면 실제 노드포인터를 얻어낸다.
		{
			m_pcNeighbor[i] = pRoot->_FindNode(pVertices, anEdge[0], anEdge[1], anEdge[2], anEdge[3]) ;
		}
	}

	if(m_pcChild[0])//자식노드에서도 재귀호출로 모두 이웃노드를 구하게 된다.
	{
		for(i=0 ; i<4 ; i++)
			m_pcChild[i]->_BuildNeighborNode(pRoot, pVertices, nPitch, nHeight) ;
	}
}

int CSecretQuadTree::GetTreeCubes(std::vector<geo::SAACube *> *pCubeSet, int nNumCube)
{
	if(((m_bIntersectPos == true) || (m_bIntersectLine == true)) && (m_bLastDepthLevel))
	{
		pCubeSet->push_back(&m_sCube) ;
		nNumCube++ ;
		return nNumCube ;
	}

	for(int i=0 ; i<4 ; i++)
		if(m_pcChild[i]) nNumCube = m_pcChild[i]->GetTreeCubes(pCubeSet, nNumCube) ;

	return nNumCube ;
}

//int CSecretQuadTree::_GetTriangles(Vector3 *pvCamera, TERRAINVERTEX *pVertices, TERRAININDEX *pIndices)
//{
//}
//
//int CSecretQuadTree::GetTriangles(Vector3 *pvCamera, TERRAINVERTEX *pVertices, TERRAININDEX *pIndices)
//{
//    return _GetTriangles(pvCamera, pVertices, pIndices) ;
//}
std::list<STrueRenderingObject *> *CSecretQuadTree::GetTRObjects()
{
	return &m_sTRObjects ;
}

void CSecretQuadTree::_PlaceObjects()
{
	std::list<STrueRenderingObject *> *psTRObjects ;
	if(m_pcParent)//부모트리가 있으면
	{
		psTRObjects = m_pcParent->GetTRObjects() ;
		if(psTRObjects)
		{
			if(psTRObjects->size() == 0)//부모트리에 오브젝트가 없으면
				return ;
		}
	}
	else
		return ;

	STrueRenderingObject *p ;

	int i ;
	float x, z, l ;
	std::list<STrueRenderingObject *>::iterator it ;
	for(it=psTRObjects->begin(); it!=psTRObjects->end(); it++)
	{
		p = (*it) ;		

		x = (m_vCenter.x-(*it)->matWorld._41) ;
		z = (m_vCenter.z-(*it)->matWorld._43) ;
		l = sqrt(x*x+z*z) ;
		if(l <= (m_fRadius+(*it)->cBBox.m_fRadius))
		{
			if(m_bLastDepthLevel)
			{
				//TRACE("placeobject %s this=0x%x \r\n", (*it)->asCStrTreeItem[0].GetString(), this) ;
				(*it)->apcQuadTrees[(*it)->nNumQuadTree++] = this ;
				assert((*it)->nNumQuadTree < MAXNUM_INCLUDEDTILES) ;//여기에서 걸리는 에러
			}
			m_sTRObjects.push_back((*it)) ;
		}
	}
    
	for(i=0; i<4; i++)
	{
		if(m_pcChild[i])
			m_pcChild[i]->_PlaceObjects() ;
	}

	if(!m_bLastDepthLevel)//마지막트리가 아닌경우는 오브젝트 모두 제거
		m_sTRObjects.resize(0) ;
}

void CSecretQuadTree::PlaceObjects(int nNum, STrueRenderingObject *psTRObjects)
{
	int i ;
	for(i=0; i<nNum; i++)
		m_sTRObjects.push_back(&psTRObjects[i]) ;

	for(i=0; i<4; i++)
	{
		m_pcChild[i]->_PlaceObjects() ;
	}
	m_sTRObjects.resize(0) ;
}

CSecretQuadTree *CSecretQuadTree::GetNeighbor(int nNeighbor)
{
	return m_pcNeighbor[nNeighbor] ;
}

void SDivideTile::set(int tl, int tr, int bl, int br, TERRAINVERTEX *psVertices)
{
	avEdge[0].set(psVertices[tl].pos.x, psVertices[tl].pos.y, psVertices[tl].pos.z) ;
	avEdge[1].set(psVertices[tr].pos.x, psVertices[tr].pos.y, psVertices[tr].pos.z) ;
	avEdge[2].set(psVertices[bl].pos.x, psVertices[bl].pos.y, psVertices[bl].pos.z) ;
	avEdge[3].set(psVertices[br].pos.x, psVertices[br].pos.y, psVertices[br].pos.z) ;
	
	int c = (tl+tr+bl+br)/4 ;
	vCenter.set(psVertices[c].pos.x, psVertices[c].pos.y, psVertices[c].pos.z) ;
}

//#######################################################################//
//                            CSecretTile                                //
//#######################################################################//

CSecretTile::CSecretTile()
{
	m_psTileBit = NULL ;

	for(int i=0 ; i<MAXNUM_TILEPATCH ; i++)
	{
		m_apsIndices[i] = NULL ;
		m_anNumIndex[i] = 0 ;
	}
    //m_psTriangles = NULL ;

	m_nMaxNumBitInLine = m_nMaxNumLine = 0 ;
	m_dwClassSize = sizeof(CSecretTile) ;
}

CSecretTile::~CSecretTile()
{
	Release() ;
}

void CSecretTile::Release()
{
	SAFE_DELETEARRAY(m_psTileBit) ;

	for(int i=0 ; i<MAXNUM_TILEPATCH ; i++)
		SAFE_DELETEARRAY(m_apsIndices[i]) ;

	//SAFE_DELETEARRAY(m_psTriangles) ;
}
/*
void CSecretTile::CreateTriangles(int tl, int tr, int bl, int br, TERRAINVERTEX *pVertices)
{
	int i, n, t ;
	int tc, bc, lc, rc, c ;
	// 상단변 가운데
	tc = (tr+tl)/2 ;
	// 하단변 가운데
	bc = (br+bl)/2 ;
	// 좌측변 가운데
    lc = (bl+tl)/2 ;	
	// 우측변 가운데
    rc = (br+tr)/2 ;
	// 한가운데
	c = (tl+tr+bl+br)/4 ;

	//현재 타일을 4분할 한다.
	m_apsDivideTile[0].set(tl, tc, lc, c, pVertices) ;
	m_apsDivideTile[1].set(tc, tr, c, rc, pVertices) ;
	m_apsDivideTile[2].set(lc, c, bl, bc, pVertices) ;
	m_apsDivideTile[3].set(c, rc, bc, br, pVertices) ;


	int nNumTriangle=0 ;
	for(i=0 ; i<MAXNUM_TILEPATCH ; i++)
		nNumTriangle += m_anNumIndex[i] ;

	for(i=0 ; i<4 ; i++)
	{
		for(n=0 ; n<MAXNUM_TILEPATCH ; n++)
			m_apsDivideTile[i].anTriangleCount[n] = 0 ;

	}

	m_psTriangles = new geo::STriangle[nNumTriangle] ;
    
	bool bInclude ;
	Vector3 vPos ;
	int count=0 ;
	for(i=0 ; i<MAXNUM_TILEPATCH ; i++)
	{
		for(n=0 ; n<m_anNumIndex[i] ; n++)
		{
			m_psTriangles[count].set(
				Vector3(pVertices[m_apsIndices[i][n].i1].pos.x, pVertices[m_apsIndices[i][n].i1].pos.y, pVertices[m_apsIndices[i][n].i1].pos.z),
				Vector3(pVertices[m_apsIndices[i][n].i2].pos.x, pVertices[m_apsIndices[i][n].i2].pos.y, pVertices[m_apsIndices[i][n].i2].pos.z),
				Vector3(pVertices[m_apsIndices[i][n].i3].pos.x, pVertices[m_apsIndices[i][n].i3].pos.y, pVertices[m_apsIndices[i][n].i3].pos.z)) ;

			vPos = m_psTriangles[count].GetMidPos() ;
			bInclude = false ;
			for(t=0 ; t<4 ; t++)
			{
				if((vPos.x >= m_apsDivideTile[t].avEdge[0].x)//왼쪽 x보다 크고
					&& (vPos.x <= m_apsDivideTile[t].avEdge[1].x)//오른쪽 x보다 작고
					&& (vPos.z <= m_apsDivideTile[t].avEdge[0].z)//위쪽 z보다 작고
					&& (vPos.z >= m_apsDivideTile[t].avEdge[2].z))//아래쪽 z보다 크고
				{
					m_apsDivideTile[t].aapsTriangles[i][m_apsDivideTile[t].anTriangleCount[i]++] = &m_psTriangles[count] ;
					bInclude = true ;
					break ;
				}
			}
            assert(bInclude) ;
			count++ ;
		}
	}
}
*/
void CSecretTile::_Make2Triangles(int tl, int tr, int bl, int br, TERRAININDEX *pIndices)
{
	pIndices->i1 = tl ;
	pIndices->i2 = tr ;
	pIndices->i3 = bl ;

	pIndices->i1 = bl ;
	pIndices->i2 = tr ;
	pIndices->i3 = br ;
}

void CSecretTile::_ClassifyBitTile(int nBitCount, int nLineCount, std::vector<STileBit *> *pTileBitSets, STileBit *psTileBit)
{
	if(nLineCount == 0)
	{
		if(nBitCount == 0)//왼쪽, 위
		{
			pTileBitSets[TILEPOS_LEFTTOP].push_back(psTileBit) ;
		}
		else if(nBitCount == (m_nMaxNumBitInLine-1))//오른쪽, 위
		{
			pTileBitSets[TILEPOS_RIGHTTOP].push_back(psTileBit) ;
		}
		else//그냥 위
		{
			pTileBitSets[TILEPOS_TOP].push_back(psTileBit) ;
		}
	}
	else if(nLineCount == (m_nMaxNumLine-1))
	{
		if(nBitCount == 0)//왼쪽, 아래
		{
			pTileBitSets[TILEPOS_LEFTBOTTOM].push_back(psTileBit) ;
		}
		else if(nBitCount == (m_nMaxNumBitInLine-1))//오른쪽, 아래
		{
			pTileBitSets[TILEPOS_RIGHTBOTTOM].push_back(psTileBit) ;
		}
		else//그냥 아래
		{
			pTileBitSets[TILEPOS_BOTTOM].push_back(psTileBit) ;
		}
	}
	else
	{
		if(nBitCount == 0)
		{
			pTileBitSets[TILEPOS_LEFT].push_back(psTileBit) ;
		}
		else if(nBitCount == (m_nMaxNumBitInLine-1))
		{
			pTileBitSets[TILEPOS_RIGHT].push_back(psTileBit) ;
		}
		else
		{
			pTileBitSets[TILEPOS_INSIDE].push_back(psTileBit) ;
		}
	}
}

void CSecretTile::GernerateZeroLevelIndices(int tl, int tr, int bl, int br)
{
	int i ;
	for(i=0 ; i<MAXNUM_TILEPATCH ; i++)
	{
		m_anNumIndex[i] = 2 ;
		m_apsIndices[i] = new TERRAININDEX[m_anNumIndex[i]] ;

		m_apsIndices[i][0].i1 = tl ;
		m_apsIndices[i][0].i2 = tr ;
		m_apsIndices[i][0].i3 = bl ;

		m_apsIndices[i][1].i1 = bl ;
		m_apsIndices[i][1].i2 = tr ;
		m_apsIndices[i][1].i3 = br ;

		m_dwClassSize += sizeof(TERRAININDEX)*m_anNumIndex[i] ;
	}
}

void CSecretTile::GernerateOneLevelIndices(int tl, int tr, int bl, int br)
{
	int i ;

	std::vector<TERRAININDEX> aIndexSet[MAXNUM_TILEPATCH] ;

	//TRACE("\r\n") ;

	for(i=0 ; i<MAXNUM_TILEPATCH ; i++)
	{
		//TRACE("### [%02d]Patch Start###\r\n", i) ;
		_MakePatch(anPatchs[i], tl, tr, bl, br, &aIndexSet[i]) ;
		//TRACE("### [%02d]Patch End###\r\n", i) ;
		//TRACE("\r\n") ;
	}
	//TRACE("\r\n") ;

	for(i=0 ; i<MAXNUM_TILEPATCH ; i++)
	{
		if(aIndexSet[i].size() > 0)
		{
			m_anNumIndex[i] = (int)aIndexSet[i].size() ;
			m_apsIndices[i] = new TERRAININDEX[m_anNumIndex[i]] ;

			int count=0 ;
			std::vector<TERRAININDEX>::iterator it ;
			for(it=aIndexSet[i].begin() ; it!=aIndexSet[i].end() ; it++)
				m_apsIndices[i][count++] = (*it) ;

			m_dwClassSize += sizeof(TERRAININDEX)*m_anNumIndex[i] ;
		}
	}
}

void CSecretTile::DivideTile(int tl, int tr, int bl, int br, int nLevel)
{
	int i ;
	int tc, bc, lc, rc, c ;
	int nBitWidth, nBitHeight ;

	// 상단변 가운데
	tc = (tr+tl)/2 ;
	// 하단변 가운데
	bc = (br+bl)/2 ;
	// 좌측변 가운데
    lc = (bl+tl)/2 ;	
	// 우측변 가운데
    rc = (br+tr)/2 ;
	// 한가운데
	c = (tl+tr+bl+br)/4 ;

	std::vector<STileBit *> *pTileBitSets=NULL ;

	pTileBitSets = new std::vector<STileBit *>[MAXNUM_TILEPOS] ;
	for(i=0 ; i<MAXNUM_TILEPOS ; i++)
		pTileBitSets[i].resize(0) ;


	//레벨이 따른 전체 비트 갯수
	int nNumTileBit = (int)pow(4.0f, (float)nLevel) ;

	nBitWidth = (int)((tr-tl)/pow(2.0f, (float)nLevel)) ;
	nBitHeight = (int)((bl-tl)/pow(2.0f, (float)nLevel)) ;

	m_psTileBit = new STileBit[nNumTileBit] ;

	m_nMaxNumBitInLine = (tr-tl)/nBitWidth ;
	m_nMaxNumLine = (bl-tl)/nBitHeight ;

	int nPos = tl ;
	int nCount=0, nLineEnd=tr, nLineCount=0, nBitCountInLine=0 ;

	while(true)
	{
		m_psTileBit[nCount].set(nPos, nPos+nBitWidth, nPos+nBitHeight, nPos+nBitHeight+nBitWidth) ;
		//TRACE("[%02d] tl=%02d tr=%02d bl=%02d br=%02d BitCount=%02d LineCount=%02d\r\n",
		//	nCount, nPos, nPos+nBitWidth, nPos+nBitHeight, nPos+nBitHeight+nBitWidth, nBitCountInLine, nLineCount) ;

		_ClassifyBitTile(nBitCountInLine, nLineCount, pTileBitSets, &m_psTileBit[nCount]) ;

		nCount++ ;
		nBitCountInLine++ ;

		if(nPos+nBitHeight+nBitWidth >= br)
			break ;

		nPos = nPos+nBitWidth ;

		if(nPos == nLineEnd)
		{
			nPos += (nBitHeight) ;
			nLineEnd = nPos ;
			nPos = nLineEnd-(tr-tl) ;
			nLineCount++ ;
			nBitCountInLine=0 ;
		}
	}

	std::vector<STileBit *>::iterator it ;

	//TRACE("\r\n") ;
	//for(i=0 ; i<MAXNUM_TILEPOS ; i++)
	//{
	//	for(it=pTileBitSets[i].begin() ; it!=pTileBitSets[i].end() ; it++)
	//	{
	//		TRACE("[%02d] tl=%02d tr=%02d bl=%02d br=%02d\r\n", i, (*it)->tl, (*it)->tr, (*it)->bl, (*it)->br) ;
	//	}
	//	TRACE("\r\n") ;
	//}

	//std::vector<TERRAININDEX> aIndexSet[MAXNUM_TILEPATCH] ;
	std::vector<TERRAININDEX> *aIndexSet=NULL ;
	aIndexSet = new std::vector<TERRAININDEX>[MAXNUM_TILEPATCH] ;

	for(i=0 ; i<MAXNUM_TILEPATCH ; i++)
		_GenerateIndices(anPatchs[i], pTileBitSets, aIndexSet) ;

	for(i=0 ; i<MAXNUM_TILEPATCH ; i++)
	{
		if(aIndexSet[i].size() > 0)
		{
			m_anNumIndex[i] = (int)aIndexSet[i].size() ;
			m_apsIndices[i] = new TERRAININDEX[m_anNumIndex[i]] ;

			m_dwClassSize += sizeof(TERRAININDEX)*m_anNumIndex[i] ;

			int count=0 ;
			std::vector<TERRAININDEX>::iterator it ;
			for(it=aIndexSet[i].begin() ; it!=aIndexSet[i].end() ; it++)
				m_apsIndices[i][count++] = (*it) ;
		}
	}

	SAFE_DELETEARRAY(aIndexSet) ;

	SAFE_DELETEARRAY(pTileBitSets) ;
	SAFE_DELETEARRAY(m_psTileBit) ;
	
	int xxx = 0 ;
}

int CSecretTile::GetPatchTypeFromNeighbor(int nNeighbor)
{
	if( (nNeighbor & NEIGHBOR_UP) && (nNeighbor & NEIGHBOR_DOWN) && (nNeighbor & NEIGHBOR_LEFT) && (nNeighbor & NEIGHBOR_RIGHT) ) return 0 ;
	else if( (nNeighbor & NEIGHBOR_UP) && (nNeighbor & NEIGHBOR_DOWN) && (nNeighbor & NEIGHBOR_LEFT) )							  return 1 ;
	else if( (nNeighbor & NEIGHBOR_UP) && (nNeighbor & NEIGHBOR_DOWN) && (nNeighbor & NEIGHBOR_RIGHT) )                           return 2 ;
	else if( (nNeighbor & NEIGHBOR_DOWN) && (nNeighbor & NEIGHBOR_LEFT) && (nNeighbor & NEIGHBOR_RIGHT) )                         return 3 ;
    else if( (nNeighbor & NEIGHBOR_UP) && (nNeighbor & NEIGHBOR_LEFT) && (nNeighbor & NEIGHBOR_RIGHT) )                           return 4 ;
	else if( (nNeighbor & NEIGHBOR_UP) && (nNeighbor & NEIGHBOR_RIGHT) )                                                          return 5 ;
	else if( (nNeighbor & NEIGHBOR_DOWN) && (nNeighbor & NEIGHBOR_RIGHT) )                                                        return 6 ;
	else if( (nNeighbor & NEIGHBOR_DOWN) && (nNeighbor & NEIGHBOR_LEFT) )                                                         return 7 ;
	else if( (nNeighbor & NEIGHBOR_UP) && (nNeighbor & NEIGHBOR_LEFT) )                                                           return 8 ;
	else if( (nNeighbor & NEIGHBOR_LEFT) && (nNeighbor & NEIGHBOR_RIGHT) )                                                        return 9 ;
	else if( (nNeighbor & NEIGHBOR_UP) && (nNeighbor & NEIGHBOR_DOWN) )                                                           return 10 ;
	else if( (nNeighbor & NEIGHBOR_RIGHT) )                                                                                       return 11 ;
	else if( (nNeighbor & NEIGHBOR_LEFT) )                                                                                        return 12 ;
	else if( (nNeighbor & NEIGHBOR_DOWN) )                                                                                        return 13 ;
	else if( (nNeighbor & NEIGHBOR_UP) )                                                                                          return 14 ;
	else                                                                                                                          return 15 ;
}

void CSecretTile::_GenerateIndices(int nNeighbor, std::vector<STileBit *> *pTileBitSets, std::vector<TERRAININDEX> *pIndexSet)
{
	int i ;
	int nType ;
	std::vector<STileBit *>::iterator it ;

	nType = GetPatchTypeFromNeighbor(nNeighbor) ;
    
	//TRACE("### [%02d]Patch Start###\r\n", nType) ;

	for(i=0 ; i<MAXNUM_TILEPOS ; i++)
	{
		if(pTileBitSets[i].size() > 0)
		{
			for(it=pTileBitSets[i].begin() ; it!=pTileBitSets[i].end() ; it++)
				_MakePatch(apnSubPatchs[nType][i], (*it)->tl, (*it)->tr, (*it)->bl, (*it)->br, &pIndexSet[nType]) ;
            //TRACE("\r\n") ;
		}
	}

	//TRACE("### [%02d]Patch End###\r\n", nType) ;
}

int CSecretTile::GetIndices(int nNeighbor, TERRAININDEX *pIndices, int tl, data::SKeepingArray<TERRAININDEX> *psExceptionalIndices)
{
	bool bException ;
	int i, n, nPatchType = GetPatchTypeFromNeighbor(nNeighbor) ;

	TERRAININDEX *pExcept ;
	TERRAININDEX *pDest = pIndices ;
	TERRAININDEX *pSrc = m_apsIndices[nPatchType] ;
    for(i=0 ; i<m_anNumIndex[nPatchType] ; i++, pDest++, pSrc++)
	{
		//홀컵이 있는 곳은 미리 인덱스로 저장해 두었다가 저장된 인덱스와 일치하면 그 인덱스는 빼버린다.
		//여기에서 비교코드를 넣어라.
		bException = false ;
		for(n=0 ; n<psExceptionalIndices->nCurPos ; n++)
		{
			pExcept = psExceptionalIndices->GetAt(n) ;
			if(((pSrc->i1+tl) == pExcept->i1) && ((pSrc->i2+tl) == pExcept->i2) && ((pSrc->i3+tl) == pExcept->i3))
			{
				bException = true ;
				break ;
			}
		}

		if(!bException)
		{
			pDest->i1 = pSrc->i1+tl ;
			pDest->i2 = pSrc->i2+tl ;
			pDest->i3 = pSrc->i3+tl ;
		}
	}

	//memcpy(pIndices, m_apsIndices[nPatchType], sizeof(TERRAININDEX)*m_anNumIndex[nPatchType]) ;
	return m_anNumIndex[nPatchType] ;
}

void CSecretTile::GenerateIndexBuffer(int tl, int tr, int bl, int br, TERRAININDEX *pIndices, int nLevel)
{
	int i ;
    int tc, bc, lc, rc, c ;

	// 상단변 가운데
	tc = (tr+tl)/2 ;
	// 하단변 가운데
	bc = (br+bl)/2 ;
	// 좌측변 가운데
    lc = (bl+tl)/2 ;	
	// 우측변 가운데
    rc = (br+tr)/2 ;
	// 한가운데
	c = (tl+tr+bl+br)/4 ;

	//1. 온전한 본체
	_Make2Triangles(tl, tc, lc, c, pIndices++) ;
	_Make2Triangles(tc, tr, c, rc, pIndices++) ;
	_Make2Triangles(lc, c, bl, bc, pIndices++) ;
	_Make2Triangles(c, rc, bc, br, pIndices++) ;

	for(i=0 ; i<15 ; i++)
	{
		_MakePatch(anPatchs[i], tl, tr, bl, br, pIndices) ;
	}
}

void CSecretTile::_MakePatch(int nNeighbor, int tl, int tr, int bl, int br, std::vector<TERRAININDEX> *pIndexSet)
{
	int tc, bc, lc, rc, c ;

	// 상단변 가운데
	tc = (tr+tl)/2 ;
	// 하단변 가운데
	bc = (br+bl)/2 ;
	// 좌측변 가운데
    lc = (bl+tl)/2 ;	
	// 우측변 가운데
    rc = (br+tr)/2 ;
	// 한가운데
	c = (tl+tr+bl+br)/4 ;

	TERRAININDEX index ;

	if(nNeighbor == 0)
	{
		index.i1 = tl ;
		index.i2 = tc ;
		index.i3 = lc ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = lc ;
		index.i2 = tc ;
		index.i3 = c ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = tc ;
		index.i2 = tr ;
		index.i3 = c ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

        index.i1 = c ;
		index.i2 = tr ;
		index.i3 = rc ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = lc ;
		index.i2 = c ;
		index.i3 = bl ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = bl ;
		index.i2 = c ;
		index.i3 = bc ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = c ;
		index.i2 = rc ;
		index.i3 = bc ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = bc ;
		index.i2 = rc ;
		index.i3 = br ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		return ;
	}

	if(nNeighbor & NEIGHBOR_UP)
	{
		index.i1 = tl ;
		index.i2 = tr ;
		index.i3 = c ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
	}
	if(nNeighbor & NEIGHBOR_DOWN)
	{
		index.i1 = bl ;
		index.i2 = c ;
		index.i3 = br ;
        pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
	}
	if(nNeighbor & NEIGHBOR_LEFT)
	{
		index.i1 = tl ;
		index.i2 = c ;
		index.i3 = bl ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
	}
	if(nNeighbor & NEIGHBOR_RIGHT)
	{
		index.i1 = c ;
		index.i2 = tr ;
		index.i3 = br ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
	}

    if(nNeighbor & NEIGHBOR_UP)
	{
		if(nNeighbor & NEIGHBOR_DOWN)
		{
			if(nNeighbor & NEIGHBOR_LEFT)
			{
				if(nNeighbor & NEIGHBOR_RIGHT)//여기까지 오면 사방으로 모든 패치가 다 있음.
				{//더 이상 인덱스 추가 필요없음.
					return ;
				}
				else//여기는 위, 아래, 좌
				{
					index.i1 = c ;
					index.i2 = tr ;
					index.i3 = rc ;
					pIndexSet->push_back(index) ;
					//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

					index.i1 = c ;
					index.i2 = rc ;
					index.i3 = br ;
					pIndexSet->push_back(index) ;
					//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
					return ;
				}
			}
			else if(nNeighbor & NEIGHBOR_RIGHT)// 여기는 위, 아래, 우
			{
				index.i1 = tl ;
				index.i2 = c ;
				index.i3 = lc ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = lc ;
				index.i2 = c ;
				index.i3 = bl ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
				return ;
			}
			else//여기는 위, 아래
			{
                index.i1 = tl ;
				index.i2 = c ;
				index.i3 = lc ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = lc ;
				index.i2 = c ;
				index.i3 = bl ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = c ;
				index.i2 = tr ;
				index.i3 = rc ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = c ;
				index.i2 = rc ;
				index.i3 = br ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
				return ;
			}
		}//if(nNeighbor & NEIGHBOR_DOWN)
		else if(nNeighbor & NEIGHBOR_LEFT)
		{
            if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 위, 좌, 우
			{
				index.i1 = bl ;
				index.i2 = c ;
				index.i3 = bc ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = bc ;
				index.i2 = c ;
				index.i3 = br ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
				return ;
			}
			else//여기까지는 위, 좌
			{
                index.i1 = c ;
				index.i2 = tr ;
				index.i3 = rc ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = bl ;
				index.i2 = c ;
				index.i3 = bc ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = c ;
				index.i2 = rc ;
				index.i3 = bc ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = bc ;
				index.i2 = rc ;
				index.i3 = br ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
				return ;
			}
		}//if(nNeighbor & NEIGHBOR_LEFT)
		else if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 위, 우
		{
			index.i1 = tl ;
			index.i2 = c ;
			index.i3 = lc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = lc ;
			index.i2 = c ;
			index.i3 = bl ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = bl ;
			index.i2 = c ;
			index.i3 = bc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = bc ;
			index.i2 = c ;
			index.i3 = br ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
			return ;
		}//if(nNeighbor & NEIGHBOR_RIGHT)
		else//여기까지는 위에만 있는경우
		{
			index.i1 = tl ;
			index.i2 = c ;
			index.i3 = lc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = tr ;
			index.i2 = rc ;
			index.i3 = c ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

            index.i1 = lc ;
			index.i2 = c ;
			index.i3 = bl ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = bl ;
			index.i2 = c ;
			index.i3 = bc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = c ;
			index.i2 = rc ;
			index.i3 = bc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = bc ;
			index.i2 = rc ;
			index.i3 = br ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
			return ;
		}
	}
	else if(nNeighbor & NEIGHBOR_DOWN)
	{
		if(nNeighbor & NEIGHBOR_LEFT)
		{
			if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 아래, 좌, 우
			{
				index.i1 = tl ;
				index.i2 = tc ;
				index.i3 = c ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = tc ;
				index.i2 = tr ;
				index.i3 = c ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
				return ;
			}
			else//여기까지는 아래, 좌
			{
				index.i1 = tl ;
				index.i2 = tc ;
				index.i3 = c ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = tc ;
				index.i2 = tr ;
				index.i3 = c ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = c ;
				index.i2 = tr ;
				index.i3 = rc ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

				index.i1 = c ;
				index.i2 = rc ;
				index.i3 = br ;
				pIndexSet->push_back(index) ;
				//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
				return ;
			}
		}//if(nNeighbor & NEIGHBOR_LEFT)
		else if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 아래, 우
		{
			index.i1 = tl ;
			index.i2 = tc ;
			index.i3 = lc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = lc ;
			index.i2 = tc ;
			index.i3 = c ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = tc ;
			index.i2 = tr ;
			index.i3 = c ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = lc ;
			index.i2 = c ;
			index.i3 = bl ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
			return ;
		}//if(nNeighbor & NEIGHBOR_RIGHT)
		else//여기까지는 아래
		{
			index.i1 = tl ;
			index.i2 = tc ;
			index.i3 = lc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = lc ;
			index.i2 = tc ;
			index.i3 = c ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = tc ;
			index.i2 = tr ;
			index.i3 = c ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = c ;
			index.i2 = tr ;
			index.i3 = rc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = lc ;
			index.i2 = c ;
			index.i3 = bl ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = c ;
			index.i2 = rc ;
			index.i3 = br ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
			return ;
		}
	}
	else if(nNeighbor & NEIGHBOR_LEFT)
	{
		if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 좌, 우
		{
			index.i1 = tl ;
			index.i2 = tc ;
			index.i3 = c ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = tc ;
			index.i2 = tr ;
			index.i3 = c ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = bl ;
			index.i2 = c ;
			index.i3 = bc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = c ;
			index.i2 = br ;
			index.i3 = bc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
			return ;
		}
		else//여기까지는 좌
		{
			index.i1 = tl ;
			index.i2 = tc ;
			index.i3 = c ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = bl ;
			index.i2 = c ;
			index.i3 = bc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = tc ;
			index.i2 = tr ;
			index.i3 = c ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = c ;
			index.i2 = tr ;
			index.i3 = rc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = c ;
			index.i2 = rc ;
			index.i3 = bc ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

			index.i1 = bc ;
			index.i2 = rc ;
			index.i3 = br ;
			pIndexSet->push_back(index) ;
			//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
			return ;
		}
	}
	else if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 우
	{
		index.i1 = tl ;
		index.i2 = tc ;
		index.i3 = lc ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = lc ;
		index.i2 = tc ;
		index.i3 = c ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = lc ;
		index.i2 = c ;
		index.i3 = bl ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = bl ;
		index.i2 = c ;
		index.i3 = bc ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = tc ;
		index.i2 = tr ;
		index.i3 = c ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;

		index.i1 = c ;
		index.i2 = br ;
		index.i3 = bc ;
		pIndexSet->push_back(index) ;
		//TRACE("index(%02d %02d %02d)\r\n", index.i1, index.i2, index.i3) ;
		return ;
	}
}

void CSecretTile::_MakePatch(int nNeighbor, int tl, int tr, int bl, int br, TERRAININDEX *pIndices)
{
	int tc, bc, lc, rc, c ;

	// 상단변 가운데
	tc = (tr+tl)/2 ;
	// 하단변 가운데
	bc = (br+bl)/2 ;
	// 좌측변 가운데
    lc = (bl+tl)/2 ;	
	// 우측변 가운데
    rc = (br+tr)/2 ;
	// 한가운데
	c = (tl+tr+bl+br)/4 ;

	if(nNeighbor & NEIGHBOR_UP)
	{
		pIndices->i1 = tl ;
		pIndices->i2 = tr ;
		pIndices->i3 = c ;
		pIndices++ ;
	}
	if(nNeighbor & NEIGHBOR_DOWN)
	{
		pIndices->i1 = bl ;
		pIndices->i2 = c ;
		pIndices->i3 = br ;
		pIndices++ ;
	}
	if(nNeighbor & NEIGHBOR_LEFT)
	{
		pIndices->i1 = tl ;
		pIndices->i2 = c ;
		pIndices->i3 = bl ;
		pIndices++ ;
	}
	if(nNeighbor & NEIGHBOR_RIGHT)
	{
		pIndices->i1 = c ;
		pIndices->i2 = tr ;
		pIndices->i3 = br ;
		pIndices++ ;
	}

    if(nNeighbor & NEIGHBOR_UP)
	{
		if(nNeighbor & NEIGHBOR_DOWN)
		{
			if(nNeighbor & NEIGHBOR_LEFT)
			{
				if(nNeighbor & NEIGHBOR_RIGHT)//여기까지 오면 사방으로 모든 패치가 다 있음.
				{//더 이상 인덱스 추가 필요없음.
				}
				else//여기는 위, 아래, 좌
				{
					pIndices->i1 = c ;
					pIndices->i2 = tr ;
					pIndices->i3 = rc ;
					pIndices++ ;

					pIndices->i1 = c ;
					pIndices->i2 = rc ;
					pIndices->i3 = br ;
					pIndices++ ;
				}
			}
			else if(nNeighbor & NEIGHBOR_RIGHT)// 여기는 위, 아래, 우
			{
				pIndices->i1 = tl ;
				pIndices->i2 = c ;
				pIndices->i3 = lc ;
				pIndices++ ;

				pIndices->i1 = lc ;
				pIndices->i2 = c ;
				pIndices->i3 = bl ;
				pIndices++ ;
			}
			else//여기는 위, 아래
			{
                pIndices->i1 = tl ;
				pIndices->i2 = c ;
				pIndices->i3 = lc ;
				pIndices++ ;

				pIndices->i1 = lc ;
				pIndices->i2 = c ;
				pIndices->i3 = bl ;
				pIndices++ ;

				pIndices->i1 = c ;
				pIndices->i2 = tr ;
				pIndices->i3 = rc ;
				pIndices++ ;

				pIndices->i1 = c ;
				pIndices->i2 = rc ;
				pIndices->i3 = br ;
				pIndices++ ;
			}
		}//if(nNeighbor & NEIGHBOR_DOWN)
		else if(nNeighbor & NEIGHBOR_LEFT)
		{
            if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 위, 좌, 우
			{
				pIndices->i1 = bl ;
				pIndices->i2 = c ;
				pIndices->i3 = bc ;
				pIndices++ ;

				pIndices->i1 = bc ;
				pIndices->i2 = c ;
				pIndices->i3 = br ;
				pIndices++ ;
			}
			else//여기까지는 위, 좌
			{
                pIndices->i1 = c ;
				pIndices->i2 = tr ;
				pIndices->i3 = rc ;
				pIndices++ ;

				pIndices->i1 = bl ;
				pIndices->i2 = c ;
				pIndices->i3 = bc ;
				pIndices++ ;

				pIndices->i1 = c ;
				pIndices->i2 = rc ;
				pIndices->i3 = bc ;
				pIndices++ ;

				pIndices->i1 = bc ;
				pIndices->i2 = rc ;
				pIndices->i3 = br ;
				pIndices++ ;
			}
		}//if(nNeighbor & NEIGHBOR_LEFT)
		else if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 위, 우
		{
			pIndices->i1 = tl ;
			pIndices->i2 = c ;
			pIndices->i3 = lc ;
			pIndices++ ;

			pIndices->i1 = lc ;
			pIndices->i2 = c ;
			pIndices->i3 = bl ;
			pIndices++ ;

			pIndices->i1 = bl ;
			pIndices->i2 = c ;
			pIndices->i3 = bc ;
			pIndices++ ;

			pIndices->i1 = bc ;
			pIndices->i2 = c ;
			pIndices->i3 = br ;
			pIndices++ ;
		}//if(nNeighbor & NEIGHBOR_RIGHT)
		else//여기까지는 위에만 있는경우
		{
			pIndices->i1 = tl ;
			pIndices->i2 = c ;
			pIndices->i3 = lc ;
			pIndices++ ;

			pIndices->i1 = tr ;
			pIndices->i2 = rc ;
			pIndices->i3 = c ;
			pIndices++ ;

			_Make2Triangles(lc, c, bl, bc, pIndices++) ;
			_Make2Triangles(c, rc, bc, br, pIndices++) ;
		}
	}
	else if(nNeighbor & NEIGHBOR_DOWN)
	{
		if(nNeighbor & NEIGHBOR_LEFT)
		{
			if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 아래, 좌, 우
			{
				pIndices->i1 = tl ;
				pIndices->i2 = tr ;
				pIndices->i3 = c ;
				pIndices++ ;

				pIndices->i1 = tc ;
				pIndices->i2 = tr ;
				pIndices->i3 = c ;
				pIndices++ ;
			}
			else//여기까지는 아래, 좌
			{
				pIndices->i1 = tl ;
				pIndices->i2 = tr ;
				pIndices->i3 = c ;
				pIndices++ ;

				pIndices->i1 = tc ;
				pIndices->i2 = tr ;
				pIndices->i3 = c ;
				pIndices++ ;

				pIndices->i1 = c ;
				pIndices->i2 = tr ;
				pIndices->i3 = rc ;
				pIndices++ ;

				pIndices->i1 = c ;
				pIndices->i2 = rc ;
				pIndices->i3 = br ;
				pIndices++ ;
			}
		}//if(nNeighbor & NEIGHBOR_LEFT)
		else if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 아래, 우
		{
			pIndices->i1 = tl ;
			pIndices->i2 = tc ;
			pIndices->i3 = lc ;
			pIndices++ ;

			pIndices->i1 = lc ;
			pIndices->i2 = tc ;
			pIndices->i3 = c ;
			pIndices++ ;

			pIndices->i1 = tc ;
			pIndices->i2 = tr ;
			pIndices->i3 = c ;
			pIndices++ ;

			pIndices->i1 = lc ;
			pIndices->i2 = c ;
			pIndices->i3 = bl ;
			pIndices++ ;
		}//if(nNeighbor & NEIGHBOR_RIGHT)
		else//여기까지는 아래
		{
			_Make2Triangles(tl, tc, lc, c, pIndices++) ;
			_Make2Triangles(tc, tr, c, rc, pIndices++) ;

			pIndices->i1 = lc ;
			pIndices->i2 = c ;
			pIndices->i3 = bl ;
			pIndices++ ;

			pIndices->i1 = c ;
			pIndices->i2 = rc ;
			pIndices->i3 = br ;
			pIndices++ ;
		}
	}
	else if(nNeighbor & NEIGHBOR_LEFT)
	{
		if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 좌, 우
		{
			pIndices->i1 = tl ;
			pIndices->i2 = tc ;
			pIndices->i3 = c ;
			pIndices++ ;

			pIndices->i1 = tc ;
			pIndices->i2 = tr ;
			pIndices->i3 = c ;
			pIndices++ ;

			pIndices->i1 = bl ;
			pIndices->i2 = c ;
			pIndices->i3 = bc ;
			pIndices++ ;

			pIndices->i1 = c ;
			pIndices->i2 = br ;
			pIndices->i3 = bc ;
			pIndices++ ;
		}
		else//여기까지는 좌
		{
			pIndices->i1 = tl ;
			pIndices->i2 = tc ;
			pIndices->i3 = c ;
			pIndices++ ;

			pIndices->i1 = bl ;
			pIndices->i2 = c ;
			pIndices->i3 = bc ;
			pIndices++ ;

			_Make2Triangles(tc, tr, c, rc, pIndices++) ;
			_Make2Triangles(c, rc, bc, br, pIndices++) ;
		}
	}
	else if(nNeighbor & NEIGHBOR_RIGHT)//여기까지는 우
	{
		_Make2Triangles(tl, tc, lc, c, pIndices++) ;
		_Make2Triangles(lc, c, bl, bc, pIndices++) ;

		pIndices->i1 = tc ;
		pIndices->i2 = tr ;
		pIndices->i3 = c ;
		pIndices++ ;

		pIndices->i1 = c ;
		pIndices->i2 = br ;
		pIndices->i3 = bc ;
		pIndices++ ;
	}
}

/*
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//위 && 아래 && 좌 && 우
	if( (nNeighbor & NEIGHBOR_UP) && (nNeighbor & NEIGHBOR_DOWN) && (nNeighbor & NEIGHBOR_LEFT) &&(nNeighbor & NEIGHBOR_RIGHT) )
	{
		pIndices->i1 = tl ;
		pIndices->i2 = tr ;
		pIndices->i3 = c ;
		pIndices++ ;

		pIndices->i1 = bl ;
		pIndices->i2 = c ;
		pIndices->i3 = br ;
		pIndices++ ;

		pIndices->i1 = tl ;
		pIndices->i2 = c ;
		pIndices->i3 = bl ;
		pIndices++ ;

		pIndices->i1 = c ;
		pIndices->i2 = tr ;
		pIndices->i3 = br ;
		pIndices++ ;
	}
	//위 && 아래 && 좌
	if( (nNeighbor & NEIGHBOR_UP) && (nNeighbor & NEIGHBOR_DOWN) && (nNeighbor & NEIGHBOR_LEFT) )
	{
		pIndices->i1 = tl ;
		pIndices->i2 = tr ;
		pIndices->i3 = c ;
		pIndices++ ;

		pIndices->i1 = bl ;
		pIndices->i2 = c ;
		pIndices->i3 = br ;
		pIndices++ ;

		pIndices->i1 = tl ;
		pIndices->i2 = c ;
		pIndices->i3 = bl ;
		pIndices++ ;

		pIndices->i1 = c ;
		pIndices->i2 = tr ;
		pIndices->i3 = br ;
		pIndices++ ;
	}

	else if(nNeighbor & NEIGHBOR_UP)
	{
		pIndices->i1 = tl ;
		pIndices->i2 = c ;
		pIndices->i3 = lc ;
		pIndices++ ;

		pIndices->i1 = tl ;
		pIndices->i2 = tr ;
		pIndices->i3 = c ;
		pIndices++ ;

		pIndices->i1 = tr ;
		pIndices->i2 = rc ;
		pIndices->i3 = c ;
		pIndices++ ;

        _Make2Triangles(lc, c, bl, bc, pIndices++) ;
		_Make2Triangles(c, rc, bc, br, pIndices++) ;
	}
	else if(nNeighbor & NEIGHBOR_DOWN)
	{
		_Make2Triangles(tl, tc, lc, c, pIndices++) ;
		_Make2Triangles(tc, tr, c, rc, pIndices++) ;

		pIndices->i1 = lc ;
		pIndices->i2 = c ;
		pIndices->i3 = bl ;
		pIndices++ ;

		pIndices->i1 = bl ;
		pIndices->i2 = c ;
		pIndices->i3 = br ;
		pIndices++ ;

		pIndices->i1 = c ;
		pIndices->i2 = rc ;
		pIndices->i3 = br ;
		pIndices++ ;
	}
	else if(nNeighbor & NEIGHBOR_LEFT)
	{
		pIndices->i1 = tl ;
		pIndices->i2 = tc ;
		pIndices->i3 = c ;
		pIndices++ ;

		pIndices->i1 = tl ;
		pIndices->i2 = c ;
		pIndices->i3 = bl ;
		pIndices++ ;

		pIndices->i1 = bl ;
		pIndices->i2 = c ;
		pIndices->i3 = bc ;
		pIndices++ ;

		_Make2Triangles(tc, tr, c, rc, pIndices++) ;
		_Make2Triangles(c, rc, bc, br, pIndices++) ;
	}
	else if(nNeighbor & NEIGHBOR_RIGHT)
	{
		_Make2Triangles(tl, tc, lc, c, pIndices++) ;
		_Make2Triangles(lc, c, bl, bc, pIndices++) ;

		pIndices->i1 = tc ;
		pIndices->i2 = tr ;
		pIndices->i3 = c ;
		pIndices++ ;

		pIndices->i1 = c ;
		pIndices->i2 = tr ;
		pIndices->i3 = br ;
		pIndices++ ;

		pIndices->i1 = c ;
		pIndices->i2 = br ;
		pIndices->i3 = bc ;
		pIndices++ ;
	}
	*/

CSecretTileSubTree::CSecretTileSubTree(CSecretTileSubTree *pcParent)
{
	for(int i=0 ; i<4 ; i++)
		m_pcChild[i] = NULL ;
    m_pcParent = pcParent ;

	m_ppppsIndices = NULL ;
	m_ppnNumIndex = NULL ;
	m_nNumLevel = 0 ;
}

CSecretTileSubTree::CSecretTileSubTree(int tl, int tr, int bl, int br)
{
	for(int i=0 ; i<4 ; i++)
		m_pcChild[i] = NULL ;
    m_pcParent = NULL ;

	m_ppppsIndices = NULL ;
	m_ppnNumIndex = NULL ;
	m_nNumLevel = 0 ;

    m_nCorner[CORNER_TL]	= tl ;
	m_nCorner[CORNER_TR]	= tr ;
	m_nCorner[CORNER_BL]	= bl ;
	m_nCorner[CORNER_BR]	= br ;
	m_nCenter				= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] + 
								m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 4;
}

CSecretTileSubTree::~CSecretTileSubTree()
{
	_Release() ;
}

void CSecretTileSubTree::_Release()
{
	int i, n ;
    if(m_ppppsIndices)
	{
		for(i=0 ; i<m_nNumLevel ; i++)
		{
			for(n=0 ; n<MAXNUM_TILEPATCH ; n++)
				SAFE_DELETEARRAY(m_ppppsIndices[i][n]) ;

            SAFE_DELETEARRAY(m_ppppsIndices[i]) ;
			SAFE_DELETEARRAY(m_ppnNumIndex[i]) ;
		}
        SAFE_DELETEARRAY(m_ppppsIndices) ;
		SAFE_DELETEARRAY(m_ppnNumIndex) ;
	}

	for(i=0 ; i<4 ; i++)
		SAFE_DELETE(m_pcChild[i]) ;
}

void CSecretTileSubTree::_SetCorners( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR )
{
	m_nCorner[CORNER_TL] = nCornerTL;
	m_nCorner[CORNER_TR] = nCornerTR;
	m_nCorner[CORNER_BL] = nCornerBL;
	m_nCorner[CORNER_BR] = nCornerBR;
	m_nCenter			 = ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] + 
							 m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 4;
}

CSecretTileSubTree *CSecretTileSubTree::_AddChild(int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR)
{
	CSecretTileSubTree *pcChild = new CSecretTileSubTree(this) ;
	pcChild->_SetCorners( nCornerTL, nCornerTR, nCornerBL, nCornerBR );
	return pcChild ;
}

bool CSecretTileSubTree::_Division(TERRAINVERTEX *pVertices, float fBestHeight, CSecretTile *pcTile, int nNumLevel)
{
	int		nTopEdgeCenter;
	int		nBottomEdgeCenter;
	int		nLeftEdgeCenter;
	int		nRightEdgeCenter;
	int		nCentralPoint;

	// 상단변 가운데
	nTopEdgeCenter		= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] ) / 2;
	// 하단변 가운데 
	nBottomEdgeCenter	= ( m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 2;
	// 좌측변 가운데
	nLeftEdgeCenter		= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_BL] ) / 2;
	// 우측변 가운데
	nRightEdgeCenter	= ( m_nCorner[CORNER_TR] + m_nCorner[CORNER_BR] ) / 2;
	// 한가운데
	nCentralPoint		= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] + 
							m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 4;

    //더이상 분할할수 없을경우
	if((m_nCorner[CORNER_TR]-m_nCorner[CORNER_TL]) <= TILEDIVISION_MINSIZE)
	{
		float l = (float)(nTerrainTileSize*TILEDIVISION_MINSIZE/2) ;
		//float l = pVertices[m_nCorner[CORNER_TR]].pos.x - pVertices[m_nCorner[CORNER_TL]].pos.x ;
        l = l*l ;
		m_fRadius = sqrtf(l+l) ;//2d 평면적인 입장에서 반지름

		int i, n, t ;
		m_ppppsIndices = new TERRAININDEX***[nNumLevel] ;
		m_ppnNumIndex = new int*[nNumLevel] ;
		for(i=0 ; i<nNumLevel ; i++)
		{
			m_ppppsIndices[i] = new TERRAININDEX**[MAXNUM_TILEPATCH] ;
            m_ppnNumIndex[i] = new int[MAXNUM_TILEPATCH] ;
			for(n=0 ; n<MAXNUM_TILEPATCH ; n++)
			{
				m_ppnNumIndex[i][n] = 0 ;

				m_ppppsIndices[i][n] = new TERRAININDEX*[TILEDIVISION_MAXNUMINDEX] ;
				for(t=0 ; t<TILEDIVISION_MAXNUMINDEX ; t++)
					m_ppppsIndices[i][n][t] = NULL ;
			}
		}
		m_nNumLevel = nNumLevel ;
 
		geo::STriangle tri ;
        Vector3 v, vCenter ;
		float r ;

		for(i=0 ; i<nNumLevel ; i++)
		{
			for(n=0 ; n<MAXNUM_TILEPATCH ; n++)
			{
				for(t=0 ; t<pcTile[i].m_anNumIndex[n] ; t++)
				{
					//TRACE("pcTile[%04d] tilePatch[%04d] index[%04d] (%04d,  %04d, %04d)\r\n", i, n, t,
					//	pcTile[i].m_apsIndices[n][t].i1, pcTile[i].m_apsIndices[n][t].i2, pcTile[i].m_apsIndices[n][t].i3) ;

                    //평면적인 시각으로 접근해서 y가 모두 0인 거임.
					tri.set(Vector3(pVertices[pcTile[i].m_apsIndices[n][t].i1].pos.x, 0, pVertices[pcTile[i].m_apsIndices[n][t].i1].pos.z),
						Vector3(pVertices[pcTile[i].m_apsIndices[n][t].i2].pos.x, 0, pVertices[pcTile[i].m_apsIndices[n][t].i2].pos.z),
						Vector3(pVertices[pcTile[i].m_apsIndices[n][t].i3].pos.x, 0, pVertices[pcTile[i].m_apsIndices[n][t].i3].pos.z)) ;

					v = tri.GetMidPos() ;
					r = ((v-tri.avVertex[0].vPos).Magnitude() + (v-tri.avVertex[1].vPos).Magnitude() + (v-tri.avVertex[2].vPos).Magnitude())/3.0f ;
					vCenter.set(pVertices[m_nCenter].pos.x, 0, pVertices[m_nCenter].pos.z) ;

					if((v-vCenter).Magnitude() <= (r+m_fRadius))
					//if((v.x >= (pVertices[m_nCorner[CORNER_TL]].pos.x)) && (v.x <= (pVertices[m_nCorner[CORNER_TR]].pos.x))
					//	&& (v.z >= (pVertices[m_nCorner[CORNER_BL]].pos.z)) && (v.z <= (pVertices[m_nCorner[CORNER_TL]].pos.z)))
					{
						m_ppppsIndices[i][n][m_ppnNumIndex[i][n]++] = &pcTile[i].m_apsIndices[n][t] ;
						//assert(m_ppnNumIndex[i][n] <= (l*l*2*4)) ;
						assert(m_ppnNumIndex[i][n] <= TILEDIVISION_MAXNUMINDEX) ;
						//TRACE("NumLevel=%04d TilePatch=%04d index(%04d %04d %04d\r\n", i, n,
						//	pcTile[i].m_apsIndices[n][t].i1, pcTile[i].m_apsIndices[n][t].i2, pcTile[i].m_apsIndices[n][t].i3) ;
					}
				}
			}
		}

        return false ;
	}

	// 4개의 자식노드 추가
	m_pcChild[CORNER_TL] = _AddChild( m_nCorner[CORNER_TL], nTopEdgeCenter, nLeftEdgeCenter, nCentralPoint );
	m_pcChild[CORNER_TR] = _AddChild( nTopEdgeCenter, m_nCorner[CORNER_TR], nCentralPoint, nRightEdgeCenter );
	m_pcChild[CORNER_BL] = _AddChild( nLeftEdgeCenter, nCentralPoint, m_nCorner[CORNER_BL], nBottomEdgeCenter );
	m_pcChild[CORNER_BR] = _AddChild( nCentralPoint, nRightEdgeCenter, nBottomEdgeCenter, m_nCorner[CORNER_BR] );

    return true ;
}
bool CSecretTileSubTree::_Build(TERRAINVERTEX *pVertices, float fBestHeight, CSecretTile *pcTile, int nNumLevel)
{
	if(_Division(pVertices, fBestHeight, pcTile, nNumLevel))
	{
		float l = (pVertices[m_nCenter].pos.x-pVertices[m_nCorner[CORNER_TL]].pos.x) ;
		l = l*l ;
		m_fRadius = sqrtf(l+l) ;//2d 평면적인 입장에서 반지름

		for(int i=0 ; i<4 ; i++)
			m_pcChild[i]->_Build(pVertices, fBestHeight, pcTile, nNumLevel) ;
	}

    return true ;
}
bool CSecretTileSubTree::Build(TERRAINVERTEX *pVertices, float fBestHeight, CSecretTile *pcTile, int nNumLevel)
{
	return _Build(pVertices, fBestHeight, pcTile, nNumLevel) ;
}

void CSecretTileSubTree::_SetCube(TERRAINVERTEX *pVertices, float height, int tl, geo::SAACube *psCube)
{
	Vector3 vCenter(pVertices[m_nCenter+tl].pos.x, pVertices[m_nCenter+tl].pos.y, pVertices[m_nCenter+tl].pos.z) ;
	float size = fabs(pVertices[m_nCorner[CORNER_TR]+tl].pos.x-pVertices[m_nCorner[CORNER_TL]+tl].pos.x) ;
    psCube->set(vCenter, size, height*2.0f, size) ;
}

int CSecretTileSubTree::_GetIntersectPos(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle *psTriangles, int nTriangles, int nNumLevel, int nTilePatch, int tl)
{
    Vector3 v1, v2 ;
	v1.set(pVertices[m_nCenter+tl].pos.x, 0, pVertices[m_nCenter+tl].pos.z) ;
	v2.set(pvPos->x, 0, pvPos->z) ;
	float l = (v1-v2).Magnitude() ;

	if(l <= (m_fRadius+fRadius))
	{
		int i ;
		if((m_nCorner[CORNER_TR]-m_nCorner[CORNER_TL]) <= TILEDIVISION_MINSIZE)//여기에서 삼각형과 충돌처리를 한다.
		{
			TERRAININDEX sIndex ;
			for(i=0 ; i<m_ppnNumIndex[nNumLevel][nTilePatch] ; i++)
			{
				sIndex.i1 = m_ppppsIndices[nNumLevel][nTilePatch][i]->i1+tl ;
				sIndex.i2 = m_ppppsIndices[nNumLevel][nTilePatch][i]->i2+tl ;
				sIndex.i3 = m_ppppsIndices[nNumLevel][nTilePatch][i]->i3+tl ;

				psTriangles[nTriangles++].set(Vector3(pVertices[sIndex.i1].pos.x, pVertices[sIndex.i1].pos.y, pVertices[sIndex.i1].pos.z),
					Vector3(pVertices[sIndex.i2].pos.x, pVertices[sIndex.i2].pos.y, pVertices[sIndex.i2].pos.z),
					Vector3(pVertices[sIndex.i3].pos.x, pVertices[sIndex.i3].pos.y, pVertices[sIndex.i3].pos.z)) ;
			}
			return nTriangles ;
		}

		for(i=0 ; i<4 ; i++)
		{
			if(m_pcChild[i])
				nTriangles = m_pcChild[i]->_GetIntersectPos(pvPos, fRadius, pVertices, psTriangles, nTriangles, nNumLevel, nTilePatch, tl) ;
		}
	}
	return nTriangles ;
}
int CSecretTileSubTree::GetIntersectPos(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle *psTriangles, int nTriangles, int nNumLevel, int nTilePatch, int tl)
{
    return _GetIntersectPos(pvPos, fRadius, pVertices, psTriangles, nTriangles, nNumLevel, nTilePatch, tl) ;
}

bool bIntersectTri_TileSubTree = false ;
Vector3 vIntersectTriPos_TileSubTree ;
geo::STriangle sTri ;

int CSecretTileSubTree::_GetIntersectLine(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle *psTriangles, int nTriangles, int nNumLevel, int nTilePatch, int tl, float fBestHeight)
{
	//geo::SAACube sCube ;
	//_SetCube(pVertices, fBestHeight, tl, &sCube) ;

	if(bIntersectTri_TileSubTree)
		return nTriangles ;

	float r = (m_fRadius*m_fRadius) ;
	r = sqrtf(r+r) ;

	Vector3 vPos(pVertices[m_nCenter+tl].pos.x, pVertices[m_nCenter+tl].pos.y, pVertices[m_nCenter+tl].pos.z) ;
	if(IntersectLineToSphere(psLine, vPos, r))
	//if(IntersectLineToCube(psLine, &sCube))
	{
		int i ;
		if((m_nCorner[CORNER_TR]-m_nCorner[CORNER_TL]) <= TILEDIVISION_MINSIZE)//여기에서 삼각형과 충돌처리를 한다.
		{
			TERRAININDEX sIndex ;

			for(i=0 ; i<m_ppnNumIndex[nNumLevel][nTilePatch] ; i++)
			{
				sIndex.i1 = m_ppppsIndices[nNumLevel][nTilePatch][i]->i1+tl ;
				sIndex.i2 = m_ppppsIndices[nNumLevel][nTilePatch][i]->i2+tl ;
				sIndex.i3 = m_ppppsIndices[nNumLevel][nTilePatch][i]->i3+tl ;

				psTriangles->set(Vector3(pVertices[sIndex.i1].pos.x, pVertices[sIndex.i1].pos.y, pVertices[sIndex.i1].pos.z),
								Vector3(pVertices[sIndex.i2].pos.x, pVertices[sIndex.i2].pos.y, pVertices[sIndex.i2].pos.z),
								Vector3(pVertices[sIndex.i3].pos.x, pVertices[sIndex.i3].pos.y, pVertices[sIndex.i3].pos.z)) ;

				if(IntersectLinetoTriangle(*psLine, *psTriangles, vIntersectTriPos_TileSubTree) == geo::INTERSECT_POINT)
				{
					psTriangles->avVertex[0].vNormal.set(pVertices[sIndex.i1].normal.x, pVertices[sIndex.i1].normal.y, pVertices[sIndex.i1].normal.z) ;
					psTriangles->avVertex[1].vNormal.set(pVertices[sIndex.i2].normal.x, pVertices[sIndex.i2].normal.y, pVertices[sIndex.i2].normal.z) ;
					psTriangles->avVertex[2].vNormal.set(pVertices[sIndex.i3].normal.x, pVertices[sIndex.i3].normal.y, pVertices[sIndex.i3].normal.z) ;

					psTriangles->avVertex[0].vTex.set(pVertices[sIndex.i1].t.x, pVertices[sIndex.i1].t.y) ;
					psTriangles->avVertex[1].vTex.set(pVertices[sIndex.i2].t.x, pVertices[sIndex.i2].t.y) ;
					psTriangles->avVertex[2].vTex.set(pVertices[sIndex.i3].t.x, pVertices[sIndex.i3].t.y) ;
					bIntersectTri_TileSubTree = true ;
					return nTriangles ;
				}

				//if(nTriangles >= nMaxNumTriangle)
				//{
				//	bStopIntersect = true ;
				//	return nTriangles ;
				//}



				//psTriangles[nTriangles].set(Vector3(pVertices[sIndex.i1].pos.x, pVertices[sIndex.i1].pos.y, pVertices[sIndex.i1].pos.z),
				//							Vector3(pVertices[sIndex.i2].pos.x, pVertices[sIndex.i2].pos.y, pVertices[sIndex.i2].pos.z),
				//							Vector3(pVertices[sIndex.i3].pos.x, pVertices[sIndex.i3].pos.y, pVertices[sIndex.i3].pos.z)) ;

				//psTriangles[nTriangles].avVertex[0].vNormal.set(pVertices[sIndex.i1].normal.x, pVertices[sIndex.i1].normal.y, pVertices[sIndex.i1].normal.z) ;
				//psTriangles[nTriangles].avVertex[1].vNormal.set(pVertices[sIndex.i2].normal.x, pVertices[sIndex.i2].normal.y, pVertices[sIndex.i2].normal.z) ;
				//psTriangles[nTriangles].avVertex[2].vNormal.set(pVertices[sIndex.i3].normal.x, pVertices[sIndex.i3].normal.y, pVertices[sIndex.i3].normal.z) ;

				//psTriangles[nTriangles].avVertex[0].vTex.set(pVertices[sIndex.i1].t.x, pVertices[sIndex.i1].t.y) ;
				//psTriangles[nTriangles].avVertex[1].vTex.set(pVertices[sIndex.i2].t.x, pVertices[sIndex.i2].t.y) ;
				//psTriangles[nTriangles].avVertex[2].vTex.set(pVertices[sIndex.i3].t.x, pVertices[sIndex.i3].t.y) ;

				//nTriangles++ ;
			}
			return nTriangles ;
		}

		for(i=0 ; i<4 ; i++)
		{
			if(m_pcChild[i])
				nTriangles = m_pcChild[i]->_GetIntersectLine(psLine, pVertices, psTriangles, nTriangles, nNumLevel, nTilePatch, tl, fBestHeight) ;
		}
	}
	return nTriangles ;
}
int CSecretTileSubTree::GetIntersectLine(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle *psTriangles, int nTriangles, int nNumLevel, int nTilePatch, int tl, float fBestHeight)
{
	bIntersectTri_TileSubTree = false ;
	return _GetIntersectLine(psLine, pVertices, psTriangles, nTriangles, nNumLevel, nTilePatch, tl, fBestHeight) ;
}