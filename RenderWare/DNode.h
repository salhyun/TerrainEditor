#pragma once

#include <WTypes.h>

enum DNODEMSG
{
	DNODEMSG_ERROR=0,
	DNODEMSG_OK,
	DNODEMSG_NOTFIND,
} ;


#define MAXNUM_NODE 80000000

#define DNODEATTR_HEAD 0
#define DNODEATTR_TAIL 1
#define DNODEATTR_BODY 2

struct DNode
{
	int nAttr ;
	int nCurrentIndex ;//Find�ؼ� �����Ұ�� ���� ���° ������� ���� Ȯ���ϴ� ����(��ȸ��)
	void *pvData ;
	DNode *pPrev, *pNext ;
} ;

//Double Linked List
//stl�� list�� ����� ������ �Ǿ��ִ�.
class CDNode
{
protected :
	int m_bEnable ;
	DNode *m_pHead, *m_pTail ;
	int m_nNumNode ;

	bool (*m_pfIdentity)(void *, void *) ;
	DNODEMSG _InsertNode(DNode *pPrevNode, void *pvData) ;//pPrevNode�ڿ� pvData�� ����

public :
	int m_bDeleteAll ;

public :
	CDNode() ;
	~CDNode() ;

	DNODEMSG Initialize(void *pvDataHead, void *pvDataTail, bool (*pfIdentity)(void *, void *)) ;

	DNODEMSG InsertNode(void *pvPrevData, void *pvData) ;//pvPrevData�� ����ִ� ������� pvData�� ����
	DNODEMSG InsertNode(void *pvData) ;//��������Ǵ� �ڿ� ����

	DNODEMSG DeleteNode(void *pvData) ;

	DNode *FindNode(void *pvData) ;
	DNode *FindNode(int nNumNode) ;
	DNode *GetHead() ;
	DNode *GetTail() ;

	int GetCurNumNode() ;
	int IsEnable() {return m_bEnable;}

	DNODEMSG DeleteAll() ;
} ;