#include "SecretToolBrush.h"
#include "SecretTerrain.h"
#include "MathOrdinary.h"
#include "SecretQuadTree.h"
#include "def.h"

SThreadParameters::SThreadParameters()
{
	bIntersected = bExitThread = false ;
	nStart = nEnd = 0 ;
}
SThreadParameters::SThreadParameters(int _nStart, int _nEnd)
{
	bIntersected = bExitThread = false ;
	nStart = _nStart ;
	nEnd = _nEnd ;
}
SThreadParameters::~SThreadParameters()
{
}
void SThreadParameters::EndThread()
{
	if(hThread)
	{
		DWORD dwSuspendCount = ResumeThread(hThread) ;
		//TRACE("Index=%02d suspend thread=%d\r\n", nIndex, dwSuspendCount) ;
		if(dwSuspendCount == (DWORD)-1)
		{
			DWORD dwErrorCode = GetLastError() ;
			TRACE("ErrorCode=%d CSecretToolBrush::Release\r\n", dwErrorCode) ;
		}
		else
		{
			bExitThread = true ;
		}
	}
}

//DWORD TravelTreeProc(CSecretToolBrush *pcToolBrush) ;
//DWORD TravelTreeProc(SThreadParameters *psParameters) ;

CSecretToolBrush::CSecretToolBrush()
{
	m_pLine = NULL ;
	m_pvBoundaryPos1 = NULL ;
	m_pvBoundaryPos2 = NULL ;
	m_nBrushType=CSecretToolBrush::CIRCLE ;
	m_fRadius = 0.5f ;
	m_fBrushCountPerSec = 1 ;
	//m_hThread=NULL ;
	//m_bExitThread = false ;
	//m_nNumThread = 0 ;
	//m_psThreadParameters = NULL ;
	m_nBrushingStatus = NO_BRUSHING ;
	m_psIntersectedTriangles = NULL ;
	m_bEnable = false ;
	m_bProcessed = false ;
	m_nAttr = CSecretToolBrush::DRAWBRUSH ;
}

CSecretToolBrush::~CSecretToolBrush()
{
	Release() ;
}

//void CSecretToolBrush::EndThread()
//{
//	if(m_hThread)
//	{
//		DWORD dwSuspendCount = ResumeThread(m_hThread) ;
//		//TRACE("Index=%02d suspend thread=%d\r\n", nIndex, dwSuspendCount) ;
//		if(dwSuspendCount == (DWORD)-1)
//		{
//			DWORD dwErrorCode = GetLastError() ;
//			TRACE("ErrorCode=%d CSecretToolBrush::Release\r\n", dwErrorCode) ;
//		}
//		else
//			m_bExitThread = true ;
//	}
//}

void CSecretToolBrush::Release()
{
	//if(m_psThreadParameters)
	//{
	//	int i ;
	//	bool bAllExit ;
	//	for(i=0 ; i<m_nNumThread ; i++)
	//		m_psThreadParameters[i].EndThread() ;

	//	while(true)
	//	{
	//		bAllExit = true ;
	//		for(i=0 ; i<m_nNumThread ; i++)
	//		{
	//			if(m_psThreadParameters[i].hThread != NULL)
	//			{
	//				bAllExit = false ;
	//			}
	//		}
 //           if(bAllExit)
	//			break ;
	//		else
	//		{
	//			for(i=0 ; i<m_nNumThread ; i++)
	//				m_psThreadParameters[i].EndThread() ;
	//		}
	//	}

	//	SAFE_DELETEARRAY(m_psThreadParameters) ;
	//}

	//EndThread() ;	
	//while(true)
	//{
	//	if(m_hThread != NULL)
	//		EndThread() ;
	//	else
	//		break ;
	//}

	SAFE_RELEASE(m_pLine) ;
	SAFE_DELETEARRAY(m_pvBoundaryPos1) ;
	SAFE_DELETEARRAY(m_pvBoundaryPos2) ;
	SAFE_DELETEARRAY(m_psIntersectedTriangles) ;
}

