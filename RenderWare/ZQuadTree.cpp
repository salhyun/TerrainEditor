#include "ZQuadTree.h"
#include "windows.h"
#include <stdio.h>

#include "SecretTerrain.h"
#include "MathOrdinary.h"

const float fMinRadius_inQuadTree = sqrtf(0.5f) ;

// 최초 루트노드 생성자
ZQuadTree::ZQuadTree( int cx, int cy )//cx, cy 이것은 길이단위가 아니고 버텍스 가로 세로 갯수라고 해야 되겠다.
{
	int		i;
	m_nCenter = 0;
	for( i = 0 ; i < 4 ; i++ )
	{
		m_pChild[i] = NULL;
		m_pNeighbor[i] = NULL ;
	}

	// 루트노드의 4개 코너값 설정
	m_nCorner[CORNER_TL]	= 0;
	m_nCorner[CORNER_TR]	= cx - 1;
	m_nCorner[CORNER_BL]	= cx * ( cy - 1 );
	m_nCorner[CORNER_BR]	= cx * cy - 1;
	m_nCenter				= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] + 
								m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 4;
	
	for( i = 0 ; i < 8 ; i++ )
		m_apTriangles[i] = NULL ;
	m_nNumTriangle = 0 ;

	m_bCulled = FALSE;
	m_fRadius = 0.0f;
}

// 하위 자식노드 생성자
ZQuadTree::ZQuadTree( /*ZQuadTree* pParent*/ )
{
	int		i;
	m_nCenter = 0;
	for( i = 0 ; i < 4 ; i++ )
	{
		m_pChild[i] = NULL;
		m_pNeighbor[i] = NULL ;
		m_nCorner[i] = 0;
	}

	for( i = 0 ; i < 8 ; i++ )
		m_apTriangles[i] = NULL ;
	m_nNumTriangle = 0 ;

	m_bCulled = FALSE;
	m_fRadius = 0.0f;
}

// 소멸자
ZQuadTree::~ZQuadTree()
{
	_Destroy();
}

// 메모리에서 쿼드트리를 삭제한다.
void	ZQuadTree::_Destroy()
{
	// 자식 노드들을 소멸 시킨다.
	for( int i = 0 ; i < 4 ; i++ )
	{
		if(m_pChild[i])
			delete m_pChild[i] ;
	}
}


// 4개의 코너값을 셋팅한다.
BOOL	ZQuadTree::_SetCorners( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR )
{
	m_nCorner[CORNER_TL] = nCornerTL;
	m_nCorner[CORNER_TR] = nCornerTR;
	m_nCorner[CORNER_BL] = nCornerBL;
	m_nCorner[CORNER_BR] = nCornerBR;
	m_nCenter			 = ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] + 
							 m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 4;
	return TRUE;
}


// 자식 노드를 추가한다.
ZQuadTree*	ZQuadTree::_AddChild( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR )
{
	ZQuadTree*	pChild;

	pChild = new ZQuadTree( /*this*/ );
	pChild->_SetCorners( nCornerTL, nCornerTR, nCornerBL, nCornerBR );

	return pChild;
}

// Quadtree를 4개의 자식 트리로 부분분할(subdivide)한다.
BOOL	ZQuadTree::_SubDivide()
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
	if( (m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL]) <= 1 )
	{
		m_fRadius = fMinRadius_inQuadTree ;
		return FALSE;
	}

	// 4개의 자식노드 추가
	m_pChild[CORNER_TL] = _AddChild( m_nCorner[CORNER_TL], nTopEdgeCenter, nLeftEdgeCenter, nCentralPoint );
	m_pChild[CORNER_TR] = _AddChild( nTopEdgeCenter, m_nCorner[CORNER_TR], nCentralPoint, nRightEdgeCenter );
	m_pChild[CORNER_BL] = _AddChild( nLeftEdgeCenter, nCentralPoint, m_nCorner[CORNER_BL], nBottomEdgeCenter );
	m_pChild[CORNER_BR] = _AddChild( nCentralPoint, nRightEdgeCenter, nBottomEdgeCenter, m_nCorner[CORNER_BR] );
	
	return TRUE;
}

