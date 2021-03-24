#include <windows.h>
#include "SecretProfile.h"
#include "def.h"

__int64 n64Profile_TickPerSec ;
double dbProfile_timeResolution ;

DWORD Profile_InitializeTime()
{
	LARGE_INTEGER qwTickPerSec ;

	if(!QueryPerformanceFrequency((LARGE_INTEGER *)&qwTickPerSec))
	{
		DWORD dwError = GetLastError() ;
		return dwError ;
	}

	n64Profile_TickPerSec = qwTickPerSec.QuadPart ;//Number of Tick per Second
	dbProfile_timeResolution = (double)(1.0/n64Profile_TickPerSec) ;

	return 0 ;
}

inline void Profile_GetCurrentTime(__int64 *pn64Time)
{
    //LARGE_INTEGER n64CurTime ;
	//QueryPerformanceCounter((LARGE_INTEGER *)&n64CurTime) ;
	QueryPerformanceCounter((LARGE_INTEGER *)pn64Time) ;
}

//###############################################################
//class CSecretProfileNode
//###############################################################
CSecretProfileNode::CSecretProfileNode()
{
	_InitVariables() ;
	TRACE("Construct Profile without NodeName\r\n") ;
}

CSecretProfileNode::CSecretProfileNode(char *pszNodeName, CSecretProfileNode *pcParentNode)
{
	_InitVariables() ;
	sprintf(m_szNodeName, "%s", pszNodeName) ;
	m_pcParentNode = pcParentNode ;
	reset() ;

	TRACE("Construct Profile_%s\r\n", m_szNodeName) ;
}

CSecretProfileNode::~CSecretProfileNode()
{
	CSecretProfileNode *pcSiblingNode ;
	pcSiblingNode = GetSiblingNode() ;
    SAFE_DELETE(pcSiblingNode) ;

	SAFE_DELETE(m_pcChildNode) ;

	TRACE("Destroy Profile_%s\r\n", m_szNodeName) ;
}

void CSecretProfileNode::_InitVariables()
{
	m_nCallingCounter = 0 ;
	m_n64StartTime = 0 ;
	m_fElapsedTime = 0 ;
	m_fTotalTime = 0 ;
	m_lChildCounter = 0 ;
	m_lTreeLevel = 0 ;
	m_lOderOfSibling = 0 ;
	m_pcChildNode = m_pcParentNode = m_pcSiblingNode = NULL ;
}

CSecretProfileNode *CSecretProfileNode::GetSubNode(char *pszNodeName)
{
	CSecretProfileNode *pcChildNode = m_pcChildNode ;
	while(pcChildNode)
	{
		if( strcmp(pszNodeName, pcChildNode->GetNodeName()) == 0 )
			return pcChildNode ;

		pcChildNode = pcChildNode->GetSiblingNode() ;
	}

	//*새로운 노드가 추가되어서 트리에 더해지는 형태*
	//새로추가된 노드는 이전에 부모의 자식이었던 노드를 사촌(Sibling)으로 가리키게 된다.
	//그러니까 가장먼저 추가된 노드가 첫째, 두번째 추가된 노드가 둘째, 세번째 추가된 노드가 셋째, ...
	//그리고
	//셋째가 둘째를 사촌(Sibling)으로 가리키고,
	//둘째는 첫째를 사촌(Sibling)으로 가리키고,
	//첫째는 NULL을 가리킨다.
	CSecretProfileNode *pcNode = new CSecretProfileNode(pszNodeName, this) ;
	pcNode->SetSiblingNode(m_pcChildNode) ;
	m_pcChildNode = pcNode ;
	return pcNode ;
}

void CSecretProfileNode::start()
{
	m_nCallingCounter++ ;
    Profile_GetCurrentTime(&m_n64StartTime) ;
}

void CSecretProfileNode::stop()
{
	__int64 n64CurrentTime ;
    Profile_GetCurrentTime(&n64CurrentTime) ;
	m_fElapsedTime = (float) ( (double)(n64CurrentTime-m_n64StartTime)*dbProfile_timeResolution ) ;
	m_fTotalTime += m_fElapsedTime ;
	m_fMeanTime = m_fTotalTime/(float)m_nCallingCounter ;
}

void CSecretProfileNode::reset()
{
	m_nCallingCounter = 0 ;
	m_n64StartTime = 0 ;
	m_fElapsedTime = 0 ;
}

//###############################################################
//class CSecretProfileManager
//###############################################################
CSecretProfileManager::CSecretProfileManager()
{
	Profile_InitializeTime() ;
	m_pcRootNode = new CSecretProfileNode("Root", NULL) ;
	m_pcCurrentNode = m_pcRootNode ;
	m_lUsage = RENDERTEXT ;
}

