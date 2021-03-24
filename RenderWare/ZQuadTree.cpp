#include "ZQuadTree.h"
#include "windows.h"
#include <stdio.h>

#include "SecretTerrain.h"
#include "MathOrdinary.h"

const float fMinRadius_inQuadTree = sqrtf(0.5f) ;

// ���� ��Ʈ��� ������
ZQuadTree::ZQuadTree( int cx, int cy )//cx, cy �̰��� ���̴����� �ƴϰ� ���ؽ� ���� ���� ������� �ؾ� �ǰڴ�.
{
	int		i;
	m_nCenter = 0;
	for( i = 0 ; i < 4 ; i++ )
	{
		m_pChild[i] = NULL;
		m_pNeighbor[i] = NULL ;
	}

	// ��Ʈ����� 4�� �ڳʰ� ����
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

// ���� �ڽĳ�� ������
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

// �Ҹ���
ZQuadTree::~ZQuadTree()
{
	_Destroy();
}

// �޸𸮿��� ����Ʈ���� �����Ѵ�.
void	ZQuadTree::_Destroy()
{
	// �ڽ� ������ �Ҹ� ��Ų��.
	for( int i = 0 ; i < 4 ; i++ )
	{
		if(m_pChild[i])
			delete m_pChild[i] ;
	}
}


// 4���� �ڳʰ��� �����Ѵ�.
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


// �ڽ� ��带 �߰��Ѵ�.
ZQuadTree*	ZQuadTree::_AddChild( int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR )
{
	ZQuadTree*	pChild;

	pChild = new ZQuadTree( /*this*/ );
	pChild->_SetCorners( nCornerTL, nCornerTR, nCornerBL, nCornerBR );

	return pChild;
}

// Quadtree�� 4���� �ڽ� Ʈ���� �κк���(subdivide)�Ѵ�.
BOOL	ZQuadTree::_SubDivide()
{
	int		nTopEdgeCenter;
	int		nBottomEdgeCenter;
	int		nLeftEdgeCenter;
	int		nRightEdgeCenter;
	int		nCentralPoint;

	// ��ܺ� ���
	nTopEdgeCenter		= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] ) / 2;
	// �ϴܺ� ��� 
	nBottomEdgeCenter	= ( m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 2;
	// ������ ���
	nLeftEdgeCenter		= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_BL] ) / 2;
	// ������ ���
	nRightEdgeCenter	= ( m_nCorner[CORNER_TR] + m_nCorner[CORNER_BR] ) / 2;
	// �Ѱ��
	nCentralPoint		= ( m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] + 
							m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR] ) / 4;

	// ���̻� ������ �Ұ����Ѱ�? �׷��ٸ� SubDivide() ����
	if( (m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL]) <= 1 )
	{
		m_fRadius = fMinRadius_inQuadTree ;
		return FALSE;
	}

	// 4���� �ڽĳ�� �߰�
	m_pChild[CORNER_TL] = _AddChild( m_nCorner[CORNER_TL], nTopEdgeCenter, nLeftEdgeCenter, nCentralPoint );
	m_pChild[CORNER_TR] = _AddChild( nTopEdgeCenter, m_nCorner[CORNER_TR], nCentralPoint, nRightEdgeCenter );
	m_pChild[CORNER_BL] = _AddChild( nLeftEdgeCenter, nCentralPoint, m_nCorner[CORNER_BL], nBottomEdgeCenter );
	m_pChild[CORNER_BR] = _AddChild( nCentralPoint, nRightEdgeCenter, nBottomEdgeCenter, m_nCorner[CORNER_BR] );
	
	return TRUE;
}