bool CSecretToolBrush::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CD3DEnvironment *pd3dEnvironment, CSecretPicking *pcPicking, int nBrushType, float fRadius)
{
	m_pcPicking = pcPicking ;
	//m_cDecal.Initialize(pd3dDevice, Vector3(0, 1, 0), Vector3(1, 0, 0), 1, 1, 10) ;

	m_pd3dDevice = pd3dDevice ;
	m_nBrushType = nBrushType ;
	m_fRadius = fRadius ;
	if(nBrushType == CSecretToolBrush::SQUARE)
		m_nNumBoundary = 4 ;
	else if(nBrushType == CSecretToolBrush::CIRCLE)
		m_nNumBoundary = 16 ;

    m_pvBoundaryPos1 = new Vector3[m_nNumBoundary] ;
	m_pvBoundaryPos2 = new D3DXVECTOR3[m_nNumBoundary+1] ;

	//geo::STriangle size = 128bytes
	m_psIntersectedTriangles = new geo::STriangle[MAXNUM_INTERSECTEDTRIANGLES] ;

	D3DXCreateLine(m_pd3dDevice, &m_pLine) ;
	m_pLine->SetWidth(3) ;
	m_pLine->SetAntialias(TRUE) ;

	m_nSelectedTexNum = 0 ;

	//m_nNumThread = 2 ;
	//m_psThreadParameters = new SThreadParameters[m_nNumThread] ;
 //   int i ;
	//for(i=0 ; i<m_nNumThread ; i++)
	//{
	//	m_psThreadParameters[i].nStart = i*(m_nNumBoundary/m_nNumThread) ;
	//	m_psThreadParameters[i].nEnd = m_psThreadParameters[i].nStart+((m_nNumBoundary/m_nNumThread)-1) ;
	//	m_psThreadParameters[i].pcToolBrush = this ;
	//	m_psThreadParameters[i].nIndex = i ;

	//	m_psThreadParameters[i].hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TravelTreeProc, &m_psThreadParameters[i], CREATE_SUSPENDED, &m_psThreadParameters[i].dwThreadID) ;
	//	if(m_psThreadParameters[i].hThread == NULL)
	//	{
	//		DWORD dwError = GetLastError() ;
	//		TRACE("ErrorCode:%d in CSecretToolBrush::Initialize\r\n", dwError) ;
	//		return false ;
	//	}
	//	SetThreadPriority(m_psThreadParameters[i].hThread, THREAD_PRIORITY_ABOVE_NORMAL) ;
	//}

    return true ;
}
void CSecretToolBrush::SetTerrain(CSecretTerrain *pcTerrain)
{
	m_pcTerrain = pcTerrain ;
	//반드시 최초에 액션을 한번 넣어주고 시작한다.
	m_pcTerrain->m_sHeightMap.EndBrushing(0, m_pd3dDevice) ;
}
void CSecretToolBrush::PreComputeBrushPos(Matrix4 *pmatView, D3DXMATRIX *pmatProj)
{
	m_pmatView = pmatView ;
	m_pmatProj = pmatProj ;

	//Matrix4 matProj = MatrixConvert(*pmatProj) ;
	//m_cPicking.SetLine(pvScreenPos, *pmatView, matProj) ;

	int i ;
	float theta, add ;
	if(m_nBrushType == CSecretToolBrush::SQUARE)
	{
		add = deg_rad(90) ;
		theta = deg_rad(45) ;
	}
	else if(m_nBrushType == CSecretToolBrush::CIRCLE)
	{
		add = deg_rad(360.0f/m_nNumBoundary) ;
		theta = 0 ;
	}

	for(i=0 ; i<m_nNumBoundary ; i++, theta += add)
	{
		m_pvBoundaryPos1[i].x = m_vCenterPos.x + cosf(theta)*m_fRadius ;
		m_pvBoundaryPos1[i].y = 0 ;
		m_pvBoundaryPos1[i].z = m_vCenterPos.z + sinf(theta)*m_fRadius ;

		//TRACE("BoundaryPos1 (%07.03f %07.03f %07.03f)\r\n", m_pvBoundaryPos1[i].x, m_pvBoundaryPos1[i].y, m_pvBoundaryPos1[i].z) ;

	}
	ResetIntersected() ;
}

