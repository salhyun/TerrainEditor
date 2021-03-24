#pragma once

#include "def.h"

enum SECRETNODEATTR
{
	SECRETNODEATTR_ANIMATION=0x01,//���ϸ��̼�
	SECRETNODEATTR_USESUBMATERIAL=0x02,//���� �������
	SECRETNODEATTR_USENORMALMAP=0x04,//�븻�� ���
	SECRETNODEATTR_USEAMBIENTDEGREE=0x08,//�ں��Ʈ(����ȯ���� ���� ����)�� ���
	SECRETNODEATTR_SUNLIT=0x10,//SunLight Scattering
	SECRETNODEATTR_ALPHABLENDING=0x20,
	SECRETNODEATTR_TWOSIDED=0x40,
	SECRETNODEATTR_NOTEXTURE=0x80,
	SECRETNODEATTR_REFLECTED=0x10, // ��, �ſ￡ �ݻ�Ǿ����� �ִ� �Ӽ�

    //���⿡�ٰ� �߰�

	SECRETNODEATTR_NOLIGHTING=0x100,
} ;

enum SECRETNODEKIND
{
	SECRETNODEKIND_NONE=0,
	SECRETNODEKIND_BONE,//����
	SECRETNODEKIND_RIGID,//��ü(�������� ����)
	SECRETNODEKIND_RIGIDBUMP,//�븻���� ����� rigid
	SECRETNODEKIND_DUMMY,//3DMax���� HELPEROBJECT�� ����Ŵ
	SECRETNODEKIND_SKINNING,//Vertex Blend����Ÿ�� ���� ���
	SECRETNODEKIND_BILLBOARD,
} ;

//SecretEngine���� �������� ������Ʈ�� ǥ���ϴµ� ���� �⺻������ Mesh�� ����Ѵ�.
class CSecretNode//�޽��� ���� ����Ŭ����
{
protected :
    SECRETNODEKIND m_NodeKind ;//����� ����
	int m_nObjectID ;//�ڽ��� ���̵�
	int m_nParentID ;//�θ��� ���̵�
	int m_nMaterialID ;//����(�ؽ���)�� ���̵�

	char m_szNodeName[256] ;
	char m_szParentNodeName[256] ;

	CSecretNode *m_pcParentNode ;//�θ��带 ����Ų��.

	unsigned long m_lAttr ;//����� �Ӽ�

public :
	bool m_bParent ;//�θ��� ���翩��
	unsigned long m_lEigenIndex ;//use for Double Linked list, ����� ������

public :
	CSecretNode(unsigned long lEigenIndex) ;//ó�� �����Ҷ� ����� �������� �ο��޴´�.
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