// ����� �������� �ε����� �����Ѵ�.
int		ZQuadTree::_GenTriIndex( int nTris, LPVOID pIndex, TERRAINVERTEX *pHeightMap, CSecretFrustum* pcSecretFrustum )
{
	// �ø��� ����� �׳� ����
	if( m_bCulled )
	{
		m_bCulled = FALSE;
		return nTris;
	}

	Vector3 vCameraPosition=pcSecretFrustum->GetCameraPosition() ;

	// ���� �������� ��µǾ�� �ϴ°�? �ƴϸ� �� �����ؾ� �Ǵ°�?
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

		if((m_nCorner[CORNER_TR]-m_nCorner[CORNER_TL]) <= 1)//������ ����ϰ��� �׳� ����̴�.
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

		//4���� �̿���尡 ����LODLevel���� ��µǾ�� �Ǵ��� �׽�Ʈ
		for(i=0 ; i<4 ; i++)
		{
			abEnable[i] = true ;

			if(m_pNeighbor[i])
				abEnable[i] = m_pNeighbor[i]->_IsVisible(pHeightMap, vCameraPosition, 0.1f) ;
		}

		//����LODLevel ���� ��� ��°����ϴٸ� �����ʿ���� ������� ����̴�.
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

        //Remark : ���һﰢ���� ���鶧 �ð�������� ����°��� ��������!

		if(!abEnable[NEIGHBORNODE_UP])//�̿������ ���κ��� ��� ����
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

		if(!abEnable[NEIGHBORNODE_DOWN])//�̿������ �Ʒ��κ��� ��� ����
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

		if(!abEnable[NEIGHBORNODE_LEFT])//�̿������ ���ʺκ��� ��� ����
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

		if(!abEnable[NEIGHBORNODE_RIGHT])//�̿������ �����ʺκ��� ��� ����
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

	// �ڽ� ���� �˻�
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

	// ��豸�ȿ� �ִ°�?
    //	if( m_fRadius == 0.0f ) g_pLog->Log( "Index:[%d], Radius:[%f]",m_nCenter, m_fRadius );
	Vector3 v=VectorConvert((D3DXVECTOR3 *)(pHeightMap+m_nCenter)) ;
	bInSphere = pcSecretFrustum->IsInSphere(v, m_fRadius );
	if( !bInSphere ) return FRUSTUM_OUT;	// ��豸 �ȿ� ������ �������� �������� �׽�Ʈ ����

	// ����Ʈ���� 4���� ��� �������� �׽�Ʈ
	v=VectorConvert((D3DXVECTOR3*)(pHeightMap+m_nCorner[0])) ;
	b[0] = pcSecretFrustum->IsIn(v);

	v=VectorConvert((D3DXVECTOR3*)(pHeightMap+m_nCorner[1])) ;
	b[1] = pcSecretFrustum->IsIn(v);
	
	v=VectorConvert((D3DXVECTOR3*)(pHeightMap+m_nCorner[2])) ;
	b[2] = pcSecretFrustum->IsIn(v);

	v=VectorConvert((D3DXVECTOR3*)(pHeightMap+m_nCorner[3])) ;
	b[3] = pcSecretFrustum->IsIn(v);

	// 4����� �������� �ȿ� ����
	if( (b[0] + b[1] + b[2] + b[3]) == 4 ) return FRUSTUM_COMPLETELY_IN;

	// �Ϻκ��� �������ҿ� �ִ� ���
	return FRUSTUM_PARTIALLY_IN;
}

// _IsInFrustum()�Լ��� ����� ���� �������� �ø� ����
void	ZQuadTree::_FrustumCull( TERRAINVERTEX* pHeightMap, CSecretFrustum* pcSecretFrustum )
{
	int ret;

	ret = _IsInFrustum( pHeightMap, pcSecretFrustum );
	switch( ret )
	{
		case FRUSTUM_COMPLETELY_IN :	// �������ҿ� ��������, ������� �˻� �ʿ����
			m_bCulled = FALSE;
			return;
		case FRUSTUM_PARTIALLY_IN :		// �������ҿ� �Ϻ�����, ������� �˻� �ʿ���
			m_bCulled = FALSE;
			break;
		case FRUSTUM_OUT :				// �������ҿ��� �������, ������� �˻� �ʿ����
			m_bCulled = TRUE;
			return;
	}
	if( m_pChild[0] ) m_pChild[0]->_FrustumCull( pHeightMap, pcSecretFrustum );
	if( m_pChild[1] ) m_pChild[1]->_FrustumCull( pHeightMap, pcSecretFrustum );
	if( m_pChild[2] ) m_pChild[2]->_FrustumCull( pHeightMap, pcSecretFrustum );
	if( m_pChild[3] ) m_pChild[3]->_FrustumCull( pHeightMap, pcSecretFrustum );
}