void CSecretToolBrush::ComputeBrushPos(float fTimeSinceLastFrame, Vector3 *pvCamera, char *pszMsg)
{
	//m_pmatView = pmatView ;
	//m_pmatProj = pmatProj ;

	//Matrix4 matProj = MatrixConvert(*pmatProj) ;
	//m_cPicking.SetLine(pvScreenPos, *pmatView, matProj) ;

    int i ;
	static bool bFirst=false ;
	char sz2[256], sz3[256] ;
	ZeroMemory(sz2, 256) ;
	ZeroMemory(sz3, 256) ;

	geo::STriangle tri ;
	//if(m_cPicking.IsPickingPoint(tri))
	if(m_pcPicking->m_bIntersect)
	{
		m_vtPrevPicking = m_vtCurPicking ;
		memcpy(&m_vtCurPicking, m_pcPicking->GetCurVertex(), sizeof(SPickingVertex)) ;

		if(!bFirst)
		{
			m_vtPrevPicking = m_vtCurPicking ;
			bFirst = true ;
		}

		if(IsAttr(CSecretToolBrush::SNAPGRID))
		{
			m_vCenterPos.x = floorf(m_vtCurPicking.pos.x)+0.5f ;
			m_vCenterPos.y = m_vtCurPicking.pos.y ;
			m_vCenterPos.z = floorf(m_vtCurPicking.pos.z)+0.5f ;
		}
		else
			m_vCenterPos = m_vtCurPicking.pos ;

        sprintf(sz2, "After Picking Pos(%03.07f %03.07f %03.07f) tex(%03.07f %03.07f)", enumVector(m_vCenterPos), m_vtCurPicking.tex.x, m_vtCurPicking.tex.y) ;
		//m_cDecal.SetPos(vPos) ;
		//m_cDecal.MakingBoundingPlanes() ;
	}

	for(i=0 ; i<m_nNumBoundary ; i++)
	{
		m_pvBoundaryPos1[i].y = m_pcTerrain->m_sHeightMap.GetHeightMap(m_pvBoundaryPos1[i].x, m_pvBoundaryPos1[i].z) ;
	}
	m_bIntersectedAll = true ;

	//float fWidth = 50.0f/(m_vtCurPicking.pos-(*pvCamera)).Magnitude() ;
	//if(fWidth < 2.5f)	fWidth = 2.5f ;
	//else if(fWidth > 2.0f) fWidth = 2.0f ;
	//m_pLine->SetWidth(fWidth) ;

	//m_vCenterPos.set(0, 0, 0) ;

    sprintf(pszMsg, "%s", sz2) ;

	//m_vCenterPos.set(0, 0, 0) ;

	//float theta, add ;
	//if(m_nBrushType == BRUSHTYPE::SQUARE)
	//{
	//	add = deg_rad(90) ;
	//	theta = deg_rad(45) ;
	//}
	//else if(m_nBrushType == BRUSHTYPE::CIRCLE)
	//{
	//	add = deg_rad(360.0f/m_nNumBoundary) ;
	//	theta = 0 ;
	//}

	//for(i=0 ; i<m_nNumBoundary ; i++, theta += add)
	//{
	//	m_pvBoundaryPos1[i].x = m_vCenterPos.x + cosf(theta)*m_fRadius ;
	//	m_pvBoundaryPos1[i].y = 0 ;
	//	m_pvBoundaryPos1[i].z = m_vCenterPos.z + sinf(theta)*m_fRadius ;
	//}
	//ResetIntersected() ;

	//현재 보여지는 지형의 트리에서 m_vCenterPos로 여행을 해서 m_fRadius 의 크기보다 한단계 더 큰 트리까지 도달한다.
	//그 트리에서 각 BoundaryPos 위에 있는 삼각형을 찾아내고 y값을 얻어낸다.

	//sprintf(sz3, "Decal Pos(%03.07f %03.07f %03.07f)", m_cDecal.GetPos()->x, m_cDecal.GetPos()->y, m_cDecal.GetPos()->z) ;
	//sprintf(pszMsg, "%s %s %s", sz1, sz2, sz3) ;

	//int nTris = m_pcTerrain->m_pcZQuadTree->GetIntersectedTriangle(m_cDecal.GetPos(), m_cDecal.GetRadius(), m_pcTerrain->m_sHeightMap.pVertices, m_ppsIntersectedTriangles) ;
	//m_cDecal.AddInBoundTriangles(m_ppsIntersectedTriangles, nTris) ;

	//for(i=0 ; i<m_nNumThread ; i++)
	//{
	//	m_psThreadParameters[i].bIntersected = false ;
	//	ResumeThread(m_psThreadParameters[i].hThread) ;
	//}

	m_pcTerrain->m_sHeightMap.nBrushDecision = m_nBrushDecision ;

	//if(bBrushing)//마우스버튼이 눌려진 상태
	if(m_nBrushDecision > IDLEING)
	{
		if(m_nBrushingStatus == NO_BRUSHING)
		{
			m_nBrushingStatus = START_BRUSHING ;
			m_fElapsedTime = 0.0f ;
			m_fBrushMustCount = 0.0f ;
			m_fBrushHasCounted = 0 ;
			//m_fBrushCountPerSec = 20.0f ;
		}
		else if(m_nBrushingStatus == START_BRUSHING)
			m_nBrushingStatus = BRUSHING ;
	}
	else//마우스버튼이 눌려지지 않는 상태
	{
		if(m_nBrushingStatus == START_BRUSHING)
            m_nBrushingStatus = END_BRUSHING ;
		else if(m_nBrushingStatus == BRUSHING)
			m_nBrushingStatus = END_BRUSHING ;
		else if(m_nBrushingStatus == END_BRUSHING)
			m_nBrushingStatus = NO_BRUSHING ;
	}

	g_cDebugMsg.SetDebugMsg(0, "BrushingStatus : %d", m_nBrushingStatus) ;
	g_cDebugMsg.SetDebugMsg(1, "CurAction : %d", m_pcTerrain->m_sHeightMap.pcActionManagement->m_nCurAction) ;

	for(i=0 ; i<MAXNUM_ACTION ; i++)
	{
		if(m_pcTerrain->m_sHeightMap.pcActionManagement->m_apcAction[i])
			g_cDebugMsg.SetDebugMsg(i+2, "Action[%02d] index:%d Enable:%d",
			i, m_pcTerrain->m_sHeightMap.pcActionManagement->m_apcAction[i]->m_nIndex, m_pcTerrain->m_sHeightMap.pcActionManagement->m_apcAction[i]->m_bEnable) ;
		else
			g_cDebugMsg.SetDebugMsg(i+2, "Action[%02d] index:Nothing", i) ;
	}

	//static int c=0 ;
	//if(c++ < 30)
	//{
	//	m_fElapsedTime += fTimeSinceLastFrame ;
	//	m_fBrushMustCount = m_fBrushCountPerSec*m_fElapsedTime ;
	//	m_nBrushCurCount = (int)(m_fBrushMustCount-m_fBrushHasCounted) ;
	//	m_fBrushHasCounted += m_nBrushCurCount ;
	//	TRACE("curCount=%03d, mustCount=%03.07f ElapsedTime=%03.07f\r\n", m_nBrushCurCount, m_fBrushMustCount, m_fElapsedTime) ;
	//}
	//else
	//	c = 0 ;

	//if(m_cTimer.Process_fixed())
	{
		if(m_nBrushingStatus == START_BRUSHING)//눌려진순간 한번 찍고
		{
			//Undo를 한번도 안하고 계속 그리는 거라면 그냥 그대로 그려도 되는데
			//Undo를 1번이상 한 상태에서 다시 그린다면 현재 텍스쳐 화면을 그대로 원래 cTerrain->m_sHeightMap.SaveKeepingAlpha에서 찾아서
			//텍스쳐 복사를 한 상태에서 부러슁을 한다.
			//if(m_pcTerrain->m_sHeightMap.pcActionManagement->GetCurNumAction() > 0)
			//{
			//	CActionBrushing *pcAction = (CActionBrushing *)m_pcTerrain->m_sHeightMap.pcActionManagement->GetCurAction() ;
			//	m_pcTerrain->m_sHeightMap.apTexAlpha[m_nSelectedTexNum] = m_pcTerrain->m_sHeightMap.apSaveKeepingAlpha[m_nSelectedTexNum] ;
			//	CopyTexture(pcAction->m_pTex, m_pcTerrain->m_sHeightMap.apTexAlpha[m_nSelectedTexNum], m_pd3dDevice) ;
			//	m_pcTerrain->m_sHeightMap.pcActionManagement->InsertAction(NULL) ;//현재 액션넘버가 0보다 크기때문에 InsertAction은 모두 배열을 모두 한칸씩 앞당길 것이다.
			//}//아~~ 언두리두기능 일단 정지 -_- 20090302

			m_pcTerrain->m_sHeightMap.BrushingAlpha(m_nSelectedTexNum, 1, m_vtPrevPicking.tex, m_vtCurPicking.tex, m_fRadius*m_pcTerrain->m_sHeightMap.nAlphaMapSize/m_pcTerrain->m_sHeightMap.lWidth) ;
			//m_pcTerrain->m_sHeightMap.DrawCircleBrushing(m_nSelectedTexNum, (int)m_vtCurPicking.tex.x, (int)m_vtCurPicking.tex.y, (int)m_fRadius, 0xffffffff) ;
		}
		else if(m_nBrushingStatus == BRUSHING)//브러싱 하는 동안 찍고
		{
			m_fElapsedTime += fTimeSinceLastFrame ;
			m_fBrushMustCount = m_fBrushCountPerSec*m_fElapsedTime ;
			m_nBrushCurCount = (int)(m_fBrushMustCount-m_fBrushHasCounted) ;
			m_fBrushHasCounted += m_nBrushCurCount ;

            if(m_nBrushCurCount > 0)
				m_pcTerrain->m_sHeightMap.BrushingAlpha(m_nSelectedTexNum, m_nBrushCurCount, m_vtPrevPicking.tex, m_vtCurPicking.tex, m_fRadius*m_pcTerrain->m_sHeightMap.nAlphaMapSize/m_pcTerrain->m_sHeightMap.lWidth) ;
		}
		else if(m_nBrushingStatus == END_BRUSHING)//끝난다음 현재상태를 저장한다.
		{
			//아~~ 언두리두기능 일단 정지 -_- 20090302
			//m_pcTerrain->m_sHeightMap.EndBrushing(m_nSelectedTexNum, m_pd3dDevice) ;
		}
	}
	m_bProcessed = true ;
}