// 출력할 폴리곤의 인덱스를 생성한다.
int		ZQuadTree::_GenTriIndex( int nTris, LPVOID pIndex, TERRAINVERTEX *pHeightMap, CSecretFrustum* pcSecretFrustum )
{
	// 컬링된 노드라면 그냥 리턴
	if( m_bCulled )
	{
		m_bCulled = FALSE;
		return nTris;
	}

	Vector3 vCameraPosition=pcSecretFrustum->GetCameraPosition() ;

	// 현재 시점에서 출력되어야 하는가? 아니면 더 분할해야 되는가?
	//if( _IsVisible() )
	if(_IsVisible(pHeightMap, vCameraPosition, 0.1f))
	{
#ifdef _USE_INDEX16
		LPWORD p = ((LPWORD)pIndex) + nTris * 3;
#define CAST (WORD)
#else
		LPDWORD p = ((LPDWORD)pIndex) + nTris * 3;
#define CAST (DWORD)
#endif

		if((m_nCorner[CORNER_TR]-m_nCorner[CORNER_TL]) <= 1)//최하위 노드일경우는 그냥 출력이다.
		{
			*p++ = CAST m_nCorner[0];
			*p++ = CAST m_nCorner[1];
			*p++ = CAST m_nCorner[2];
			nTris++;
			*p++ = CAST m_nCorner[2];
			*p++ = CAST m_nCorner[1];
			*p++ = CAST m_nCorner[3];
			nTris++;

			return nTris;
		}

		int i ;
		bool abEnable[4] ;

		//4개의 이웃노드가 현재LODLevel에서 출력되어야 되는지 테스트
		for(i=0 ; i<4 ; i++)
		{
			abEnable[i] = true ;

			if(m_pNeighbor[i])
				abEnable[i] = m_pNeighbor[i]->_IsVisible(pHeightMap, vCameraPosition, 0.1f) ;
		}

		//현재LODLevel 에서 모두 출력가능하다면 분할필요없이 예정대로 출력이다.
		if(abEnable[0] && abEnable[1] && abEnable[2] && abEnable[3])
		{
			*p++ = CAST m_nCorner[0];
			*p++ = CAST m_nCorner[1];
			*p++ = CAST m_nCorner[2];
			nTris++;
			*p++ = CAST m_nCorner[2];
			*p++ = CAST m_nCorner[1];
			*p++ = CAST m_nCorner[3];
			nTris++;

			return nTris;
		}

		int ntemp ;

        //Remark : 분할삼각형을 만들때 시계방향으로 만드는것을 잊지마라!

		if(!abEnable[NEIGHBORNODE_UP])//이웃노드중 윗부분의 노드 분할
		{
            ntemp = (m_nCorner[CORNER_TL]+m_nCorner[CORNER_TR])/2 ;

			*p++ = CAST m_nCenter ;
			*p++ = CAST m_nCorner[CORNER_TL] ;
			*p++ = CAST ntemp ;
			nTris++ ;

            *p++ = CAST m_nCenter ;
			*p++ = CAST ntemp ;
			*p++ = CAST m_nCorner[CORNER_TR] ;
			nTris++ ;
		}
		else
		{
			*p++ = CAST m_nCenter ;
			*p++ = CAST m_nCorner[CORNER_TL] ;
			*p++ = CAST m_nCorner[CORNER_TR] ;
			nTris++ ;
		}

		if(!abEnable[NEIGHBORNODE_DOWN])//이웃노드중 아래부분의 노드 분할
		{
			ntemp = (m_nCorner[CORNER_BL]+m_nCorner[CORNER_BR])/2 ;

			*p++ = CAST m_nCenter ;
			*p++ = CAST ntemp ;
			*p++ = CAST m_nCorner[CORNER_BL] ;
			nTris++ ;

			*p++ = CAST m_nCenter ;
			*p++ = CAST m_nCorner[CORNER_BR] ;
			*p++ = CAST ntemp ;
			nTris++ ;
		}
		else
		{
			*p++ = CAST m_nCenter ;
			*p++ = CAST m_nCorner[CORNER_BR] ;
			*p++ = CAST m_nCorner[CORNER_BL] ;
			nTris++ ;
		}

		if(!abEnable[NEIGHBORNODE_LEFT])//이웃노드중 왼쪽부분의 노드 분할
		{
			ntemp = (m_nCorner[CORNER_TL]+m_nCorner[CORNER_BL])/2 ;

			*p++ = CAST m_nCenter ;
			*p++ = CAST m_nCorner[CORNER_BL] ;
			*p++ = CAST ntemp ;
			nTris++ ;

			*p++ = CAST m_nCenter ;
			*p++ = CAST ntemp ;
			*p++ = CAST m_nCorner[CORNER_TL] ;
			nTris++ ;
		}
		else
		{
			*p++ = CAST m_nCenter ;
			*p++ = CAST m_nCorner[CORNER_BL] ;
			*p++ = CAST m_nCorner[CORNER_TL] ;
			nTris++ ;
		}

		if(!abEnable[NEIGHBORNODE_RIGHT])//이웃노드중 오른쪽부분의 노드 분할
		{
			ntemp = (m_nCorner[CORNER_TR]+m_nCorner[CORNER_BR])/2 ;

			*p++ = CAST m_nCenter ;
			*p++ = CAST m_nCorner[CORNER_TR] ;
			*p++ = CAST ntemp ;
			nTris++ ;

			*p++ = CAST m_nCenter ;
			*p++ = CAST ntemp ;
			*p++ = CAST m_nCorner[CORNER_BR] ;
			nTris++ ;
		}
		else
		{
			*p++ = CAST m_nCenter ;
			*p++ = CAST m_nCorner[CORNER_TR] ;
			*p++ = CAST m_nCorner[CORNER_BR] ;
			nTris++ ;
		}

		return nTris ;
	}

	// 자식 노드들 검색
	if( m_pChild[CORNER_TL] ) nTris = m_pChild[CORNER_TL]->_GenTriIndex( nTris, pIndex, pHeightMap, pcSecretFrustum );
	if( m_pChild[CORNER_TR] ) nTris = m_pChild[CORNER_TR]->_GenTriIndex( nTris, pIndex, pHeightMap, pcSecretFrustum  );
	if( m_pChild[CORNER_BL] ) nTris = m_pChild[CORNER_BL]->_GenTriIndex( nTris, pIndex, pHeightMap, pcSecretFrustum  );
	if( m_pChild[CORNER_BR] ) nTris = m_pChild[CORNER_BR]->_GenTriIndex( nTris, pIndex, pHeightMap, pcSecretFrustum  );

	return nTris;
}

int ZQuadTree::_IsInFrustum( TERRAINVERTEX* pHeightMap, CSecretFrustum* pcSecretFrustum )
{
	BOOL	b[4];
	BOOL	bInSphere;

	// 경계구안에 있는가?
    //	if( m_fRadius == 0.0f ) g_pLog->Log( "Index:[%d], Radius:[%f]",m_nCenter, m_fRadius );
	Vector3 v=VectorConvert((D3DXVECTOR3 *)(pHeightMap+m_nCenter)) ;
	bInSphere = pcSecretFrustum->IsInSphere(v, m_fRadius );
	if( !bInSphere ) return FRUSTUM_OUT;	// 경계구 안에 없으면 점단위의 프러스텀 테스트 생략

	// 쿼드트리의 4군데 경계 프러스텀 테스트
	v=VectorConvert((D3DXVECTOR3*)(pHeightMap+m_nCorner[0])) ;
	b[0] = pcSecretFrustum->IsIn(v);

	v=VectorConvert((D3DXVECTOR3*)(pHeightMap+m_nCorner[1])) ;
	b[1] = pcSecretFrustum->IsIn(v);
	
	v=VectorConvert((D3DXVECTOR3*)(pHeightMap+m_nCorner[2])) ;
	b[2] = pcSecretFrustum->IsIn(v);

	v=VectorConvert((D3DXVECTOR3*)(pHeightMap+m_nCorner[3])) ;
	b[3] = pcSecretFrustum->IsIn(v);

	// 4개모두 프러스텀 안에 있음
	if( (b[0] + b[1] + b[2] + b[3]) == 4 ) return FRUSTUM_COMPLETELY_IN;

	// 일부분이 프러스텀에 있는 경우
	return FRUSTUM_PARTIALLY_IN;
}

