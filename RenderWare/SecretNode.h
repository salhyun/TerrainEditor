#pragma once

#include "def.h"

enum SECRETNODEATTR
{
	SECRETNODEATTR_ANIMATION=0x01,//에니메이션
	SECRETNODEATTR_USESUBMATERIAL=0x02,//하위 재질사용
	SECRETNODEATTR_USENORMALMAP=0x04,//노말맵 사용
	SECRETNODEATTR_USEAMBIENTDEGREE=0x08,//앰비언트(주위환경의 대한 음영)맵 사용
	SECRETNODEATTR_SUNLIT=0x10,//SunLight Scattering
	SECRETNODEATTR_ALPHABLENDING=0x20,
	SECRETNODEATTR_TWOSIDED=0x40,
	SECRETNODEATTR_NOTEXTURE=0x80,
	SECRETNODEATTR_REFLECTED=0x10, // 물, 거울에 반사되어질수 있는 속성

    //여기에다가 추가

	SECRETNODEATTR_NOLIGHTING=0x100,
} ;

enum SECRETNODEKIND
{
	SECRETNODEKIND_NONE=0,
	SECRETNODEKIND_BONE,//뼈대
	SECRETNODEKIND_RIGID,//강체(움직이지 않음)
	SECRETNODEKIND_RIGIDBUMP,//노말맵을 사용한 rigid
	SECRETNODEKIND_DUMMY,//3DMax에서 HELPEROBJECT를 가리킴
	SECRETNODEKIND_SKINNING,//Vertex Blend데이타를 가진 노드
	SECRETNODEKIND_BILLBOARD,
} ;

//SecretEngine에서 공간상의 오브젝트를 표현하는데 가장 기본단위로 Mesh를 사용한다.
class CSecretNode//메쉬의 가장 상위클래스
{
protected :
    SECRETNODEKIND m_NodeKind ;//노드의 종류
	int m_nObjectID ;//자신의 아이디
	int m_nParentID ;//부모노드 아이디
	int m_nMaterialID ;//재질(텍스쳐)의 아이디

	char m_szNodeName[256] ;
	char m_szParentNodeName[256] ;

	CSecretNode *m_pcParentNode ;//부모노드를 가리킨다.

	unsigned long m_lAttr ;//노드의 속성

public :
	bool m_bParent ;//부모노드 존재여부
	unsigned long m_lEigenIndex ;//use for Double Linked list, 노드의 고유값

public :
	CSecretNode(unsigned long lEigenIndex) ;//처음 생성할때 노드의 고유값을 부여받는다.
	virtual ~CSecretNode() ;

	SECRETNODEKIND GetNodeKind() {return m_NodeKind;}
	int GetObjectID() {return m_nObjectID;}
	int GetParentID() {return m_nParentID;}
	int GetMaterialID() {return m_nMaterialID;}
	unsigned long GetAttr() {return m_lAttr;}
	void SetAttr(unsigned long lAttr) {m_lAttr = lAttr;}
	void AddAttr(unsigned long lAttr) {m_lAttr |= lAttr;}
	void RemoveAttr(unsigned long lAttr) {m_lAttr &= (~lAttr);}
	int GetKind() {return m_NodeKind;}
	char *GetNodeName() {return m_szNodeName;}
	char *GetParentNodeName() {return m_szParentNodeName;}
	void SetParentNode(CSecretNode *pcNode) {m_pcParentNode = pcNode;}
	CSecretNode *GetParentNode() {return m_pcParentNode;} ;
	void SetMaterialID(int nID) {m_nMaterialID = nID;} ;

	virtual void Render() {} ;

	virtual void PreRenderSubSet() {} ;
	virtual void RenderSubSet(int nCount) {} ;
	virtual void PostRenderSubSet() {} ;

} ;