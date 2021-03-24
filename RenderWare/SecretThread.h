#pragma once

#include "windows.h"

class CSecretThread
{
public :
	DWORD m_dwThreadID ;
	DWORD m_dwExitCode ;
	DWORD m_dwSuspendedCount ;

	HANDLE m_hThread ;

public :
	CSecretThread() ;
	~CSecretThread() ;

	void Release() ;
} ;