// _IsInFrustum()함수의 결과에 따라 프러스텀 컬링 수행
void	ZQuadTree::_FrustumCull( TERRAINVERTEX* pHeightMap, CSecretFrustum* pcSecretFrustum )
{
	int ret;

	ret = _IsInFrustum( pHeightMap, pcSecretFrustum );
	switch( ret )
	{
		case FRUSTUM_COMPLETELY_IN :	// 프러스텀에 완전포함, 하위노드 검색 필요없음
			m_bCulled = FALSE;
			return;
		case FRUSTUM_PARTIALLY_IN :		// 프러스텀에 일부포함, 하위노드 검색 필요함
			m_bCulled = FALSE;
			break;
		case FRUSTUM_OUT :				// 프러스텀에서 완전벗어남, 하위노드 검색 필요없음
			m_bCulled = TRUE;
			return;
	}
	if( m_pChild[0] ) m_pChild[0]->_FrustumCull( pHeightMap, pcSecretFrustum );
	if( m_pChild[1] ) m_pChild[1]->_FrustumCull( pHeightMap, pcSecretFrustum );
	if( m_pChild[2] ) m_pChild[2]->_FrustumCull( pHeightMap, pcSecretFrustum );
	if( m_pChild[3] ) m_pChild[3]->_FrustumCull( pHeightMap, pcSecretFrustum );
}

// 쿼드트리를 만든다.
//m_nCorner[] 의 값은 높이맵의 인덱스값이다. 다른말로 오프셋값 왼쪽상단부터 0 에서 오른쪽 하단 Width*Height
BOOL	ZQuadTree::_BuildQuadTree( TERRAINVERTEX* pHeightMap )
{
	if( _SubDivide() )
	{
		// 좌측상단과, 우측 하단의 거리를 구한다.
		D3DXVECTOR3 v = *((D3DXVECTOR3*)(pHeightMap+m_nCorner[CORNER_TL])) - 
					    *((D3DXVECTOR3*)(pHeightMap+m_nCorner[CORNER_BR]));
		// v의 거리값이 이 노드를 감싸는 경계구의 지름이므로, 
		// 2로 나누어 반지름을 구한다.
		m_fRadius	  = D3DXVec3Length( &v ) / 2.0f;

		m_pChild[CORNER_TL]->_BuildQuadTree( pHeightMap );
		m_pChild[CORNER_TR]->_BuildQuadTree( pHeightMap );
		m_pChild[CORNER_BL]->_BuildQuadTree( pHeightMap );
		m_pChild[CORNER_BR]->_BuildQuadTree( pHeightMap );
	}

	return TRUE;
}

bool ZQuadTree::Build(TERRAINVERTEX *pHeightMap)
{
	_BuildQuadTree(pHeightMap) ;

	//Root Node
	//m_nCorner[CORNER_TR]+1 : Pitch
	//m_nCorner[CORNER_BL]+1 : Height
	_BuildNeighborNode(this, pHeightMap, m_nCorner[CORNER_TR]+1, (m_nCorner[CORNER_BR]+1)/(m_nCorner[CORNER_TR]+1)) ;

	return true ;
}

//	삼각형의 인덱스를 만들고, 출력할 삼각형의 개수를 반환한다.
int		ZQuadTree::GenerateIndex( LPVOID pIndex, TERRAINVERTEX *pHeightMap, CSecretFrustum* pcSecretFrustum )
{
//	_FrustumCull( pHeightMap, pFrustum );

	return _GenTriIndex( 0, pIndex, pHeightMap, pcSecretFrustum );
}

int ZQuadTree::_GetLODLevel(TERRAINVERTEX* pHeightMap, Vector3 &vCamera, float fLODRatio)
{
	Vector3 vPos = VectorConvert((D3DXVECTOR3 *)(pHeightMap+m_nCenter)) ;
	float fLevel = (vCamera-vPos).Magnitude()*fLODRatio ;

	if(fLevel > 1)
        return (int)(fLevel+0.5f) ;

	return 1 ;
}

//현재 시점에서 보여야 할지 아니면 더 분할해야 할지를 결정하는 것이기 때문에
//현재타일의 반지름 >= (vCamera-vPos).Length * coef 되면 더 분할해야되고
//현재타일의 반지름 <= (vCamera-vPos).Length * coef 되면 현재시점에서 보여진다.
bool ZQuadTree::_IsVisible(TERRAINVERTEX* pHeightMap, Vector3 &vCamera, float fLODRatio)
{
	//int ndetail = GetLODLevel(pHeightMap, vCamera, fLODRatio) ;
    //int ndist = (m_nCorner[CORNER_TR]-m_nCorner[CORNER_TL]) ;

	fLODRatio = 1.0f ;

	D3DXVECTOR3 v1 = (pHeightMap+m_nCorner[CORNER_TR])->pos ;
	D3DXVECTOR3 v2 = (pHeightMap+m_nCorner[CORNER_TL])->pos ;

	float dist = D3DXVec3Length(&(v1-v2)) ;

	float level ;
	Vector3 vPos = VectorConvert((D3DXVECTOR3 *)(pHeightMap+m_nCenter)) ;
	level = (vCamera-vPos).Magnitude() ;//*fLODRatio ;
	level *= fLODRatio ;

	float radius = m_fRadius*2.0f ;

	bool ret = ( radius <= level ) ;

	//TRACE("(%02d %02d %02d %02d) is %d\r\n", m_nCorner[CORNER_TL], m_nCorner[CORNER_TR], m_nCorner[CORNER_BL], m_nCorner[CORNER_BR], ret) ;

	//bool ret = ( dist  <= GetLODLevel(pHeightMap, vCamera, fLODRatio)) ;

	//g_cDebugMsg.SetDebugMsg(22, "vCamera(%07.03f %07.03f %07.03f)", enumVector(vCamera)) ;

	//return ((m_nCorner[CORNER_TR]-m_nCorner[CORNER_TL]) <= GetLODLevel(pHeightMap, vCamera, fLODRatio)) ;
	//return ( (m_fRadius*2.0f) <= GetLODLevel(pHeightMap, vCamera, fLODRatio)) ;
	return ret ;
}