void CSecretToolBrush::ResetIntersected()
{
	ZeroMemory(m_abIntersected, sizeof(bool)*32) ;
	m_bIntersectedAll = false ;
}
/*
bool CSecretToolBrush::IntersectedQuadTreeTile(Vector3 vTileCenter, float fTileRadius, int nPatchType, CSecretTile *pcTile)
{
	if( (m_vCenterPos-vTileCenter).Magnitude() > (m_fRadius+fTileRadius) )
		return false ;

    geo::STriangle *psTriangle ;
	geo::SLine line(Vector3(0, 1000.0f, 0), Vector3(0, -1, 0), 2000.0f) ;
	Vector3 v ;

	int i, n, t ;
    for(i=0 ; i<4 ; i++)
	{
		if((m_vCenterPos.x >= pcTile->m_apsDivideTile[i].avEdge[0].x) && (m_vCenterPos.x <= pcTile->m_apsDivideTile[i].avEdge[1].x)
			&& (m_vCenterPos.z <= pcTile->m_apsDivideTile[i].avEdge[0].z) && (m_vCenterPos.z >= pcTile->m_apsDivideTile[i].avEdge[2].z))
		{
			for(t=0 ; t<pcTile->m_apsDivideTile[i].anTriangleCount[nPatchType] ; t++)
			{
				m_bIntersectedAll = true ;
				psTriangle = pcTile->m_apsDivideTile[i].aapsTriangles[nPatchType][t] ;

				for(n=0 ; n<m_nNumBoundary ; n++)
				{
					if(!m_abIntersected[n])
					{
						m_bIntersectedAll = false ;
						line.s.x = m_pvBoundaryPos1[n].x ;
						line.s.z = m_pvBoundaryPos1[n].z ;

						if(IntersectLinetoTriangle(line, *psTriangle, v, true) == geo::INTERSECT_POINT)
						{
							v *= (*m_pmatView) ;
							m_pvBoundaryPos2[n] = D3DXVECTOR3(v.x, v.y, v.z) ;
							m_abIntersected[n] = true ;
						}
					}
				}
				if(m_bIntersectedAll)//모든점이 교차되었다면
					return true ;
			}
		}
	}
	return true ;
}
*/

