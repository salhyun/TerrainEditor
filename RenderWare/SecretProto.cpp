#include "SecretProto.h"

CSecretProto::CSecretProto()
{
	m_bEnable = false ;
	m_nKind = m_nAttr = 0 ;
}
CSecretProto::~CSecretProto()
{
}

void CSecretProto::SetEnable(bool bEnable)
{
	m_bEnable = bEnable ;
}
bool CSecretProto::IsEnable()
{
	return m_bEnable ;
}
void CSecretProto::SetKind(int nKind)
{
	m_nKind = nKind ;
}
int CSecretProto::GetKind()
{
    return m_nKind ;
}
void CSecretProto::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_nAttr |= nAttr ;
	else
		m_nAttr &= (~nAttr) ;
}
int CSecretProto::GetAttr()
{
	return m_nAttr ;
}