int ZQuadTree::_GetNeighborNodeIndex(int nEdge, int nPitch, int nHeight, int *pnEdge)
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
	case NEIGHBORNODE_UP :
		pnEdge[0] = anEdge[0]-(nEdgeHeight*nPitch) ;
		pnEdge[1] = anEdge[1]-(nEdgeHeight*nPitch) ;
		pnEdge[2] = anEdge[0] ;
		pnEdge[3] = anEdge[1] ;

		break ;

	case NEIGHBORNODE_DOWN :
		pnEdge[0] = anEdge[2] ;
		pnEdge[1] = anEdge[3] ;
		pnEdge[2] = anEdge[2]+(nEdgeHeight*nPitch) ;
		pnEdge[3] = anEdge[3]+(nEdgeHeight*nPitch) ;

		break ;

	case NEIGHBORNODE_LEFT :
		pnEdge[0] = anEdge[0]-nGap ;
		pnEdge[1] = anEdge[0] ;
		pnEdge[2] = anEdge[2]-nGap ;
		pnEdge[3] = anEdge[2] ;

		break ;

	case NEIGHBORNODE_RIGHT :
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

inline bool IsInRect(RECT *pRect, POINT *pPoint)
{
	if((pPoint->x >= pRect->left) && (pPoint->x <= pRect->right) && (pPoint->y <= pRect->top) && (pPoint->y >= pRect->bottom))
		return true ;
	return false ;
}

//자식노드 4개의 엣지 위치값(코너인덱스값과 pHeightMap값을 가지고 얻음)
//현재의 Center의 위치 와 비교하면서 코너값이 서로 일치하는 노드에서 리턴하게끔 되어있다.
//모든 노드를 순차적으로 검색한다.
//Remark : 아직 컬링되지않는 상태이기 때문에 모든노드중에 선택되어진다.
ZQuadTree *ZQuadTree::_FindNode(TERRAINVERTEX *pHeightMap, int nTL, int nTR, int nBL, int nBR)
{
	//현재 노드와 일치하면 리턴한다.
	if((m_nCorner[0] == nTL) && (m_nCorner[1] == nTR) && (m_nCorner[2] == nBL) && (m_nCorner[3] == nBR))
		return this ;

	if(m_pChild[0])//자식노드가 있다면
	{
		//RECT rt ;
		//POINT pt ;
		geo::SPoint point ;
		geo::SRect rect ;
		int i, nCenter ;

		nCenter = (nTL+nTR+nBL+nBR)/4 ;

		point.x = pHeightMap[nCenter].pos.x ;// (int)pHeightMap[nCenter].pos.x ;
		point.y = pHeightMap[nCenter].pos.z ;//(int)pHeightMap[nCenter].pos.z ;

		//4개의 자식노드를 검색한다.
		for(i=0 ; i<4 ; i++)
		{
			//Rect를 구해서 
			//SetRect(&rt, (int)pHeightMap[m_pChild[i]->m_nCorner[CORNER_TL]].pos.x,
			//			 (int)pHeightMap[m_pChild[i]->m_nCorner[CORNER_TL]].pos.z,
			//			 (int)pHeightMap[m_pChild[i]->m_nCorner[CORNER_BR]].pos.x,
			//			 (int)pHeightMap[m_pChild[i]->m_nCorner[CORNER_BR]].pos.z) ;

			rect.set(pHeightMap[m_pChild[i]->m_nCorner[CORNER_TL]].pos.x,
				pHeightMap[m_pChild[i]->m_nCorner[CORNER_BR]].pos.x,
				pHeightMap[m_pChild[i]->m_nCorner[CORNER_TL]].pos.z,				
				pHeightMap[m_pChild[i]->m_nCorner[CORNER_BR]].pos.z) ;

			//현재의 Center위치값이 Rect안에 있는 점이라면 자식노드를 따라서 재귀호출을 계속한다.
			if((point.x >= rect.left) && (point.x <= rect.right) && (point.y <= rect.top) && (point.y >= rect.bottom))
				return m_pChild[i]->_FindNode(pHeightMap, nTL, nTR, nBL, nBR) ;

			//if(IsInRect(&rt, &pt))
			//	return m_pChild[i]->FindNode(pHeightMap, nTL, nTR, nBL, nBR) ;
		}
	}
	return NULL ;
}

void ZQuadTree::_BuildNeighborNode(ZQuadTree *pRoot, TERRAINVERTEX *pHeightMap, int nPitch, int nHeight)
{
	int i, nCenter ;
	int anEdge[4] ;

	//TRACE("***4 conner are (%02d %02d %02d %02d)\r\n", m_nCorner[0], m_nCorner[1], m_nCorner[2], m_nCorner[3]) ;

    for(i=0 ; i<4 ; i++)
	{
		//현재의 4귀퉁이를 모두구하고
		anEdge[0] = m_nCorner[0] ;
		anEdge[1] = m_nCorner[1] ;
		anEdge[2] = m_nCorner[2] ;
		anEdge[3] = m_nCorner[3] ;

		//4귀퉁이를 가지고 위[0], 아래[1], 왼쪽[2], 오른쪽[3] 의 이웃노드를 구한다.
        nCenter = _GetNeighborNodeIndex(i, nPitch, nHeight, anEdge) ;

		//TRACE("    Neighbor conner is (%02d %02d %02d %02d) is %02d\r\n", anEdge[0], anEdge[1], anEdge[2], anEdge[3], nCenter) ;

		if(nCenter >= 0)//이웃노드를 구하는데 성공했다면 실제 노드포인터를 얻어낸다.
		{
			m_pNeighbor[i] = pRoot->_FindNode(pHeightMap, anEdge[0], anEdge[1], anEdge[2], anEdge[3]) ;
			//if(m_pNeighbor[i] != NULL)
			//	TRACE("    (%02d %02d %02d %02d) Neighbor is founded\r\n", anEdge[0], anEdge[1], anEdge[2], anEdge[3]) ;
			//else
			//	TRACE("    (%02d %02d %02d %02d) Neighbor isn't founded\r\n", anEdge[0], anEdge[1], anEdge[2], anEdge[3]) ;
		}
	}

	//TRACE("4 conner are (%02d %02d %02d %02d)***\r\n", m_nCorner[0], m_nCorner[1], m_nCorner[2], m_nCorner[3]) ;

	if(m_pChild[0])//자식노드에서도 재귀호출로 모두 이웃노드를 구하게 된다.
	{
		for(i=0 ; i<4 ; i++)
			m_pChild[i]->_BuildNeighborNode(pRoot, pHeightMap, nPitch, nHeight) ;
	}
}