bool CSecretToolBrush::IntersectedQuadTreeTile(Vector3 vTileCenter, float fTileRadius, TERRAINVERTEX *pVertices, TERRAININDEX *pIndices, int nTriangles)
{
	//if( (m_vCenterPos-vTileCenter).Magnitude() > (m_fRadius+fTileRadius) )
	//	return false ;
	float x, z, l ;
	x = (vTileCenter.x-m_vCenterPos.x) ;
	z = (vTileCenter.z-m_vCenterPos.z) ;
	l = sqrtf((x*x)+(z*z)) ;
	if(l > (m_fRadius+fTileRadius))
		return false ;

	geo::STriangle tri ;
	geo::SLine line(Vector3(0, 1000.0f, 0), Vector3(0, -1, 0), 2000.0f) ;
	Vector3 v ;
	
    int i, n ;
	for(i=0 ; i<nTriangles ; i++)
	{
		tri.set(Vector3(pVertices[pIndices[i].i1].pos.x, pVertices[pIndices[i].i1].pos.y, pVertices[pIndices[i].i1].pos.z),
				Vector3(pVertices[pIndices[i].i2].pos.x, pVertices[pIndices[i].i2].pos.y, pVertices[pIndices[i].i2].pos.z),
				Vector3(pVertices[pIndices[i].i3].pos.x, pVertices[pIndices[i].i3].pos.y, pVertices[pIndices[i].i3].pos.z)) ;

		m_bIntersectedAll = true ;
		for(n=0 ; n<m_nNumBoundary ; n++)
		{
			if(!m_abIntersected[n])
			{
				m_bIntersectedAll = false ;
				line.s.x = m_pvBoundaryPos1[n].x ;
				line.s.z = m_pvBoundaryPos1[n].z ;

                if(IntersectLinetoTriangle(line, tri, v, true) == geo::INTERSECT_POINT)
				{
					v *= (*m_pmatView) ;
					m_pvBoundaryPos2[n] = D3DXVECTOR3(v.x, v.y, v.z) ;
                    m_abIntersected[n] = true ;
				}
			}
		}
		if(m_bIntersectedAll)//모든점이 교차되었다면
			return true ;
	}
	return true ;
}

