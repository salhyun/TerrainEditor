#pragma once

#include <WTypes.h>

struct sManagementBuffer
{
	void *pbyAdress ;
	bool bEnable ;
} ;

class CArrangedBuffer
{
private :
	BYTE *m_pbyBuffer ;
	sManagementBuffer *m_pManagement ;
	ULONG m_lTotalSize, m_lUnitSize ;
	ULONG m_lNumUnit, m_lAllocNode ;

public :
	CArrangedBuffer()
	{
		m_pbyBuffer=NULL ;
		m_pManagement=NULL ;
	}
	~CArrangedBuffer()
	{
        ReleaseAll() ;
	}

    void Initialize(ULONG lTotalSize, ULONG lUnitSize)
	{
		m_pbyBuffer = new BYTE[lTotalSize] ;
		m_lNumUnit = lTotalSize/lUnitSize ;

		m_pManagement = new sManagementBuffer[m_lNumUnit] ;

		m_lTotalSize = lTotalSize ;
		m_lUnitSize = lUnitSize ;

		ULONG i, n=0 ;
		for(i=0 ; i<m_lNumUnit ; i++, n+=lUnitSize)
		{
			m_pManagement[i].pbyAdress = &m_pbyBuffer[n] ;
			m_pManagement[i].bEnable = false ;
		}

		m_lAllocNode=0 ;
	}
	void *AllocUnit()
	{
		ULONG i ;
		for(i=0 ; i<m_lNumUnit ; i++)
		{
			if(!m_pManagement[m_lAllocNode].bEnable)
			{
				m_pManagement[m_lAllocNode].bEnable = true ;
				return (void *)(m_pManagement[m_lAllocNode++].pbyAdress) ;
			}

			if(++m_lAllocNode >= m_lNumUnit)
				m_lAllocNode = 0 ;
		}

		return NULL ;
	}
	bool DeleteUnit(void *pvBuffer)
	{
		ULONG i ;
		for(i=0 ; i<m_lNumUnit ; i++)
		{
			if(m_pManagement[i].bEnable && (m_pManagement[i].pbyAdress == pvBuffer))
			{
				m_pManagement[i].bEnable = false ;
				return true ;
			}
		}
		return false ;
	}

	void DeleteAll()//메모리 해제는 하지않고 내용삭제
	{
		ULONG i ;
		for(i=0 ; i<m_lNumUnit ; i++)
			m_pManagement[i].bEnable = false ;
	}

	void ReleaseAll()//잡아놓은 메모리 해제
	{
		if(m_pManagement)
		{
			delete []m_pManagement ;
			m_pManagement = NULL ;
		}
		if(m_pbyBuffer)
		{
			delete []m_pbyBuffer ;
			m_pbyBuffer = NULL ;
		}
	}
} ;