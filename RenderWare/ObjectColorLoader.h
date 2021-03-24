#pragma once

#include "D3DDef.h"
#include "Vector3.h"

struct SObjectColor
{
	bool bEnable ;
	char szName[64] ;
	float r, g, b ;

	SObjectColor() ;
	~SObjectColor() ;
} ;

class CObjectColorLoader
{
public :

private :
	int m_nNumObject ;
	SObjectColor *m_psObjectColors ;
	float m_fDefaultColor ;

public :
	CObjectColorLoader() ;
	~CObjectColorLoader() ;

	void Initialize(int nNumObject) ;
	bool AddObject(char *pszName) ;
	
	bool Load(char *pszFileName) ;
	bool Save(char *pszFileName) ;
	void SetDefault() ;
	bool FindColor(char *pszName, Vector3 &v) ;
	bool InputColor(char *pszName, Vector3 &v) ;

	void Release() ;

} ;