bool CSecretToolBrush::IntersectedQuadTreeTile(Vector3 vTileCenter, float fTileRadius, int nNumLevel, int nPatchType, int tl, CSecretTileSubTree *pcSubTree)
{
	float x, z, l ;
	x = (vTileCenter.x-m_vCenterPos.x) ;
	z = (vTileCenter.z-m_vCenterPos.z) ;
	l = sqrtf((x*x)+(z*z)) ;
	if(l > (m_fRadius+fTileRadius))
		return false ;

	Vector3 v ;
	geo::SLine line(Vector3(0, 1000.0f, 0), Vector3(0, -1, 0), 2000.0f) ;
	int nTriangles = pcSubTree->GetIntersectPos(&m_vCenterPos, m_fRadius, m_pcTerrain->m_sHeightMap.pVertices, m_psIntersectedTriangles, 0, nNumLevel, nPatchType, tl) ;

	assert(nTriangles <= MAXNUM_INTERSECTEDTRIANGLES) ;

	int i, n ;
	for(i=0 ; i<nTriangles ; i++)
	{
		m_bIntersectedAll = true ;
		for(n=0 ; n<m_nNumBoundary ; n++)
		{
			if(!m_abIntersected[n])
			{
				m_bIntersectedAll = false ;
				line.s.x = m_pvBoundaryPos1[n].x ;
				line.s.z = m_pvBoundaryPos1[n].z ;

                if(IntersectLinetoTriangle(line, m_psIntersectedTriangles[i], v, true) == geo::INTERSECT_POINT)
				{
					v *= (*m_pmatView) ;
					m_pvBoundaryPos2[n] = D3DXVECTOR3(v.x, v.y, v.z) ;
                    m_abIntersected[n] = true ;
				}
			}
		}
		if(m_bIntersectedAll)//모든점이 교차되었다면
			return true ;
	}

	return true ;
}

