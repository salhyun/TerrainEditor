#include "SecretTimer.h"

CSecretTimer::CSecretTimer(void)
{
}

CSecretTimer::~CSecretTimer(void)
{
}

DWORD CSecretTimer::Initialize_fixed(int nFrame)
{
	m_dbResolution = 0.001 ;
	m_n64TickPerSec = 1000 ;
	m_bEnable = false ;
	m_nAvgCount = 0 ;
	m_nAvgFrame = 0 ;
	m_n64SumIntervalTime = 0 ;

	LARGE_INTEGER qwTickPerSec ;

	if(!QueryPerformanceFrequency((LARGE_INTEGER *)&qwTickPerSec))
	{
		DWORD dwError = GetLastError() ;
		return dwError ;
	}

	if(nFrame)
	{
		m_n64TickPerSec = qwTickPerSec.QuadPart ;//Number of Tick per Second
		m_dbResolution = (double)(1.0/m_n64TickPerSec) ;
		m_nFreq = (unsigned long)(1.0/(m_dbResolution*(double)nFrame)) ;
	}
	else
	{
		m_n64TickPerSec = qwTickPerSec.QuadPart ;//Number of Tick per Second
		m_dbResolution = (double)(1.0/m_n64TickPerSec) ;
        m_nFreq = 0 ;
	}

	//For ShowCurrent Frame
	m_nFramePerSec = nFrame ;

	if(!QueryPerformanceCounter((LARGE_INTEGER *)&m_n64PreTime))
	{
		DWORD dwError = GetLastError() ;
		return dwError ;
	}	

    return 0 ;
}

bool CSecretTimer::Process_fixed()
{
	QueryPerformanceCounter((LARGE_INTEGER *)&m_n64CurTime) ;

	if(m_nFreq)
	{
		m_n64IntervalTime = m_n64CurTime-m_n64PreTime ;

		if(m_n64IntervalTime >= (INT64)m_nFreq)
		{
			m_n64SumIntervalTime += m_n64IntervalTime ;
			m_nAvgCount++ ;

			m_n64PreTime = m_n64CurTime ;
			m_bEnable = true ;
		}
		else
			m_bEnable = false ;

		if(m_n64SumIntervalTime >= m_n64TickPerSec)
		{
			m_nAvgFrame = m_nAvgCount ;
			m_n64SumIntervalTime = 0 ;
			m_nAvgCount = 0 ;
		}
	}
	else
	{
		m_n64IntervalTime = m_n64CurTime-m_n64PreTime ;
        m_nAvgCount++ ;

		if(m_n64IntervalTime >= m_n64TickPerSec)
		{
			m_nAvgFrame = m_nAvgCount ;
            m_nAvgCount = 0 ;

			m_n64PreTime = m_n64CurTime ;
		}
	}

	return m_bEnable ;
}

int CSecretTimer::GetAvgFrame()
{
	return m_nAvgFrame ;
}

DWORD CSecretTimer::Initialize_real()
{
	m_dbResolution = 0.001 ;
	m_n64TickPerSec = 1000 ;
	m_bEnable = false ;
	m_nAvgCount = 0 ;
	m_nAvgFrame = 0 ;
	m_n64SumIntervalTime = 0 ;

	LARGE_INTEGER qwTickPerSec ;

	if(!QueryPerformanceFrequency((LARGE_INTEGER *)&qwTickPerSec))
	{
		DWORD dwError = GetLastError() ;
		return dwError ;
	}

	m_n64TickPerSec = qwTickPerSec.QuadPart ;//Number of Tick per Second
	m_dbResolution = (double)(1.0/m_n64TickPerSec) ;

	if(!QueryPerformanceCounter((LARGE_INTEGER *)&m_n64PreTime))
	{
		DWORD dwError = GetLastError() ;
		return dwError ;
	}	

    return 0 ;
}

float CSecretTimer::Process_real()
{
	QueryPerformanceCounter((LARGE_INTEGER *)&m_n64CurTime) ;
	m_n64IntervalTime = m_n64CurTime-m_n64PreTime ;
	m_n64PreTime = m_n64CurTime ;

	m_n64SumIntervalTime += m_n64IntervalTime ;
	m_nAvgCount++ ;

    if(m_n64SumIntervalTime >= m_n64TickPerSec)
	{
		m_n64SumIntervalTime=0 ;
		m_nAvgFrame = m_nAvgCount ;
		m_nAvgCount=0 ;
	}

	return (float)((double)m_n64IntervalTime*m_dbResolution) ;
}



