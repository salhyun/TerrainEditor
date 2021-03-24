#include "SecretThread.h"

CSecretThread::CSecretThread()
{
	m_dwThreadID = m_dwExitCode = m_dwSuspendedCount = 0 ;
    m_hThread = 0 ;
}
CSecretThread::~CSecretThread()
{
	this->Release() ;
}

void CSecretThread::Release()
{
	if(m_hThread)
	{
		CloseHandle(m_hThread) ;
		m_hThread = NULL ;
	}
}