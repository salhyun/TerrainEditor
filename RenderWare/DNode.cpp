#include "DNode.h"
#include <assert.h>

CDNode::CDNode()
{
	m_pHead = m_pTail = NULL ;
	m_nNumNode = 0 ;
	m_bDeleteAll = false ;
	m_bEnable = false ;
}

CDNode::~CDNode()
{
}

DNODEMSG CDNode::Initialize(void *pvDataHead, void *pvDataTail, bool (*pfIdentity)(void *, void *))
{
	m_pHead = new DNode ;
    m_pTail = new DNode ;

	m_pHead->pPrev = m_pHead ;
	m_pHead->pNext = m_pTail ;
	m_pHead->pvData = pvDataHead ;
	m_pHead->nAttr = DNODEATTR_HEAD ;

	m_pTail->pPrev = m_pHead ;
	m_pTail->pNext = m_pTail ;
	m_pTail->pvData = pvDataTail ;
	m_pTail->nAttr = DNODEATTR_TAIL ;

	m_pfIdentity = pfIdentity ;

	m_nNumNode = 0 ;

	m_bEnable = true ;

	return DNODEMSG_OK ;
}

int CDNode::GetCurNumNode()
{
	return m_nNumNode ;
}

DNode *CDNode::GetHead()
{
	return m_pHead ;
}

DNode *CDNode::GetTail()
{
	return m_pTail ;
}

DNode *CDNode::FindNode(void *pvData)
{
	int nCount=-1 ;//Head 부터 시작하기 때문에 -1부터 카운트 한다.
	DNode *pNode=m_pHead ;

	while(pNode != m_pTail)
	{
		if((*m_pfIdentity)(pNode->pvData, pvData))
		{
			pNode->nCurrentIndex = nCount ;
			return pNode ;
		}

		pNode = pNode->pNext ;
		nCount++ ;
	}

    return NULL ;
}

DNode *CDNode::FindNode(int nNumNode)
{
	DNode *pNode = m_pHead->pNext ;

	for(int i=0 ; i<nNumNode ; i++)
	{
		if(pNode == m_pTail)
			return NULL ;

		pNode = pNode->pNext ;
	}

	return pNode ;
}

DNODEMSG CDNode::_InsertNode(DNode *pPrevNode, void *pvData)//pPrevNode뒤에 pvData를 삽입
{
	DNode *pNode = new DNode ;

	assert(pNode) ;

	pNode->pvData = pvData ;
	pNode->nAttr = DNODEATTR_BODY ;

//	pNextNode->pPrev->pNext = pNode ;
//	pNode->pPrev = pNextNode->pPrev ;
//	pNextNode->pPrev = pNode ;
//	pNode->pNext = pNextNode ;

	pNode->pPrev = pPrevNode ;
	pNode->pNext = pPrevNode->pNext ;
	pPrevNode->pNext = pNode ;
	pNode->pNext->pPrev = pNode ;

	m_nNumNode++ ;
	//assert(m_nNumNode < MAXNUM_NODE) ;

	return DNODEMSG_OK ;
}

DNODEMSG CDNode::InsertNode(void *pvPrevData, void *pvData)//pvPrevData가 들어있는 노드위에 pvData를 삽입
{
	DNode *pPrevNode = FindNode(pvPrevData) ;
	if(pPrevNode == NULL)
		return DNODEMSG_NOTFIND ;

	return _InsertNode(pPrevNode, pvData) ;
}

DNODEMSG CDNode::DeleteNode(void *pvData)
{
	DNode *pNode = FindNode(pvData) ;
	if(pNode == NULL)
		return DNODEMSG_NOTFIND ;

	pNode->pPrev->pNext = pNode->pNext ;
	pNode->pNext->pPrev = pNode->pPrev ;

	m_nNumNode-- ;

	delete pNode ;
	return DNODEMSG_OK ;
}

DNODEMSG CDNode::InsertNode(void *pvData)//현재진행되는 뒤에 삽입
{
	return _InsertNode(m_pTail->pPrev, pvData) ;
}

DNODEMSG CDNode::DeleteAll()
{
	int nCount=0 ;
	DNode *pNode2, *pNode1 = m_pHead ;

	while(true)
	{
		if(pNode1 == m_pTail)
		{
			delete pNode1 ;
			pNode1 = NULL ;
			break ;
		}

        pNode2 = pNode1 ;
		pNode1 = pNode1->pNext ;
		delete pNode2 ;

		//if(nCount++ > MAXNUM_NODE)
		//	return DNODEMSG_ERROR ;
	}
	m_bDeleteAll = true ;
	m_bEnable = false ;

	return DNODEMSG_OK ;
}

