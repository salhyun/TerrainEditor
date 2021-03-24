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
	int nCurrentIndex ;//Find해서 리턴할경우 현재 몇번째 노드인지 값을 확인하는 변수(일회성)
	void *pvData ;
	DNode *pPrev, *pNext ;
} ;

//Double Linked List
//stl의 list와 비슷한 구조로 되어있다.
class CDNode
{
protected :
	int m_bEnable ;
	DNode *m_pHead, *m_pTail ;
	int m_nNumNode ;

	bool (*m_pfIdentity)(void *, void *) ;
	DNODEMSG _InsertNode(DNode *pPrevNode, void *pvData) ;//pPrevNode뒤에 pvData를 삽입

public :
	int m_bDeleteAll ;

public :
	CDNode() ;
	~CDNode() ;

	DNODEMSG Initialize(void *pvDataHead, void *pvDataTail, bool (*pfIdentity)(void *, void *)) ;

	DNODEMSG InsertNode(void *pvPrevData, void *pvData) ;//pvPrevData가 들어있는 노드위에 pvData를 삽입
	DNODEMSG InsertNode(void *pvData) ;//현재진행되는 뒤에 삽입

	DNODEMSG DeleteNode(void *pvData) ;

	DNode *FindNode(void *pvData) ;
	DNode *FindNode(int nNumNode) ;
	DNode *GetHead() ;
	DNode *GetTail() ;

	int GetCurNumNode() ;
	int IsEnable() {return m_bEnable;}

	DNODEMSG DeleteAll() ;
} ;