void CSecretToolBrush::SettleRemainPoint()
{
	int n ;
	Vector3 v ;
	for(n=0 ; n<m_nNumBoundary ; n++)
	{
		if(!m_abIntersected[n])
		{
			v = m_pvBoundaryPos1[n] * (*m_pmatView) ;
			m_pvBoundaryPos2[n] = D3DXVECTOR3(v.x, v.y, v.z) ;
		}
	}
}
void CSecretToolBrush::SetBrushDecision(int nDecision)
{
	m_nBrushDecision = nDecision ;
}
int CSecretToolBrush::GetBrushDecision()
{
	return m_nBrushDecision ;
}
void CSecretToolBrush::SetSize(float fRadius)
{
    m_fRadius = fRadius ;
}
void CSecretToolBrush::SetStrength(int nStrength)
{
    m_fBrushCountPerSec = (float)nStrength ;
}

void CSecretToolBrush::Render()
{
	if(!m_bProcessed || !(m_nAttr & CSecretToolBrush::DRAWBRUSH))
		return ;

	int i ;
	static D3DXVECTOR3 avPos[33] ;

	if(m_bIntersectedAll)
	{
		for(i=0 ; i<m_nNumBoundary ; i++)
			avPos[i] = m_pvBoundaryPos2[i] ;

		avPos[i] = avPos[0] ;

		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;

		m_pLine->Begin() ;
		m_pLine->DrawTransform(avPos, m_nNumBoundary+1, m_pmatProj, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)) ;
		m_pLine->End() ;

		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) ;
	}
	m_bProcessed = false ;
}
void CSecretToolBrush::SetEnable(bool bEnable)
{
	m_bEnable = bEnable ;
}
void CSecretToolBrush::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_nAttr |= nAttr ;
	else
		m_nAttr &= (~nAttr) ;
}
int CSecretToolBrush::GetAttr()
{
	return m_nAttr ;
}
bool CSecretToolBrush::IsAttr(int nAttr)
{
	if(m_nAttr & nAttr)
		return true ;
	return false ;
}

