#pragma once

class CSecretProto
{
protected :
	bool m_bEnable ;
	int m_nKind ;
	int m_nAttr ;

public :
	CSecretProto() ;
	~CSecretProto() ;

    void SetEnable(bool bEnable=true) ;
	bool IsEnable() ;
	void SetKind(int nKind) ;
	int GetKind() ;
	void AddAttr(int nAttr, bool bEnable=true) ;
	int GetAttr() ;
} ;