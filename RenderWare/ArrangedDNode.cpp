#include "ArrangedDNode.h"
#include <assert.h>

CArrangedDNode::CArrangedDNode()
{
	m_nMaxNumNode = 0 ;
	m_nNumNode = 0 ;
}

CArrangedDNode::CArrangedDNode(int nMaxNumNode)
{
	AllocateArrangedBuffer(nMaxNumNode) ;
}

CArrangedDNode::~CArrangedDNode()
{
	ReleaseAll() ;
}

void CArrangedDNode::AllocateArrangedBuffer(int nMaxNumNode)
{
	m_nMaxNumNode = nMaxNumNode ;
	m_cArrangedBuffer.Initialize(sizeof(DNode)*m_nMaxNumNode, sizeof(DNode)) ;
}

DNODEMSG CArrangedDNode::Initialize(void *pvDataHead, void *pvDataTail, bool (*pfIdentity)(void *, void *))
{
	//m_pHead = new DNode ;
    //m_pTail = new DNode ;

	m_pHead = (DNode *)m_cArrangedBuffer.AllocUnit() ;
	m_pTail = (DNode *)m_cArrangedBuffer.AllocUnit() ;

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

	return DNODEMSG_OK ;
}

DNODEMSG CArrangedDNode::_InsertNode(DNode *pPrevNode, void *pvData)//pPrevNode뒤에 pvData를 삽입
{
	DNode *pNode = (DNode *)m_cArrangedBuffer.AllocUnit() ;
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

	return DNODEMSG_OK ;
}

DNODEMSG CArrangedDNode::InsertNode(void *pvPrevData, void *pvData)//pvPrevData가 들어있는 노드위에 pvData를 삽입
{
	DNode *pPrevNode = FindNode(pvPrevData) ;
	if(pPrevNode == NULL)
		return DNODEMSG_NOTFIND ;

	return _InsertNode(pPrevNode, pvData) ;
}

DNODEMSG CArrangedDNode::InsertNode(void *pvData)//현재진행되는 뒤에 삽입
{
	return _InsertNode(m_pTail->pPrev, pvData) ;
}

DNODEMSG CArrangedDNode::DeleteNode(void *pvData)
{
	DNode *pNode = FindNode(pvData) ;
	if(pNode == NULL)
		return DNODEMSG_NOTFIND ;

	pNode->pPrev->pNext = pNode->pNext ;
	pNode->pNext->pPrev = pNode->pPrev ;

	m_nNumNode-- ;

	//delete pNode ;
	bool bSuccess = m_cArrangedBuffer.DeleteUnit((void *)pNode) ;
	assert(bSuccess) ;

	return DNODEMSG_OK ;
}

DNODEMSG CArrangedDNode::DeleteAll()
{
	m_cArrangedBuffer.DeleteAll() ;
	return DNODEMSG_OK ;
}

void CArrangedDNode::ReleaseAll()
{
	m_cArrangedBuffer.ReleaseAll() ;
}