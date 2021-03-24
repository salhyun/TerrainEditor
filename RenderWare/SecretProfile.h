#pragma once

#include <stdio.h>

//###############################################################
//class CSecretProfileNode
//###############################################################
class CSecretProfileNode
{
private :
	unsigned long m_lIndex ;
	unsigned long m_lChildCounter ;
	unsigned long m_lTreeLevel, m_lOderOfSibling ;//m_lOderOfSibling 몇번째 자식이냐? 부모로부터
	char m_szNodeName[256] ;
	__int64 m_n64StartTime ;
	int m_nCallingCounter ;
	float m_fElapsedTime, m_fTotalTime, m_fMeanTime ;

	CSecretProfileNode *m_pcChildNode ;
	CSecretProfileNode *m_pcParentNode ;
	CSecretProfileNode *m_pcSiblingNode ;

	void _InitVariables() ;

public :
	CSecretProfileNode() ;
	CSecretProfileNode(char *pszNodeName, CSecretProfileNode *pcParentNode) ;
	~CSecretProfileNode() ;

	unsigned long GetIndex() {return m_lIndex;};
	char *GetNodeName() {return m_szNodeName;};
	float GetElapsedTime() {return m_fElapsedTime;};
	float GetMeanTime() {return m_fMeanTime;};
	CSecretProfileNode *GetChildNode() {return m_pcChildNode;};
	CSecretProfileNode *GetParentNode() {return m_pcParentNode;};
	CSecretProfileNode *GetSiblingNode() {return m_pcSiblingNode;};
	void SetChildNode(CSecretProfileNode *pcChildNode) {m_pcChildNode=pcChildNode;};
	void SetParentNode(CSecretProfileNode *pcParentNode) {m_pcParentNode=pcParentNode;};
	void SetSiblingNode(CSecretProfileNode *pcSiblingNode) {m_pcSiblingNode=pcSiblingNode;};

    CSecretProfileNode *GetSubNode(char *pszNodeName) ;
	void start() ;
	void stop() ;
	void reset() ;
} ;

#define PROFILE_MAXNUM_NODE 256

struct SProfileNodeReport
{
	bool bEnable ;
	char szNodeName[256], szParentNodeName[256] ;
    int nLevel ;
	float fElapsedTime, fMeanTime ;
	//char szReport[256] ;
} ;

//###############################################################
//class CSecretProfileManager
//###############################################################
class CSecretProfileManager
{
public :
	enum { TRACETEXT, RENDERTEXT } ;

private :
	unsigned long m_lUsage ;
    CSecretProfileNode *m_pcRootNode ;
	CSecretProfileNode *m_pcCurrentNode ;

    void _AddProfileNode(char *pszNodeName) ;
	CSecretProfileNode *_GetSiblingNode(char *pszNodeName, CSecretProfileNode *pcNode) ;
	CSecretProfileNode *_GetNodes(char *pszNodeName, CSecretProfileNode *pcParentNode) ;

	int m_nReportNode ;
    SProfileNodeReport m_asReport[PROFILE_MAXNUM_NODE] ;

public :
	CSecretProfileManager() ;
	~CSecretProfileManager() ;

	void SetUsage(unsigned long lUsage) {m_lUsage=lUsage;};
	unsigned long GetUsage() {return m_lUsage;};

	void StartProfile(char *pszNodeName) ;
	void StopProfile() ;

	bool GetNodeReports(char *pszNodeName, SProfileNodeReport *psNodeReports, int *pnNumNode) ;
	CSecretProfileNode *GetProfileNode(char *pszNodeName) ;


} ;

#if defined(DEBUG) | defined(_DEBUG)
#define PROFILE_START(ProfileManager, NodeName) ProfileManager->StartProfile(#NodeName)
#else
#define PROFILE_START(ProfileManager, NodeName)
#endif

#if defined(DEBUG) | defined(_DEBUG)
#define PROFILE_STOP(ProfileManager) ProfileManager->StopProfile()
#else
#define PROFILE_STOP(ProfileManager)
#endif