CSecretProfileManager::~CSecretProfileManager()
{
	SAFE_DELETE(m_pcRootNode) ;
}

void CSecretProfileManager::StartProfile(char *pszNodeName)
{
	if( strcmp(pszNodeName, m_pcCurrentNode->GetNodeName()) != 0)
		m_pcCurrentNode = m_pcCurrentNode->GetSubNode(pszNodeName) ;

	m_pcCurrentNode->start() ;
	if(m_lUsage == TRACETEXT)
		TRACE(">>START Profile_%s\r\n", m_pcCurrentNode->GetNodeName()) ;
}

void CSecretProfileManager::StopProfile()
{
	m_pcCurrentNode->stop() ;
	if(m_lUsage == TRACETEXT)
		TRACE("<<STOP Profile_%s ElapsedTime=%g MeanTime=%g \r\n",
		m_pcCurrentNode->GetNodeName(), m_pcCurrentNode->GetElapsedTime(), m_pcCurrentNode->GetMeanTime()) ;

	m_pcCurrentNode = m_pcCurrentNode->GetParentNode() ;
}

CSecretProfileNode *CSecretProfileManager::_GetSiblingNode(char *pszNodeName, CSecretProfileNode *pcNode)
{
	while(pcNode)
	{
		if( strcmp(pcNode->GetNodeName(), pszNodeName) == 0)
			return pcNode ;

		pcNode = pcNode->GetSiblingNode() ;
	}
	return NULL ;
}

CSecretProfileNode *CSecretProfileManager::_GetNodes(char *pszNodeName, CSecretProfileNode *pcParentNode)
{
	CSecretProfileNode *pcNode, *pcLastChild ;
	pcNode = pcLastChild = pcParentNode->GetChildNode() ;

	if(pcLastChild == NULL)
		return NULL ;

	//Finding Siblings by Stack oder
	pcNode = _GetSiblingNode(pszNodeName, pcNode) ;
	if(pcNode)
		return pcNode ;

	//Finding Sub-Nodes(ChildNode)
    CSecretProfileNode *pcChildNode = pcLastChild->GetChildNode() ;
	//pcNode = pcLastChild->GetChildNode() ;
	while(pcChildNode)
	{
		//Finding Siblings by Stack oder
		pcNode = _GetSiblingNode(pszNodeName, pcChildNode) ;
		if(pcNode)
			return pcNode ;

		//Recurring Call
        pcNode = _GetNodes(pszNodeName, pcChildNode) ;
		if(pcNode)
			return pcNode ;
		
		pcChildNode = pcChildNode->GetSiblingNode() ;
	}

    return NULL ;
}

bool CSecretProfileManager::GetNodeReports(char *pszNodeName, SProfileNodeReport *psNodeReports, int *pnNumNode)
{
	CSecretProfileNode *pcNode = _GetNodes(pszNodeName, m_pcRootNode) ;
	if(!pcNode)
		return false ;

	static SProfileNodeReport asNodeReports[PROFILE_MAXNUM_NODE] ;
	int nNodeCount=0 ;

	SProfileNodeReport *pnNodeReport ;
	CSecretProfileNode *pcChildNode = pcNode->GetChildNode() ;
	while(pcChildNode)
	{
		pnNodeReport = &asNodeReports[nNodeCount++] ;

		pnNodeReport->bEnable = true ;
		sprintf(pnNodeReport->szNodeName, "%s", pcChildNode->GetNodeName()) ;
		sprintf(pnNodeReport->szParentNodeName, "%s", pcChildNode->GetParentNode()->GetNodeName()) ;
		pnNodeReport->fElapsedTime = pcChildNode->GetElapsedTime() ;
		pnNodeReport->fMeanTime = pcChildNode->GetMeanTime() ;
		//sprintf(pnNodeReport->szReport, "%s %09.06f %09.06f", pcChildNode->GetNodeName(), pcChildNode->GetElapsedTime(), pcChildNode->GetMeanTime()) ;

		pcChildNode = pcChildNode->GetSiblingNode() ;
	}

    memcpy(psNodeReports, asNodeReports, sizeof(SProfileNodeReport)*nNodeCount) ;
	(*pnNumNode) = nNodeCount ;
	return true ;
}

CSecretProfileNode *CSecretProfileManager::GetProfileNode(char *pszNodeName)
{
	return _GetNodes(pszNodeName, m_pcRootNode) ;
}