void ZQuadTree::_SetTriangle(geo::STriangle *psTriangle, TERRAINVERTEX *pVertex0, TERRAINVERTEX *pVertex1, TERRAINVERTEX *pVertex2)
{
	psTriangle->avVertex[0].vPos.set(pVertex0->pos.x, pVertex0->pos.y, pVertex0->pos.z) ;
	psTriangle->avVertex[0].vNormal.set(pVertex0->normal.x, pVertex0->normal.y, pVertex0->normal.z) ;
	psTriangle->avVertex[0].vTex.set(pVertex0->t.x, pVertex0->t.y) ;

	psTriangle->avVertex[1].vPos.set(pVertex1->pos.x, pVertex1->pos.y, pVertex1->pos.z) ;
	psTriangle->avVertex[1].vNormal.set(pVertex1->normal.x, pVertex1->normal.y, pVertex1->normal.z) ;
	psTriangle->avVertex[1].vTex.set(pVertex1->t.x, pVertex1->t.y) ;

	psTriangle->avVertex[2].vPos.set(pVertex2->pos.x, pVertex2->pos.y, pVertex2->pos.z) ;
	psTriangle->avVertex[2].vNormal.set(pVertex2->normal.x, pVertex2->normal.y, pVertex2->normal.z) ;
	psTriangle->avVertex[2].vTex.set(pVertex2->t.x, pVertex2->t.y) ;

	psTriangle->sPlane.MakePlane(psTriangle->avVertex[0].vPos, psTriangle->avVertex[1].vPos, psTriangle->avVertex[2].vPos) ;

	psTriangle->bSelected = false ;
}

int ZQuadTree::GenerateTriangles(TERRAINMAP *pTerrainMap, Vector3 *pvCameraPos)
{
	return _GenerateTriangles(pTerrainMap->pVertices, pTerrainMap->pIndices, pTerrainMap->psTriangles, pvCameraPos, 0) ;
}

int ZQuadTree::_GenerateTriangles(TERRAINVERTEX *pVertices, TERRAININDEX *pIndices, geo::STriangle *pTriangles, Vector3 *pvCameraPos, int nTris)
{
    m_nNumTriangle = 0 ;

	if( m_bCulled )
	{
		m_bCulled = FALSE;
		return nTris ;
	}

	LPWORD p ;
	geo::STriangle *pTris ;

	//최하단 노드까지 내려왔다는 것은 출력해줘야 된다는 것임.
	if((m_nCorner[CORNER_TR]-m_nCorner[CORNER_TL]) <= 1)//최하위 노드일경우는 그냥 출력이다.
	{
		p = ((LPWORD)pIndices) + nTris * 3 ;
		pTris = pTriangles + nTris ;

		*p++ = (WORD)m_nCorner[0];
		*p++ = (WORD)m_nCorner[1];
		*p++ = (WORD)m_nCorner[2];

		//cup 연산에서 사용되어질 삼각형을 만든다.
		_SetTriangle(pTris, &pVertices[m_nCorner[0]], &pVertices[m_nCorner[1]], &pVertices[m_nCorner[2]]) ;

		m_apTriangles[m_nNumTriangle++] = pTris++ ;

		nTris++;

		*p++ = (WORD)m_nCorner[2];
		*p++ = (WORD)m_nCorner[1];
		*p++ = (WORD)m_nCorner[3];

		//cup 연산에서 사용되어질 삼각형을 만든다.
		_SetTriangle(pTris, &pVertices[m_nCorner[2]], &pVertices[m_nCorner[1]], &pVertices[m_nCorner[3]]) ;

		m_apTriangles[m_nNumTriangle++] = pTris++ ;

		nTris++;

		return nTris;
	}

	//현재 시점에서 출력되어야 하는가? 아니면 더 분할해야 되는가?
	//_IsVisible 이 실패를 하면 자식노드로 내려가는데 자식노드가 없다.
	//if( _IsVisible() )
	if(_IsVisible(pVertices, *pvCameraPos, 0.1f))
	{
		p = ((LPWORD)pIndices) + nTris * 3 ;
		pTris = pTriangles + nTris ;

		int i ;
		bool abEnable[4] ;

		//4개의 이웃노드가 현재LODLevel에서 출력되어야 되는지 테스트
		for(i=0 ; i<4 ; i++)
		{
			abEnable[i] = true ;

			if(m_pNeighbor[i])
				abEnable[i] = m_pNeighbor[i]->_IsVisible(pVertices, *pvCameraPos, 0.1f) ;
		}

		//현재LODLevel 에서 모두 출력가능하다면 분할필요없이 예정대로 출력이다.
		if(abEnable[0] && abEnable[1] && abEnable[2] && abEnable[3])
		{
			*p++ = (WORD)m_nCorner[0];
			*p++ = (WORD)m_nCorner[1];
			*p++ = (WORD)m_nCorner[2];

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[m_nCorner[0]], &pVertices[m_nCorner[1]], &pVertices[m_nCorner[2]]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++;


			*p++ = (WORD)m_nCorner[2];
			*p++ = (WORD)m_nCorner[1];
			*p++ = (WORD)m_nCorner[3];

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[m_nCorner[2]], &pVertices[m_nCorner[1]], &pVertices[m_nCorner[3]]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++;

			return nTris;
		}

		static int ntemp ;
		static WORD wIndex0, wIndex1, wIndex2 ;

        //Remark : 분할삼각형을 만들때 시계방향으로 만드는것을 잊지마라!

		if(!abEnable[NEIGHBORNODE_UP])//이웃노드중 윗부분의 노드 분할
		{
			ntemp = (m_nCorner[CORNER_TL]+m_nCorner[CORNER_TR])/2 ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_TL] ;
            wIndex2 = ntemp ;// (m_nCorner[CORNER_TL]+m_nCorner[CORNER_TR])/2 ;

			*p++ = wIndex0 ;// (WORD)m_nCenter ;
			*p++ = wIndex1 ;// (WORD)m_nCorner[CORNER_TL] ;
			*p++ = wIndex2 ;// (WORD)ntemp ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)ntemp ;
			wIndex2 = (WORD)m_nCorner[CORNER_TR] ;

            *p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}
		else
		{
			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_TL] ;
			wIndex2 = (WORD)m_nCorner[CORNER_TR] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

            //cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}

		if(!abEnable[NEIGHBORNODE_DOWN])//이웃노드중 아래부분의 노드 분할
		{
			ntemp = (m_nCorner[CORNER_BL]+m_nCorner[CORNER_BR])/2 ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)ntemp ;
			wIndex2 = (WORD)m_nCorner[CORNER_BL] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_BR] ;
			wIndex2 = (WORD)ntemp ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}
		else
		{
			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_BR] ;
			wIndex2 = (WORD)m_nCorner[CORNER_BL] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}

		if(!abEnable[NEIGHBORNODE_LEFT])//이웃노드중 왼쪽부분의 노드 분할
		{
			ntemp = (m_nCorner[CORNER_TL]+m_nCorner[CORNER_BL])/2 ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_BL] ;
			wIndex2 = (WORD)ntemp ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)ntemp ;
			wIndex2 = (WORD)m_nCorner[CORNER_TL] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}
		else
		{
			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_BL] ;
			wIndex2 = (WORD)m_nCorner[CORNER_TL] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}

		if(!abEnable[NEIGHBORNODE_RIGHT])//이웃노드중 오른쪽부분의 노드 분할
		{
			ntemp = (m_nCorner[CORNER_TR]+m_nCorner[CORNER_BR])/2 ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_TR] ;
			wIndex2 = (WORD)ntemp ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)ntemp ;
			wIndex2 = (WORD)m_nCorner[CORNER_BR] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}
		else
		{
			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_TR] ;
			wIndex2 = (WORD)m_nCorner[CORNER_BR] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup 연산에서 사용되어질 삼각형을 만든다.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}

		return nTris ;
	}

	// 자식 노드들 검색
	if( m_pChild[CORNER_TL] ) nTris = m_pChild[CORNER_TL]->_GenerateTriangles( pVertices, pIndices, pTriangles, pvCameraPos, nTris );
	if( m_pChild[CORNER_TR] ) nTris = m_pChild[CORNER_TR]->_GenerateTriangles( pVertices, pIndices, pTriangles, pvCameraPos, nTris );
	if( m_pChild[CORNER_BL] ) nTris = m_pChild[CORNER_BL]->_GenerateTriangles( pVertices, pIndices, pTriangles, pvCameraPos, nTris );
	if( m_pChild[CORNER_BR] ) nTris = m_pChild[CORNER_BR]->_GenerateTriangles( pVertices, pIndices, pTriangles, pvCameraPos, nTris );

	return nTris;
}

