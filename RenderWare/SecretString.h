#pragma once

#pragma warning(disable:4996)//warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

#include <stdio.h>
#include <string.h>
#include "stdarg.h"

#define SAL_MAXNUMSTR 256

class CSecretString
{
public :
	enum {FIXED, INSTANCE};
private :
	char m_szString[SAL_MAXNUMSTR] ;

public :
	bool m_bEnable ;
    unsigned long m_dwAttr ;

public :
	CSecretString(char *pszStr="") {sprintf(this->m_szString, "%s", pszStr) ; m_bEnable=false; m_dwAttr=INSTANCE;}
	~CSecretString() {}

	int GetLength() {return (int)strlen(this->m_szString) ;}
	void Clear() {sprintf(m_szString, ""); m_bEnable=false;}
	bool IsEmpty() {return !m_bEnable;}
	unsigned long GetAttr() {return m_dwAttr;}
	void SetAttr(unsigned long dwAttr) {m_dwAttr=dwAttr;}

	char *GetString()
	{
//		if(!m_bEnable)
//			return NULL ;

		return m_szString ;
	}

	bool SetString(char *pszStr, ...)
	{
		if(strlen(pszStr) >= SAL_MAXNUMSTR)
			return false ;

		va_list ap ;
		va_start(ap, pszStr) ;
		vsprintf(this->m_szString, pszStr, ap) ;
		m_bEnable=true ;
		return true ;
	}

	bool SetString(char *pszStr)
	{
		if(strlen(pszStr) >= SAL_MAXNUMSTR)
			return false ;

		sprintf(m_szString, "%s", pszStr) ;
		m_bEnable=true ;
		return true ;
	}

	int Compare(char *pszStr)
	{
		return strcmp(this->m_szString, pszStr) ;
	}

	CSecretString operator +(CSecretString &T)
	{
		CSecretString cSecretStr("") ;
		sprintf(cSecretStr.m_szString, "%s%s", this->m_szString, T.m_szString) ;
		return cSecretStr ;
	}
	CSecretString operator +=(CSecretString &T)
	{
		CSecretString cSecretStr("") ;
		sprintf(cSecretStr.m_szString, "%s%s", this->m_szString, T.m_szString) ;
		return cSecretStr ;
	}

} ;