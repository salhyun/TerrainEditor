#include "ObjectColorLoader.h"
#include "def.h"

SObjectColor::SObjectColor()
{
	bEnable = false ;
	sprintf(szName, "") ;
	r = g = b = 0.0f ;
}
SObjectColor::~SObjectColor()
{
}

CObjectColorLoader::CObjectColorLoader()
{
	m_nNumObject = 0 ;
	m_psObjectColors = NULL ;
	m_fDefaultColor = 0.0f ;
}
CObjectColorLoader::~CObjectColorLoader()
{
	Release() ;
}
void CObjectColorLoader::Release()
{
	SAFE_DELETEARRAY(m_psObjectColors) ;
}
void CObjectColorLoader::Initialize(int nNumObject)
{
	m_nNumObject = nNumObject ;
	m_psObjectColors = new SObjectColor[m_nNumObject] ;
}
bool CObjectColorLoader::AddObject(char *pszName)
{
	SObjectColor *psObjectColors = m_psObjectColors ;
	for(int i=0 ; i<m_nNumObject ; i++, psObjectColors++)
	{
		if(!psObjectColors->bEnable)
		{
			sprintf(psObjectColors->szName, "%s", pszName) ;
			psObjectColors->r = psObjectColors->g = psObjectColors->b = m_fDefaultColor ;
			TRACE("add object color \"%s\" (r=%g, g=%g, b=%g)\r\n", pszName, psObjectColors->r, psObjectColors->g, psObjectColors->b) ;
			psObjectColors->bEnable = true ;
			return true ;
		}
	}
	return false ;
}
bool CObjectColorLoader::Load(char *pszFileName)
{
	FILE *pf = fopen(pszFileName, "r+b") ;
	if(pf == NULL)
		return false ;

	fread(&m_nNumObject, 4, 1, pf) ;

	m_psObjectColors = new SObjectColor[m_nNumObject] ;
	fread(m_psObjectColors, sizeof(SObjectColor), m_nNumObject, pf) ;

	fclose(pf) ;
	return true ;
}
bool CObjectColorLoader::Save(char *pszFileName)
{
	FILE *pf = fopen(pszFileName, "w+b") ;
	if(pf == NULL)
		return false ;

	fwrite(&m_nNumObject, 4, 1, pf) ;
	fwrite(m_psObjectColors, sizeof(SObjectColor), m_nNumObject, pf) ;

	fclose(pf) ;
	return true ;	
}
void CObjectColorLoader::SetDefault()
{
	if(m_nNumObject > 0)
	{
		for(int i=0 ; i<m_nNumObject ; i++)
			m_psObjectColors[i].r = m_psObjectColors[i].g = m_psObjectColors[i].b = m_fDefaultColor ;
	}
}
bool CObjectColorLoader::FindColor(char *pszName, Vector3 &v)
{
	SObjectColor *psObjectColors = m_psObjectColors ;
	for(int i=0 ; i<m_nNumObject ; i++, psObjectColors++)
	{
		if(psObjectColors->bEnable)
		{
			if(!strcmp(pszName, psObjectColors->szName))
			{
				v.set(psObjectColors->r, psObjectColors->g, psObjectColors->b) ;
				return true ;
			}
		}
	}
	return false ;
}
bool CObjectColorLoader::InputColor(char *pszName, Vector3 &v)
{
	SObjectColor *psObjectColors = m_psObjectColors ;
	for(int i=0 ; i<m_nNumObject ; i++, psObjectColors++)
	{
		if(psObjectColors->bEnable)
		{
			if(!strcmp(pszName, psObjectColors->szName))
			{
				psObjectColors->r = v.x ;
				psObjectColors->g = v.y ;
				psObjectColors->b = v.z ;
				return true ;
			}
		}
	}
	return false ;
}