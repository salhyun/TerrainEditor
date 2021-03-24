#include "SecretNode.h"
#include <stdio.h>

CSecretNode::CSecretNode(unsigned long lEigenIndex)//ó�� �����Ҷ� ����� �������� �ο��޴´�.
{
	m_lEigenIndex = lEigenIndex ;
    m_NodeKind = SECRETNODEKIND_NONE ;
	m_nObjectID = 0 ;
	m_nParentID = -1 ;
	m_nMaterialID = -1 ;
	m_pcParentNode = NULL ;
	m_bParent = false ;

	m_lAttr = 0 ;

	sprintf(m_szNodeName, "%s", "none") ;
	sprintf(m_szParentNodeName, "%s", "none") ;

	//TRACE("CSecretNode construction EigenIndex=%d\r\n", lEigenIndex) ;
}

CSecretNode::~CSecretNode()
{
	//TRACE("CSecretNode destruction\r\n") ;
}