void ZQuadTree::GetIntersectedTriangle(Vector3 *pvPos, TERRAINVERTEX *pVertices, Vector3 *pvIntersectedPos, geo::STriangle **ppsIntersectedTriangle, bool *pbIntersected)
{
	if((*pbIntersected))
		return ;

	if( m_bCulled )
	{
		m_bCulled = FALSE;
		return ;
	}

	if( (pvPos->x >= pVertices[m_nCorner[CORNER_TL]].pos.x) && (pvPos->x <= pVertices[m_nCorner[CORNER_TR]].pos.x)
		 && (pvPos->z <= pVertices[m_nCorner[CORNER_TL]].pos.z) && (pvPos->z >= pVertices[m_nCorner[CORNER_BL]].pos.z) )
	{
		if(m_nNumTriangle)//nNumTriangle이 있다는 것은 미리 실행한 GernerateTriangles함수에 의해서 화면에 보여지고 있다는 것임.
		{
			Vector3 vBaryCenter ;
			geo::STriangle *psTriangle ;
			geo::SLine line(Vector3(pvPos->x, 1000.0f, pvPos->z), Vector3(0, -1, 0), 2000.0f) ;
			for(int i=0 ; i<m_nNumTriangle ; i++)
			{
				psTriangle = m_apTriangles[i] ;
				if(IntersectLinetoTriangle(line, *psTriangle, vBaryCenter) == geo::INTERSECT_POINT)
				{
					pvIntersectedPos->set(
						psTriangle->avVertex[0].vPos.x*vBaryCenter.x + psTriangle->avVertex[1].vPos.x*vBaryCenter.y + psTriangle->avVertex[2].vPos.x*vBaryCenter.z,
						psTriangle->avVertex[0].vPos.y*vBaryCenter.x + psTriangle->avVertex[1].vPos.y*vBaryCenter.y + psTriangle->avVertex[2].vPos.y*vBaryCenter.z,
						psTriangle->avVertex[0].vPos.z*vBaryCenter.x + psTriangle->avVertex[1].vPos.z*vBaryCenter.y + psTriangle->avVertex[2].vPos.z*vBaryCenter.z ) ;
					psTriangle->bSelected = true ;
					(*ppsIntersectedTriangle) = psTriangle ;
					(*pbIntersected) = true ;

					//TRACE("Selected Triangle p0(%07.03f %07.03f %07.03f) p1(%07.03f %07.03f %07.03f) p2(%07.03f %07.03f %07.03f)\r\n",
					//	enumVector(psTriangle->avVertex[0].vPos), enumVector(psTriangle->avVertex[1].vPos), enumVector(psTriangle->avVertex[2].vPos)) ;

					//TRACE("Intersected point (%07.03f %07.03f %07.03f)\r\n", pvIntersectedPos->x, pvIntersectedPos->y, pvIntersectedPos->z) ;
					return ;
				}
			}
		}
		else
		{
			if( m_pChild[CORNER_TL] ) m_pChild[CORNER_TL]->GetIntersectedTriangle(pvPos, pVertices, pvIntersectedPos, ppsIntersectedTriangle, pbIntersected) ;
			if( m_pChild[CORNER_TR] ) m_pChild[CORNER_TR]->GetIntersectedTriangle(pvPos, pVertices, pvIntersectedPos, ppsIntersectedTriangle, pbIntersected) ;
			if( m_pChild[CORNER_BL] ) m_pChild[CORNER_BL]->GetIntersectedTriangle(pvPos, pVertices, pvIntersectedPos, ppsIntersectedTriangle, pbIntersected) ;
			if( m_pChild[CORNER_BR] ) m_pChild[CORNER_BR]->GetIntersectedTriangle(pvPos, pVertices, pvIntersectedPos, ppsIntersectedTriangle, pbIntersected) ;
		}
	}
}

