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

	DNODEMSG InsertNode(void *pvPrevData, void *pvData) ;//pvPrevData가 들어있는 노드위에 pvData를 삽입
	DNODEMSG InsertNode(void *pvData) ;//현재진행되는 뒤에 삽입

	DNODEMSG DeleteNode(void *pvData) ;

	//DNode *FindNode(void *pvData) ;
	//DNode *FindNode(int nNumNode) ;
	//DNode *GetHead() ;
	//DNode *GetTail() ;

	//int GetCurNumNode() ;

	DNODEMSG DeleteAll() ;//모든 버퍼를 초기상태로 되돌린다.
	void ReleaseAll() ;//모든버퍼를 메모리에서 삭제한다.

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

	DNODEMSG InsertNode(void *pvPrevData, void *pvData) ;//pvPrevData가 들어있는 노드위에 pvData를 삽입
	DNODEMSG InsertNode(void *pvData) ;//현재진행되는 뒤에 삽입

	DNODEMSG DeleteNode(void *pvData) ;

	DNode *FindNode(void *pvData) ;
	DNode *FindNode(int nNumNode) ;
	DNode *GetHead() ;
	DNode *GetTail() ;

	int GetCurNumNode() ;

	DNODEMSG DeleteAll() ;//모든 버퍼를 초기상태로 되돌린다.
	void ReleaseAll() ;//모든버퍼를 메모리에서 삭제한다.
} ;
*/