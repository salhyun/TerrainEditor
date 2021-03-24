#pragma once

#include "ArrangedBuffer.h"
#include "DNode.h"

//CArrangedDNode inherit member function from CDNode
//member function overriding from CDNode
class CArrangedDNode : public CDNode
{
private :
	int m_nMaxNumNode ;
	CArrangedBuffer m_cArrangedBuffer ;

	DNODEMSG _InsertNode(DNode *pPrevNode, void *pvData) ;

public :
	CArrangedDNode() ;
	CArrangedDNode(int nMaxNumNode) ;
	~CArrangedDNode() ;

	void AllocateArrangedBuffer(int nMaxNumNode) ;
	DNODEMSG Initialize(void *pvDataHead, void *pvDataTail, bool (*pfIdentity)(void *, void *)) ;

	DNODEMSG InsertNode(void *pvPrevData, void *pvData) ;//pvPrevData�� ����ִ� ������� pvData�� ����
	DNODEMSG InsertNode(void *pvData) ;//��������Ǵ� �ڿ� ����

	DNODEMSG DeleteNode(void *pvData) ;

	//DNode *FindNode(void *pvData) ;
	//DNode *FindNode(int nNumNode) ;
	//DNode *GetHead() ;
	//DNode *GetTail() ;

	//int GetCurNumNode() ;

	DNODEMSG DeleteAll() ;//��� ���۸� �ʱ���·� �ǵ�����.
	void ReleaseAll() ;//�����۸� �޸𸮿��� �����Ѵ�.

} ;
/*
class CDNode
{
public :
	CDNode() ;
	CDNode(int nMaxNumNode) ;
	~CDNode() ;

	void AllocateArrangedBuffer(int nMaxNumNode) ;
	DNODEMSG Initialize(void *pvDataHead, void *pvDataTail, bool (*pfIdentity)(void *, void *)) ;

	DNODEMSG InsertNode(void *pvPrevData, void *pvData) ;//pvPrevData�� ����ִ� ������� pvData�� ����
	DNODEMSG InsertNode(void *pvData) ;//��������Ǵ� �ڿ� ����

	DNODEMSG DeleteNode(void *pvData) ;

	DNode *FindNode(void *pvData) ;
	DNode *FindNode(int nNumNode) ;
	DNode *GetHead() ;
	DNode *GetTail() ;

	int GetCurNumNode() ;

	DNODEMSG DeleteAll() ;//��� ���۸� �ʱ���·� �ǵ�����.
	void ReleaseAll() ;//�����۸� �޸𸮿��� �����Ѵ�.
} ;
*/