bool ZQuadTree::_IsIntersected(geo::SCube *psCube, TERRAINVERTEX *pVertices)
{
	//if(psCube->vVertexs[geo::SCube::SCUBE_ULT].x < pVertices[m_nCorner[CORNER_TR]].pos.x
	//	|| psCube->vVertexs[geo::SCube::SCUBE_URT].x > pVertices[m_nCorner[CORNER_TL]].pos.x)
	//{
	//	if(psCube->vVertexs[geo::SCube::SCUBE_ULT].z > pVertices[m_nCorner[CORNER_BL]].pos.z
	//		|| psCube->vVertexs[geo::SCube::SCUBE_ULB].z < pVertices[m_nCorner[CORNER_TL]].pos.z)
	//		return true ;
	//}

	//bool bb = false ;
	//float xgap, zgap, width, height ;

	//xgap = fabs(psCube->vCenter.x - (pVertices[m_nCorner[CORNER_TL]].pos.x+pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f) ;
	//zgap = fabs(psCube->vCenter.z - (pVertices[m_nCorner[CORNER_TL]].pos.z+pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f) ;
	//width = ((psCube->fWidth/2)+fabs(pVertices[m_nCorner[CORNER_TL]].pos.x-pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f) ;
	//height = ((psCube->fHeight/2)+fabs(pVertices[m_nCorner[CORNER_TL]].pos.z-pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f) ;

	//TRACE("xgap(%07.03f) <= width(%07.03f)\r\n", xgap, width) ;
	//TRACE("zgap(%07.03f) <= height(%07.03f)\r\n", zgap, height) ;

	if( fabs(psCube->vCenter.x - (pVertices[m_nCorner[CORNER_TL]].pos.x+pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f) <= ((psCube->fWidth/2)+fabs(pVertices[m_nCorner[CORNER_TL]].pos.x-pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f))
	{
        if( fabs(psCube->vCenter.z - (pVertices[m_nCorner[CORNER_TL]].pos.z+pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f) <= ((psCube->fHeight/2)+fabs(pVertices[m_nCorner[CORNER_TL]].pos.z-pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f))
			return true ;
	}
	return false ;
	//TRACE("success=%d\r\n", bb) ;
	//TRACE("\r\n") ;

	//return bb ;

	//if( fabs(psCube->vCenter.x - pVertices[m_nCenter].pos.x) <= ((psCube->fWidth/2)+fabs(pVertices[m_nCorner[CORNER_TL]].pos.x-pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f))
	//{
 //       if( fabs(psCube->vCenter.z - pVertices[m_nCenter].pos.z) <= ((psCube->fHeight/2)+fabs(pVertices[m_nCorner[CORNER_TL]].pos.z-pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f))
	//		return true ;
	//}
	//return false ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                           큐브와 교차판정                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ZQuadTree::_GetIntersectedTriangle(geo::SCube *psCube, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles)
{
	if( m_bCulled )
	{
		m_bCulled = FALSE;
		return nTriangles ;
	}

	//TRACE("center(%07.03f %07.03f)\r\n",
	//	(pVertices[m_nCorner[CORNER_TL]].pos.x+pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f,
	//	(pVertices[m_nCorner[CORNER_TL]].pos.z+pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f) ;

	if(_IsIntersected(psCube, pVertices))
	{
		if(m_nNumTriangle)//삼각형을 보유하고 있는는 말은 이 단계에서 보여진다는 말이다.
		{
			for(int i=0 ; i<m_nNumTriangle ; i++)
			{
				ppsIntersectedTriangles[nTriangles++] = m_apTriangles[i] ;

				//TRACE("[%02d]\r\n", nTriangles-1) ;
				//TRACE("pos0(%+07.03f %+07.03f %+07.03f)\r\n", enumVector(ppsIntersectedTriangles[nTriangles-1]->avVertex[0].vPos)) ;
				//TRACE("pos1(%+07.03f %+07.03f %+07.03f)\r\n", enumVector(ppsIntersectedTriangles[nTriangles-1]->avVertex[1].vPos)) ;
				//TRACE("pos2(%+07.03f %+07.03f %+07.03f)\r\n", enumVector(ppsIntersectedTriangles[nTriangles-1]->avVertex[2].vPos)) ;
				//TRACE("\r\n") ;
			}
			return nTriangles ;
		}
	}
	else
		return nTriangles ;

	if(m_pChild[0]) nTriangles = m_pChild[0]->_GetIntersectedTriangle(psCube, pVertices, ppsIntersectedTriangles, nTriangles) ;
	if(m_pChild[1]) nTriangles = m_pChild[1]->_GetIntersectedTriangle(psCube, pVertices, ppsIntersectedTriangles, nTriangles) ;
	if(m_pChild[2]) nTriangles = m_pChild[2]->_GetIntersectedTriangle(psCube, pVertices, ppsIntersectedTriangles, nTriangles) ;
	if(m_pChild[3]) nTriangles = m_pChild[3]->_GetIntersectedTriangle(psCube, pVertices, ppsIntersectedTriangles, nTriangles) ;

	return nTriangles ;
}

int ZQuadTree::GetIntersectedTriangle(geo::SCube *psCube, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles)
{
	psCube->ComputeBoundary() ;
    return _GetIntersectedTriangle(psCube, pVertices, ppsIntersectedTriangles, 0) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                           구와 교차판정                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ZQuadTree::_GetIntersectedTriangle(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles)
{
	if( m_bCulled )
	{
		m_bCulled = FALSE;
		return nTriangles ;
	}

    float width, height ;

	//평면적으로 가로 세로 길이를 구한다.
	width = pvPos->x - ((pVertices[m_nCorner[CORNER_TL]].pos.x+pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f) ;
	height = pvPos->z - ((pVertices[m_nCorner[CORNER_TL]].pos.z+pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f) ;
    //두점의 길이가 각각의 반지름의 합보다 큰경우는 해당되지 않는다.
	if(sqrtf((width*width)+(height*height)) <= (fRadius+m_fRadius))
	{
		if(m_nNumTriangle)//삼각형을 보유하고 있는는 말은 이 단계에서 보여진다는 말이다.
		{
			for(int i=0 ; i<m_nNumTriangle ; i++)
			{
				ppsIntersectedTriangles[nTriangles++] = m_apTriangles[i] ;

				//TRACE("[%02d]\r\n", nTriangles-1) ;
				//TRACE("pos0(%+07.03f %+07.03f %+07.03f)\r\n", enumVector(ppsIntersectedTriangles[nTriangles-1]->avVertex[0].vPos)) ;
				//TRACE("pos1(%+07.03f %+07.03f %+07.03f)\r\n", enumVector(ppsIntersectedTriangles[nTriangles-1]->avVertex[1].vPos)) ;
				//TRACE("pos2(%+07.03f %+07.03f %+07.03f)\r\n", enumVector(ppsIntersectedTriangles[nTriangles-1]->avVertex[2].vPos)) ;
				//TRACE("\r\n") ;
			}
			return nTriangles ;
		}
	}
	else
		return nTriangles ;

	if(m_pChild[0]) nTriangles = m_pChild[0]->_GetIntersectedTriangle(pvPos, fRadius, pVertices, ppsIntersectedTriangles, nTriangles) ;
	if(m_pChild[1]) nTriangles = m_pChild[1]->_GetIntersectedTriangle(pvPos, fRadius, pVertices, ppsIntersectedTriangles, nTriangles) ;
	if(m_pChild[2]) nTriangles = m_pChild[2]->_GetIntersectedTriangle(pvPos, fRadius, pVertices, ppsIntersectedTriangles, nTriangles) ;
	if(m_pChild[3]) nTriangles = m_pChild[3]->_GetIntersectedTriangle(pvPos, fRadius, pVertices, ppsIntersectedTriangles, nTriangles) ;
	
	return nTriangles ;
}

int ZQuadTree::GetIntersectedTriangle(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles)
{
    return _GetIntersectedTriangle(pvPos, fRadius, pVertices, ppsIntersectedTriangles, 0) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                        반직선과 교차판정                                                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ZQuadTree::_GetIntersectedTriangle(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles)
{
	if( m_bCulled )
	{
		m_bCulled = FALSE;
		return nTriangles ;
	}

	Vector3 vPos, vIntersected ;
	float d ;

	//가운데 점을 구한다.
	vPos.x = ((pVertices[m_nCorner[CORNER_TL]].pos.x+pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f) ;
	vPos.y = 0 ;
	vPos.z = ((pVertices[m_nCorner[CORNER_TL]].pos.z+pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f) ;

	if((d = IntersectLinetoPoint(*psLine, vPos, &vIntersected)) > 0)
	{
		//반직선과 교차했다면 교차된 점이 쿼드트리 영역안에 있다면
		Vector3 v(vPos.x-vIntersected.x, 0, vPos.z-vIntersected.z) ;
		if(v.Magnitude() <= m_fRadius)
		//if( (vIntersected.x >= pVertices[m_nCorner[CORNER_TL]].pos.x) && (vIntersected.x <= pVertices[m_nCorner[CORNER_TR]].pos.x)
		//	&& (vIntersected.z <= pVertices[m_nCorner[CORNER_TL]].pos.z) && (vIntersected.z >= pVertices[m_nCorner[CORNER_BL]].pos.z) )
		{
			if(m_nNumTriangle)//삼각형을 보유하고 있는는 말은 이 단계에서 보여진다는 말이다.
			{
				for(int i=0 ; i<m_nNumTriangle ; i++)
				{
					ppsIntersectedTriangles[nTriangles++] = m_apTriangles[i] ;

					//TRACE("[%02d]\r\n", nTriangles-1) ;
					//TRACE("pos0(%+07.03f %+07.03f %+07.03f)\r\n", enumVector(ppsIntersectedTriangles[nTriangles-1]->avVertex[0].vPos)) ;
					//TRACE("pos1(%+07.03f %+07.03f %+07.03f)\r\n", enumVector(ppsIntersectedTriangles[nTriangles-1]->avVertex[1].vPos)) ;
					//TRACE("pos2(%+07.03f %+07.03f %+07.03f)\r\n", enumVector(ppsIntersectedTriangles[nTriangles-1]->avVertex[2].vPos)) ;
					//TRACE("\r\n") ;
				}
				return nTriangles ;
			}
		}
	}
	else
		return nTriangles ;

	if(m_pChild[0]) nTriangles = m_pChild[0]->_GetIntersectedTriangle(psLine, pVertices, ppsIntersectedTriangles, nTriangles) ;
	if(m_pChild[1]) nTriangles = m_pChild[1]->_GetIntersectedTriangle(psLine, pVertices, ppsIntersectedTriangles, nTriangles) ;
	if(m_pChild[2]) nTriangles = m_pChild[2]->_GetIntersectedTriangle(psLine, pVertices, ppsIntersectedTriangles, nTriangles) ;
	if(m_pChild[3]) nTriangles = m_pChild[3]->_GetIntersectedTriangle(psLine, pVertices, ppsIntersectedTriangles, nTriangles) ;
	
	return nTriangles ;
}

int ZQuadTree::GetIntersectedTriangle(geo::SLine *psLine, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles)
{
    return _GetIntersectedTriangle(psLine, pVertices, ppsIntersectedTriangles, 0) ;
}