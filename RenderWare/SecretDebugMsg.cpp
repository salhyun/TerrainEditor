#include "SecretDebugMsg.h"

CSecretDebugMsg::CSecretDebugMsg()
{
	m_hwnd = NULL ;
	m_nNumNode = 0 ;
	m_nCurNode = 0 ;
	for(int i=0 ; i<MAXNUM_DEBUGSTR ; i++)
	{
		m_aszDebug[i].Clear() ;
	}
	m_lTotalTriangles = 0 ;
	m_lTotalVBSize = m_lTotalIBSize = m_lTotalTexSize = 0 ;
}

CSecretDebugMsg::~CSecretDebugMsg()
{
}
void CSecretDebugMsg::SetDebugMsg(UINT lIndex, char *pszStr, ...)
{
	m_aszDebug[lIndex].m_bEnable=true ;

	va_list ap ;
	va_start(ap, pszStr) ;
	vsprintf(m_aszDebug[lIndex].GetString(), pszStr, ap) ;
	//vsprintf(NULL, pszStr, ap) ;
	m_aszDebug[lIndex].SetAttr(CSecretString::FIXED) ;
}

void CSecretDebugMsg::DrawString(int x, int y)
{
	if(m_hwnd == NULL)
		return ;

	int i ;
	//char *pszDebug ;
	HDC hdc = GetDC(m_hwnd) ;
	RECT rt={x, y, 500, y+18} ;

	for(i=0 ; i<MAXNUM_DEBUGSTR ; i++, y += 17)
	{
		if(m_aszDebug[i].m_bEnable)
		{
			::FillRect(hdc, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH)) ;
			DrawText(hdc, m_aszDebug[i].GetString(), m_aszDebug[i].GetLength(), &rt, 0) ;
		}

		rt.top += 17 ;
		rt.bottom += 17 ;
	}

	SendMessage(m_hwnd, WM_PAINT, 0, 0) ;
	ReleaseDC(m_hwnd, hdc) ;
}
void CSecretDebugMsg::SetTriangles(UINT lNum)
{
	m_lTotalTriangles = lNum ;
}
void CSecretDebugMsg::AddTriangles(UINT lTriangles)
{
	m_lTotalTriangles += lTriangles ;
}
void CSecretDebugMsg::AddVBSize(UINT lSize)
{
	m_lTotalVBSize += lSize ;
}
void CSecretDebugMsg::SubtractVBSize(UINT lSize)
{
	m_lTotalVBSize -= lSize ;
}
void CSecretDebugMsg::AddIBSize(UINT lSize)
{
	m_lTotalIBSize += lSize ;
}
void CSecretDebugMsg::SubtractIBSize(UINT lSize)
{
	m_lTotalIBSize -= lSize ;
}
void CSecretDebugMsg::AddTexSize(UINT lSize)
{
	m_lTotalTexSize += lSize ;
}