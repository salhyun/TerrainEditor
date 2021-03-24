#pragma once

#include "D3Ddef.h"

#include "SecretNode.h"
#include "AniTrack.h"
#include "ASEData.h"
#include "Vector3.h"
#include "GeoLib.h"

class CSecretShadowMesh ;
class CollisionModel3D ;

struct SNormalVector//메쉬의 노말벡터를 저장하기 위한 CSecretMesh만을 위한 구조체
{
	Vector3 vStart, vNormal ;//위치를 저장하는 vStart, 방향을 저장하는 vNormal

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

	UINT m_lNumVertex ;//정점의 갯수
	UINT m_lNumTriangle ;//삼각형(폴리곤)의 갯수

	LPDIRECT3DVERTEXBUFFER9 m_pVB ;//렌더링에 사용되는 정점버퍼
	LPDIRECT3DINDEXBUFFER9 m_pVI ;//렌더링에 사용되는 인덱스버퍼

	bool m_bAnimaion ;//에니메이션 여부
	CAniTrack m_cAniTrack ;//에니메이션 데이타

	UINT m_lNumNormalVector ;//노말벡터 갯수
	SNormalVector *m_psNormalVector ;
	SNormalVertex *m_psNormalVertex ;

	SVertexDisk *m_psVertexDisk ;//엠비언트를 위한 정점의 위치에서 생성되는 디스크
	CSecretShadowMesh *m_pcShadowMesh ;

	geo::SPolyhedron *m_psBoundingPolyhedron ;
	CollisionModel3D *m_pCollisionModel ;

public :
	D3DXMATRIX m_matLocal, m_matChild, m_matTM, m_matSkin ;//메쉬 데이타의 로칼, 차일드, 트렌스폼, 스킨을 위한 행렬
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

	Vector3 GetPos() ;//Animation() 이후에 사용할수 있다. 그렇지않으면 올바른 값을 얻을수 없다.

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