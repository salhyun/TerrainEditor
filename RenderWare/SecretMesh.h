#pragma once

#include "D3Ddef.h"

#include "SecretNode.h"
#include "AniTrack.h"
#include "ASEData.h"
#include "Vector3.h"
#include "GeoLib.h"

class CSecretShadowMesh ;
class CollisionModel3D ;

struct SNormalVector//�޽��� �븻���͸� �����ϱ� ���� CSecretMesh���� ���� ����ü
{
	Vector3 vStart, vNormal ;//��ġ�� �����ϴ� vStart, ������ �����ϴ� vNormal

	SNormalVector() {} ;
	SNormalVector(Vector3 _vStart, Vector3 _vNormal)
	{
		set(_vStart, _vNormal) ;
	}
	void set(Vector3 &_vStart, Vector3 &_vNormal)
	{
		vStart = _vStart ;
		vNormal = _vNormal ;
	}
} ;

struct SNormalVertex
{
	Vector3 vStart, vEnd ;
} ;

class CSecretMesh : public CSecretNode
{
protected :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;

	UINT m_lNumVertex ;//������ ����
	UINT m_lNumTriangle ;//�ﰢ��(������)�� ����

	LPDIRECT3DVERTEXBUFFER9 m_pVB ;//�������� ���Ǵ� ��������
	LPDIRECT3DINDEXBUFFER9 m_pVI ;//�������� ���Ǵ� �ε�������

	bool m_bAnimaion ;//���ϸ��̼� ����
	CAniTrack m_cAniTrack ;//���ϸ��̼� ����Ÿ

	UINT m_lNumNormalVector ;//�븻���� ����
	SNormalVector *m_psNormalVector ;
	SNormalVertex *m_psNormalVertex ;

	SVertexDisk *m_psVertexDisk ;//�����Ʈ�� ���� ������ ��ġ���� �����Ǵ� ��ũ
	CSecretShadowMesh *m_pcShadowMesh ;

	geo::SPolyhedron *m_psBoundingPolyhedron ;
	CollisionModel3D *m_pCollisionModel ;

public :
	D3DXMATRIX m_matLocal, m_matChild, m_matTM, m_matSkin ;//�޽� ����Ÿ�� ��Į, ���ϵ�, Ʈ������, ��Ų�� ���� ���
	D3DXVECTOR3 m_vRotAxis ;
	D3DXVECTOR4 m_vOriginDiffuse, m_vDiffuse ;
	float m_fRotAngle ;
	float m_fRadius ;
	D3DVERTEXELEMENT9 *m_pVertexElement ;//FVF

	geo::SAACube m_sAACube ;
	bool m_bWireFrame ;
	float m_fAlphaBlendingByForce ;

public :
	CSecretMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretMesh() ;

	HRESULT Initialize(SMesh *pMesh) ;

	void SetNormalVectors(SMesh *pMesh) ;

	virtual HRESULT RefreashVertexBuffer() {return S_OK;} ;

	virtual void Render() {}
	virtual void RenderShadow() {}
	virtual void RenderNormalVector(Matrix4 &matWorld) {}
	virtual void RenderVertexDisk(Matrix4 &matWorld) {}
	virtual void RenderBoundingVolume(Matrix4 &matWorld) {}

	unsigned long GetNumVertex() { return m_lNumVertex ;}
	unsigned long GetNumTriangle() { return m_lNumTriangle ;}
	D3DXMATRIX *GetmatLocal() {return &m_matLocal;}
	D3DXMATRIX *GetmatChild() {return &m_matChild;}
	D3DXMATRIX *GetmatTM() {return &m_matTM;}
	D3DXMATRIX *GetmatSkin() {return &m_matSkin;}

	LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer() {return m_pVB;}
	LPDIRECT3DINDEXBUFFER9 GetIndexBuffer() {return m_pVI;}

	Vector3 GetPos() ;//Animation() ���Ŀ� ����Ҽ� �ִ�. �׷��������� �ùٸ� ���� ������ ����.

	void SetWireFrame(bool bEnable) ;

	void Animation(float fFrame, D3DXMATRIX *pmatParentTM) ;

	//VertexDisk
	void SetVertexDisk(SMesh *pMesh) ;
	SVertexDisk *GetVertexDisks() {return m_psVertexDisk;} ;

	void SetVertexElement(D3DVERTEXELEMENT9 *pVertexElement) ;

	virtual HRESULT InitShadowData() {return S_OK;}

	geo::SPolyhedron *GetBoundingPolyhedron() ;
	CollisionModel3D *GetCollisionModel() ;

	void Release() ;

} ;