// ����Ʈ���� �����.
//m_nCorner[] �� ���� ���̸��� �ε������̴�. �ٸ����� �����°� ���ʻ�ܺ��� 0 ���� ������ �ϴ� Width*Height
BOOL	ZQuadTree::_BuildQuadTree( TERRAINVERTEX* pHeightMap )
{
	if( _SubDivide() )
	{
		// ������ܰ�, ���� �ϴ��� �Ÿ��� ���Ѵ�.
		D3DXVECTOR3 v = *((D3DXVECTOR3*)(pHeightMap+m_nCorner[CORNER_TL])) - 
					    *((D3DXVECTOR3*)(pHeightMap+m_nCorner[CORNER_BR]));
		// v�� �Ÿ����� �� ��带 ���δ� ��豸�� �����̹Ƿ�, 
		// 2�� ������ �������� ���Ѵ�.
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

//	�ﰢ���� �ε����� �����, ����� �ﰢ���� ������ ��ȯ�Ѵ�.
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

//���� �������� ������ ���� �ƴϸ� �� �����ؾ� ������ �����ϴ� ���̱� ������
//����Ÿ���� ������ >= (vCamera-vPos).Length * coef �Ǹ� �� �����ؾߵǰ�
//����Ÿ���� ������ <= (vCamera-vPos).Length * coef �Ǹ� ����������� ��������.
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

	//��ü�ʾȿ� ���ԵǾ��ִ��� �Ǵ�
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

//�ڽĳ�� 4���� ���� ��ġ��(�ڳ��ε������� pHeightMap���� ������ ����)
//������ Center�� ��ġ �� ���ϸ鼭 �ڳʰ��� ���� ��ġ�ϴ� ��忡�� �����ϰԲ� �Ǿ��ִ�.
//��� ��带 ���������� �˻��Ѵ�.
//Remark : ���� �ø������ʴ� �����̱� ������ ������߿� ���õǾ�����.
ZQuadTree *ZQuadTree::_FindNode(TERRAINVERTEX *pHeightMap, int nTL, int nTR, int nBL, int nBR)
{
	//���� ���� ��ġ�ϸ� �����Ѵ�.
	if((m_nCorner[0] == nTL) && (m_nCorner[1] == nTR) && (m_nCorner[2] == nBL) && (m_nCorner[3] == nBR))
		return this ;

	if(m_pChild[0])//�ڽĳ�尡 �ִٸ�
	{
		//RECT rt ;
		//POINT pt ;
		geo::SPoint point ;
		geo::SRect rect ;
		int i, nCenter ;

		nCenter = (nTL+nTR+nBL+nBR)/4 ;

		point.x = pHeightMap[nCenter].pos.x ;// (int)pHeightMap[nCenter].pos.x ;
		point.y = pHeightMap[nCenter].pos.z ;//(int)pHeightMap[nCenter].pos.z ;

		//4���� �ڽĳ�带 �˻��Ѵ�.
		for(i=0 ; i<4 ; i++)
		{
			//Rect�� ���ؼ� 
			//SetRect(&rt, (int)pHeightMap[m_pChild[i]->m_nCorner[CORNER_TL]].pos.x,
			//			 (int)pHeightMap[m_pChild[i]->m_nCorner[CORNER_TL]].pos.z,
			//			 (int)pHeightMap[m_pChild[i]->m_nCorner[CORNER_BR]].pos.x,
			//			 (int)pHeightMap[m_pChild[i]->m_nCorner[CORNER_BR]].pos.z) ;

			rect.set(pHeightMap[m_pChild[i]->m_nCorner[CORNER_TL]].pos.x,
				pHeightMap[m_pChild[i]->m_nCorner[CORNER_BR]].pos.x,
				pHeightMap[m_pChild[i]->m_nCorner[CORNER_TL]].pos.z,				
				pHeightMap[m_pChild[i]->m_nCorner[CORNER_BR]].pos.z) ;

			//������ Center��ġ���� Rect�ȿ� �ִ� ���̶�� �ڽĳ�带 ���� ���ȣ���� ����Ѵ�.
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
		//������ 4�����̸� ��α��ϰ�
		anEdge[0] = m_nCorner[0] ;
		anEdge[1] = m_nCorner[1] ;
		anEdge[2] = m_nCorner[2] ;
		anEdge[3] = m_nCorner[3] ;

		//4�����̸� ������ ��[0], �Ʒ�[1], ����[2], ������[3] �� �̿���带 ���Ѵ�.
        nCenter = _GetNeighborNodeIndex(i, nPitch, nHeight, anEdge) ;

		//TRACE("    Neighbor conner is (%02d %02d %02d %02d) is %02d\r\n", anEdge[0], anEdge[1], anEdge[2], anEdge[3], nCenter) ;

		if(nCenter >= 0)//�̿���带 ���ϴµ� �����ߴٸ� ���� ��������͸� ����.
		{
			m_pNeighbor[i] = pRoot->_FindNode(pHeightMap, anEdge[0], anEdge[1], anEdge[2], anEdge[3]) ;
			//if(m_pNeighbor[i] != NULL)
			//	TRACE("    (%02d %02d %02d %02d) Neighbor is founded\r\n", anEdge[0], anEdge[1], anEdge[2], anEdge[3]) ;
			//else
			//	TRACE("    (%02d %02d %02d %02d) Neighbor isn't founded\r\n", anEdge[0], anEdge[1], anEdge[2], anEdge[3]) ;
		}
	}

	//TRACE("4 conner are (%02d %02d %02d %02d)***\r\n", m_nCorner[0], m_nCorner[1], m_nCorner[2], m_nCorner[3]) ;

	if(m_pChild[0])//�ڽĳ�忡���� ���ȣ��� ��� �̿���带 ���ϰ� �ȴ�.
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

	//���ϴ� ������ �����Դٴ� ���� �������� �ȴٴ� ����.
	if((m_nCorner[CORNER_TR]-m_nCorner[CORNER_TL]) <= 1)//������ ����ϰ��� �׳� ����̴�.
	{
		p = ((LPWORD)pIndices) + nTris * 3 ;
		pTris = pTriangles + nTris ;

		*p++ = (WORD)m_nCorner[0];
		*p++ = (WORD)m_nCorner[1];
		*p++ = (WORD)m_nCorner[2];

		//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
		_SetTriangle(pTris, &pVertices[m_nCorner[0]], &pVertices[m_nCorner[1]], &pVertices[m_nCorner[2]]) ;

		m_apTriangles[m_nNumTriangle++] = pTris++ ;

		nTris++;

		*p++ = (WORD)m_nCorner[2];
		*p++ = (WORD)m_nCorner[1];
		*p++ = (WORD)m_nCorner[3];

		//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
		_SetTriangle(pTris, &pVertices[m_nCorner[2]], &pVertices[m_nCorner[1]], &pVertices[m_nCorner[3]]) ;

		m_apTriangles[m_nNumTriangle++] = pTris++ ;

		nTris++;

		return nTris;
	}

	//���� �������� ��µǾ�� �ϴ°�? �ƴϸ� �� �����ؾ� �Ǵ°�?
	//_IsVisible �� ���и� �ϸ� �ڽĳ��� �������µ� �ڽĳ�尡 ����.
	//if( _IsVisible() )
	if(_IsVisible(pVertices, *pvCameraPos, 0.1f))
	{
		p = ((LPWORD)pIndices) + nTris * 3 ;
		pTris = pTriangles + nTris ;

		int i ;
		bool abEnable[4] ;

		//4���� �̿���尡 ����LODLevel���� ��µǾ�� �Ǵ��� �׽�Ʈ
		for(i=0 ; i<4 ; i++)
		{
			abEnable[i] = true ;

			if(m_pNeighbor[i])
				abEnable[i] = m_pNeighbor[i]->_IsVisible(pVertices, *pvCameraPos, 0.1f) ;
		}

		//����LODLevel ���� ��� ��°����ϴٸ� �����ʿ���� ������� ����̴�.
		if(abEnable[0] && abEnable[1] && abEnable[2] && abEnable[3])
		{
			*p++ = (WORD)m_nCorner[0];
			*p++ = (WORD)m_nCorner[1];
			*p++ = (WORD)m_nCorner[2];

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[m_nCorner[0]], &pVertices[m_nCorner[1]], &pVertices[m_nCorner[2]]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++;


			*p++ = (WORD)m_nCorner[2];
			*p++ = (WORD)m_nCorner[1];
			*p++ = (WORD)m_nCorner[3];

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[m_nCorner[2]], &pVertices[m_nCorner[1]], &pVertices[m_nCorner[3]]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++;

			return nTris;
		}

		static int ntemp ;
		static WORD wIndex0, wIndex1, wIndex2 ;

        //Remark : ���һﰢ���� ���鶧 �ð�������� ����°��� ��������!

		if(!abEnable[NEIGHBORNODE_UP])//�̿������ ���κ��� ��� ����
		{
			ntemp = (m_nCorner[CORNER_TL]+m_nCorner[CORNER_TR])/2 ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_TL] ;
            wIndex2 = ntemp ;// (m_nCorner[CORNER_TL]+m_nCorner[CORNER_TR])/2 ;

			*p++ = wIndex0 ;// (WORD)m_nCenter ;
			*p++ = wIndex1 ;// (WORD)m_nCorner[CORNER_TL] ;
			*p++ = wIndex2 ;// (WORD)ntemp ;

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)ntemp ;
			wIndex2 = (WORD)m_nCorner[CORNER_TR] ;

            *p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
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

            //cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}

		if(!abEnable[NEIGHBORNODE_DOWN])//�̿������ �Ʒ��κ��� ��� ����
		{
			ntemp = (m_nCorner[CORNER_BL]+m_nCorner[CORNER_BR])/2 ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)ntemp ;
			wIndex2 = (WORD)m_nCorner[CORNER_BL] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_BR] ;
			wIndex2 = (WORD)ntemp ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
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

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}

		if(!abEnable[NEIGHBORNODE_LEFT])//�̿������ ���ʺκ��� ��� ����
		{
			ntemp = (m_nCorner[CORNER_TL]+m_nCorner[CORNER_BL])/2 ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_BL] ;
			wIndex2 = (WORD)ntemp ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)ntemp ;
			wIndex2 = (WORD)m_nCorner[CORNER_TL] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
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

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}

		if(!abEnable[NEIGHBORNODE_RIGHT])//�̿������ �����ʺκ��� ��� ����
		{
			ntemp = (m_nCorner[CORNER_TR]+m_nCorner[CORNER_BR])/2 ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)m_nCorner[CORNER_TR] ;
			wIndex2 = (WORD)ntemp ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;

			wIndex0 = (WORD)m_nCenter ;
			wIndex1 = (WORD)ntemp ;
			wIndex2 = (WORD)m_nCorner[CORNER_BR] ;

			*p++ = wIndex0 ;
			*p++ = wIndex1 ;
			*p++ = wIndex2 ;

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
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

			//cup ���꿡�� ���Ǿ��� �ﰢ���� �����.
			_SetTriangle(pTris, &pVertices[wIndex0], &pVertices[wIndex1], &pVertices[wIndex2]) ;

            m_apTriangles[m_nNumTriangle++] = pTris++ ;

			nTris++ ;
		}

		return nTris ;
	}

	// �ڽ� ���� �˻�
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
		if(m_nNumTriangle)//nNumTriangle�� �ִٴ� ���� �̸� ������ GernerateTriangles�Լ��� ���ؼ� ȭ�鿡 �������� �ִٴ� ����.
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
//                                                           ť��� ��������                                                                  //
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
		if(m_nNumTriangle)//�ﰢ���� �����ϰ� �ִ´� ���� �� �ܰ迡�� �������ٴ� ���̴�.
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
//                                                           ���� ��������                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ZQuadTree::_GetIntersectedTriangle(Vector3 *pvPos, float fRadius, TERRAINVERTEX *pVertices, geo::STriangle **ppsIntersectedTriangles, int nTriangles)
{
	if( m_bCulled )
	{
		m_bCulled = FALSE;
		return nTriangles ;
	}

    float width, height ;

	//��������� ���� ���� ���̸� ���Ѵ�.
	width = pvPos->x - ((pVertices[m_nCorner[CORNER_TL]].pos.x+pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f) ;
	height = pvPos->z - ((pVertices[m_nCorner[CORNER_TL]].pos.z+pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f) ;
    //������ ���̰� ������ �������� �պ��� ū���� �ش���� �ʴ´�.
	if(sqrtf((width*width)+(height*height)) <= (fRadius+m_fRadius))
	{
		if(m_nNumTriangle)//�ﰢ���� �����ϰ� �ִ´� ���� �� �ܰ迡�� �������ٴ� ���̴�.
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
//                                                        �������� ��������                                                                   //
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

	//��� ���� ���Ѵ�.
	vPos.x = ((pVertices[m_nCorner[CORNER_TL]].pos.x+pVertices[m_nCorner[CORNER_TR]].pos.x)/2.0f) ;
	vPos.y = 0 ;
	vPos.z = ((pVertices[m_nCorner[CORNER_TL]].pos.z+pVertices[m_nCorner[CORNER_BL]].pos.z)/2.0f) ;

	if((d = IntersectLinetoPoint(*psLine, vPos, &vIntersected)) > 0)
	{
		//�������� �����ߴٸ� ������ ���� ����Ʈ�� �����ȿ� �ִٸ�
		Vector3 v(vPos.x-vIntersected.x, 0, vPos.z-vIntersected.z) ;
		if(v.Magnitude() <= m_fRadius)
		//if( (vIntersected.x >= pVertices[m_nCorner[CORNER_TL]].pos.x) && (vIntersected.x <= pVertices[m_nCorner[CORNER_TR]].pos.x)
		//	&& (vIntersected.z <= pVertices[m_nCorner[CORNER_TL]].pos.z) && (vIntersected.z >= pVertices[m_nCorner[CORNER_BL]].pos.z) )
		{
			if(m_nNumTriangle)//�ﰢ���� �����ϰ� �ִ´� ���� �� �ܰ迡�� �������ٴ� ���̴�.
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