/*
DWORD TravelTreeProc(SThreadParameters *psParameters)
{
	int i ;
	Vector3 vIntersectedPos ;
	geo::STriangle *psIntersectedTriangle ;
	bool bIntersected ;

	while(true)
	{
		if(psParameters->bExitThread)
			break ;

		for(i=psParameters->nStart ; i<=psParameters->nEnd ; i++)
		{
			psParameters->pcToolBrush->m_pvBoundaryPos2[i].x = psParameters->pcToolBrush->m_pvBoundaryPos1[i].x ;
			psParameters->pcToolBrush->m_pvBoundaryPos2[i].y = psParameters->pcToolBrush->m_pvBoundaryPos1[i].y ;
			psParameters->pcToolBrush->m_pvBoundaryPos2[i].z = psParameters->pcToolBrush->m_pvBoundaryPos1[i].z ;

			bIntersected = false ;
			psParameters->pcToolBrush->m_pcTerrain->m_pcZQuadTree->GetIntersectedTriangle(&psParameters->pcToolBrush->m_pvBoundaryPos1[i], psParameters->pcToolBrush->m_pcTerrain->m_sHeightMap.pVertices, &vIntersectedPos, &psIntersectedTriangle, &bIntersected) ;

			if(bIntersected)
			{
				//TRACE("[%02d] Before BoundaryPos(%07.03f %07.03f %07.03f)\r\n",
				//	i, vIntersectedPos.x, vIntersectedPos.y, vIntersectedPos.z) ;

				vIntersectedPos *= (*psParameters->pcToolBrush->m_pmatView) ;
				psParameters->pcToolBrush->m_pvBoundaryPos2[i].x = vIntersectedPos.x ;
				psParameters->pcToolBrush->m_pvBoundaryPos2[i].y = vIntersectedPos.y ;
				psParameters->pcToolBrush->m_pvBoundaryPos2[i].z = vIntersectedPos.z ;

				//TRACE("[%02d] After BoundaryPos(%07.03f %07.03f %07.03f)\r\n",
				//	i, vIntersectedPos.x, vIntersectedPos.y, vIntersectedPos.z) ;
			}
			else
			{
                vIntersectedPos = psParameters->pcToolBrush->m_pvBoundaryPos1[i] ;
				vIntersectedPos *= (*psParameters->pcToolBrush->m_pmatView) ;
				psParameters->pcToolBrush->m_pvBoundaryPos2[i].x = vIntersectedPos.x ;
				psParameters->pcToolBrush->m_pvBoundaryPos2[i].y = vIntersectedPos.y ;
				psParameters->pcToolBrush->m_pvBoundaryPos2[i].z = vIntersectedPos.z ;
			}
		}

		psParameters->bIntersected = true ;
		SuspendThread(psParameters->hThread) ;
	}

	CloseHandle(psParameters->hThread) ;
	psParameters->hThread = NULL ;
	//TRACE("Index=%02d CloseHandle\r\n", psParameters->nIndex) ;

	return 0 ;
}
/*
DWORD TravelTreeProc(CSecretToolBrush *pcToolBrush)
{
	int i ;
	Vector3 vIntersectedPos ;
	geo::STriangle *psIntersectedTriangle ;
	bool bIntersected ;

	while(true)
	{
		if(pcToolBrush->m_bExitThread)
			break ;

		for(i=0 ; i<pcToolBrush->m_nNumBoundary ; i++)
		{
			pcToolBrush->m_pvBoundaryPos2[i].x = pcToolBrush->m_pvBoundaryPos1[i].x ;
			pcToolBrush->m_pvBoundaryPos2[i].y = pcToolBrush->m_pvBoundaryPos1[i].y ;
			pcToolBrush->m_pvBoundaryPos2[i].z = pcToolBrush->m_pvBoundaryPos1[i].z ;

			bIntersected = false ;
			pcToolBrush->m_pcTerrain->m_pcZQuadTree->GetIntersectedTriangle(&pcToolBrush->m_pvBoundaryPos1[i], pcToolBrush->m_pcTerrain->m_sHeightMap.pVertices, &vIntersectedPos, &psIntersectedTriangle, &bIntersected) ;

			if(bIntersected)
			{
				//TRACE("[%02d] Before BoundaryPos(%07.03f %07.03f %07.03f)\r\n",
				//	i, vIntersectedPos.x, vIntersectedPos.y, vIntersectedPos.z) ;

				vIntersectedPos *= (*pcToolBrush->m_pmatView) ;
				pcToolBrush->m_pvBoundaryPos2[i].x = vIntersectedPos.x ;
				pcToolBrush->m_pvBoundaryPos2[i].y = vIntersectedPos.y ;
				pcToolBrush->m_pvBoundaryPos2[i].z = vIntersectedPos.z ;

				//TRACE("[%02d] After BoundaryPos(%07.03f %07.03f %07.03f)\r\n",
				//	i, vIntersectedPos.x, vIntersectedPos.y, vIntersectedPos.z) ;
			}
		}

        pcToolBrush->m_pvBoundaryPos2[i].x = pcToolBrush->m_pvBoundaryPos2[0].x ;
		pcToolBrush->m_pvBoundaryPos2[i].y = pcToolBrush->m_pvBoundaryPos2[0].y ;
		pcToolBrush->m_pvBoundaryPos2[i].z = pcToolBrush->m_pvBoundaryPos2[0].z ;


		pcToolBrush->m_bIntersected = true ;
		SuspendThread(pcToolBrush->m_hThread) ;
	}

	CloseHandle(pcToolBrush->m_hThread) ;
	pcToolBrush->m_hThread = NULL ;
	return true ;
}
*/