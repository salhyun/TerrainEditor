#include "SecretShape.h"
#include <stdio.h>

void CSecretShape::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_nAttr |= nAttr ;
	else
		m_nAttr &= (~nAttr) ;
}
void CSecretShape::SetName(char *pszName)
{
	sprintf(m_szName, "%s", pszName) ;
}
char *CSecretShape::GetName()
{
	return m_szName ;
}