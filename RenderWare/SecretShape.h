#pragma once

class CSecretShape
{
public :
	enum KIND { KNID_LINE=1 } ;
	enum ATTR { ATTR_CLOSED=0x01, ATTR_ONTERRAIN=0x02, ATTR_RENDER=0x04, ATTR_DDL=0x08, ATTR_SAMESIZE=0x10 } ;
protected :
	int m_nKind, m_nAttr ;
	char m_szName[256] ;

public :
	CSecretShape(int nKind) {m_nKind=nKind; m_nAttr=0;}
	virtual ~CSecretShape() {}

	int GetKind() {return m_nKind;}
	int GetAttr() {return m_nAttr;}

	void AddAttr(int nAttr, bool bEnable=true) ;

	void SetName(char *pszName) ;
	char *GetName() ;

	virtual void Render() {}
	virtual void Release() {}
} ;