#pragma once

#include <windows.h>

class CSecretTimer
{
private :
	INT64 m_n64TickPerSec ;
	INT64 m_n64PreTime, m_n64CurTime ;
	INT64 m_n64SumIntervalTime, m_n64IntervalTime ;
	INT64 m_n64Freq ;
	double m_dbResolution ;

	bool m_bEnable ;
	int m_nAvgCount, m_nAvgFrame, m_nFramePerSec ;
	unsigned long m_nFreq ;

public:
	CSecretTimer(void);
	~CSecretTimer(void);

	DWORD Initialize_fixed(int nFrame) ;
	bool Process_fixed() ;

	DWORD Initialize_real() ;
	float Process_real() ;

	int GetAvgFrame() ;
};
