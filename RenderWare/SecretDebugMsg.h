#pragma once

#include "SecretString.h"
#include <windows.h>

/////////////////////////////////////////////////////////////////////////////////
//class CSecretDebugString
/////////////////////////////////////////////////////////////////////////////////

enum DEBUGMSG_INDEX
{
	DEBUGMSG_START=0,
	DEBUGMSG_FPS,
	DEBUGMSG_LIGHT,
	DEBUGMSG_VIEW
} ;

#define MAXNUM_DEBUGSTR 256

class CSecretDebugMsg
{
public :
	HWND m_hwnd ;
	int m_nNumNode, m_nCurNode ;
    CSecretString m_aszDebug[MAXNUM_DEBUGSTR] ;


	UINT m_lTotalTriangles ;//현재 화면에 나타나는 폴리곤수
	UINT m_lTotalVBSize, m_lTotalIBSize, m_lTotalTexSize ;
	float m_ftemp ;

public :
	CSecretDebugMsg() ;
	~CSecretDebugMsg() ;

	void SethWnd(HWND hwnd) {m_hwnd = hwnd;} ;
	void SetDebugMsg(UINT lIndex, char *pszStr, ...) ;
	void DrawString(int x, int y) ;

	void SetTriangles(UINT lNum) ;
	void AddTriangles(UINT lTriangles) ;

	void AddVBSize(UINT lSize) ;
	void SubtractVBSize(UINT lSize) ;
    void AddIBSize(UINT lSize) ;
	void SubtractIBSize(UINT lSize) ;
	void AddTexSize(UINT lSize) ;
} ;